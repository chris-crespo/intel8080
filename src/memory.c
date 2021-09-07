#include <stdlib.h>
#include <string.h>

#include "memory.h"

static u8 memory[0x10000];

static File *open_file(const char *filename) {
    File *file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Could not open %s\n", filename);
        exit(1);
    }

    return file;
}

static void load_part(u16 addr, const char *rom, char part) {
    char filename[strlen(rom) + 3];
    sprintf(filename, "%s.%c", rom, part);

    File *file = open_file(filename);
    fread(&memory[addr], 1, 0x800, file);
    fclose(file);
}

void load_rom(const char *rom) {
    load_part(0x0000, rom, 'h');
    load_part(0x0800, rom, 'g');
    load_part(0x1000, rom, 'f');
    load_part(0x1800, rom, 'e');
}

void load_test(const char *test) {
    File *file = open_file(test);
    fread(&memory[0x100], 1, 0x8000, file);
    fclose(file);

    memory[0x5] = 0xd3;
    memory[0x6] = 0x1;
    memory[0x7] = 0xc9;
}

u8 memory_read(u16 addr) {
    return memory[addr];
}

void memory_write(u16 addr, u8 value) {
    memory[addr] = value; 
}
