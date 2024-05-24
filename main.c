#include <stdio.h>
#include <stdlib.h>

#include "cpu8080.h"
#include "machine.h"


int main(int argc, char* argv[])
{
    State8080* state = Init8080();
    Screen* screen = InitScreen();

    ReadFileIntoMemoryAt(state, "invaders.h", 0);
    ReadFileIntoMemoryAt(state, "invaders.g", 0x800);
    ReadFileIntoMemoryAt(state, "invaders.f", 0x1000);
    ReadFileIntoMemoryAt(state, "invaders.e", 0x1800);

    while (screen->keep_open)
    {
        Emulate8080Op(state);
        
        HandleInput(state, screen);
        
        RenderScreen(state, screen);
    }

    return 0;
}
