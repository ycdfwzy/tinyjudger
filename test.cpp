#include <cstdio>
#include <cstring>
#include <algorithm>

using namespace std;
int a[100000000];

void foo(){
	freopen("a.in", "w", stdout);
	for (int i = 0; i < 100000000; ++i){
		a[i] = i;
		printf("%d\n", a[i]);
	}
	fclose(stdin);
}

int main(){
	printf("before test!\n");
	// for (int i = 0; i < 10; ++i)
	foo();
	printf("This is a test!\n");
	return 0;
}