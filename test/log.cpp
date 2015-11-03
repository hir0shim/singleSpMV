#include <iostream>
#include <cmath>
using namespace std;
int f (int a) {
    return 1<<int(ceil(log2(a)));
}
int main () {
    for (int i = 0; i < 30; i++) {
        cout << i << "->" << f(i) << endl;
    }
}
