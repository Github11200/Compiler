#include "ast/node.h"
#include "token.h"
#include <iostream>
#include <memory>

using namespace std;

string Root::generateCode() { return "Generating..."; }

string Identifier::generateCode() { return this->name; }
string IntegerLiteral::generateCode() { return to_string(this->value); }

BinaryExpression::BinaryExpression(const TokenType operatorType, variant<BinaryExpression, IntegerLiteral, Identifier> left, variant<BinaryExpression, IntegerLiteral, Identifier> right) {
  this->operatorType = operatorType;
  visit(
      [&]<typename T>(const T &var) {
        if constexpr (is_same_v<decay_t<T>, BinaryExpression>)
          this->left = make_shared<BinaryExpression>(var);
        if constexpr (is_same_v<decay_t<T>, IntegerLiteral>)
          this->left = make_shared<IntegerLiteral>(var);
        if constexpr (is_same_v<decay_t<T>, Identifier>)
          this->left = make_shared<Identifier>(var);
      },
      left);

  visit(
      [&]<typename T>(const T &var) {
        if constexpr (is_same_v<decay_t<T>, BinaryExpression>)
          this->right = make_shared<BinaryExpression>(var);
        if constexpr (is_same_v<decay_t<T>, IntegerLiteral>)
          this->right = make_shared<IntegerLiteral>(var);
        if constexpr (is_same_v<decay_t<T>, Identifier>)
          this->right = make_shared<Identifier>(var);
      },
      right);
}

string BinaryExpression::generateCode() {
  string outputCode;
  if (holds_alternative<shared_ptr<IntegerLiteral>>(left))
    outputCode += get<shared_ptr<IntegerLiteral>>(left)->generateCode();
  else if (holds_alternative<shared_ptr<BinaryExpression>>(left))
    outputCode += get<shared_ptr<BinaryExpression>>(left)->generateCode();
  else if (holds_alternative<shared_ptr<Identifier>>(left))
    outputCode += get<shared_ptr<Identifier>>(left)->generateCode();

  switch (operatorType) {
  case TokenType::PLUS:
    outputCode += " + ";
    break;
  case TokenType::MINUS:
    outputCode += " - ";
    break;
  case TokenType::TIMES:
    outputCode += " * ";
    break;
  case TokenType::DIVIDE:
    outputCode += " / ";
    break;
  case TokenType::GREATER_THAN:
    outputCode += " > ";
    break;
  case TokenType::LESS_THAN:
    outputCode += " < ";
    break;
  case TokenType::GREATER_THAN_OR_EQUALS_TO:
    outputCode += " >= ";
    break;
  case TokenType::LESS_THAN_OR_EQUALS_TO:
    outputCode += " <= ";
    break;
  default:
    break;
  }

  if (holds_alternative<shared_ptr<IntegerLiteral>>(right))
    outputCode += get<shared_ptr<IntegerLiteral>>(right)->generateCode();
  else if (holds_alternative<shared_ptr<BinaryExpression>>(right))
    outputCode += get<shared_ptr<BinaryExpression>>(right)->generateCode();
  else if (holds_alternative<shared_ptr<Identifier>>(right))
    outputCode += get<shared_ptr<Identifier>>(right)->generateCode();

  return outputCode;
}

VariableStatement::VariableStatement(const Identifier &identifier, variant<BinaryExpression, IntegerLiteral, Identifier> value) {
  this->isPointer = false;
  this->identifier = make_shared<Identifier>(identifier);
  visit(
      [&]<typename T>(const T &var) {
        if constexpr (is_same_v<decay_t<T>, BinaryExpression>)
          this->value = make_shared<BinaryExpression>(var);
        if constexpr (is_same_v<decay_t<T>, IntegerLiteral>)
          this->value = make_shared<IntegerLiteral>(var);
        if constexpr (is_same_v<decay_t<T>, Identifier>)
          this->value = make_shared<Identifier>(var);
      },
      value);
}

VariableStatement::VariableStatement(const Identifier &pointerIdentifier) {
  this->isPointer = true;
  this->pointerIdentifier = make_shared<Identifier>(pointerIdentifier);
}

