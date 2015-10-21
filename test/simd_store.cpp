#include <iostream>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <pmmintrin.h>
#include <tmmintrin.h>
#include <smmintrin.h>
#include <nmmintrin.h>
#include <wmmintrin.h>
#include <immintrin.h>
#include <mm_malloc.h>
using namespace std;
int main () {
    int A = 32;
    double *src = (double *)_mm_malloc(A, A);
    for (int i = 0; i < A / sizeof(double); i++) {
        src[i] = i;
    }
    double *dst = (double *)_mm_malloc(A, A);
    __m256d a = _mm256_load_pd(src);
    _mm256_store_pd(dst, a);
    for (int i = 0; i < A / sizeof(double); i++) {
        cout << i << " " << dst[i] << endl;
    }
}
