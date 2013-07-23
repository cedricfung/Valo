C_FLAGS=-g -Wall -march=native -mno-avx
C_INCLUDES=-Iinclude -I../3dm/include
LDLIBS=-lm -lGL -lGLU -lglfw -lpthread -lavformat -lavcodec -lavutil -lswscale
SOURCES=../3dm/src/*.c src/*.c valo.c

clang:
	clang -std=c11 $(C_FLAGS) $(C_INCLUDES) $(LDLIBS) -o valo $(SOURCES)

gcc:
	gcc -std=c99 -Wno-psabi $(C_FLAGS) $(C_INCLUDES) $(LDLIBS) -o valo $(SOURCES)
