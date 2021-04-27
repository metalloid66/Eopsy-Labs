#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <signal.h>

#ifndef NUM_CHILD
	#define NUM_CHILD 5
#endif

#ifdef SIGNAL_USE
	void signalHandler(int, void (*)(int), struct sigaction* old);
	void interruptHandler(int);
	void terminationHandler(int);
#endif

pid_t pids[NUM_CHILD];
volatile sig_atomic_t aborted = false;

int main() {
	#ifdef SIGNAL_USE
		// Ignoring all signals 
		struct sigaction prevActions[NSIG];
		for (int signal = 1; signal < NSIG; signal++)
			signalHandler(signal, SIG_IGN, &prevActions[signal - 1]);
		
		// Restoring default child signal handler, set own keyboard interrupt signal handler
		signalHandler(SIGCHLD, SIG_DFL, NULL);
		signalHandler(SIGINT, interruptHandler, NULL);
	#endif
	
	// Start forking
	for (int i = 0; i < NUM_CHILD; i++) {
		// Fork and get pid of child
		pid_t pid = fork();
		pids[i] = pid;
		
        // Handling child processes
		if (pid == 0) {
				#ifdef SIGNAL_USE
		// Disabling keyboard interrupt, enabling custom termination handler
		signalHandler(SIGINT, SIG_IGN, NULL);
		signalHandler(SIGTERM, terminationHandler, NULL);
	#endif

	printf("child[%d]: I AM A NEW CHILD, AND MY DAD IS %d\n", getpid(), getppid());
	sleep(10);
	printf("child[%d]: Child gone\n", getpid());
			return 0;
		}
		
		// Handle fork faliure
		if (pid == -1) {
			fprintf(stderr, "parent[%d]: child #%d failed, aborting\n", getpid(), i);
			for (int j = 0; j < i; j++)
				kill(pids[j], SIGTERM);
			return 1;
		}
		
		// Delay Between Forks
		if (i != NUM_CHILD - 1)
			sleep(1);
		
		#ifdef SIGNAL_USE
			// keyborad interrupt
			if (aborted) {
				printf("Aborting due to keyboard interrupt\n");
				for (int j = 0; j <= i; j++)
					kill(pids[j], SIGTERM);
				break;
			}
		#endif
    }

	// Successful children creation
	if (!aborted)
		printf("All child processes have been successfully created\n");
        
	// Wait for children to terminate
	int termCounter = 0;
	while (1) {
		//continue on interrupts, break when finished
		if (wait(NULL) == -1) {
			if (errno == EINTR) continue;
			if (errno == ECHILD) break;
		}
		termCounter++;
	}
	// Succsseful termination
	printf("parent[%d]: %d child processes terminated\n", getpid(), termCounter);
	
	#ifdef SIGNAL_USE
		//Restore old handlers
		for (int signal = 1; signal < NSIG; signal++)
			sigaction(signal, &prevActions[signal - 1], NULL);
	#endif
}


#ifdef SIGNAL_USE
	void signalHandler(int signal, void (*handler)(int), struct sigaction* old) {
		struct sigaction action;
		action.sa_handler = handler;
		action.sa_flags = 0;
		sigemptyset(&action.sa_mask);
		sigaction(signal, &action, old);
	}
	
	void interruptHandler(int signal) {
		//print message and set abort flag
		fprintf(stderr, "parent[%d]: keyboard interrupt received\n", getpid());
		aborted = true;
	}
	void terminationHandler(int signal) {
		//print message and exit
		fprintf(stderr, "child[%d]: child terminated\n", getpid());
		exit(0);
	}
#endif
