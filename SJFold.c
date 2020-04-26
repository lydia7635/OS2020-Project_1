#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <stdbool.h>

#include "policy.h"

void printQueue(Proc *proc[]) {
	Node *ptr = waitQueue->head;
	while(ptr != NULL) {
		fprintf(stderr, "%s -> ", proc[ptr->id]->name);
		ptr = ptr->next;
	}
	fprintf(stderr, "\n");
	return;
}

void SJF(Proc *proc[], int procNum) {
	sortExec(proc, procNum);
	sortReady(proc, procNum);

#ifdef DEBUG
	for(int i = 0; i < procNum; i++)
		printf("--%s %d %d\n", proc[i]->name, proc[i]->ready, proc[i]->exec);
#endif

	initQueue();
	int runningID = -1;		// no process is running
	int createdNum = 0;
	int finishNum = 0;
	int runEnd = 0;
	int nextReadyTime = proc[0]->ready;

	for (int time = 0; finishNum < procNum; ++time) {
		while (createdNum < procNum && time == nextReadyTime) {
			createChild(proc, createdNum, LOW_PRIORITY);
			inQueue(createdNum);
			createdNum++;
			if(createdNum < procNum)
				nextReadyTime = proc[createdNum]->ready;
			if (runningID == -1) {	// we can run new process in CPU
				runningID = deQueue();
				runEnd = time + proc[runningID]->exec;
				fprintf(stderr, "runEnd = %d, time = %d\n", runEnd, time);
				setPriority(proc[runningID]->pid, HIGH_PRIORITY);
			}
			adjustSJF(proc);
			adjustHeadPriority(proc);
#ifdef DEBUG
			printQueue(proc);
#endif
		}
		
		if (!emptyQueue() && runningID == -1) {	// we can run new process in CPU
				runningID = deQueue();
#ifdef DEBUG
				printQueue(proc);
#endif
				runEnd = time + proc[runningID]->exec;
				fprintf(stderr, "runEnd = %d, time = %d\n", runEnd, time);
				adjustSJF(proc);
				setPriority(proc[runningID]->pid, HIGH_PRIORITY);
				adjustHeadPriority(proc);
		}

		unitTime();
		if(runEnd != 0 && runEnd <= time + 1) {
			waitpid(proc[runningID]->pid, NULL, 0);
			finishNum++;
			runningID = -1;
		}
		if (time % 100 == 0) fprintf(stderr, "time = %d\n", time);
	}

	printInfo(proc, procNum);

	return;
}