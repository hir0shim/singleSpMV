#include <vector>
#include <algorithm>
#include <cstring>
#include "opt_ell.h"
#include "util.h"
using namespace std;
void OptimizeProblem (const SpMat &A, const Vec &x, SpMatOpt &A_opt, VecOpt &x_opt) {
    A_opt.nRow = A.nRow;
    A_opt.nCol = A.nCol;
    A_opt.nNnz = A.nNnz;
    vector<int> cnt(A.nRow);
    for (int i = 0; i < A.nNnz; i++) {
        cnt[A.row_idx[i]]++;
    }
    int K = *max_element(cnt.begin(), cnt.end());
    A_opt.K = K;
    int *ptr = new int[A.nRow];
    memset(ptr, 0, sizeof(int)*A.nRow);
    int **col_idx = new int*[A.nRow];
    for (int i = 0; i < A.nRow; i++) col_idx[i] = new int[K];
    double **val = new double*[A.nRow];
    for (int i = 0; i < A.nRow; i++) val[i] = new double[K];
    for (int i = 0; i < A.nNnz; i++) {
        int r = A.row_idx[i];   
        int c = A.col_idx[i];
        double v = A.val[i];
        col_idx[r][ptr[r]] = c;
        val[r][ptr[r]] = v;
        ptr[r]++;
    }
    for (int i = 0; i < A.nRow; i++) {
        while (ptr[i] < K) {
            col_idx[i][ptr[i]] = ptr[i];
            val[i][ptr[i]] = 0;
            ptr[i]++;
        }
    }
    A_opt.col_idx = col_idx;
    A_opt.val = val;
    x_opt.size = x.size;
    x_opt.val = x.val;

}
extern "C" {
void SpMV (const SpMatOpt &A, const VecOpt &x, Vec &y) {
    double *xv = x.val;
    double *yv = y.val;
    int nRow = A.nRow;
    int K = A.K;
    int **col_idx = A.col_idx;
    double **val = A.val;
#pragma omp parallel 
    {
#pragma omp for
        for (int i = 0; i < nRow; i++) yv[i] = 0;
#pragma omp for
        for (int r = 0; r < nRow; r++) {
#pragma ivdep
            for (int i = 0; i < K; i++) {
                int col = col_idx[r][i];
                double lv = xv[col];
                double rv = val[r][i];
                yv[r] += lv * rv;
            }
        }
    }
}
}



