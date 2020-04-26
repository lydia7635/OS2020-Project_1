#ifndef POLICY_H
#define POLICY_H
#include <unistd.h>
#include <stdbool.h>

#define HIGH_PRIORITY 90
#define MID_PRIORITY 50
#define LOW_PRIORITY 10

#define MAX_PROC 10000

typedef struct {
	char name[32];
	int ready;
	int exec;
	pid_t pid;
} Proc;

typedef struct node {
	int id;
	struct node *pre;
	struct node *next;
} Node;

typedef struct {
	Node *head;
	Node *tail;
} Queue;

Queue *waiting;
Proc *proc[MAX_PROC];

extern int runningID;	// no process is running
extern int finishNum;

void childHandler(int signo);
void setSighandler(int signo);

void swapProc(int a, int b);
void sortReady(int procNum);
void sortExec(int procNum);

void setCPU(pid_t pid, int cpu);
void setPriority(pid_t cpid, int priority);

void unitTime();
void child(pid_t pid, int exec);
void createChild(int createNum, int priority);
void waitProcess(int procNum);
bool childEnd (int runEnd, int time);

void printInfo(int procNum);

void FIFO(int procNum);
void SJF(int procNum);

void initQueue();
bool emptyQueue();
void inQueue(int procID);
int deQueue();
void printQueue();

void adjustHeadPriority();
void adjustSJF ();

#endif