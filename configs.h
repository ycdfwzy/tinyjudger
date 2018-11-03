#ifndef tinyjudger_configs_H
#define tinyjudger_configs_H
#include <cstring>
#include <string>
#include <iostream>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

enum ErrorCode{
	NoError = 0,
	getrlimitError,
	setrlimitError,
	openinputfileError,
	openoutputfileError,
	openerrorfileError,
	dup2Error,
	tracemeError,
	execvError,
};

enum JudgeResult{
	Accept = 0,
	WrongAnswer,
	RuntimeError,
	TimeLimitExceed,
	MemoryLimitExceed,
	OutputLimitExceed,
	DangerSystemCall,
	JudgementFailed,
	CompileError
};

std::string JudgeResult2string(JudgeResult jr);

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
const runLimit compileLimit(15, 512, 64, 1024);
const runLimit runningLimit(1, 256,64, 1024);

class RunResult{
public:
	JudgeResult jr;
	int time;	// ms
	int memory;	// kb
	ErrorCode ec;

	RunResult(JudgeResult _jr, int _time = -1, int _memory = -1, int _ec = NoError)
				: jr(_jr), time(_time), memory(_memory){
					ec = (ErrorCode)_ec;
				}

	static RunResult fail_execute(){
		return RunResult(JudgementFailed);
	}

	static RunResult load(const char* file){
		RunResult ret(JudgementFailed);
		FILE* fd = fopen(file, "r");
		int judgeresult, errorcode;
		if ( fd == NULL || fscanf(fd, "%d %d %d %d",
				&judgeresult, &ret.time, &ret.memory, &errorcode) < 4){
			if (fd != NULL)
				fclose(fd);
			return RunResult(JudgementFailed);
		}
		ret.jr = (JudgeResult)judgeresult;
		ret.ec = (ErrorCode)errorcode;
		fclose(fd);
		return ret;
	}

	bool dump(const char* file){
		FILE* fd = fopen(file, "w");
		if ( fd == NULL){
			return false;
		}
		fprintf(fd, "%d %d %d %d\n", this->jr, this->time, this->memory, this->ec);
		fclose(fd);
		return true;
	}
};

class CompileResult{
public:
	JudgeResult jr;
	int time;	// ms
	int memory;	// kb
	bool success;
	std::string info;
	
	CompileResult(JudgeResult _jr, int _time = -1, int _memory = -1, bool _success=false)
					: jr(_jr), time(_time), memory(_memory), success(_success){
						this->info = "";
					}
	void getInfo(const char* file){
		char buf[512];
		int fd = open(file, O_RDONLY);
		if (fd < 0){
			std::cout << "open file failed when getInfo" << std::endl;
			return;
		}
		this->info = "";
		ssize_t len = read(fd, buf, 512);
		for (int i = 0 ; i < 500 && i < len; ++i)
			this->info += buf[i];
		if (len > 500){
			this->info += std::string("...");
		}
		close(fd);
	}
};

#endif