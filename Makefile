.PHONY: clean

all: invaders

invaders: main.o
	gcc -o invaders main.o -lmingw32 -lSDL2main -lSDL2 -mwindows

main.o: main.c
	gcc -Wall -c main.c

clean:
	del *.o
