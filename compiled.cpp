#include <iostream>
using namespace std;
int main() {
  auto a = 10;
  if (2) {
    auto b = 20;
  } else if (3) {
    auto b = 25;
  } else if (3) {
    auto b = 25;
  } else {
    auto b = 30;
  }
  return 0;
}
