#include <immintrin.h>
#include <iostream>

using namespace std;
int main () {
    int m[] =     {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
    __m512i a = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    //a = _mm512_srli_epi32(a, 2);
    a = _mm512_alignr_epi32(a, a, 8);
    _mm512_store_epi32(m, a);
    for (int i = 0; i < 16; i++) cout << m[i] << " ";
    cout << endl;
    
    return 0;
}
