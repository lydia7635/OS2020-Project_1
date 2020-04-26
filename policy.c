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
#include <signal.h>
#include <errno.h>

#include "policy.h"

int runningID = -1;	// no process is running
int finishNum = 0;

void childHandler(int signo) {
	finishNum++;
	runningID = -1;
	wait(NULL);
	return;
}

void setSighandler(int signo) {
	struct sigaction act;
	act.sa_flags = 0;
	act.sa_handler = childHandler;
	sigfillset(&act.sa_mask);
	sigaction(signo, &act, NULL);
	return;
}

void swapProc (int a, int b) {
	Proc *temp = proc[a];
	proc[a] = proc[b];
	proc[b] = temp;
	return;
}

void sortReady (int procNum) {
	for(int i = procNum - 1; i > 0; --i) {
		for(int j = 0; j < i; ++j) {
			if(proc[j]->ready > proc[j + 1]->ready) {
				swapProc(j, j + 1);
			}
		}
	}
	return;
}

void sortExec (int procNum) {
	for(int i = procNum - 1; i > 0; --i) {
		for(int j = 0; j < i; ++j) {
			if(proc[j]->exec > proc[j + 1]->exec) {
				swapProc(j, j + 1);
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
#ifdef DEBUG
	struct sched_param param2;
	sched_getparam(cpid, &param2);
	fprintf(stderr, "[%d] sched: %d\n", cpid, param2.sched_priority);
#endif

	if(priority == LOW_PRIORITY)
		sched_setscheduler(cpid, SCHED_IDLE, NULL);
	//setpriority(PRIO_PROCESS, cpid, priority);
	return;
}

void unitTime () {
	volatile unsigned long i;
	for (i = 0; i < 1000000UL; ++i);
	return;
}

void createChild (int procID, int priority) {
	pid_t cpid = fork();

	if(cpid == 0) {	// child process
		pid_t mypid = getpid();
		setCPU(mypid, 1);
#ifdef DEBUG
		fprintf(stderr, "[%d] just fork\n", mypid);
#endif
		char cpidStr[16], execStr[16];
		sprintf(cpidStr, "%d\n", mypid);
		sprintf(execStr, "%d\n", proc[procID]->exec);

		execl("./child", "./child", cpidStr, execStr, (char *)0);
#ifdef DEBUG
		fprintf(stderr, "[%d] execl failed: %d\n", mypid, errno);
		perror("execl");
#endif
		exit(EXIT_FAILURE);
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

void printInfo (int procNum) {
	for(int i = 0; i < procNum; ++i)
		printf("%s %d\n", proc[i]->name, proc[i]->pid);
	return;
}

void initQueue () {
	waiting = (Queue *)malloc(sizeof(Queue));
	waiting->head = NULL;
	waiting->tail = NULL;
	return;
}

bool emptyQueue () {
	return (waiting->head == NULL);
}

void inQueue (int procID) {
	Node *newNode = (Node *)malloc(sizeof(Node));
	newNode->id = procID;
	newNode->next = NULL;

	if (waiting->head == NULL) {
		waiting->head = newNode;
		waiting->tail = newNode;
		newNode->pre = NULL;
	}
	else {
		newNode->pre = waiting->tail;
		waiting->tail->next = newNode;
		waiting->tail = newNode;
	}
	return;
}

int deQueue () {
	if (emptyQueue())
		return -1;

	Node *oldHead = waiting->head;
	int headID = oldHead->id;

	if(waiting->head == waiting->tail) {
		waiting->head = NULL;
		waiting->tail = NULL;
	}
	else {
		waiting->head = oldHead->next;
		waiting->head->pre = NULL;
	}

	free(oldHead);
	return headID;
}

int queueHead() {
	if (emptyQueue())
		return -1;
	return waiting->head->id;
}

void printQueue() {
	Node *ptr = waiting->head;
	fprintf(stderr, "Queue: ");
	while(ptr != NULL) {
		fprintf(stderr, "%s -> ", proc[ptr->id]->name);
		ptr = ptr->next;
	}
	fprintf(stderr, "\n");
	return;
}

void adjustHeadPriority () {
	if (!emptyQueue()) {
		setPriority(proc[waiting->head->id]->pid, MID_PRIORITY);
		if(waiting->head != waiting->tail)
			setPriority(proc[waiting->head->next->id]->pid, LOW_PRIORITY);
	}
	return;
}

void adjustSJF () {
	if (!emptyQueue()) {
		// adjust process SJF order in queue
		Node *ptr = waiting->tail;
		while (ptr != waiting->head && proc[ptr->id]->exec < proc[ptr->pre->id]->exec) {
			int tmpID = ptr->id;
			ptr->id = ptr->pre->id;
			ptr->pre->id = tmpID;

			ptr = ptr->pre;
		}
	}
	return;
}

pid_t callBarrier () {
	pid_t cpid = fork();

	if(cpid == 0) {	// child process
		pid_t mypid = getpid();
		setCPU(mypid, 1);
		
		execl("./barrier", "./barrier", (char *)0);
#ifdef DEBUG
		fprintf(stderr, "[%d] execl failed: %d\n", mypid, errno);
		perror("execl");
#endif
		exit(EXIT_FAILURE);
	}
	else {	// schedule process
		setPriority(cpid, 20);
		setCPU(cpid, 1);
	}
	return cpid;
}

void removeBarrier (pid_t barrierID) {
	kill(barrierID, SIGKILL);
	waitpid(barrierID, NULL, 0);
	return;
}