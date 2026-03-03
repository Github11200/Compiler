#include "ast/ast.h"
#include "codeGenerator.h"
#include "lexer.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>

/*
- Finish function parameter types and return types
 */

using namespace std;

ifstream getFile() {
  system("clear");
  cout << "Please give the relative file path: ";
  string filePath;
  cin >> filePath;

  ifstream file(filePath);
  return file;
}

void compile() {
  ifstream file;
  while (true) {
    file = getFile();
    if (file.is_open())
      break;

    cout << "Couldn't open file. Please select one of the options below:" << endl;
    cout << "\t1. Retry" << endl;
    cout << "\t2. Exit" << endl;
    int option;
    cin >> option;

    switch (option) {
    case 1:
      getFile();
      break;
    case 2:
      return;
    }
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
  system("g++ compiled.cpp -o compiled && ./compiled");
  file.close();
}

int main() {
  compile();
  // bool exit = false;
  // while (!exit) {
  //   system("clear");
  //   cout << "Welcome to the Verbosity compiler. Please choose one of the options below:" << endl;
  //   cout << "\t1. Compile and Run" << endl;
  //   cout << "\t2. About" << endl;
  //   cout << "\t3. Exit" << endl;
  //   cout << ">> ";

  //   int option = -1;
  //   cin >> option;
  //   while (option > 3 || option < 1) {
  //     cout << "Invalid option, please try again.";
  //     cout << ">> ";
  //     cin >> option;
  //   }

  //   switch (option) {
  //   case 1:
  //     compile();
  //     break;
  //   case 2: {
  //     system("clear");
  //     cout << "About" << endl;
  //     string input = "d";
  //     while (!input.empty()) {
  //       input = "";
  //       getline(std::cin, input);
  //     }
  //     break;
  //   }
  //   case 3:
  //     exit = true;
  //     break;
  //   }
  // }

  return 0;
}
