#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

bool was_in_handler = false;

void handler(int, siginfo_t*, void*);

int ssignum = 228;
int pidnum = 1337;

// int signals[] = {
// 	SIGHUP,
// 	SIGINT,
// 	SIGQUIT,
// 	SIGILL,
// 	SIGABRT,
// 	SIGFPE,
// 	SIGSEGV,
// 	SIGPIPE,
// 	SIGALRM,
// 	SIGTERM,
// 	SIGUSR2,
// 	SIGUSR1,
// 	SIGCHLD,
// 	SIGCONT,
// 	SIGTSTP,
// 	SIGTTIN,
// 	SIGTTOU,
// 	SIGBUS,   
// 	SIGPOLL,           
// 	SIGPROF,
// 	SIGSYS,
// 	SIGTRAP,
// 	SIGURG,
// 	SIGVTALRM,
// 	SIGXCPU,
// 	SIGXFSZ,
// 	SIGIOT,
// 	SIGEMT,
// 	SIGSTKFLT,
// 	SIGIO,
// 	SIGCLD,
// 	SIGPWR,
// 	SIGINFO,
// 	SIGLOST,
// 	SIGWINCH,
// 	SIGUNUSED
// };

int main() {
	struct sigaction action;
  	sigset_t mask;
	sigemptyset(&mask);
	int i;
	for (i = 1; i < 31; ++i){
		if (i != SIGKILL && i != SIGSTOP) {
			int res = sigaddset(&mask, i);
			if (res == -1) {
				printf("error while adding signal %d\n", i);
				exit(0);
			}
		}
	}
	action.sa_sigaction = &handler;
	action.sa_mask = mask;
	action.sa_flags = SA_SIGINFO;
	for (i = 1; i <= 31; ++i){
		if (i != SIGKILL && i != SIGSTOP) {
			int res = sigaction (i, &action, NULL);
			if (res == -1) {
				printf("error while setting action for signal %d\n", i);
				exit(0);
			}
		}
	}
  	sleep(10);
  	if (!was_in_handler) {
		printf("No signals were caught\n");
	} else {
		printf("%d from %d\n", ssignum, pidnum);
	}
}

void handler(int signum, siginfo_t* info, void* context) {
	if (!was_in_handler) {
		was_in_handler = true;
		ssignum = info->si_signo;
		pidnum = info->si_pid;
	}
}