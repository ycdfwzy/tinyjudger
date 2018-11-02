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

using namespace std;

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
	setLimit(RLIMIT_CPU, 3, 3);

	// clearenv();
	// setenv("USER", "poor_program", 1);
	// setenv("LOGNAME", "poor_program", 1);

	if (ptrace(PTRACE_TRACEME, 0, NULL, NULL)==-1){
		printf("ptrace failed in childMainWork\n");
		exit(16);
	}

    char *args[3];
    args[0] = "/home/ycdfwzy/myworkspace/tinyjudger/test";
	args[1] = "test";
	args[2] = (char *)0;
	// char *args[]={"ls","-l","/home",(char *)0};
	printf("before execv\n");
    if (execv(args[0], args)==-1){
    	printf("execv failed in childMainWork\n");
    	exit(17);
    }
    printf("end childMainWork\n");
    // return;
}

pid_t apid; // this is an assist pid for forking a new process
			// which is designed to avoid too long TLE

void parentMainWork(pid_t childpid){
	printf("childpid=%d\n", childpid);
	apid = fork();
	if (apid < 0){
		printf("Error while forking in parentMainWork()\n");
		return;
	} else
	if (apid == 0){
		struct timespec t;
		t.tv_sec = 3;
		t.tv_nsec = 0;
		nanosleep(&t, NULL);
		exit(0);
	} else
	{
		while (true){
			int stat = 0;
			int sig = 0;
			struct rusage ruse;

			pid_t p = wait4(-1, &stat, __WALL, &ruse);
			printf("p=%d\n", p);
			if (p == apid){
				if (WIFEXITED(stat) || WIFSIGNALED(stat)) {
					printf("TLE! line 84\n");
					return;
				}
				continue;
			}

			if (p != childpid){
				printf("p=%d  childpid=%d\n", p, childpid);
				// childpid = p;
			}

			// check TLE
			int usertim = ruse.ru_utime.tv_sec * 1000 + ruse.ru_utime.tv_usec / 1000;
			printf("time=%d\n", usertim);
			if (usertim > 3000){
				printf("TLE! line 98\n");
				return;
			}

			//

			if (WIFEXITED(stat)){
				printf("AC!\n");
				return;
			}
			if (WIFSIGNALED(stat)){
				printf("SIGNALED!\n");
				switch(WTERMSIG(stat)) {
				case SIGXCPU: // nearly impossible
					// stop_all();
					// return RunResult(RS_TLE);
					printf("TLE! line 111\n");
					return;
				case SIGXFSZ:
					printf("OLE!\n");
					return;
					// stop_all();
					// return RunResult(RS_OLE);
				default:

					break;
					// stop_all();
					// return RunResult(RS_RE);
				}

				continue;	
			}
			if (WIFSTOPPED(stat)){
				printf("STOPPED!\n");
				sig = WSTOPSIG(stat);
				printf("signal=%d\n", sig);
				// printf("%d\n", sig == (SIGTRAP | 0x80));
				if (sig == SIGTRAP) {
					switch ((stat >> 16) & 0xffff) {
						case PTRACE_EVENT_CLONE:
						case PTRACE_EVENT_FORK:
						case PTRACE_EVENT_VFORK:
							printf("(stat >> 16) & 0xffff = PTRACE_EVENT_CLONE/FORK/VFORK\n");
							sig = 0;
							break;
						case PTRACE_EVENT_EXEC:
							printf("(stat >> 16) & 0xffff = PTRACE_EVENT_EXEC\n");
							// rp_children[p].mode = 1;
							sig = 0;
							break;
						case 0:
							printf("(stat >> 16) & 0xffff = 0\n");
							break;
						default:
							// stop_all();
							// return RunResult(RS_JGF);
							printf("Nothing\n");
							return;
					}
				}

				switch(sig) {
					case SIGXCPU:
						printf("TLE! line 157\n");
						return;
					case SIGXFSZ:
						printf("OLE!\n");
						return;
					case SIGTRAP:
						sig = 0;
						printf("Trace/breakpoint trap!\n");
						break;
				}

				// continue;
			}

			ptrace(PTRACE_SYSCALL, p, NULL, sig);
		}
	}
}

int main(){
	pid_t pid = fork();
	if (pid < 0){
		printf("Error while forking\n");
	} else
	if (pid == 0){ // this is child process
		printf("This is child process!\n");
		childMainWork();
	} else
	{	// this is parent process
		printf("This is parent process!\n");
		parentMainWork(pid);
	}
	return 0;
}
