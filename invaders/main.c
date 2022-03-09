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

u32 issue_vector(u32 interval, void *param) {
    static bool bottom = false;

    CPU *cpu = (CPU *)param; 

    cpu->cycles = 0;
    u64 count = 0;
    while (count < 33333) { // 2MHz / 60s 
        u64 cycles = cpu->cycles;

        if (cpu->interrupts_enabled && cpu->interrupt_vector) {
            cpu_execute(cpu, cpu->interrupt_vector);
            cpu->interrupt_vector = 0;
        }
        else {
            u8 opcode = read_byte(cpu, cpu->pc++);
            cpu_execute(cpu, opcode);
        }

        count += cpu->cycles - cycles; 

        if (cpu->cycles >= 33344 / 2) { 
            // TODO: Current approach causes the 0xcf interrupt to be issued twice
            // before the other.
            cpu->cycles = 0;
            cpu->interrupt_vector = 0xcf;
        }
    }

    cpu->interrupt_vector = 0xd7;
    screen_draw(&cpu->memory[0x2400]);

    return interval;
}

int main(void) {
    CPU cpu;

    cpu_init(&cpu, in, out);
    load_rom(cpu.memory);

    screen_init();
    keyboard_init();

    SDL_TimerID timer_id = SDL_AddTimer(16, issue_vector, &cpu);

    while (1) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (window_close(event)) {
                SDL_RemoveTimer(timer_id);
                screen_quit();
            }
        }

    }
}
