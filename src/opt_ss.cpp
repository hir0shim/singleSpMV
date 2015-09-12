#include "opt_ss.h"
#include "util.h"
#include <algorithm>
#include <vector>
#include <utility>
#include <cassert>
#include <iostream>
void OptimizeProblem (const SpMat &A, const Vec &x, SpMatOpt &A_opt, VecOpt &x_opt) {
    x_opt.size = x.size;
    x_opt.val = x.val;

    int nRow = A.nRow;
    int nCol = A.nCol;
    int nNnz = A.nNnz;

    //------------------------------
    // Format specific 
    //------------------------------
    
    int *row_idx = A.row_idx;
    int *col_idx = A.col_idx;
    double *val = A.val;

    int W = 512 / sizeof(double);
    int H = nNnz / W + (nNnz % W != 0);
    int *row_ptr = new int[nRow+1];
    int **row_idx_2d = new int*[H];
    int **col_idx_2d = new int*[H];
    double **val_2d = new double*[H];
    double **val_result_2d = new double*[H];
    bool **flag_2d = new bool*[H];

    row_idx_2d[0] = new int[H*W];
    col_idx_2d[0] = new int[H*W];
    val_2d[0] = new double[H*W];
    val_result_2d[0] = new double[H*W];
    flag_2d[0] = new bool[H*W];

    for (int i = 1; i < H; i++) {
        row_idx_2d[i] = row_idx_2d[i-1] + W;
        col_idx_2d[i] = col_idx_2d[i-1] + W;
        val_2d[i] = val_2d[i-1] + W;
        val_result_2d[i] = val_result_2d[i-1] + W;
        flag_2d[i] = flag_2d[i-1] + W;
    }
    row_ptr[0] = 0;
    int cur_row = 0;
    for (int i = 0; i < H; i++) {
        for (int j = 0; j < W; j++) {
            int p = i*W + j;
            if (p < nNnz) {
                row_idx_2d[i][j] = row_idx[p];
                col_idx_2d[i][j] = col_idx[p];
                val_2d[i][j] = val[p];
                int r = row_idx[p];
                while (cur_row <= r) { row_ptr[cur_row++] = p; }
            } else {
                row_idx_2d[i][j] = nRow;
                col_idx_2d[i][j] = 0;
                val_2d[i][j] = 0;
            }
        }
    }
    while (cur_row <= nRow) row_ptr[cur_row++] = nNnz;
    A_opt.nRow = nRow;
    A_opt.nCol = nCol;
    A_opt.nNnz = nNnz;
    A_opt.H = H;
    A_opt.W = W;
    A_opt.row_ptr = row_ptr;
    A_opt.row_idx = row_idx_2d;
    A_opt.col_idx = col_idx_2d;
    A_opt.flag = flag_2d;
    A_opt.val = val_2d;
    A_opt.val_result = val_result_2d;
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
        int H = A.H;
        int W = A.W;
        int **col_idx = A.col_idx;
        double **val = A.val;
        double **val_result = A.val_result;
#pragma omp parallel
        {
#pragma omp for
            for (int i = 0; i < H; i++) {
#pragma ivdep
                for (int j = 0; j < W; j++) {
                    val_result[i][j] = val[i][j] * xv[col_idx[i][j]];
                }
            }
            int *row_ptr = A.row_ptr;
#pragma omp for
            for (int i = 0; i < nRow; i++) {
                yv[i] = 0;
#pragma ivdep
                for (int j = row_ptr[i]; j < row_ptr[i+1]; j++) {
                    yv[i] += *(val_result[0] + j);
                }
            }
        }
    }
}



