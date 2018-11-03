/* This code is to run a program safely.
   Reference: UOJ judger source 
			  https://github.com/vfleaking/uoj/tree/master/judge_client/1
 */
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <algorithm>
#include <cstring>
#include <sys/reg.h>
#include <sys/ptrace.h>
#include <asm/unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/user.h>
#include <string>
#include <iostream>
#include <errno.h>
#include "parseArgs.h"
#include "configs.h"

using namespace std;

RunConfig runConfig;

void setLimit(int r, int rcur, int rmax = -1)  {
	if (rmax == -1)
		rmax = rcur;
	struct rlimit l;
	if (getrlimit(r, &l) == -1) {
		exit(getrlimitError);
	}
	l.rlim_cur = rcur;
	l.rlim_max = rmax;
	if (setrlimit(r, &l) == -1) {
		exit(setrlimitError);
	}
}

void childMainWork(){
	// cout << "in child Main Work" << endl;
	printf("in child Main Work\n");
	setLimit(RLIMIT_CPU, runConfig.lim.time, runConfig.lim.realTime);
	// rlimit store space in bytes
	setLimit(RLIMIT_FSIZE, runConfig.lim.output << 20);
	setLimit(RLIMIT_STACK, runConfig.lim.stack << 20);

	// char path[128];
	// if (getcwd(path, 128) != NULL);
	// 	printf("pwd=%s\n", path);

	printf("inputFileName=%s\n", runConfig.inputFileName.c_str());
	// cout << "inputFileName=" << runConfig.inputFileName << endl;
	if (runConfig.inputFileName != "stdin"){
		if (freopen(runConfig.inputFileName.c_str(), "r", stdin) == NULL){
			// cout << "error when open inputFileName" << endl;
			printf("error when open inputFileName\n");
			exit(openinputfileError);
		}
	}

	// cout << "outputFileName=" << runConfig.outputFileName << endl;
	printf("outputFileName=%s\n", runConfig.outputFileName.c_str());
	if (runConfig.outputFileName != "stdout" && runConfig.outputFileName != "stderr") {
		if (freopen(runConfig.outputFileName.c_str(), "w", stdout) == NULL){
			// cout << "error when open outputFileName" << endl;
			printf("error when open outputFileName\n");
			exit(openoutputfileError);
		}
	}

	// cout << "errorFileName=" << runConfig.errorFileName << endl;
	printf("errorFileName=%s\n", runConfig.errorFileName.c_str());
	if (runConfig.errorFileName != "stderr") {
		if (runConfig.errorFileName == "stdout"){
			if (dup2(STDOUT_FILENO, STDERR_FILENO) == -1){
				// cout << "error when dup stdout to stderr" << endl;
				printf("error when dup stdout to stderr\n");
				exit(dup2Error);
			}
		} else
		{
			if (freopen(runConfig.errorFileName.c_str(), "w", stderr) == NULL) {
				// cout << "error when open inputFileName" << endl;
				printf("error when open inputFileName\n");
				exit(openerrorfileError);
			}
		}

		if (runConfig.outputFileName == "stderr") {
			if (dup2(STDERR_FILENO, STDOUT_FILENO) == -1){
				// cout << "error when dup stderr to stdout" << endl;
				printf("error when dup stderr to stdout\n");
				exit(dup2Error);
			}
		}
	}


	// let parent trace this program
	if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1){
		// cout << "ptrace error. errno = " << errno << endl;
		printf("ptrace error. errno = %d", errno);
		exit(tracemeError);
	}

	size_t sz = runConfig.argArr.size();
	char** argv = new char*[sz+1];
	for (int i = 0; i < sz; ++i){
		argv[i] = new char[runConfig.argArr[i].length()+1];
		strcpy(argv[i], runConfig.argArr[i].c_str());
		argv[i][runConfig.argArr[i].length()] = '\0';
	}
	argv[sz] = NULL;

	for (int i = 0; i < sz; ++i){
		// cout << string(argv[i]) << endl;
		printf("%s ", argv[i]);
	}
	printf("\n");

	// if (getcwd(path, 128) != NULL);
	// 	printf("pwd=%s\n", path);
    
    if (execv(argv[0], argv) == -1){
    	// cout << "execv error. errno = " << errno << endl;
    	printf("execv error. errno = %d", errno);
    	exit(execvError);
    }
}

pid_t apid; // this is an assist pid for forking a new process
			// which is designed to avoid too long TLE

enum RunMode {
	NotStart = 0,
	Running,
	RunOtherProcess
};
const int MaxProcess = 16;
int cntProcess = 0;
class mProcess{
public:
	pid_t pid;
	RunMode mode;
	mProcess(){
		pid = -1;
		mode = NotStart;
	}
	mProcess(pid_t _pid, RunMode _mode = NotStart):
		pid(_pid), mode(_mode){}
} mp[MaxProcess];

bool add_process(pid_t p){
	if (cntProcess >= MaxProcess)
		return false;
	mp[cntProcess].pid = p;
	mp[cntProcess++].mode = NotStart;
	return true;
}

