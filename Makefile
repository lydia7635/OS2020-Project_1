all: main.c policy.c
	gcc main.c policy.c FIFO.c SJF.c RR.c PSJF.c -o main
	gcc child.c -o child
	gcc barrier.c -o barrier
debug: main.c policy.c
	gcc -DDEBUG main.c policy.c FIFO.c SJF.c RR.c PSJF.c -o main
	gcc -DDEBUG child.c -o child
	gcc -DDEBUG barrier.c -o barrier