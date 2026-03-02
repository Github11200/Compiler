#include "codeGenerator.h"
#include "ast/node.h"

using namespace std;

CodeGenerator::CodeGenerator(shared_ptr<Root> rootNode) { this->rootNode = rootNode; }

void CodeGenerator::generate(string fileName) {
  fstream outputFile;

  string functionCode = "";
  string currentCode = "";

  outputFile.open(fileName, fstream::out);
  outputFile << "#include <iostream>\n";
  outputFile << "using namespace std;\n";
  currentCode += "int main() {\n";
  for (Root *pointer = rootNode.get(); const auto &node : pointer->nodes) {
    if (FunctionStatement *functionStatement = dynamic_cast<FunctionStatement *>(node.get()))
      functionCode += node->generateCode();
    else
      currentCode += node->generateCode();
  }
  currentCode += "return 0;\n";
  currentCode += "}\n";

  outputFile << functionCode << currentCode << endl;

  outputFile.close();
}
