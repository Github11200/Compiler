#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <utility>
#include <vector>

enum class TokenType {
  PLUS,
  MINUS,
  TIMES,
  DIVIDE,

  LET,
  BE,
  POINTER,
  TO,
  STOP,
  DEFINE,
  AS,
  END,
  WITH,
  IF,
  GREATER_THAN,
  GREATER_THAN_OR_EQUALS_TO,
  LESS_THAN,
  LESS_THAN_OR_EQUALS_TO,
  THEN,
  OR,
  EQUALS,
  OTHERWISE,
  FOR,
  REPEAT,
  JUST,
  SAY,
  QUOTE,

  IDENTIFIER,
  INTEGER_LITERAL
};

extern std::vector<TokenType> inequalitySymbols;

class Token {
public:
  TokenType tokenType;
  std::string tokenString;

  Token(const TokenType tokenType, std::string tokenString) : tokenType(tokenType), tokenString(std::move(tokenString)) {}
};

#endif
