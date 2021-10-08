#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"

static void load_test(u8 *memory, const char *test) {
    File *file = fopen(test, "rb");
    if (!file) {
        fprintf(stderr, "Could not open %s.\n", test);
        exit(1);
    }

    fread(memory, 1, 0x10000, file);
    fclose(file);
}

static void test(const char *filename) {
    CPU cpu;
    cpu_init(&cpu, 0, 0);

    load_test(&cpu.memory[0x100], filename);
    cpu.pc = 0x100;

    cpu.memory[0x0] = 0xd3;
    cpu.memory[0x1] = 0x00;

    cpu.memory[0x5] = 0xd3;
    cpu.memory[0x6] = 0x01;
    cpu.memory[0x7] = 0xc9;

    while (1) {
        cpu_execute(&cpu, 0);
    }
}

int main(void) {
    test("tests/8080PRE.COM"); 
    test("tests/8080EXM.COM");
}
