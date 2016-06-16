all:
	gcc -g -m32 -O2 -std=c11 -I include -lm -o luavm src/luavm.c src/amethyst_dummy.c src/video_dummy.c
