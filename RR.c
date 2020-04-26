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

#include "policy.h"

#define QUANTUM 500

void RR(int procNum) {

	initQueue();
	
	sortReady(procNum);


#ifdef DEBUG
	for(int i = 0; i < procNum; i++)
		printf("--%s %d %d\n", proc[i]->name, proc[i]->ready, proc[i]->exec);
#endif
	
	int createdNum = 0;
	int accumQuantum = 0;

	for (int time = 0; finishNum < procNum; ++time) {

		while (createdNum < procNum && time == proc[createdNum]->ready) {
			createChild(createdNum, LOW_PRIORITY);
			inQueue(createdNum);
			adjustHeadPriority();
			createdNum++;
#ifdef DEBUG
			printQueue();
#endif
		}

		if (runningID != -1) {
			--(proc[runningID]->exec);
			if(proc[runningID]->exec <= 0) {
				waitpid(proc[runningID]->pid, NULL, 0);
#ifdef DEBUG
				fprintf(stderr, "wait for %d success!\n", runningID);
#endif
				++finishNum;
				runningID = -1;
			}
		}
		
		if (!emptyQueue() && runningID == -1) {	// we can run new process in CPU
				accumQuantum = 0;
				runningID = deQueue();
				setPriority(proc[runningID]->pid, HIGH_PRIORITY);
				adjustHeadPriority();
#ifdef DEBUG
				fprintf(stderr, "runningpid = %d\n", proc[runningID]->pid);
				printQueue();
#endif
		}

		if (time != 0 && accumQuantum == QUANTUM) {
			accumQuantum = 0;
			if (runningID != -1) {
				setPriority(proc[runningID]->pid, LOW_PRIORITY);
				inQueue(runningID);
			}
			if (!emptyQueue()) {
				runningID = deQueue();
				setPriority(proc[runningID]->pid, HIGH_PRIORITY);
				adjustHeadPriority();
			}
#ifdef DEBUG
			if (runningID != -1)
				fprintf(stderr, "runningpid = %d\n", proc[runningID]->pid);
			printQueue();
#endif
		}

		unitTime();

#ifdef DEBUG
		if (time % 100 == 0) fprintf(stderr, "[%d] time = %d, accumT = %d, finish = %d\n", getpid(), time, accumQuantum, finishNum);
#endif

		if (runningID != -1)
			++accumQuantum;
	}
	// waitProcess(procNum);
	printInfo(procNum);

	return;
}