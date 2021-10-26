#include <stdbool.h>
#include <SDL.h>

#include "cpu.h"
#include "screen.h"

#define USAGE "Usage: ./build/emulator [rom].\n"

static inline bool window_close(SDL_Event event) {
    return event.type == SDL_WINDOWEVENT 
        && event.window.event == SDL_WINDOWEVENT_CLOSE;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, USAGE);
        return 1;
    }

    screen_init();

    CPU cpu;
    cpu_init(&cpu, 0, 0);

    while (1) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (window_close(event))
                screen_quit();
        }

        cpu_execute(&cpu);
    }
}
