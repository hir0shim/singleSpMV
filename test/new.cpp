#include <iostream>
using namespace std;
int main () {
    int N = 10000000;
    int n = 0;
    while (1) {
        int *a = new int[N];
        for (int i = 0; i < N; i++) a[i] = 1;
        for (int i = 0; i < N; i++) n += a[i];
        cout << n << endl;
    }
}
