#ifndef POLICY_H
#define POLICY_H
#include <unistd.h>
#include <stdbool.h>

#define HIGH_PRIORITY 90
#define MID_PRIORITY 50
#define LOW_PRIORITY 10

typedef struct {
	char name[32];
	int ready;
	int exec;
	pid_t pid;
} Proc;

int qHead;
int qTail;
int qwait[100];

void swap(Proc **a, Proc **b);
void sortReady(Proc *proc[], int procNum);
void sortExec(Proc *proc[], int procNum);

void setCPU(pid_t pid, int cpu);
void setPriority(pid_t cpid, int priority);
void adjustHeadPriority(Proc *proc[]);

void unitTime();
void child(pid_t pid, int exec);
void createChild(Proc *proc[], int createNum, int priority);
void waitProcess(int procNum);
bool childEnd (int runEnd, int time);

void printInfo(Proc *proc[], int procNum);

void FIFO(Proc *proc[], int procNum);
void SJF(Proc *proc[], int procNum);

void initQueue();
bool emptyQueue();
void inQueue(int procID);
int deQueue();

void adjustSJF (Proc *proc[]);

#endif