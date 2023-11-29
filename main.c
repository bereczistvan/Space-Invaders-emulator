#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <SDL2/sdl.h>

#include "cpu8080.h"


void ReadFileIntoMemoryAt(State8080* state, const char* filename, uint32_t offset)
{
    FILE* f = fopen(filename,"rb");
    if (f==NULL)
    {
        char* msg = "Could not open ";
        char* error_msg = malloc(strlen(msg) + strlen(filename) + 1);
        strcpy(error_msg, msg);
        strcat(error_msg, filename);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", error_msg, NULL);
        free(error_msg);
        exit(1);
    }

    fseek(f, 0L, SEEK_END);
    int fsize = ftell(f);
    fseek(f, 0L, SEEK_SET);

    uint8_t* buffer = &state->memory[offset];
    fread(buffer, fsize, 1, f);
    fclose(f);
}

void clearScreen(SDL_Renderer* renderer)
{
    SDL_SetRenderDrawColor(renderer,0,0,0,255);
    SDL_RenderClear(renderer);
}

void drawUpper(SDL_Renderer* renderer, State8080* state)
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int i = 0; i < 256 / 8; i++)
    {
        for (int j = 0; j < 224 / 2; j++)
        {
            for (int p = 0; p < 8; p++)
            {
                if (((state->memory[0x2400 + i + (j * (256 / 8))] >>  p) & 0x1) == 0x1)
                {
                    SDL_RenderDrawPoint(renderer, j,(256 - (i * 8 + p)));
                }
            }
        }
    }
}

void drawLower(SDL_Renderer* renderer, State8080* state)
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int i = 0; i < 256 / 8; i++)
    {
        for (int j = (224 / 2); j < 224; j++)
        {
            for (int p = 0; p < 8; p++)
            {
                if (((state->memory[0x2400 + i + (j * (256 / 8))] >>  p) & 0x1) == 0x1)
                {
                    SDL_RenderDrawPoint(renderer, j, (256 - (i * 8 + p)));
                }
            }
        }
    }
}

int main(int argc, char* argv[])
{
    State8080* state = Init8080();
    
    ReadFileIntoMemoryAt(state, "invaders.h", 0);
    ReadFileIntoMemoryAt(state, "invaders.g", 0x800);
    ReadFileIntoMemoryAt(state, "invaders.f", 0x1000);
    ReadFileIntoMemoryAt(state, "invaders.e", 0x1800);

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("Failed to initialize SDL\n");
        return -1;
    }

    SDL_Window *window = NULL;
    SDL_Renderer* renderer = NULL;
    SDL_CreateWindowAndRenderer(224, 256, 0, &window, &renderer);

    if (!window)
    {
        printf("Failed to create window\n");
        return -1;
    }

    uint32_t a = 0;
    uint32_t b = 0;
    double delta = 0;
    uint8_t half = 0;
    int keep_window_open = 1;

    while (keep_window_open)
    {
        Emulate8080Op(state);
        SDL_Event e;
        while (SDL_PollEvent(&e) > 0)
        {
            if (e.type == SDL_KEYDOWN)
            {
                switch (e.key.keysym.sym)
                {
                    case SDLK_RSHIFT:
                        state->portsin[1] |= 0b00000100;    // P1 Start
                        break;
                    case SDLK_LSHIFT:
                        state->portsin[1] |= 0b00000010;    // P2 Start
                        break;
                    case SDLK_RETURN:
                        state->portsin[1] |= 0b00000001;    // Insert Coin
                        break;
                    case SDLK_RCTRL:
                        state->portsin[1] |= 0b00010000;    // P1 Shoot
                        break;
                    case SDLK_LEFT:
                        state->portsin[1] |= 0b00100000;    // P1 Left
                        break;
                    case SDLK_RIGHT:
                        state->portsin[1] |= 0b01000000;    // P1 Right
                        break;
                    case SDLK_LCTRL:
                        state->portsin[2] |= 0b00010000;    // P2 Shoot
                        break;
                    case SDLK_a:
                        state->portsin[2] |= 0b00100000;    // P2 Left
                        break;
                    case SDLK_d:
                        state->portsin[2] |= 0b01000000;    // P2 Right
                        break;
                    case SDLK_ESCAPE:
                        keep_window_open = 0;
                        break;
                }
            }
            else if (e.type == SDL_KEYUP)
            {
                switch (e.key.keysym.sym)
                {
                    case SDLK_RSHIFT:
                        state->portsin[1] &= 0b11111011;    // P1 Start
                        break;
                    case SDLK_LSHIFT:
                        state->portsin[1] &= 0b11111101;    // P2 Start
                        break;
                    case SDLK_RETURN:
                        state->portsin[1] &= 0b11111110;    // Insert Coin
                        break;
                    case SDLK_RCTRL:
                        state->portsin[1] &= 0b11101111;    // P1 Shoot
                        break;
                    case SDLK_LEFT:
                        state->portsin[1] &= 0b11011111;    // P1 Left
                        break;
                    case SDLK_RIGHT:
                        state->portsin[1] &= 0b10111111;    // P1 Right
                        break;
                    case SDLK_LCTRL:
                        state->portsin[2] &= 0b11101111;    // P2 Shoot
                        break;
                    case SDLK_a:
                        state->portsin[2] &= 0b11011111;    // P2 Left
                        break;
                    case SDLK_d:
                        state->portsin[2] &= 0b10111111;    // P2 Right
                        break;
                }
            }
            else if (e.type == SDL_QUIT)
            {
                keep_window_open = 0;
            }

        }
        a = SDL_GetTicks();
        delta = a - b;
        if (delta > (1000/60.0))
        {
            if (state->inte)
            {
                if (half == 0)
                {
                    clearScreen(renderer);
                    drawUpper(renderer, state);

                    state->inte = 0;
                    push(state, state->pc);
                    state->pc = 0x08;
                    half = 1;
                }
                else if (half == 1)
                {
                    drawLower(renderer, state);
                    SDL_RenderPresent(renderer);

                    state->inte = 0;
                    push(state, state->pc);
                    state->pc = 0x10;
                    half = 0;
                    b = a;
                }
            }
        }
    }

    return 0;
}
