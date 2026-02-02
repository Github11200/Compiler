#include "codeGenerator.h"

using namespace std;

CodeGenerator::CodeGenerator(shared_ptr<Root> rootNode) {
  this->rootNode = rootNode;
}

void CodeGenerator::generate(string fileName) {
  fstream outputFile;

  outputFile.open(fileName, fstream::out);
  outputFile << "#include <iostream>" << endl;
  outputFile << "using namespace std;" << endl;
  outputFile << "int main() {" << endl;
  for (Root *pointer = rootNode.get(); const auto &node : pointer->nodes)
    outputFile << node->generateCode() << endl;
  outputFile << "return 0;" << endl;
  outputFile << "}" << endl;

  outputFile.close();
}
