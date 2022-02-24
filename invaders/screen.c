#include <stdlib.h>
#include <SDL.h>

#include "types.h"
#include "screen.h"

#define RASTER_WIDTH  256
#define RASTER_HEIGHT 224

#define SCREEN_WIDTH  RASTER_HEIGHT
#define SCREEN_HEIGHT RASTER_WIDTH

#define WHITE 0xf0, 0xf0, 0xf0
#define BLACK 0x08, 0x08, 0x08

static SDL_Window   *window;
static SDL_Renderer *renderer;

void screen_init(void) {
    SDL_Init(SDL_INIT_EVENTS | SDL_INIT_TIMER);

    window = SDL_CreateWindow("Space Invaders",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN); 

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);

    screen_clear();
}

void screen_clear(void) {
    SDL_SetRenderDrawColor(renderer, 0x08, 0x08, 0x08, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}

void screen_draw(u8 *memory) {
    /**
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
    */

    /**
     * Memory:
     *                  256 bits (32 bytes)
     * (0,0)---------------------------------------------------(255,0)
     *   |                                                         |        224 bits
     * (0,223)-------------------------------------------------(255,223)
     *
     * Screen:
     *                      224 pixels 
     * (255,0)-----------------------------------------(255,223)
     *     |                                               |                256 pixels
     *   (0,0)-------------------------------------------(0,223)
     */

    for (int row = 0; row < RASTER_WIDTH; row++) {
        for (int col = 0; col < RASTER_HEIGHT ; col++) {
            u8 byte = memory[col * 32 + row];
            for (int k = 0; k < 8; k++) {
                bool pixel = byte & (1 << k);
                if (pixel) 
                    SDL_SetRenderDrawColor(renderer, WHITE, 255);
                else 
                    SDL_SetRenderDrawColor(renderer, BLACK, 255);

                SDL_RenderDrawPoint(renderer, col, SCREEN_HEIGHT - 1 - (row * 8 + k));
            }
        }
    }

    SDL_RenderPresent(renderer);
}

void screen_draw_bottom(u8 *memory) {
    for (int row = 0; row < RASTER_WIDTH / 16; row++) {
        for (int col = 0; col < RASTER_HEIGHT; col++) {
            u8 byte = memory[col * 32 + row];
            for (int k = 0; k < 8; k++) {
                bool pixel = byte & (1 << k);
                if (pixel) 
                    SDL_SetRenderDrawColor(renderer, WHITE, 255);
                else 
                    SDL_SetRenderDrawColor(renderer, BLACK, 255);

                SDL_RenderDrawPoint(renderer, col, SCREEN_HEIGHT - (row * 8 + k));
            }
        }
    }

    SDL_RenderPresent(renderer);
}

void screen_draw_top(u8 *memory) {
    for (int row = RASTER_WIDTH / 16; row < RASTER_WIDTH / 8; row++) {
        for (int col = 0; col < RASTER_HEIGHT; col++) {
            u8 byte = memory[col * 32 + row];
            for (int k = 0; k < 8; k++) {
                bool pixel = byte & (1 << k);
                if (pixel) 
                    SDL_SetRenderDrawColor(renderer, WHITE, 255);
                else 
                    SDL_SetRenderDrawColor(renderer, BLACK, 255);

                SDL_RenderDrawPoint(renderer, col, SCREEN_HEIGHT - (row * 8 + k));
            }
        }
    }

    SDL_RenderPresent(renderer);
}


void screen_quit(void) {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    exit(0);
}
