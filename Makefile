all:
	gcc -std=c99 -g -Wall -Wno-psabi -march=native -lm -lGL -lGLU -lglfw -lpthread -lavformat -lavcodec -lavutil -lswscale -I../3dm/include -o valo *.c ../3dm/src/*.c
