#include "ast/ast.h"
#include "ast/node.h"
#include "token.h"
#include <cstddef>
#include <memory>
#include <optional>

using namespace std;

BindingPower AST::getBindingPower(TokenType op) {
  if (op == TokenType::PLUS || op == TokenType::MINUS)
    return BindingPower(1, 1.5);
  if (op == TokenType::TIMES || op == TokenType::DIVIDE)
    return BindingPower(2, 2.5);
  throw new string("What is this token");
}

bool AST::keywordIsStartOfNewCodeBlock(TokenType keyword) {
  if (keyword == TokenType::AS || keyword == TokenType::THEN || keyword == TokenType::REPEAT || keyword == TokenType::JUST)
    return true;
  return false;
}

vector<Token> AST::extractBody(int &i, const vector<Token> &tokens, TokenType keyword) {
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

void AST::incrementToKeyword(int &i, const std::vector<Token> &tokens, std::vector<Token> &currentNodes, TokenType keyword) {
  for (; i < tokens.size(); ++i) {
    currentNodes.push_back(tokens[i]);
    if (tokens[i].tokenType == keyword) {
      ++i;
      break;
    }
  }
}

optional<int> AST::isInequality(const vector<Token> &statement) {
  for (int i = 0; i < statement.size(); ++i) {
    TokenType currentTokenType = statement[i].tokenType;
    // If there is an inequality symbol then return the current index
    if (ranges::find(inequalitySymbols, currentTokenType) != ranges::end(inequalitySymbols))
      return i;
  }
  return nullopt;
}

variant<TYPES> AST::evaluateExpression(const vector<Token> &statement, int &i, int minimumBindingPower) {
  if (statement[0].tokenType == TokenType::QUOTE) {
    string stringLiteralValue;
    for (int i = 1; i < statement.size() - 1; ++i)
      stringLiteralValue += statement[i].tokenString;
    return StringLiteral(stringLiteralValue);
  } else if (statement[0].tokenType == TokenType::CALL)
    return *evaluateFunctionCallStatement(statement, false).get();

  // The statement is something like let x be 5;
  if (statement.size() == 1 || i == statement.size() - 1) {
    if (statement[i].tokenType == TokenType::INTEGER_LITERAL)
      return IntegerLiteral(stoi(statement[i].tokenString));
    else
      return Identifier(statement[i].tokenString);
  }

  optional<int> inequalityIndex = isInequality(statement);
  if (inequalityIndex.has_value()) {
    vector<Token> leftArray, rightArray;
    for (int j = 0; j < inequalityIndex.value(); ++j)
      leftArray.push_back(statement[j]);
    for (int j = inequalityIndex.value() + 1; j < statement.size(); ++j)
      rightArray.push_back(statement[j]);

    int iCopy = i;
    variant<TYPES> left = evaluateExpression(leftArray, iCopy, 0);
    iCopy = i;
    variant<TYPES> right = evaluateExpression(rightArray, iCopy, 0);

    return BinaryExpression(statement[inequalityIndex.value()].tokenType, left, right);
  }

  // We will assume it contains an operator otherwise
  unique_ptr<BinaryExpression> binaryExpression = nullptr;
  variant<TYPES> leftHandSide = evaluateExpression(vector<Token>({statement[0]}), i);
  for (; i < statement.size();) {
    TokenType op;
    BindingPower bindingPower(0, 0);
    if (i < statement.size() - 1) {
      op = statement[i + 1].tokenType;
      bindingPower = getBindingPower(op);

      if (bindingPower.left < minimumBindingPower)
        break;

      i += 2;
    }

    variant<TYPES> rightHandSide = evaluateExpression(statement, i, bindingPower.right);
    if (binaryExpression == nullptr)
      binaryExpression = make_unique<BinaryExpression>(op, leftHandSide, rightHandSide);
    else
      binaryExpression = make_unique<BinaryExpression>(op, *binaryExpression, rightHandSide);

    if (i == statement.size() - 1)
      break;
  }

  if (binaryExpression == nullptr)
    return leftHandSide;
  return *binaryExpression;
}

shared_ptr<VariableStatement> AST::evaluateVariableStatement(const vector<Token> &statement) {
  string identifier = statement[1].tokenString;

  // Get the variable type
  bool isPointer = statement[4].tokenType == TokenType::POINTER;
  Type variableType(statement[3].tokenType, isPointer);

  vector<Token> expressionTokens;
  int i = isPointer ? 6 : 5; // If it isn't a pointer then we should start an index earlier
  for (; i < statement.size() - 1; ++i)
    expressionTokens.push_back(statement[i]);

  i = 0;
  variant<TYPES> expression = evaluateExpression(expressionTokens, i, 0);
  return make_shared<VariableStatement>(identifier, variableType, expression);
}

shared_ptr<FunctionStatement> AST::evaluateFunctionStatement(const CodeBlock &functionBlock) {
  string identifier = functionBlock.statement[1].tokenString;
  vector<Parameter> parameters;

  int i = 4;

  // The function has parameters
  if (functionBlock.statement[2].tokenType == TokenType::WITH) {
    i = 3;
    for (; functionBlock.statement[i].tokenType != TokenType::GIVES; ++i) {
      string variableName = functionBlock.statement[i].tokenString;

      bool isParameterPointer = functionBlock.statement[i + 3].tokenType == TokenType::POINTER;
      Type parameterType(functionBlock.statement[i + 2].tokenType, isParameterPointer);

      parameters.push_back(Parameter(Identifier(variableName), parameterType));
      if (isParameterPointer)
        i += 3;
      else
        i += 2;
    }
    i += 2;
  }

  bool isReturnTypePointer = functionBlock.statement[i + 1].tokenType == TokenType::POINTER;
  TokenType type = functionBlock.statement[i].tokenType;

  Type returnType(type, isReturnTypePointer);

  vector<shared_ptr<ASTNode>> functionBody = constructAST(functionBlock.bodyTokens).get()->nodes;
  return make_shared<FunctionStatement>(identifier, functionBody, parameters, returnType);
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
    if (body[i].tokenType == TokenType::END || body[i].tokenType == TokenType::OTHERWISE)
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

    if (body[k].tokenType == TokenType::THEN || body[k].tokenType == TokenType::OTHERWISE)
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
    vector<shared_ptr<ASTNode>> bodyTokensAST = constructAST(currentBlockTokens).get()->nodes;
    IfStatementBlock newBlock(nullopt, bodyTokensAST);

    if (!isLastElseBlock) { // This means there is a statement to evaluate
      int i = 0;
      newBlock = IfStatementBlock(evaluateExpression(currentBlockStatement, i, 0), bodyTokensAST);
    }

    ifStatementBlocks.push_back(newBlock);
  }

  return make_shared<IfStatement>(ifStatementBlocks);
}

