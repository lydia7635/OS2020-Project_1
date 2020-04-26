#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "policy.h"

int main() {
	char policy[8];
	int procNum;

	scanf("%s", policy);
	scanf("%d", &procNum);

	for (int i = 0; i < procNum; i++) {
		proc[i] = (Proc *)malloc(sizeof(Proc));
		scanf("%s%d%d", proc[i]->name, &(proc[i]->ready), &(proc[i]->exec));
	}

	setCPU(0, 0);
	setPriority(getpid(), 90);

	pid_t barrierPid = callBarrier();

	if (strncmp(policy, "FIFO", 4) == 0)
		FIFO(procNum);
	else if (strncmp(policy, "PSJF", 4) == 0)
		PSJF(procNum);
	else if (strncmp(policy, "RR", 2) == 0)
		RR(procNum);
	else
		SJF(procNum);

	removeBarrier(barrierPid);

	return 0;
}
