#include <stdbool.h>
#include <SDL.h>

#include "cpu.h"
#include "memory.h"
#include "screen.h"

#define USAGE "Usage: ./build/emulator [rom].\n"

static bool window_close(SDL_Event event) {
    return event.type == SDL_WINDOWEVENT 
        && event.window.event == SDL_WINDOWEVENT_CLOSE;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, USAGE);
        return 1;
    }

    load_rom(argv[1]);
    screen_init();

    while (1) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (window_close(event))
                screen_quit();
        }

        cpu_tick();
    }
}
