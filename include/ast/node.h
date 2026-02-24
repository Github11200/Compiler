#ifndef NODE_H
#define NODE_H

#include "token.h"
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

struct ASTNode {
  virtual ~ASTNode() = default;
  virtual std::string generateCode() = 0;
};

struct Root final : ASTNode {
  std::vector<std::shared_ptr<ASTNode>> nodes;

  std::string generateCode() override;
};

struct IntegerLiteral final : ASTNode {
  int value;

  IntegerLiteral(const int value) : value(value) {}

  std::string generateCode() override;
};

struct BinaryExpression final : ASTNode {
  TokenType operatorType;
  std::variant<std::shared_ptr<BinaryExpression>, std::shared_ptr<IntegerLiteral>> left;
  std::variant<std::shared_ptr<BinaryExpression>, std::shared_ptr<IntegerLiteral>> right;

  BinaryExpression(TokenType operatorType, std::variant<BinaryExpression, IntegerLiteral> left, std::variant<BinaryExpression, IntegerLiteral> right);

  std::string generateCode() override;
};

struct VariableStatement final : ASTNode {
  bool isPointer;
  std::optional<std::string> pointerIdentifier;

  std::string identifier;
  std::variant<std::shared_ptr<BinaryExpression>, std::shared_ptr<IntegerLiteral>> value;

  VariableStatement(const std::string &pointerIdentifier);
  VariableStatement(const std::string &identifier, std::variant<BinaryExpression, IntegerLiteral> value);

  std::string generateCode() override;
};

struct FunctionStatement final : ASTNode {
  std::string identifier;
  std::vector<std::string> parameters;
  std::vector<std::shared_ptr<ASTNode>> body;

  FunctionStatement(const std::string &identifier, const std::vector<std::shared_ptr<ASTNode>> &body, const std::vector<std::string> &parameters);

  std::string generateCode() override;
};

struct IfStatementBlock {
  std::optional<std::variant<std::shared_ptr<BinaryExpression>, std::shared_ptr<IntegerLiteral>>> condition;
  std::vector<std::shared_ptr<ASTNode>> body;

  IfStatementBlock(const std::optional<std::variant<BinaryExpression, IntegerLiteral>> condition, const std::vector<std::shared_ptr<ASTNode>> &body);
};

struct IfStatement final : ASTNode {
  std::vector<IfStatementBlock> ifStatementBlocks;

  IfStatement(const std::vector<IfStatementBlock> &ifStatementBlocks);

  std::string generateCode() override;
};

struct LoopStatement final : ASTNode {
  std::shared_ptr<BinaryExpression> condition;
  std::vector<std::shared_ptr<ASTNode>> body;

  LoopStatement(const BinaryExpression condition, const std::vector<std::shared_ptr<ASTNode>> &body);

  std::string generateCode() override;
};

#endif
