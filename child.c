#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <sys/syscall.h>

void unitTime() {
	volatile unsigned long i;
	for (i = 0; i < 1000000UL; ++i);
	return;
}

void main(int argc, char *argv[]) {
	pid_t pid = atoi(argv[1]);
	int exec = atoi(argv[2]);

#ifdef DEBUG
	fprintf(stderr, ">>>>> start pid = %d\n", pid);
	struct sched_param param;
	sched_getparam(pid, &param);
	fprintf(stderr, "[%d] child! sched: %d\n", pid, param.sched_priority);
#endif

	struct timespec start;
	struct timespec end;
	syscall(334, &start);
	for(int i = 0; i < exec; ++i)
		unitTime();
	syscall(334, &end);
#ifdef DEBUG
	fprintf(stderr, "<<<<< end pid = %d\n", pid);
#endif
	syscall(333, pid, &start, &end);
	exit(0);
}