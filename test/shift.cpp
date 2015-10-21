#include <iostream>
using namespace std;
int main () {
    __int64 a = 1<<63;
    cout << a << endl;
    unsigned __int64 b = (unsigned __int64)1<<63;
    a = b;
    cout << a << endl;
}
