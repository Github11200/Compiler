#include "lexer.h"
#include "token.h"

using namespace std;

Lexer::Lexer(string sourceCode) {
  keywords.insert("let");
  keywords.insert("be");
  keywords.insert("pointer");
  keywords.insert("to");
  keywords.insert("stop");
  keywords.insert("define");
  keywords.insert("as");
  keywords.insert("end");
  keywords.insert("with");
  keywords.insert("if");
  keywords.insert("greater");
  keywords.insert("than");
  keywords.insert("then");
  keywords.insert("less");
  keywords.insert("or");
  keywords.insert("equal");
  keywords.insert("otherwise");
  keywords.insert("for");
  keywords.insert("repeat");
  keywords.insert("plus");
  keywords.insert("times");
  keywords.insert("divide");
  keywords.insert("minus");
  keywords.insert("just");
  keywords.insert("quote");
  keywords.insert("say");
  keywords.insert("call");
  keywords.insert("give");
  keywords.insert("back");
  keywords.insert("of");
  keywords.insert("type");
  keywords.insert("integer");
  keywords.insert("string");
  keywords.insert("float");
  keywords.insert("void");
  keywords.insert("gives");

  set<string> delimeters = {" ", "stop", "then", "as", "end", "otherwise", "repeat", "back"};
  this->splitSourceCode = splitString(sourceCode, delimeters);
  this->index = 0;
}

vector<Token> Lexer::getTokens() {
  vector<Token> tokens;
  bool isCurrentlyString = false;
  for (; index < splitSourceCode.size(); ++index) {
    string currentToken = splitSourceCode[index];

    Token token(TokenType::IDENTIFIER, " ");

    if (currentToken == " " && isCurrentlyString)
      token.tokenType = TokenType::SPACE;
    else if (currentToken == " " && !isCurrentlyString)
      continue;

    if (keywords.contains(currentToken)) {
      if (currentToken == "let")
        token.tokenType = TokenType::LET;
      else if (currentToken == "be")
        token.tokenType = TokenType::BE;
      else if (currentToken == "pointer")
        token.tokenType = TokenType::POINTER;
      else if (currentToken == "to")
        token.tokenType = TokenType::TO;
      else if (currentToken == "stop")
        token.tokenType = TokenType::STOP;
      else if (currentToken == "define")
        token.tokenType = TokenType::DEFINE;
      else if (currentToken == "as")
        token.tokenType = TokenType::AS;
      else if (currentToken == "end")
        token.tokenType = TokenType::END;
      else if (currentToken == "if")
        token.tokenType = TokenType::IF;
      else if (currentToken == "then")
        token.tokenType = TokenType::THEN;
      else if (currentToken == "or")
        token.tokenType = TokenType::OR;
      else if (currentToken == "with")
        token.tokenType = TokenType::WITH;
      else if (currentToken == "equal")
        token.tokenType = TokenType::EQUALS;
      else if (currentToken == "otherwise")
        token.tokenType = TokenType::OTHERWISE;
      else if (currentToken == "for")
        token.tokenType = TokenType::FOR;
      else if (currentToken == "repeat")
        token.tokenType = TokenType::REPEAT;
      else if (currentToken == "plus")
        token.tokenType = TokenType::PLUS;
      else if (currentToken == "minus")
        token.tokenType = TokenType::MINUS;
      else if (currentToken == "times")
        token.tokenType = TokenType::TIMES;
      else if (currentToken == "divide")
        token.tokenType = TokenType::DIVIDE;
      else if (currentToken == "just")
        token.tokenType = TokenType::JUST;
      else if (currentToken == "say")
        token.tokenType = TokenType::SAY;
      else if (currentToken == "give")
        token.tokenType = TokenType::GIVE;
      else if (currentToken == "gives")
        token.tokenType = TokenType::GIVES;
      else if (currentToken == "back")
        token.tokenType = TokenType::BACK;
      else if (currentToken == "integer")
        token.tokenType = TokenType::INTEGER;
      else if (currentToken == "string")
        token.tokenType = TokenType::STRING;
      else if (currentToken == "float")
        token.tokenType = TokenType::FLOAT;
      else if (currentToken == "void")
        token.tokenType = TokenType::VOID;
      else if (currentToken == "quote") {
        token.tokenType = TokenType::QUOTE;
        if (!isCurrentlyString)
          ++index;
        else
          tokens.pop_back();
        isCurrentlyString = !isCurrentlyString;
      } else if (currentToken == "call")
        token.tokenType = TokenType::CALL;
      else if (currentToken == "of") {
        currentToken += " type";
        token.tokenType = TokenType::OF_TYPE;
        ++index;
        while (index < splitSourceCode.size() && splitSourceCode[index] == " ")
          ++index;
        if (splitSourceCode[index] != "type")
          throw new string("You need the keyword type.");
      } else if (currentToken == "greater" || currentToken == "less") {
        // This is if the tokens are "5 greater/less than or equals to 4"
        if (splitSourceCode[index + 4] == "or") {
          token.tokenType = currentToken == "greater" ? TokenType::GREATER_THAN_OR_EQUALS_TO : TokenType::LESS_THAN_OR_EQUALS_TO;
          currentToken += " than or equals to";
          for (; index < splitSourceCode.size() && splitSourceCode[index] != "to"; ++index) {
          }
        } else {
          token.tokenType = currentToken == "greater" ? TokenType::GREATER_THAN : TokenType::LESS_THAN;
          currentToken += " than";
          for (; index < splitSourceCode.size() && splitSourceCode[index] != "than"; ++index) {
          }
        }
      }
    }

    if (token.tokenType != TokenType::IDENTIFIER) {
      token.tokenString = currentToken;
      tokens.push_back(token);
      continue;
    }

    if (isInteger(currentToken))
      token.tokenType = TokenType::INTEGER_LITERAL;
    else if (!keywords.contains(currentToken))
      token.tokenType = TokenType::IDENTIFIER;
    else
      throw new string("What are you doing.");

    token.tokenString = currentToken;
    tokens.push_back(token);
  }

  return tokens;
}
