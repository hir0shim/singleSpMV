#include <vector>
#include <algorithm>
#include <cstring>
#include "opt_ell.h"
#include "util.h"
using namespace std;
void OptimizeProblem (const SpMat &A, const Vec &x, SpMatOpt &A_opt, VecOpt &x_opt) {
    x_opt.size = x.size;
    x_opt.val = x.val;

    int nRow = A.nRow;
    int nCol = A.nCol;
    int nNnz = A.nNnz;

    //------------------------------
    // Format specific 
    //------------------------------
    vector<vector<Element>> G(nRow);
    {
        int *row_idx = A.row_idx;
        int *col_idx = A.col_idx;
        double *val = A.val;
        for (int i = 0; i < nNnz; i++) {
            G[row_idx[i]].emplace_back(row_idx[i], col_idx[i], val[i]);
        }
    }
    {
        int K = 0;
        for (int i = 0; i < nRow; i++) {
            K = max<int>(K, G[i].size());
        }
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
        A_opt.nRow = nRow;
        A_opt.nCol = nCol;
        A_opt.nNnz = nNnz;
        A_opt.K = K;
        A_opt.col_idx = col_idx;
        A_opt.val = val;
    }
}
extern "C" {
    void SpMV (const SpMatOpt &A, const VecOpt &x, Vec &y) {
        double *xv = x.val;
        double *yv = y.val;
        int nRow = A.nRow;
        int nCol = A.nCol;
        int nNnz = A.nNnz;

        //------------------------------
        // Format specific 
        //------------------------------
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



