#include "ast/ast.h"
#include "ast/node.h"
#include "token.h"
#include <optional>
#include <random>

using namespace std;

bool AST::keywordIsStartOfNewCodeBlock(TokenType keyword) {
  if (keyword == TokenType::AS || keyword == TokenType::THEN ||
      keyword == TokenType::REPEAT || keyword == TokenType::JUST)
    return true;
  return false;
}

vector<Token> AST::extractBody(int &i, const vector<Token> &tokens,
                               TokenType keyword) {
  int depth = 0;
  vector<Token> currentNodes;
  for (; i < tokens.size(); ++i) {
    if (tokens[i].tokenType == keyword && depth == 0)
      break;
    if (keywordIsStartOfNewCodeBlock(tokens[i].tokenType))
      ++depth;
    if (tokens[i].tokenType == TokenType::END)
      --depth;
    currentNodes.push_back(tokens[i]);
  }

  return currentNodes;
}

void AST::incrementToKeyword(int &i, const std::vector<Token> &tokens,
                             std::vector<Token> &currentNodes,
                             TokenType keyword) {
  for (; i < tokens.size(); ++i) {
    currentNodes.push_back(tokens[i]);
    if (tokens[i].tokenType == keyword) {
      ++i;
      break;
    }
  }
}

variant<BinaryExpression, IntegerLiteral>
AST::evaluateExpression(const vector<Token> &statement) {
  // The statement is something like let x be 5;
  if (statement.size() == 1)
    return IntegerLiteral(stoi(statement[0].tokenString));

  // We will assume it contains an operator otherwise
  unique_ptr<BinaryExpression> binaryExpression = nullptr;
  for (int i = 0; i < statement.size(); ++i) {
    if (!isOperator(statement[i].tokenType))
      continue;

    if (i + 1 == statement.size())
      throw "An operand must be added after the operator.";

    IntegerLiteral leftInteger(stoi(statement[i - 1].tokenString));
    IntegerLiteral rightInteger(stoi(statement[i + 1].tokenString));
    BinaryExpression newExpression(statement[i].tokenType, leftInteger,
                                   rightInteger);

    // If there's already an expression then this should go to it's right
    if (binaryExpression != nullptr)
      binaryExpression->right =
          std::make_shared<BinaryExpression>(newExpression);
    else
      binaryExpression = make_unique<BinaryExpression>(newExpression);
  }

  return *binaryExpression;
}

shared_ptr<VariableStatement>
AST::evaluateVariableStatement(const vector<Token> &statement) {
  string identifier = statement[1].tokenString;

  if (statement[3].tokenType == TokenType::POINTER)
    return make_shared<VariableStatement>(statement[5].tokenString);

  vector<Token> expressionTokens;
  for (int i = 3; i < statement.size() - 1; ++i)
    expressionTokens.push_back(statement[i]);

  variant<BinaryExpression, IntegerLiteral> expression =
      evaluateExpression(expressionTokens);
  return make_shared<VariableStatement>(identifier, expression);
}

shared_ptr<FunctionStatement>
AST::evaluateFunctionStatement(const CodeBlock &functionBlock) {
  string identifier = functionBlock.statement[1].tokenString;
  vector<string> parameters;

  // The function has parameters
  if (functionBlock.statement[2].tokenType == TokenType::WITH)
    for (int i = 3; functionBlock.statement[i].tokenType != TokenType::AS; ++i)
      parameters.push_back(functionBlock.statement[i].tokenString);

  vector<shared_ptr<ASTNode>> functionBody =
      constructAST(functionBlock.bodyTokens).get()->nodes;
  return make_shared<FunctionStatement>(identifier, functionBody, parameters);
}

// Figure out how many blocks there are
// Loop through each block's statement

