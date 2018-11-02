#ifndef tinyjudger_res_H
#define tinyjudger_res_H
#include <cstring>
#include <string>
#include <vector>

enum JudgeResult{
	Accept = 0,
	WrongAnswer,
	RuntimeError,
	TimeLimitExceed,
	MemoryLimitExceed,
	OutputLimitExceed,
	DangerSystemCall,
	JudgementFailed
};

class runLimit{
public:
	int time;		// time in second
	int realTime;	// real time in second
	int memory;		// memory limit in MB
	int output;		// output limit in MB
	int stack;		// stack limit in MB

	runLimit(){
		time = 1;
		realTime = -1;
		memory = 128;
		output = 64;
		stack = 1024;
	}
	
	runLimit(int _time, int _memory, int _output, int _stack, int _realTime=-1)
			: time(_time), realTime(_realTime), memory(_memory), output(_output), stack(_stack)
			{}
};

class RunConfig{
public:
	std::string resultFileName;
	std::string inputFileName;
	std::string outputFileName;
	std::string errorFileName;
	std::string Lang;
	std::string path;
	runLimit lim;
	bool safe;

	std::vector<std::string> argArr;
};

const runLimit defaultLimit(1, 128, 64, 1024);

#endif