bool del_process(pid_t p){
	for (int i = 0; i < cntProcess; ++i)
	if (mp[i].pid == p){
		for (int j = i+1; j < cntProcess; ++j)
			mp[j-1] = mp[j];
		--cntProcess;
		return true;
	}
	return false;
}

int index_process(pid_t p){
	for (int i = 0; i < cntProcess; ++i)
	if (mp[i].pid == p){
		return i;
	}
	return -1;
}

void kill_process(pid_t p = -1){
	if (p == -1){
		kill(apid, SIGKILL);
		for (int i = 0; i < cntProcess; ++i){
			kill(mp[i].pid, SIGKILL);
		}
	} else
	{
		kill(p, SIGKILL);
	}
}

bool check_safe_syscall(pid_t){
	return true;
}

void on_syscall_exit(pid_t){
	return;
}

RunResult parentMainWork(pid_t childpid){
	cntProcess = 0;
	if (!add_process(childpid)){
		// cout << "Error while add_process first." << endl;
		printf("Error while add_process first.\n");
		return RunResult(JudgementFailed);
	}

	// cout << "childpid = " << childpid << endl;
	printf("childpid=%d\n", childpid);
	apid = fork();

	if (apid < 0){
		printf("Error while forking in parentMainWork. errno = %d\n", errno);
		// cout << "Error while forking in parentMainWork. errno = " << errno << endl;
		return RunResult(JudgementFailed);
	} else
	if (apid == 0){
		/* avoid too long TLE */
		struct timespec t;
		t.tv_sec = runConfig.lim.time;
		t.tv_nsec = 0;
		nanosleep(&t, NULL);
		exit(NoError);
	} else

	{
		// cout << "in father process!" << endl;
		printf("in father process!\n");
		while (true){
			int stat = 0;
			int sig = 0;
			struct rusage ruse;

			pid_t p = wait4(-1, &stat, __WALL, &ruse);
			if (p == apid){
				if (WIFEXITED(stat) || WIFSIGNALED(stat)) {
					// cout << "TLE detected by assist process!" << endl;
					printf("TLE detected by assist process!\n");
					return RunResult(TimeLimitExceed);
				}
				continue;
			}

			int idx = index_process(p);
			if (idx < 0){
				// cout << "new process! pid = " << p << endl;
				printf("new process! pid = %d\n", p);
				if (!add_process(p)){
					kill_process();
					printf("DSC detected by add_process failed!\n");
					// cout << "DSC detected by add_process failed!" << endl;
					return RunResult(DangerSystemCall);
				}
				idx = index_process(p);
			}

			int usertim = ruse.ru_utime.tv_sec * 1000 + ruse.ru_utime.tv_usec / 1000;
			int usermem = ruse.ru_maxrss;
			// check TLE
			if (usertim > runConfig.lim.time*1000) {
				kill_process();
				printf("TLE detected by rusage!\n");
				// cout << "TLE detected by rusage!" << endl;
				return RunResult(TimeLimitExceed, usertim, usermem);
			}
			// check MLE
			// cout << "Memory = " << usermem << "KB" << endl;
			// cout << "Memory limit = " << runConfig.lim.memory << "MB" << endl;
			if (usermem > runConfig.lim.memory*1024) {
				kill_process();
				// cout << "MLE detected by rusage!" << endl;
				printf("MLE detected by rusage!\n");
				return RunResult(MemoryLimitExceed, usertim, usermem);
			}


			if (WIFEXITED(stat)){
				// cout << "in WIFEXITED" << endl;
				// printf("in WIFEXITED\n");
				if (mp[idx].mode == NotStart){
					kill_process();
					// cout << "JGF detected by mp[idx].mode == NotStart" << endl;
					printf("JGF detected by mp[idx].mode == NotStart\n");
					return RunResult(JudgementFailed, usertim, usermem, WEXITSTATUS(stat));
				} else
				{
					if (idx == 0){	// first process should be child process!
						kill_process();
						// cout << "AC detected by child process " << endl;
						printf("AC detected by child process\n");
						return RunResult(Accept, usertim, usermem, WEXITSTATUS(stat));
					} else
					{
						del_process(p);		// client program exit
						continue;
					}
				}
			}

			if (WIFSIGNALED(stat)){
				// cout << "in WIFSIGNALED" << endl;
				// printf("in WIFSIGNALED\n");
				if (p == mp[0].pid){
					switch(WTERMSIG(stat)) {
					case SIGXCPU: // nearly impossible
						kill_process();
						// cout << "TLE detected by SIGNAL stat == SIGCPU" << endl;
						printf("TLE detected by SIGNAL stat == SIGCPU\n");
						return RunResult(TimeLimitExceed, usertim, usermem);
					case SIGXFSZ:
						kill_process();
						// cout << "OLE detected by SIGNAL stat == SIGXFSZ" << endl;
						printf("OLE detected by SIGNAL stat == SIGXFSZ\n");
						return RunResult(OutputLimitExceed, usertim, usermem);
					default:
						kill_process();
						// cout << "RE detected by SIGNAL stat default" << endl;
						printf("RE detected by SIGNAL stat default\n");
						return RunResult(RuntimeError, usertim, usermem);
					}
				} else
				{
					del_process(p);
					continue;
				}
			}

			if (WIFSTOPPED(stat)){
				// cout << "in WIFSTOPPED" << endl;
				// printf("in WIFSTOPPED\n");
				sig = WSTOPSIG(stat);

				if (mp[idx].mode == NotStart){
					if ((idx == 0 && sig == SIGTRAP) || (idx != 0 && sig == SIGSTOP)){
						if (idx == 0){
							// PTRACE_O_EXITKILL: Send a SIGKILL signal to the tracee
							//                    if the tracer exits.
							// PTRACE_O_TRACESYSGOOD: When delivering system call traps,
							//					  set bit 7 in the signal number(SIGTRAP | 0x80)
							int ptrace_data = PTRACE_O_EXITKILL | PTRACE_O_TRACESYSGOOD;
							if (runConfig.safe){
								// Stop the tracee at the next clone(), fork(), vfork(), execve()
								// when wait return, status will be set as
								//    status>>8 == (SIGTRAP | (PTRACE_EVENT_CLONE<<8))
								//    status>>8 == (SIGTRAP | (PTRACE_EVENT_FORK<<8))
								//    status>>8 == (SIGTRAP | (PTRACE_EVENT_VFORK<<8))
								//    status>>8 == (SIGTRAP | (PTRACE_EVENT_EXEC<<8))
								ptrace_data |= PTRACE_O_TRACECLONE | PTRACE_O_TRACEFORK | PTRACE_O_TRACEVFORK |PTRACE_O_TRACEEXEC;
							}
							if (ptrace(PTRACE_SETOPTIONS, p, NULL, ptrace_data) == -1){
								// cout << "ptrace PTRACE_SETOPTIONS for pid=" << p << endl;
								printf("ptrace PTRACE_SETOPTIONS for pid=%d\n", p);
								kill_process();
								return RunResult(JudgementFailed, usertim, usermem);
							}
						}
						sig = 0;
					}
					mp[idx].mode = Running;
				} else

				// system call traps
				if (sig == (SIGTRAP | 0x80)){
					if (mp[idx].mode == Running){
						// check safe syscall
						if (runConfig.safe && !check_safe_syscall(p)){
							kill_process();
							// cout << "DangerSystemCall detected by check_safe_syscall" << endl;
							printf("DangerSystemCall detected by check_safe_syscall\n");
							return RunResult(DangerSystemCall);
						}
						mp[idx].mode = RunOtherProcess;
					} else
					{
						if (runConfig.safe){
							on_syscall_exit(p);
						}
						mp[idx].mode = Running;
					}
					sig = 0;
				} else

				if (sig == SIGTRAP) {
					switch ((stat >> 16) & 0xffff) {
						case PTRACE_EVENT_CLONE:
						case PTRACE_EVENT_FORK:
						case PTRACE_EVENT_VFORK:
							sig = 0;
							break;
						case PTRACE_EVENT_EXEC:
							mp[idx].mode = RunOtherProcess;
							sig = 0;
							break;
						case 0:
							break;
						default:
							kill_process();
							// cout << "sig == trap, but unknown ptrace event!" << endl;
							printf("sig == trap, but unknown ptrace event!\n");
							return RunResult(JudgementFailed);
					}
				}

				switch(sig) {
					case SIGXCPU:
						// cout << "TLE detected by sig == stoped. SIGXCPU" << endl;
						printf("TLE detected by sig == stoped. SIGXCPU\n");
						return RunResult(TimeLimitExceed);
					case SIGXFSZ:
						// cout << "OLE detected by sig == stoped. SIGXFSZ" << endl;
						printf("OLE detected by sig == stoped. SIGXFSZ\n");
						return RunResult(OutputLimitExceed);
				}
			}
			// Restart the stopped tracee as for PTRACE_CONT, but arrange for
            // the tracee to be stopped at the next entry to or exit from a
            // system call, or after execution of a single instruction,
            // respectively
			ptrace(PTRACE_SYSCALL, p, NULL, sig);
		}
	}
}

int main(int argc, char **argv){
	parse_args(argc, argv, runConfig);

	pid_t pid = fork();
	if (pid < 0){
		// cout << "Error while forking" << endl;
		printf("Error while forking\n");
	} else
	if (pid == 0){ // this is child process
		// cout << "This is child process!" << endl;
		printf("This is child process!\n");
		childMainWork();
	} else
	{	// this is parent process
		// cout << "This is parent process!" << endl;
		printf("This is parent process!\n");
		RunResult x = parentMainWork(pid);
		x.dump(runConfig.resultFileName.c_str());
		// printf("time=%d\n", x.time);
		// printf("mem=%d\n", x.memory);
		// printf("JudgeResult=%d\n", x.jr);
		// printf("ErrorCode=%d\n", x.ec);
	}
	return 0;
}