shared_ptr<LoopStatement> AST::evaluateLoopStatement(const CodeBlock &loopBlock) {
  // Loop from after the for keyword to before the repeat keyword
  vector<Token> expressionTokens;
  for (int i = 1; i < loopBlock.statement.size() - 1; ++i)
    expressionTokens.push_back(loopBlock.statement[i]);

  int i = 0;
  BinaryExpression evaluatedExpression = get<BinaryExpression>(evaluateExpression(expressionTokens, i, 0));

  vector<shared_ptr<ASTNode>> loopStatementBody = constructAST(loopBlock.bodyTokens).get()->nodes;

  return make_shared<LoopStatement>(evaluatedExpression, Identifier(loopBlock.statement[1].tokenString), loopStatementBody);
}

shared_ptr<PrintStatement> AST::evaluatePrintStatement(const vector<Token> &statement) {
  vector<Token> expression;
  for (int i = 1; i < statement.size() - 1; ++i)
    expression.push_back(statement[i]);
  int i = 0;
  variant<TYPES> evaluatedExpression = evaluateExpression(expression, i);
  return make_shared<PrintStatement>(evaluatedExpression);
}

shared_ptr<FunctionCallStatement> AST::evaluateFunctionCallStatement(const vector<Token> &statement, bool hasSemicolon) {
  string functionName = statement[1].tokenString;
  vector<variant<TYPES>> parameters;

  // There are parameters to pass into the function
  if (statement[2].tokenType == TokenType::WITH) {
    vector<Token> currentExpression;
    for (int i = 3; i < statement.size() && statement[i].tokenType != TokenType::STOP; ++i) {
      if (statement[i].tokenType == TokenType::COMMA) {
        int j = 0;
        parameters.push_back(evaluateExpression(currentExpression, j));
        currentExpression.clear();
      } else
        currentExpression.push_back(statement[i]);
    }

    int j = 0;
    parameters.push_back(evaluateExpression(currentExpression, j));
  }
  return make_shared<FunctionCallStatement>(functionName, hasSemicolon, parameters);
}

shared_ptr<ReturnStatement> AST::evaluateReturnStatement(const vector<Token> &statement) {
  vector<Token> expression;
  for (int i = 1; i < statement.size() - 1; ++i)
    expression.push_back(statement[i]);
  int i = 0;
  variant<TYPES> evaluatedExpression = evaluateExpression(expression, i);
  return make_shared<ReturnStatement>(evaluatedExpression);
}

shared_ptr<Root> AST::constructAST(const vector<Token> &tokens) {
  Root rootNode;

  vector<Token> currentNodes;
  for (int i = 0; i < tokens.size();) {
    currentNodes.push_back(tokens[i]);
    std::shared_ptr<ASTNode> newNode = nullptr;

    if (tokens[i].tokenType == TokenType::GIVE) {
      incrementToKeyword(++i, tokens, currentNodes, TokenType::BACK);
      newNode = evaluateReturnStatement(currentNodes);
    } else if (tokens[i].tokenType == TokenType::SAY) {
      incrementToKeyword(++i, tokens, currentNodes, TokenType::STOP);
      newNode = evaluatePrintStatement(currentNodes);
    } else if (tokens[i].tokenType == TokenType::CALL) {
      incrementToKeyword(++i, tokens, currentNodes, TokenType::STOP);
      newNode = evaluateFunctionCallStatement(currentNodes, true);
    } else if (tokens[i].tokenType == TokenType::LET) {
      incrementToKeyword(++i, tokens, currentNodes, TokenType::STOP);
      newNode = evaluateVariableStatement(currentNodes);
    } else if (tokens[i].tokenType == TokenType::IF) {
      incrementToKeyword(++i, tokens, currentNodes, TokenType::THEN);
      vector<Token> body = extractBody(i, tokens);
      for (Token t : body)
        currentNodes.push_back(t);
      newNode = evaluateIfStatement(currentNodes);
      ++i;
    } else if (tokens[i].tokenType == TokenType::DEFINE) {
      incrementToKeyword(++i, tokens, currentNodes, TokenType::AS);
      newNode = evaluateFunctionStatement({.statement = currentNodes, .bodyTokens = extractBody(i, tokens)});
      ++i;
    } else if (tokens[i].tokenType == TokenType::FOR) {
      incrementToKeyword(++i, tokens, currentNodes, TokenType::REPEAT);
      newNode = evaluateLoopStatement({.statement = currentNodes, .bodyTokens = extractBody(i, tokens)});
      ++i;
    }

    if (newNode != nullptr) {
      rootNode.nodes.push_back(newNode);
      currentNodes.clear();
    }
  }

  return make_shared<Root>(rootNode);
}
