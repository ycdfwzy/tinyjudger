#include "tradiJudger.h"

using namespace std;

int main(int argc, char** argv){
	CompileResult cr = runCompiler("/home/ycdfwzy/myworkspace/tinyjudger",
									"/usr/bin/g++", "test.cpp", "-o", "test", NULL);
	if (cr.success){
		cout << "Compile Passed!" << endl;
		RunResult rr = runProgram("/home/ycdfwzy/myworkspace/tinyjudger",
								  "test.txt", "test.in", "test.out", "test.err", "C++",
								  "./test", NULL);
		cout << JudgeResult2string(rr.jr) << endl;
	} else
	{
		cout << cr.info << endl;
	}
	// int e = execute("./executor ./test");
	// cout << "errorcode = " << e << endl;
	return 0;
}