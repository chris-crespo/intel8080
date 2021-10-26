#include <stdlib.h>
#include <stdbool.h>
#include <SDL.h>

#include "memory.h"
#include "screen.h"
#include "types.h"

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

static inline void set_color(u8 color) {
    SDL_SetRenderDrawColor(renderer, color, color, color, 255);
}

void screen_clear(void) {
    set_color(0x08);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}

void screen_draw(void) {
    for (int i = 0; i < SCREEN_HEIGHT; i++) {
        for (int j = 0; j < SCREEN_WIDTH / 8; j++) {
            u8 byte = memory_read(0x2400 + i * SCREEN_WIDTH / 8 + j);

            for (int k = 0; k < 8; k++) {
                bool pixel = byte & (0x80 >> k);
                set_color(pixel ? 0xf0 : 0x08);
                SDL_RenderDrawPoint(renderer, j * 8 + k, i);
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
