#include <iostream>
using namespace std;
int main () {
    int begin = 4;
    int end = 40;
    int W = 4;
    int simd_begin = (begin & (~(W-1))) + ((begin & (W - 1)) ? W:0);
    int simd_end = end - (end&(W-1));
    cout << begin << " " << simd_begin << " " << simd_end << " " << end << endl;
}

