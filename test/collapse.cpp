#include <iostream>
#include <cctype>
#include<cstdlib>
using namespace std;
int main(int argc, char** argv) {
    int M = atoi(argv[1]);
    int N = atoi(argv[2]);
    int nBlock = 32;
    int W = 4;
    int *H = new int[nBlock];
    int*** col_idx = new int**[nBlock];
    for (int i = 0; i < nBlock; i++) H[i] = rand()%M;
    for (int i = 0; i < nBlock; i++) {
        col_idx[i] = new int*[H[i]];
        for (int j = 0; j < H[i]; j++) {
            col_idx[i][j] = new int[W];
            for (int k = 0; k < W; k++) {
                col_idx[i][j][k] = rand()%N;
            }
        }
    }

    int *** val = new int**[nBlock];
    for (int i = 0; i < nBlock; i++) {
        val[i] = new int*[H[i]];
        for (int j = 0; j < H[i]; j++) {
            val[i][j] = new int[W];
            for (int k = 0; k < W; k++) {
                val[i][j][k] = rand()%N;
            }
        }
    }
    double *xv = new double[N];
    for (int i = 0; i < N; i++) {
        xv[i] = rand();
    }

#pragma omp parallel for collapse(2)
    for (int b = 0; b < nBlock; b++) {
        for (int i = 0; i < H[b]; i++) {
            for (int j = 0; j < W; j++) {
                val[b][i][j] *= xv[col_idx[b][i][j]];
            }
        }
    }
    for (int i = 0; i < nBlock; i++) {
        for (int j = 0; j < H[i]; j++) {
            for (int k = 0; k < W; k++) {
                cout << val[i][j][k] << endl;
            }
        }
    }
}