shared_ptr<IfStatement> AST::evaluateIfStatement(const vector<Token> &body) {
  // Loop through the body to find any other code blocks (from else if
  // statements)
  int numberOfBlocks = 1;
  int depth = 0;
  int i = 0;
  for (; i < body.size(); ++i) {
    if (body[i].tokenType == TokenType::END ||
        body[i].tokenType == TokenType::OTHERWISE)
      --depth;
    if (keywordIsStartOfNewCodeBlock(body[i].tokenType))
      ++depth;
    if (body[i].tokenType == TokenType::OTHERWISE && depth == 0)
      ++numberOfBlocks;
  }

  vector<IfStatementBlock> ifStatementBlocks;

  int k = 0;
  for (int j = 0; j < numberOfBlocks; ++j) {
    vector<Token> currentBlockTokens;
    vector<Token> currentBlockStatement;
    bool isLastElseBlock = j == numberOfBlocks - 1 && numberOfBlocks > 1;

    int expressionStartIndex = 0;
    if (j == 0) // This is the first block with just an if keyword, so start at
                // index 1
      expressionStartIndex = 1;
    else if (isLastElseBlock) // The last else block is "otherwise just" where
                              // there is no statement
      expressionStartIndex = 1;
    else // This is the "otherwise if" block so start at index 2
      expressionStartIndex = 2;

    k += expressionStartIndex;
    if (!isLastElseBlock) {
      for (; k < body.size(); ++k) {
        if (body[k].tokenType == TokenType::THEN)
          break;
        currentBlockStatement.push_back(body[k]);
      }
    }

    ++k;
    if (isLastElseBlock || numberOfBlocks == 1)
      currentBlockTokens = extractBody(k, body, TokenType::END);
    else
      currentBlockTokens = extractBody(k, body, TokenType::OTHERWISE);

    for (auto token : currentBlockTokens)
      cout << token.tokenString << endl;
    cout << "====" << endl;

    // Check to make sure this isn't the last otherwise statement since it has
    // no condition
    vector<shared_ptr<ASTNode>> bodyTokensAST =
        constructAST(currentBlockTokens).get()->nodes;
    IfStatementBlock newBlock(nullopt, bodyTokensAST);

    if (!isLastElseBlock) // This means there is a statement to evaluate
      newBlock = IfStatementBlock(evaluateExpression(currentBlockStatement),
                                  bodyTokensAST);

    ifStatementBlocks.push_back(newBlock);
  }

  return make_shared<IfStatement>(ifStatementBlocks);
}

shared_ptr<LoopStatement> AST::evaluateLoopStatement(const CodeBlock &loopBlock) {
  // Loop from after the for keyword to before the repeat keyword
  vector<Token> expressionTokens;
  for (int i = 1; i < loopBlock.statement.size() - 1; ++i)
    expressionTokens.push_back(loopBlock.statement[i]);

  BinaryExpression evaluatedExpression =
      get<BinaryExpression>(evaluateExpression(expressionTokens));

  vector<shared_ptr<ASTNode>> loopStatementBody =
      constructAST(loopBlock.bodyTokens).get()->nodes;

  return make_shared<LoopStatement>(evaluatedExpression, loopStatementBody);
}

shared_ptr<Root> AST::constructAST(const vector<Token> &tokens) {
  Root rootNode;

  vector<Token> currentNodes;
  for (int i = 0; i < tokens.size(); ++i) {
    currentNodes.push_back(tokens[i]);
    std::shared_ptr<ASTNode> newNode = nullptr;

    if (tokens[i].tokenType == TokenType::STOP) {
      newNode = evaluateVariableStatement(currentNodes);
    } else if (tokens[i].tokenType == TokenType::IF) {
      incrementToKeyword(++i, tokens, currentNodes, TokenType::END);
      newNode = evaluateIfStatement(currentNodes);
    } else if (tokens[i].tokenType == TokenType::DEFINE) {
      incrementToKeyword(++i, tokens, currentNodes, TokenType::AS);
      newNode = evaluateFunctionStatement(
          {.statement = currentNodes, .bodyTokens = extractBody(i, tokens)});
    } else if (tokens[i].tokenType == TokenType::FOR) {
      incrementToKeyword(++i, tokens, currentNodes, TokenType::REPEAT);
      newNode = evaluateLoopStatement(
          {.statement = currentNodes, .bodyTokens = extractBody(i, tokens)});
    }

    if (newNode != nullptr) {
      rootNode.nodes.push_back(newNode);
      currentNodes.clear();
    }
  }

  return make_shared<Root>(rootNode);
}
