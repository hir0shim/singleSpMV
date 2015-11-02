#include<iostream>
#include<new>
using namespace std;

int main() {
	char *ch;
	int i;

	try {
		for (i = 0 ; ; i++) {
			ch = new char[1048575];
			cout << i << "MB\r";
		}
	}
	catch(bad_alloc) {
		cout << "BAD ALLOC Exception : " << i  << "MB\n";
		return 1;
	}
	return 0;
}
