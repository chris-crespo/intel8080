#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>

#include "cpu.h"
#include "screen.h"
#include "input.h"
#include "shift.h"

static File *open_rom(void) {
    File *file = fopen("roms/invaders/invaders", "rb");
    if (!file) {
        fprintf(stderr, "Could not open rom.\n");
        exit(1);
    }

    return file;
}

static void load_rom(u8 *memory) {
    File *rom = open_rom();
    fread(memory, sizeof(u8), 0x2000, rom);
    fclose(rom);
}

static void in(CPU *cpu, u8 port) {
    switch (port) {
        case 1: {
            cpu->regs.a = port1();
            break;
        }
        case 2: {
            cpu->regs.a = 0x0;
            break;
        }
        case 3: {
            cpu->regs.a = shift_read();
            break;
        }
        default: {
            fprintf(stderr, "Input port %d not handled.\n", port);
            exit(1);
        }
    }
}

static void out(CPU *cpu, u8 port) {
    switch (port) {
        case 2: {
            shift_offset(cpu->regs.a & 0x07);
            break;
        }
        case 3: {
            break;
        }
        case 4: {
            shift_write(cpu->regs.a);
            break;
        }
        case 5: {
            break;
        }
        case 6: {
            break; // Watchdog
        }
        default: {
            fprintf(stderr, "Output port %d not handled.\n", port);
            exit(1);
        }
    }
}

static inline bool window_close(SDL_Event event) {
    return event.type == SDL_WINDOWEVENT
        && event.window.event == SDL_WINDOWEVENT_CLOSE;
}

static void run_until_screen_interrupt(CPU *cpu, u8 interrupt) {
    // Mid/End of Screen interrupt happens every half frame.
    while (cpu->cycles < 16667) // (2MHz / 60s) / 2
        cpu_step(cpu);

    cpu_execute(cpu, interrupt);
    cpu->cycles = 0;
}

static void run_frame(CPU *cpu) {
    u32 start = SDL_GetTicks();

    run_until_screen_interrupt(cpu, 0xcf);
    run_until_screen_interrupt(cpu, 0xd7);
    screen_draw(&cpu->memory[0x2400]);

    while (SDL_GetTicks() - start < 17);
}

int main(void) {
    CPU cpu;

    cpu_init(&cpu, in, out);
    load_rom(cpu.memory);

    screen_init();
    keyboard_init();

    while (1) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (window_close(event))
                screen_quit();
        }

        run_frame(&cpu);
    }
}
