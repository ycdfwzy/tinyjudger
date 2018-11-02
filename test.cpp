#include <cstdio>
#include <cstring>
#include <algorithm>

using namespace std;

void foo(){
	for (int i = 0; i < 100000000; ++i);
}

int main(){
	printf("before test!\n");
	for (int i = 0; i < 10; ++i)
		foo();
	printf("This is a test!\n");
	return 0;
}