string VariableStatement::generateCode() {
  string outputCode = "auto ";
  outputCode += " " + this->identifier->generateCode() + " = ";
  if (holds_alternative<shared_ptr<IntegerLiteral>>(value))
    outputCode += get<shared_ptr<IntegerLiteral>>(value)->generateCode();
  else if (holds_alternative<shared_ptr<BinaryExpression>>(value))
    outputCode += get<shared_ptr<BinaryExpression>>(value)->generateCode();
  else if (holds_alternative<shared_ptr<Identifier>>(value))
    outputCode += get<shared_ptr<Identifier>>(value)->generateCode();

  outputCode += ";";

  return outputCode;
}

FunctionStatement::FunctionStatement(const Identifier &identifier, const vector<shared_ptr<ASTNode>> &body, const vector<string> &parameters) {
  this->identifier = make_shared<Identifier>(identifier);
  if (!parameters.empty())
    this->parameters = parameters;
  this->body = body;
}

string FunctionStatement::generateCode() {
  string outputCode = "auto ";
  outputCode += " " + this->identifier->generateCode() + " (";
  for (int i = 0; i < parameters.size(); ++i) {
    outputCode += "auto " + parameters[i];
    if (i < parameters.size() - 1)
      outputCode += ",";
  }
  outputCode += ") {";
  for (const auto &node : body)
    outputCode += node.get()->generateCode();
  outputCode += "}";
  return outputCode;
}

IfStatementBlock::IfStatementBlock(const optional<variant<BinaryExpression, IntegerLiteral, Identifier>> condition, const vector<shared_ptr<ASTNode>> &body) {
  if (condition.has_value()) {
    visit(
        [&]<typename T>(const T &var) {
          if constexpr (is_same_v<decay_t<T>, BinaryExpression>)
            this->condition = make_shared<BinaryExpression>(var);
          if constexpr (is_same_v<decay_t<T>, IntegerLiteral>)
            this->condition = make_shared<IntegerLiteral>(var);
          if constexpr (is_same_v<decay_t<T>, Identifier>)
            this->condition = make_shared<Identifier>(var);
        },
        condition.value());
  } else
    this->condition = nullopt;
  this->body = body;
}

IfStatement::IfStatement(const std::vector<IfStatementBlock> &ifStatementBlocks) { this->ifStatementBlocks = ifStatementBlocks; }

string IfStatement::generateCode() {
  string outputCode = "";

  for (int i = 0; i < ifStatementBlocks.size(); ++i) {
    IfStatementBlock ifStatementBlock = ifStatementBlocks[i];

    if (ifStatementBlocks.size() > 1 && i == ifStatementBlocks.size() - 1)
      outputCode += "else {";
    else if (i == 0)
      outputCode += "if (";
    else
      outputCode += "else if (";

    if (ifStatementBlock.condition.has_value()) {
      if (holds_alternative<shared_ptr<BinaryExpression>>(ifStatementBlock.condition.value()))
        outputCode += get<shared_ptr<BinaryExpression>>(ifStatementBlock.condition.value())->generateCode() + ") {";
      if (holds_alternative<shared_ptr<IntegerLiteral>>(ifStatementBlock.condition.value()))
        outputCode += get<shared_ptr<IntegerLiteral>>(ifStatementBlock.condition.value())->generateCode() + ") {";
    }

    for (const auto &bodyStatement : ifStatementBlock.body)
      outputCode += bodyStatement->generateCode();
    outputCode += "}";
  }
  return outputCode;
}

LoopStatement::LoopStatement(const BinaryExpression condition, const Identifier identifier, const std::vector<std::shared_ptr<ASTNode>> &body) {
  this->condition = make_shared<BinaryExpression>(condition);
  this->body = body;
  this->identifier = make_shared<Identifier>(identifier);
}

string LoopStatement::generateCode() {
  string outputCode = "for (int ";
  outputCode += this->identifier->generateCode();
  outputCode += " = 0;";
  outputCode += this->condition->generateCode();
  outputCode += ";++" + this->identifier->generateCode();
  outputCode += ") {";
  for (auto item : body)
    outputCode += item->generateCode();
  outputCode += "}";
  return outputCode;
}
