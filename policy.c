#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <stdbool.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "policy.h"

void swap (Proc **a, Proc **b) {
	Proc *temp = *a;
	*a = *b;
	*b = temp;
	return;
}

void sortReady (Proc *proc[], int procNum) {
	for(int i = procNum - 1; i > 0; --i) {
		for(int j = 0; j < i; ++j) {
			if(proc[j]->ready > proc[j + 1]->ready) {
				swap(&proc[j], &proc[j + 1]);
			}
		}
	}
	return;
}

void sortExec (Proc *proc[], int procNum) {
	for(int i = procNum - 1; i > 0; --i) {
		for(int j = 0; j < i; ++j) {
			if(proc[j]->exec > proc[j + 1]->exec) {
				swap(&proc[j], &proc[j + 1]);
			}
		}
	}
	return;
}

void setCPU (pid_t pid, int cpu) {
	cpu_set_t assignedSET;
	CPU_ZERO(&assignedSET);
	CPU_SET(cpu, &assignedSET);
	sched_setaffinity(pid, sizeof(cpu_set_t), &assignedSET);
	return;
}

void setPriority (pid_t cpid, int priority) {
	struct sched_param param;
	param.sched_priority = priority;
	int work = sched_setscheduler(cpid, SCHED_FIFO, &param);
	sched_getparam(cpid, &param);
	fprintf(stderr, "[%d] sched: %d\n", cpid, param.sched_priority);
	//setpriority(PRIO_PROCESS, cpid, priority);
	return;
}

void adjustHeadPriority (Proc *proc[]) {
	if (!emptyQueue()) {
		setPriority(proc[qwait[qHead]]->pid, MID_PRIORITY);
		if(qHead != qTail)
			setPriority(proc[qwait[qHead + 1]]->pid, LOW_PRIORITY);
	}
	return;
}

void unitTime () {
	volatile unsigned long i;
	for (i = 0; i < 1000000UL; ++i);
	return;
}

void createChild (Proc *proc[], int procID, int priority) {
	pid_t cpid;

	if((cpid = fork()) == 0) {	// child process
		cpid = getpid();
		fprintf(stderr, "[%d] just fork\n", cpid);
		char cpidStr[16], execStr[16];
		sprintf(cpidStr, "%d\n", cpid);
		sprintf(execStr, "%d\n", proc[procID]->exec);

		execl("./child", "./child", cpidStr, execStr, (char *)0);
		exit(0);
	}
	else {	// schedule process
		proc[procID]->pid = cpid;
		setPriority(cpid, priority);
		setCPU(cpid, 1);
	}
	return;
}

void waitProcess (int procNum) {
	for (int i = 0; i < procNum; ++i)
		wait(NULL);
	return;
}

bool childEnd (int runEnd, int time) {
	return (runEnd <= time);
}

void printInfo (Proc *proc[], int procNum) {
	for(int i = 0; i < procNum; ++i)
		printf("%s %d\n", proc[i]->name, proc[i]->pid);
	return;
}

void initQueue () {
	qHead = -1;
	qTail = -1;
	return;
}

bool emptyQueue () {
	return (qHead == -1);
}

void inQueue (int procID) {
	if(qHead == -1) {
		qHead = 0;
		qTail = 0;
		qwait[0] = procID;
	}
	else {
		++qTail;
		qwait[qTail] = procID;
	}
	return;
}

int deQueue () {
	if (qHead == -1)
		return -1;
	int headID = qwait[qHead];

	if(qHead == qTail) {
		qHead = -1;
		qTail = -1;
	}
	else
		++qHead;
	return headID;
}

void adjustSJF (Proc *proc[]) {
	if (!emptyQueue()) {
		// adjust process SJF order in queue

		int ptr = qTail;
		while (ptr != qHead && proc[qwait[ptr]]->exec < proc[qwait[ptr - 1]]->exec) {
			int temp = qwait[ptr];
			qwait[ptr] = qwait[ptr - 1];
			qwait[ptr - 1] = temp;
			--ptr;
		}
	}
	return;
}