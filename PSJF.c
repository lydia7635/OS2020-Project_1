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

void PSJF(int procNum) {

	initQueue();
	
	sortReady(procNum);


#ifdef DEBUG
	for(int i = 0; i < procNum; i++)
		printf("--%s %d %d\n", proc[i]->name, proc[i]->ready, proc[i]->exec);
#endif
	
	int createdNum = 0;

	for (int time = 0; finishNum < procNum; ++time) {

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

		while (createdNum < procNum && time == proc[createdNum]->ready) {
			createChild(createdNum, LOW_PRIORITY);
			inQueue(createdNum);
			adjustSJF();
			adjustHeadPriority();
			createdNum++;
#ifdef DEBUG
			printQueue();
#endif
		}

		if (!emptyQueue() && runningID != -1
			&& proc[runningID]->exec > proc[queueHead()]->exec) {
			setPriority(proc[runningID]->pid, LOW_PRIORITY);
			inQueue(runningID);

			runningID = deQueue();
			setPriority(proc[runningID]->pid, HIGH_PRIORITY);
			adjustSJF();
			adjustHeadPriority();
		}
		
		if (!emptyQueue() && runningID == -1) {	// we can run new process in CPU
				runningID = deQueue();
				setPriority(proc[runningID]->pid, HIGH_PRIORITY);
				adjustHeadPriority();
#ifdef DEBUG
				fprintf(stderr, "runningpid = %d\n", proc[runningID]->pid);
#endif
		}	

		unitTime();

#ifdef DEBUG
		if (time % 100 == 0) {
			fprintf(stderr, "[%d] time = %d, finish = %d\n", getpid(), time, finishNum);
			printQueue();
		}
#endif
	}

	printInfo(procNum);

	return;
}