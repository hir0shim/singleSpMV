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
    }
}



