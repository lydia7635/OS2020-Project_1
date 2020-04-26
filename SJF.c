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

void printQueue(Proc *proc[]) {
	int ptr = qHead;
	while(ptr != -1 && ptr != qTail + 1) {
		fprintf(stderr, "%s -> ", proc[qwait[ptr]]->name);
		++ptr;
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

	for (int time = 0; finishNum < procNum; ++time) {
		while (createdNum < procNum && time == proc[createdNum]->ready) {
			createChild(proc, createdNum, LOW_PRIORITY);
			inQueue(createdNum);
			adjustSJF(proc);
			adjustHeadPriority(proc);
			createdNum++;

			if (runningID == -1) {	// we can run new process in CPU
				runningID = deQueue();
				runEnd = time + proc[runningID]->exec;
				fprintf(stderr, "runningpid = %d\n", proc[runningID]->pid);
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
				fprintf(stderr, "runningpid = %d\n", proc[runningID]->pid);
				setPriority(proc[runningID]->pid, HIGH_PRIORITY);
				adjustHeadPriority(proc);
		}

		unitTime();
		if(runEnd != 0 && runEnd <= time) {
			finishNum++;
			runningID = -1;
		}
		if (time % 100 == 0) fprintf(stderr, "[%d] time = %d\n",getpid(), time);
	}
	waitProcess(procNum);
	printInfo(proc, procNum);

	return;
}