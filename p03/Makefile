all:pingpong
	gcc pingpong-dispatcher.c pingpong.o queue.o -o pingpong-dispatcher
pingpong:
	gcc -c -Wall pingpong.c
	gcc -c -Wall queue.c
