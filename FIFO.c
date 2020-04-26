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

void FIFO(int procNum) {

	sortReady(procNum);
#ifdef DEBUG
	for(int i = 0; i < procNum; i++)
		printf("--%s %d %d\n", proc[i]->name, proc[i]->ready, proc[i]->exec);
#endif
	
	int createdNum = 0;

	for (int time = 0; createdNum < procNum; ++time) {
		while (createdNum < procNum && time == proc[createdNum]->ready) {
			createChild(createdNum, 99 - createdNum);
			++createdNum;
		}
		unitTime();
#ifdef DEBUG
		if (time % 100 == 0)
			fprintf(stderr, "[%d] time = %d\n", getpid(), time);
#endif
	}
	waitProcess(procNum);

	printInfo(procNum);

	return;
}