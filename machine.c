#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include "cpu8080.h"
#include "machine.h"

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

Screen* InitScreen(void)
{
    Screen* screen = calloc(1, sizeof(Screen));

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("Failed to initialize SDL\n");
        exit(-1);
    }

    screen->window = NULL;
    screen->renderer = NULL;
    SDL_CreateWindowAndRenderer(448, 512, 0, &screen->window, &screen->renderer);
    SDL_RenderSetLogicalSize(screen->renderer, 224, 256);

    if (!screen->window)
    {
        printf("Failed to create window\n");
        exit(-1);
    }

    screen->keep_open = 1;

    SDL_ShowCursor(SDL_DISABLE);
    return screen;
}

void DeleteScreen(Screen* screen)
{
    SDL_DestroyRenderer(screen->renderer);
    SDL_DestroyWindow(screen->window);
    free(screen);
    SDL_Quit();
}

void RenderScreen(State8080* state, Screen* screen)
{
    screen->begin = SDL_GetTicks();
    screen->delta = screen->begin - screen->end;
    if (screen->delta > (1000/60.0))
    {
        if (state->inte)
        {
            if (screen->screen_half == 0)
            {
                clearScreen(screen->renderer);
                drawUpper(screen->renderer, state);
                state->inte = 0;
                push(state, state->pc);
                state->pc = 0x08;
                screen->screen_half = 1;
            }
            else if (screen->screen_half == 1)
            {
                drawLower(screen->renderer, state);
                SDL_RenderPresent(screen->renderer);
                state->inte = 0;
                push(state, state->pc);
                state->pc = 0x10;
                screen->screen_half = 0;
                screen->end = screen->begin;
            }
        }
    }
}

void HandleInput(State8080* state, Screen* screen)
{
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
                    screen->keep_open = 0;
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
            screen->keep_open = 0;
        }
    }
}