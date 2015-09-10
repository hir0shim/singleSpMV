#include <iostream>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <omp.h>
/*
#include <ittnotify.h> // VTune Amplifier
*/
#include "opt.h"
#include "util.h"
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
    int loop = 1;
    cerr << "Calculating SpMV ... ";
    {
        double elapsedTime;
        elapsedTime = -GetTimeBySec();
        do {
            for (int i = 0; i < loop; i++) {
                SpMV(A_opt, x_opt, y);
            }
            loop *= 2;
        } while (GetTimeBySec() + elapsedTime < 1.0);
    }

    double minElapsedTime;
    /*
    __itt_domain *domain = __itt_domain_create("MySpMV.Region");
    __itt_string_handle *handle = __itt_string_handle_create("MySpMV.Handle");
    __itt_task_begin(domain, __itt_null, __itt_null, handle);
    */
    {
        const int nTry = 10;
        for (int t = 0; t < nTry; t++) {
            double elapsedTime = -GetTimeBySec();
            for (int i = 0; i < loop; i++) {
                SpMV(A_opt, x_opt, y);
            }
            elapsedTime += GetTimeBySec();
            elapsedTime /= loop;
            minElapsedTime = t ? min(minElapsedTime, elapsedTime) : elapsedTime;
        }
    }
    /*
    __itt_task_end(domain);
    */
    cerr << "done." << endl;

    //    ViewVec(y);

#ifdef VERIFY
    cerr << "Verifying ... ";
    if (!VerifyResult(A, x, y)) {
        printf("*** invalid result ***\n");
    }
    cerr << "done." << endl;
#endif

    printf("++++++++++++++++++++++++++++++++++++++++\n");
    bool isDefinedArch = false;
#ifdef CPU
    printf("%25s\t%s\n", "Architecture", "CPU");
    isDefinedArch = true;
#endif
#ifdef MIC
    printf("%25s\t%s\n", "Architecture", "MIC");
    isDefinedArch = true;
#endif
#ifdef GPU
    printf("%25s\t%s\n", "Architecture", "GPU");
    isDefinedArch = true;
#endif
    assert(isDefinedArch == true);


    bool isDefinedFormat = false;
#ifdef OPT_CRS
    printf("%25s\t%s\n", "MatrixFormat", "CRS");
    isDefinedFormat = true;
#endif
#ifdef OPT_COO
    printf("%25s\t%s\n", "MatrixFormat", "COO");
    isDefinedFormat = true;
#endif
#ifdef OPT_ELL
    printf("%25s\t%s\n", "MatrixFormat", "ELL");
    isDefinedFormat = true;
#endif
#ifdef OPT_JDS
    printf("%25s\t%s\n", "MatrixFormat", "JDS");
    isDefinedFormat = true;
#endif
#ifdef OPT_DIA
    printf("%25s\t%s\n", "MatrixFormat", "DIA");
    isDefinedFormat = true;
#endif
#ifdef OPT_SS
    printf("%25s\t%s\n", "MatrixFormat", "SS");
    isDefinedFormat = true;
#endif
#ifdef OPT_MKL
    printf("%25s\t%s\n", "MatrixFormat", "MKL");
    isDefinedFormat = true;
#endif
#ifdef OPT_CUSPARSE
    printf("%25s\t%s\n", "MatrixFormat", "CUSPARSE");
    isDefinedFormat = true;
#endif
    assert(isDefinedFormat == true);
    printf("%25s\t%s\n", "Matrix", GetBasename(matFile).c_str());
    printf("%25s\t%s\n", "MatrixPath", matFile.c_str());
    printf("%25s\t%lf\n", "Performance(GFLOPS)", nNnz*2/minElapsedTime/1e9);
    printf("%25s\t%d\n", "nRow", nRow);
    printf("%25s\t%d\n", "nCol", nCol);
    printf("%25s\t%d\n", "nNnz", nNnz);
#pragma omp parallel
    {
#pragma omp master
        printf("%25s\t%d\n", "nThread", omp_get_num_threads());
    }
    printf("----------------------------------------\n");
    return 0;
}
