#include "ast/node.h"
#include "token.h"
#include <iostream>
#include <memory>
#include <variant>

using namespace std;

void visitor(variant<SHARED_POINTER_TYPES> &value, variant<TYPES> &inputVariant) {
  visit(
      [&]<typename T>(const T &var) {
        if constexpr (is_same_v<decay_t<T>, BinaryExpression>)
          value = make_shared<BinaryExpression>(var);
        if constexpr (is_same_v<decay_t<T>, IntegerLiteral>)
          value = make_shared<IntegerLiteral>(var);
        if constexpr (is_same_v<decay_t<T>, StringLiteral>)
          value = make_shared<StringLiteral>(var);
        if constexpr (is_same_v<decay_t<T>, Identifier>)
          value = make_shared<Identifier>(var);
        if constexpr (is_same_v<decay_t<T>, FunctionCallStatement>)
          value = make_shared<FunctionCallStatement>(var);
      },
      inputVariant);
}

string generatedCode(variant<SHARED_POINTER_TYPES> &value) {
  if (holds_alternative<shared_ptr<IntegerLiteral>>(value))
    return get<shared_ptr<IntegerLiteral>>(value)->generateCode();
  if (holds_alternative<shared_ptr<StringLiteral>>(value))
    return get<shared_ptr<StringLiteral>>(value)->generateCode();
  else if (holds_alternative<shared_ptr<BinaryExpression>>(value))
    return get<shared_ptr<BinaryExpression>>(value)->generateCode();
  else if (holds_alternative<shared_ptr<Identifier>>(value))
    return get<shared_ptr<Identifier>>(value)->generateCode();
  else if (holds_alternative<shared_ptr<FunctionCallStatement>>(value))
    return get<shared_ptr<FunctionCallStatement>>(value)->generateCode();
  return "";
}

string Root::generateCode() { return "Generating..."; }

string Identifier::generateCode() { return this->name; }
string IntegerLiteral::generateCode() { return to_string(this->value); }
string StringLiteral::generateCode() { return "\"" + this->value + "\""; }
string ReturnStatement::generateCode() { return "return " + generatedCode(this->value) + ";"; }
string PrintStatement::generateCode() { return "cout << " + generatedCode(this->value) + " << endl;"; }
string Parameter::generateCode() { return type.generateCode() + " " + name.generateCode(); }

string Type::generateCode() {
  string pointerString = this->isPointer ? "*" : "";
  switch (this->type) {
  case TokenType::STRING:
    return "string" + pointerString;
  case TokenType::INTEGER:
    return "int" + pointerString;
  case TokenType::FLOAT:
    return "double" + pointerString;
  case TokenType::VOID:
    return "void" + pointerString;
  }

  return "";
}

FunctionCallStatement::FunctionCallStatement(const string &name, bool semicolon, vector<variant<TYPES>> parameters) {
  this->name = name;
  this->semicolon = semicolon;
  this->parameters.resize(parameters.size());

  for (int i = 0; i < parameters.size(); ++i)
    visitor(this->parameters[i], parameters[i]);
}

string FunctionCallStatement::generateCode() {
  string parametersString = "(";
  for (int i = 0; i < parameters.size(); ++i) {
    parametersString += generatedCode(parameters[i]);
    if (i < parameters.size() - 1)
      parametersString += ",";
  }
  parametersString += ")";
  return name + parametersString + (semicolon ? ";" : "");
}

BinaryExpression::BinaryExpression(const TokenType operatorType, variant<TYPES> left, variant<TYPES> right) {
  this->operatorType = operatorType;
  visitor(this->left, left);
  visitor(this->right, right);
}

string BinaryExpression::generateCode() {
  string outputCode;

  outputCode += generatedCode(left);

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

  outputCode += generatedCode(right);

  return outputCode;
}

VariableStatement::VariableStatement(const Identifier &identifier, Type variableType, variant<TYPES> value) {
  this->isPointer = false;
  this->variableType = make_shared<Type>(variableType);
  this->identifier = make_shared<Identifier>(identifier);
  visitor(this->value, value);
}

string VariableStatement::generateCode() {
  string outputCode = this->variableType->generateCode() + " ";
  outputCode += " " + this->identifier->generateCode() + " = ";
  outputCode += generatedCode(value);

  outputCode += ";";

  return outputCode;
}

FunctionStatement::FunctionStatement(const Identifier &identifier, const vector<shared_ptr<ASTNode>> &body, const vector<Parameter> &parameters, const Type &returnType) {
  this->identifier = make_shared<Identifier>(identifier);
  if (!parameters.empty())
    this->parameters = parameters;
  this->body = body;
  this->returnType = make_shared<Type>(returnType);
}

string FunctionStatement::generateCode() {
  string outputCode = returnType->generateCode() + " ";
  outputCode += " " + this->identifier->generateCode() + " (";
  for (int i = 0; i < parameters.size(); ++i) {
    outputCode += parameters[i].generateCode();
    if (i < parameters.size() - 1)
      outputCode += ",";
  }
  outputCode += ") {";
  for (const auto &node : body)
    outputCode += node.get()->generateCode();
  outputCode += "}";
  return outputCode;
}

IfStatementBlock::IfStatementBlock(optional<variant<TYPES>> condition, const vector<shared_ptr<ASTNode>> &body) {
  if (condition.has_value()) {
    variant<SHARED_POINTER_TYPES> convertedCondition;
    variant<TYPES> conditionValue = condition.value();
    visitor(convertedCondition, conditionValue);
    this->condition = convertedCondition;
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

PrintStatement::PrintStatement(variant<TYPES> &value) { visitor(this->value, value); }

ReturnStatement::ReturnStatement(variant<TYPES> &value) { visitor(this->value, value); }
