#include <stdlib.h>
#include <SDL.h>
#include "input.h"

static const u8 *keyboard;


void keyboard_init(void) {
    keyboard = SDL_GetKeyboardState(0);

}

u8 port1() {
    SDL_PumpEvents();
    return keyboard[SDL_SCANCODE_RETURN] /* Coin insert */
        | ((u8)keyboard[SDL_SCANCODE_2]) << 1 /* 2p start */
        | (u8)keyboard[SDL_SCANCODE_1] << 2 /* 1p start */
        | 1 << 3 /* Always 1 */
        | (u8)keyboard[SDL_SCANCODE_SPACE] << 4 /* 1p shot */
        | (u8)keyboard[SDL_SCANCODE_A] << 5 /* 1p left */
        | (u8)keyboard[SDL_SCANCODE_D] << 6; /* 1p right */
        
}
