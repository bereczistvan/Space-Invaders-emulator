#ifndef MACHINEINVADERS_H
#define MACHINEINVADERS_H

#include <SDL2/SDL.h>
#include "cpu8080.h"

typedef struct Screen
{
    uint8_t screen_half;
    uint8_t keep_open;
    uint32_t begin;
    uint32_t end;
    double delta;
    SDL_Window *window;
    SDL_Renderer* renderer;
}Screen;

void clearScreen(SDL_Renderer* renderer);

void drawUpper(SDL_Renderer* renderer, State8080* state);

void drawLower(SDL_Renderer* renderer, State8080* state);

void HandleInput(State8080* state, Screen* screen);

Screen* InitScreen(void);

void DeleteScreen(Screen* screen);

void RenderScreen(State8080* state, Screen* screen);

#endif