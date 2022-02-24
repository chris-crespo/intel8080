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
            break; // Wathdog
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
    if (bottom) {
        screen_draw_bottom(&cpu->memory[0x2400]);
        cpu->interrupt_vector = 0xcf;
    }
    else {
        screen_draw_top(&cpu->memory[0x2400]);
        cpu->interrupt_vector = 0xd7;
    }

    bottom = !bottom;
    return interval;
}

int main(void) {
    CPU cpu;

    cpu_init(&cpu, in, out);
    load_rom(cpu.memory);

    screen_init();
    keyboard_init();

    SDL_AddTimer(8, issue_vector, &cpu);

    while (1) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (window_close(event))
                screen_quit();
        }

        if (cpu.interrupts_enabled && cpu.interrupt_vector) {
            cpu_execute(&cpu, cpu.interrupt_vector);
            cpu.interrupt_vector = 0;
        }
        else {
            u8 opcode = read_byte(&cpu, cpu.pc++);
            cpu_execute(&cpu, opcode);
        }
    }
}
