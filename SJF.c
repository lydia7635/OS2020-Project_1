#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>

#include "policy.h"

void SJF(int procNum) {

	initQueue();
	
	sortExec(procNum);
	sortReady(procNum);

	setSighandler(SIGCHLD);

#ifdef DEBUG
	for(int i = 0; i < procNum; i++)
		printf("--%s %d %d\n", proc[i]->name, proc[i]->ready, proc[i]->exec);
#endif
	
	int createdNum = 0;

	for (int time = 0; finishNum < procNum; ++time) {
		while (createdNum < procNum && time == proc[createdNum]->ready) {
			createChild(createdNum, LOW_PRIORITY);
			inQueue(createdNum);
			adjustSJF();
			adjustHeadPriority();
			createdNum++;

			if (runningID == -1) {	// we can run new process in CPU
				runningID = deQueue();
#ifdef DEBUG
				fprintf(stderr, "runningpid = %d\n", proc[runningID]->pid);
#endif
				setPriority(proc[runningID]->pid, HIGH_PRIORITY);
				adjustSJF();
				adjustHeadPriority();
			}
#ifdef DEBUG
			printQueue();
#endif
		}
		
		if (!emptyQueue() && runningID == -1) {	// we can run new process in CPU
				runningID = deQueue();
#ifdef DEBUG
				fprintf(stderr, "runningpid = %d\n", proc[runningID]->pid);
				printQueue();
#endif
				setPriority(proc[runningID]->pid, HIGH_PRIORITY);
				adjustHeadPriority();
		}

		unitTime();
#ifdef DEBUG
		if (time % 100 == 0) fprintf(stderr, "[%d] time = %d, finishNum = %d\n", getpid(), time, finishNum);
#endif
	}
	// waitProcess(procNum);
	printInfo(procNum);

	return;
}