#pragma once
#include "util.h"
struct SpMatOpt {
    int nRow;
    int nCol;
    int nNnz;

    int nDiag;
    int *ioff;
    double **diag;
};
struct VecOpt {
    int size;
    double *val;
};
void OptimizeProblem (const SpMat &A, const Vec &x, SpMatOpt &A_opt, VecOpt &x_opt);
extern "C" {
void SpMV (const SpMatOpt &A, const VecOpt &x, Vec &y);
}
