#include <iostream>
#include <algorithm>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <cassert>
#include <cmath>
using namespace std;
int main () {
    string line;
    do {
        getline(cin, line);
    } while (line[0] == '%');
    cout << "begin" << endl;
    stringstream ss(line);
    int N, M, L;
    ss >> N >> M >> L;
    assert(N == M);
    vector<int> cntr(M);
    vector<int> cntc(N);
    for (int i = 0; i < L; i++) {
        int r, c;
        double v;
        cin >> r >> c >> v;
        r--; c--;
        cntr[r]++;
        cntc[c]++;
    }
    cout << "maxr " << *max_element(cntr.begin(), cntr.end()) << endl;
    cout << "minr " << *min_element(cntr.begin(), cntr.end()) << endl;
    cout << "maxc " << *max_element(cntc.begin(), cntc.end()) << endl;
    cout << "minc " << *min_element(cntc.begin(), cntc.end()) << endl;

    return 0;
}
