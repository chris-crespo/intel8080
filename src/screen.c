#include <stdlib.h>
#include <SDL.h>
#include "screen.h"

#define SCALE 1
#define SCREEN_WIDTH  256 * SCALE
#define SCREEN_HEIGHT 224 * SCALE 

static SDL_Window   *window; 
static SDL_Renderer *renderer;

void screen_init(void) {
    SDL_Init(SDL_INIT_EVENTS);

    window = SDL_CreateWindow("Intel8080 Emulator", 
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    screen_clear();
}

void screen_clear(void) {
    SDL_SetRenderDrawColor(renderer, 0x08, 0x08, 0x08, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}

void screen_quit(void) {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    exit(0);
}
