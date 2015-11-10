#include <iostream>
#include <omp.h>
using namespace std;
int main () {
#pragma omp parallel
    {
#pragma omp master
        cout << omp_get_num_threads() << endl;
    }
    /*
    int a = 0;
#pragma omp parallel 
    {
        for (int i = 0; i < 2; i++) {
#pragma omp single
            {
            a++;
            cout << a << endl;
            }
#pragma omp for
            for (int j = 0; j < 2; j++) {
                cout << "a" << endl;
                //cout << "2 " << omp_get_num_threads() << endl;
            }
        }
    }
    */
}   
