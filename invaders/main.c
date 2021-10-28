#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>

#include "cpu.h"
#include "disassembler.h"
#include "screen.h"

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

static void in(CPU *cpu) {
    fprintf(stderr, "Out not implemented.\n");
}

static void out(CPU *cpu, u8 port) {
    switch (port) {
        case 3: {
            printf("Looks like this plays some sounds.\n");
            printf("Input byte? %02x\n", cpu->regs.c);
            break;
        }
        case 5: {
            printf("More sounds?\n");
            printf("Is this the byte? %02x\n", cpu->regs.a);
            break;
        }
        case 6: {
            printf("What's this for?\n");
            printf("Input byte? %02x\n", cpu->regs.a);
            break;
        }
        default: {
            fprintf(stderr, "Port %d not handled.\n", port);
            exit(1);
        }
    }
}

static inline bool window_close(SDL_Event event) {
    return event.type == SDL_WINDOWEVENT
        && event.window.event == SDL_WINDOWEVENT_CLOSE;
}

static u32 last_tick = 0;

int main(void) {
    CPU cpu;

    cpu_init(&cpu, in, out);
    load_rom(cpu.memory);

    screen_init();

    while (1) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (window_close(event))
                screen_quit();
        }

        if (SDL_GetTicks() - last_tick > 10) {
            screen_draw(&cpu.memory[0x2400]);
            last_tick = SDL_GetTicks();
        }

        u8 opcode = read_byte(&cpu, cpu.pc++);
        disassemble_opcode(&cpu, opcode);
        cpu_execute(&cpu, opcode);
    }
}
