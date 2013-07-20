clang:
	clang -std=c11 -g -Wall -march=native -mno-avx -fstrict-aliasing -lm -lGL -lGLU -lglfw -lpthread -lavformat -lavcodec -lavutil -lswscale -I../3dm/include -o valo *.c ../3dm/src/*.c

gcc:
	gcc -std=c99 -g -Wall -Wno-psabi -march=native -mno-avx -fstrict-aliasing -lm -lGL -lGLU -lglfw -lpthread -lavformat -lavcodec -lavutil -lswscale -I../3dm/include -o valo *.c ../3dm/src/*.c
