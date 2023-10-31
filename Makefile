.PHONY: clean

all: invaders

invaders: main.o cpu8080.o
	gcc -o invaders main.o cpu8080.o -lmingw32 -lSDL2main -lSDL2 -mwindows

main.o: main.c
	gcc -Wall -c main.c

cpu8080.o: cpu8080.c
	gcc -Wall -c cpu8080.c

clean:
	del *.o
