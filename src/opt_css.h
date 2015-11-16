#pragma once
#include "util.h"
struct SpMatOpt {
    int nRow;
    int nCol;
    int nNnz;

    //const int B = 400000;
    const int W = ALIGNMENT / sizeof(double);

    int B;
    int nBlock;
    int *H;
    int **row_ptr; // [block][row]
    idx_t ***col_idx; // [block][vi][vj]
    double ***val;
};
struct VecOpt {
    int size;
    double *val;
};
void OptimizeProblem (const SpMat &A, const Vec &x, SpMatOpt &A_opt, VecOpt &x_opt);
extern "C" {
void SpMV (const SpMatOpt &A, const VecOpt &x, Vec &y);
}
