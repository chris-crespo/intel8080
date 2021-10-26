#include <stdlib.h>
#include <SDL.h>

#include "types.h"
#include "screen.h"

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 224

static SDL_Window   *window;
static SDL_Renderer *renderer;

void screen_init(void) {
    SDL_Init(SDL_INIT_EVENTS);

    window = SDL_CreateWindow("Space Invaders",
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

#define WHITE 0xf0, 0xf0, 0xf0
#define BLACK 0x08, 0x08, 0x08

void screen_draw(u8 *memory) {
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 224; j++) {
            u8 byte = memory[j * 32 + i];
            for (int k = 0; k < 8; k++) {
                bool pixel = byte & (1 << k);
                if (pixel) 
                    SDL_SetRenderDrawColor(renderer, WHITE, 255);
                else 
                    SDL_SetRenderDrawColor(renderer, BLACK, 255);

                SDL_RenderDrawPoint(renderer, i + k, j);
            }
        }
    }
}

void screen_quit(void) {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    exit(0);
}
