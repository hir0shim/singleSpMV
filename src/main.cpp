#include <iostream>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <omp.h>
#include "opt_crs.h"
#include "util.h"
#define NUMBER_OF_SPMV 1000
using namespace std;
int main (int argc, char **argv) {
    srand(3);
    if (argc < 2) {
        printf("Usage: %s <matrix>\n", argv[0]);
        exit(1);
    }
    string matFile = argv[1];
    SpMat A;
    cerr << "Loading sparse matrix " << matFile << " ... ";
    LoadSparseMatrix(A, matFile);
    cerr << "done." << endl;
    int nRow = A.nRow;
    int nCol = A.nCol;
    int nNnz = A.nNnz;
    Vec x = CreateRandomVector(nCol);
    Vec y = CreateRandomVector(nRow);
    SpMatOpt A_opt;
    VecOpt x_opt;
    cerr << "Optimizing ... ";
    OptimizeProblem(A, x, A_opt, x_opt);
    cerr << "done." << endl;
    double elapsedTime = -GetTimeBySec();
    cerr << "Calculating SpMV ... ";
    for (int i = 0; i < NUMBER_OF_SPMV; i++) {
        SpMV(A_opt, x_opt, y);
    }
    cerr << "done." << endl;
    elapsedTime += GetTimeBySec();
    elapsedTime /= NUMBER_OF_SPMV;

//    ViewVec(y);

    cerr << "Verifying ... ";
    if (!VerifyResult(A, x, y)) {
        printf("*** invalid result ***\n");
        return 0;
    }
    cerr << "done." << endl;
    bool isDefinedFormat = false;
#ifdef OPT_CRS
    printf("%25s\t%s\n", "Matrix format", "CRS");
    isDefinedFormat = true;
#endif
#ifdef OPT_COO
    printf("%25s\t%s\n", "Matrix format", "COO");
    isDefinedFormat = true;
#endif
#ifdef OPT_MKL
    printf("%25s\t%s\n", "Matrix format", "MKL");
    isDefinedFormat = true;
#endif
    assert(isDefinedFormat == true);
    printf("%25s\t%s\n", "Matrix", GetBasename(matFile).c_str());
    printf("%25s\t%s\n", "Matrix Path", matFile.c_str());
    printf("%25s\t%lf\n", "Performance (GFLOPS)", nNnz*2/elapsedTime/1e9);
    printf("%25s\t%d\n", "nRow", nRow);
    printf("%25s\t%d\n", "nCol", nCol);
    printf("%25s\t%d\n", "nNnz", nNnz);
    /*
#pragma omp parallel
    {
#pragma omp master
        printf("%25s\t%d\n", "nThread", omp_get_num_threads());
    }
    */
    return 0;
}
