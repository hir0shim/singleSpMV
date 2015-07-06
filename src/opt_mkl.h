#pragma once
#include "util.h"
struct SpMatOpt {
    int nRow;
    int nCol;
    int nNnz;
    int *ptr;
    int *idx;
    double *val;
};
struct VecOpt {
    int size;
    double *val;
};
void OptimizeProblem (const SpMat &A, const Vec &x, SpMatOpt &A_opt, VecOpt &x_opt);
void SpMV (const SpMatOpt &A, const VecOpt &x, Vec &y);
