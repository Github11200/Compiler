#ifndef AST_H
#define AST_H

#include "ast/node.h"
#include "token.h"
#include "utils.h"
#include <concepts>
#include <memory>
#include <optional>
#include <set>
#include <stack>

struct CodeBlock {
  std::vector<Token> statement;
  std::vector<Token> bodyTokens;
};

struct BindingPower {
  double left, right;

  BindingPower(double left, double right) : left(left), right(right) {}
};

class AST {
private:
  std::vector<std::set<std::string>> scopes; // Stores the identifiers

  static BindingPower getBindingPower(TokenType op);

  static bool keywordIsStartOfNewCodeBlock(TokenType keyword);

  static std::optional<int> isInequality(const std::vector<Token> &statement);

  static std::variant<TYPES> evaluateExpression(const std::vector<Token> &statement, int &i, int minimumBindingPower = 0);

  static std::shared_ptr<VariableStatement> evaluateVariableStatement(const std::vector<Token> &statement);

  std::shared_ptr<FunctionStatement> evaluateFunctionStatement(const CodeBlock &functionBlock);

  std::shared_ptr<IfStatement> evaluateIfStatement(const std::vector<Token> &body);

  std::shared_ptr<LoopStatement> evaluateLoopStatement(const CodeBlock &loopBlock);

  static std::vector<Token> extractBody(int &i, const std::vector<Token> &tokens, TokenType keyword = TokenType::END);

  static void incrementToKeyword(int &i, const std::vector<Token> &tokens, std::vector<Token> &currentNodes, TokenType keyword);

public:
  AST() = default;

  std::shared_ptr<Root> constructAST(const std::vector<Token> &tokens);
};

#endif
