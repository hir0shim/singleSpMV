#include <iostream>
using namespace std;
int main () {

    int n = 15;
    int a = 4;
    cout << n << " - " << (n&(a-1)) << " = " << (n-(n&(a-1))) << endl;
    for (int i = 0; i < n-(n&(a-1)); i+=a) {
        cout << i << endl;
    }
}

