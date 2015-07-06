#pragma once
#include <string>
using namespace std;

// COO foramt
struct SpMat {
    int nRow, nCol, nNnz;
    int *row_idx;
    int *col_idx;
    double *val;
    /*
    ~SpMat () {
        delete [] row_idx;
        delete [] col_idx;
        delete [] val;
    }
    */
};
struct Vec {
    int size;
    double *val;
    /*
    ~Vec () {
        delete [] val;
    }
    */
};
void LoadSparseMatrix (SpMat &A, const string &matFile); 
double GetTimeBySec ();
bool VerifyResult (const SpMat &A, const Vec &x, const Vec &y);
string GetBasename (const string &path);
Vec CreateRandomVector (int size);
void ViewVec (const Vec &v);
