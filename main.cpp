#include "ast/ast.h"
#include "codeGenerator.h"
#include "lexer.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>

using namespace std;

int main() {
  string filePath = "code.txt";
  ifstream file(filePath);

  if (!file.is_open()) {
    cout << "Couldn't open file." << endl;
    filesystem::path currentPath = filesystem::current_path();
    cout << currentPath << endl;
    return -1;
  }

  string line;
  string code;

  while (getline(file, line))
    code += line;

  Lexer lexer(code);
  vector<Token> tokens = lexer.getTokens();
  AST ast;

  shared_ptr<Root> rootNode = ast.constructAST(tokens);
  CodeGenerator codeGenerator(rootNode);

  codeGenerator.generate("compiled.cpp");
  file.close();

  return 0;
}
