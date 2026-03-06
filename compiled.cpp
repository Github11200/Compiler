#include <iostream>
using namespace std;
void  func (int x) {cout << x << endl;}int  doSomething (int x,int y) {func(x);func(y);for (int i = 0;i < 10;++i) {cout << i << endl;}if (y > x) {return y;}else {return x;}}int main() {
cout << doSomething(5,6) << endl;return 0;
}

