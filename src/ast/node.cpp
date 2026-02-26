#include "ast/node.h"
#include "token.h"
#include <memory>

using namespace std;

string Root::generateCode() { return "Generating..."; }

string IntegerLiteral::generateCode() { return to_string(this->value); }

BinaryExpression::BinaryExpression(const TokenType operatorType, variant<BinaryExpression, IntegerLiteral> left, variant<BinaryExpression, IntegerLiteral> right) {
  this->operatorType = operatorType;
  visit(
      [&]<typename T>(const T &var) {
        if constexpr (is_same_v<decay_t<T>, BinaryExpression>)
          this->left = make_shared<BinaryExpression>(var);
        if constexpr (is_same_v<decay_t<T>, IntegerLiteral>)
          this->left = make_shared<IntegerLiteral>(var);
      },
      left);

  visit(
      [&]<typename T>(const T &var) {
        if constexpr (is_same_v<decay_t<T>, BinaryExpression>)
          this->right = make_shared<BinaryExpression>(var);
        if constexpr (is_same_v<decay_t<T>, IntegerLiteral>)
          this->right = make_shared<IntegerLiteral>(var);
      },
      right);
}

string BinaryExpression::generateCode() {
  string outputCode;
  if (holds_alternative<shared_ptr<IntegerLiteral>>(left))
    outputCode += get<shared_ptr<IntegerLiteral>>(left)->generateCode();
  else if (holds_alternative<shared_ptr<BinaryExpression>>(left))
    outputCode += get<shared_ptr<BinaryExpression>>(left)->generateCode();

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

  return outputCode;
}

VariableStatement::VariableStatement(const string &identifier, variant<BinaryExpression, IntegerLiteral> value) {
  this->isPointer = false;
  this->identifier = identifier;
  visit(
      [&]<typename T>(const T &var) {
        if constexpr (is_same_v<decay_t<T>, BinaryExpression>)
          this->value = make_shared<BinaryExpression>(var);
        if constexpr (is_same_v<decay_t<T>, IntegerLiteral>)
          this->value = make_shared<IntegerLiteral>(var);
      },
      value);
}

VariableStatement::VariableStatement(const string &pointerIdentifier) {
  this->isPointer = true;
  this->pointerIdentifier = pointerIdentifier;
}

string VariableStatement::generateCode() {
  string outputCode = "auto ";
  outputCode += this->identifier + " = ";
  if (holds_alternative<shared_ptr<IntegerLiteral>>(value))
    outputCode += get<shared_ptr<IntegerLiteral>>(value)->generateCode();
  else if (holds_alternative<shared_ptr<BinaryExpression>>(value))
    outputCode += get<shared_ptr<BinaryExpression>>(value)->generateCode();

  outputCode += ";";

  return outputCode;
}

FunctionStatement::FunctionStatement(const string &identifier, const vector<shared_ptr<ASTNode>> &body, const vector<string> &parameters) {
  this->identifier = identifier;
  if (!parameters.empty())
    this->parameters = parameters;
  this->body = body;
}

string FunctionStatement::generateCode() {
  string outputCode = "auto ";
  outputCode += this->identifier + "(";
  for (const auto parameter : parameters)
    outputCode += "auto " + parameter + ",";
  outputCode += ") {";
  for (const auto &node : body)
    outputCode += node.get()->generateCode();
  outputCode += "}";
  return outputCode;
}

IfStatementBlock::IfStatementBlock(const optional<variant<BinaryExpression, IntegerLiteral>> condition, const vector<shared_ptr<ASTNode>> &body) {
  if (condition.has_value()) {
    visit(
        [&]<typename T>(const T &var) {
          if constexpr (is_same_v<decay_t<T>, BinaryExpression>)
            this->condition = make_shared<BinaryExpression>(var);
          if constexpr (is_same_v<decay_t<T>, IntegerLiteral>)
            this->condition = make_shared<IntegerLiteral>(var);
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

LoopStatement::LoopStatement(const BinaryExpression condition, const std::vector<std::shared_ptr<ASTNode>> &body) {
  this->condition = make_shared<BinaryExpression>(condition);
  this->body = body;
}

string LoopStatement::generateCode() {
  // TODO: Implement code generation
  string outputCode = "for";
  this->condition->generateCode();
  return outputCode;
}
