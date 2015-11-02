#include <immintrin.h>
#include <iostream>

using namespace std;
int main () {
    int m[] =     {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
    __m512i a = {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2};
    __m512i b = _mm512_mask_load_epi32(a, 0x00ff, m);
    _mm512_store_epi32(m, b);
    for (int i = 0; i < 16; i++) cout << m[i] << " ";
    cout << endl;
    
    return 0;
}
