all:pingpong
	gcc pingpong-contab-prio.c pingpong.o queue.o -o pingpong-contab-prio
	gcc pingpong-contab.c pingpong.o queue.o -o pingpong-contab
pingpong:
	gcc -c -Wall pingpong.c
	gcc -c -Wall queue.c