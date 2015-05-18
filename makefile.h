ALL: a2
SHELL = /bin/sh
DIRS =
a2: a2.c
	mpicc -o a2 a2.c