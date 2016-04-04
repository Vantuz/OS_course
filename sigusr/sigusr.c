#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>

bool was_in_handler = false;

void handler(int, siginfo_t*, void*);

int main() {
	
	struct sigaction action;
  	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR1);
	sigaddset(&mask, SIGUSR2);
	action.sa_sigaction = &handler;
	action.sa_mask = mask;
	action.sa_flags = SA_SIGINFO;
  	sigaction (SIGUSR1, &action, NULL);
  	sigaction (SIGUSR2, &action, NULL);
  	sleep(10);
  	if (!was_in_handler) {
		printf("No signals were caught\n");
	}
}

void handler(int signum, siginfo_t* info, void* context) {
	if (!was_in_handler) {
		was_in_handler = true;
		char* signame;
		if (signum == SIGUSR1) {
			signame = "SIGUSR1";
		} else {
			signame = "SIGUSR2";
		}
		printf("%s from %d\n", signame, info->si_pid);
	}
}