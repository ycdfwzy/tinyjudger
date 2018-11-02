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
		exit(55);
	}
	l.rlim_cur = rcur;
	l.rlim_max = rmax;
	if (setrlimit(r, &l) == -1) {
		exit(55);
	}
}

void childMainWork(){
	setLimit(RLIMIT_CPU, runConfig.lim.time, runConfig.lim.realTime);
	// rlimit store space in bytes
	setLimit(RLIMIT_FSIZE, runConfig.lim.output << 20);
	setLimit(RLIMIT_STACK, runConfig.lim.stack << 20);

	// let parent trace this program
	if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1){
		cout << "ptrace error. errno = " << errno << endl;
		exit(16);
	}

	size_t sz = runConfig.argArr.size();
	char** argv = new char*[sz+1];
	for (int i = 0; i < sz; ++i){
		argv[i] = new char[runConfig.argArr[i].length()+1];
		strcpy(argv[i], runConfig.argArr[i].c_str());
		argv[i][runConfig.argArr[i].length()] = '\0';
	}
	argv[sz] = NULL;

	// for (int i = 0; i < sz; ++i){
	// 	cout << string(argv[i]) << endl;
	// }
    
    if (execv(argv[0], argv) == -1){
    	cout << "execv error. errno = " << errno << endl;
    	exit(17);
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

JudgeResult parentMainWork(pid_t childpid){
	cntProcess = 0;
	if (!add_process(childpid)){
		cout << "Error while add_process first." << endl;
		return JudgementFailed;
	}

	cout << "chidpid = " << childpid << endl;
	apid = fork();

	if (apid < 0){
		cout << "Error while forking in parentMainWork. errno = " << errno << endl;
		return JudgementFailed;
	} else
	if (apid == 0){
		/* avoid too long TLE */
		struct timespec t;
		t.tv_sec = runConfig.lim.time;
		t.tv_nsec = 0;
		nanosleep(&t, NULL);
		exit(0);
	} else

	{
		cout << "in father process!" << endl;
		while (true){
			int stat = 0;
			int sig = 0;
			struct rusage ruse;

			pid_t p = wait4(-1, &stat, __WALL, &ruse);
			if (p == apid){
				if (WIFEXITED(stat) || WIFSIGNALED(stat)) {
					cout << "TLE detected by assist process!" << endl;
					return TimeLimitExceed;
				}
				continue;
			}

			int idx = index_process(p);
			if (idx < 0){
				cout << "new process! pid = " << p << endl;
				if (!add_process(p)){
					kill_process();
					cout << "DSC detected by add_process failed!" << endl;
					return DangerSystemCall;
				}
				idx = index_process(p);
			}

			// check TLE
			int usertim = ruse.ru_utime.tv_sec * 1000 + ruse.ru_utime.tv_usec / 1000;
			if (usertim > runConfig.lim.time*1000) {
				kill_process();
				cout << "TLE detected by rusage!" << endl;
				return TimeLimitExceed;
			}
			// check MLE
			int usermem = ruse.ru_maxrss;
			// cout << "Memory = " << usermem << "KB" << endl;
			// cout << "Memory limit = " << runConfig.lim.memory << "MB" << endl;
			if (usermem > runConfig.lim.memory*1024) {
				kill_process();
				cout << "MLE detected by rusage!" << endl;
				return MemoryLimitExceed;
			}


			if (WIFEXITED(stat)){
				cout << "in WIFEXITED" << endl;
				if (mp[idx].mode == NotStart){
					kill_process();
					cout << "JGF detected by mp[idx].mode == NotinProg" << endl;
					return JudgementFailed;
				} else
				{
					if (p == mp[0].pid){	// first process should be child process!
						kill_process();
						cout << "AC detected by child process " << endl;
						return Accept;
					} else
					{
						del_process(p);		// client program exit
						continue;
					}
				}
			}

			if (WIFSIGNALED(stat)){
				cout << "in WIFSIGNALED" << endl;
				if (p == mp[0].pid){
					switch(WTERMSIG(stat)) {
					case SIGXCPU: // nearly impossible
						kill_process();
						cout << "TLE detected by SIGNAL stat == SIGCPU" << endl;
						return TimeLimitExceed;
					case SIGXFSZ:
						kill_process();
						cout << "OLE detected by SIGNAL stat == SIGXFSZ" << endl;
						return OutputLimitExceed;
					default:
						kill_process();
						cout << "RE detected by SIGNAL stat default" << endl;
						return RuntimeError;
					}
				} else
				{
					del_process(p);
					continue;
				}
			}

			if (WIFSTOPPED(stat)){
				cout << "in WIFSTOPPED" << endl;
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
								cout << "ptrace PTRACE_SETOPTIONS for pid=" << p << endl;
								kill_process();
								return JudgementFailed;
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
							cout << "DangerSystemCall detected by check_safe_syscall" << endl;
							return DangerSystemCall;
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
							cout << "sig == trap, but unknown ptrace event!" << endl;
							return JudgementFailed;
					}
				}

				switch(sig) {
					case SIGXCPU:
						cout << "TLE detected by sig == stoped. SIGXCPU" << endl;
						return TimeLimitExceed;
					case SIGXFSZ:
						cout << "OLE detected by sig == stoped. SIGXFSZ" << endl;
						return OutputLimitExceed;
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
	// for (int i = 0; i < runConfig.argArr.size(); ++i)
	// 	cout << runConfig.argArr[i] << endl;

	pid_t pid = fork();
	if (pid < 0){
		cout << "Error while forking" << endl;
	} else
	if (pid == 0){ // this is child process
		cout << "This is child process!" << endl;
		childMainWork();
	} else
	{	// this is parent process
		cout << "This is parent process!" << endl;
		JudgeResult x = parentMainWork(pid);
		cout << x << endl;
	}
	return 0;
}
