all:
	gcc -g -m32 -O2 -std=c11 -o luavm luavm.c eelphant_dummy.c
