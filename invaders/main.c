#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"

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

int main(void) {
    CPU cpu;
    cpu_init(&cpu, 0, 0);
    load_rom(cpu.memory);

    while (1) {
        cpu_execute(&cpu);
    }
}
