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
static SDL_Surface  *surface;

void screen_init(void) {
    SDL_Init(SDL_INIT_EVENTS | SDL_INIT_TIMER);

    window = SDL_CreateWindow("Space Invaders",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN); 

    surface  = SDL_GetWindowSurface(window);
}

void screen_draw(u8 *memory) {
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

    // TODO: Cleanup code
    for (int row = 0; row < RASTER_HEIGHT; row++) {
        for (int col = 0; col < RASTER_WIDTH / 8; col++) {
            u8 byte = memory[row * 32 + col];
            for (int k = 0; k < 8; k++) {
                bool pixel = byte & (1 << k);
                SDL_LockSurface(surface);
                int x = row;
                int y = SCREEN_HEIGHT - (col * 8 + k) - 1;
                u8 *pixels = (u8 *)surface->pixels;
                u32 *drawing_pixel = (u32 *)(pixels + y * surface->pitch + x * 4);
                if (pixel)
                    *drawing_pixel = SDL_MapRGB(surface->format, WHITE);
                else
                    *drawing_pixel = SDL_MapRGB(surface->format, BLACK);
                SDL_UnlockSurface(surface);
            }
        }
    }

    SDL_UpdateWindowSurface(window);
}

void screen_draw_bottom(u8 *memory) {
    for (int row = 0; row < RASTER_WIDTH / 16; row++) {
        for (int col = 0; col < RASTER_HEIGHT; col++) {
            u8 byte = memory[col * 32 + row];
            for (int k = 0; k < 8; k++) {
                bool pixel = byte & (1 << k);

                SDL_LockSurface(surface);
                int x = col;
                int y = SCREEN_HEIGHT - (row * 8 + k) - 1;
                u8 *pixels = (u8 *)surface->pixels;
                u32 *drawing_pixel = (u32 *)(pixels + y * surface->pitch + x * 4);
                if (pixel)
                    *drawing_pixel = SDL_MapRGB(surface->format, WHITE);
                else
                    *drawing_pixel = SDL_MapRGB(surface->format, BLACK);
                SDL_UnlockSurface(surface);
            }
        }
    }
    SDL_UpdateWindowSurface(window);
}

void screen_draw_top(u8 *memory) {
    for (int row = RASTER_WIDTH / 16; row < RASTER_WIDTH / 8; row++) {
        for (int col = 0; col < RASTER_HEIGHT; col++) {
            u8 byte = memory[col * 32 + row];
            for (int k = 0; k < 8; k++) {
                bool pixel = byte & (1 << k);

                SDL_LockSurface(surface);
                int x = col;
                int y = SCREEN_HEIGHT - (row * 8 + k) - 1;
                u8 *pixels = (u8 *)surface->pixels;
                u32 *drawing_pixel = (u32 *) (pixels + y * surface->pitch + x * 4);
                if (pixel)
                    *drawing_pixel = SDL_MapRGB(surface->format, WHITE);
                else
                    *drawing_pixel = SDL_MapRGB(surface->format, BLACK);
                SDL_UnlockSurface(surface);
            }
        }
    }

    SDL_UpdateWindowSurface(window);
}

void screen_quit(void) {
    SDL_DestroyWindow(window);
    SDL_Quit();

    exit(0);
}
