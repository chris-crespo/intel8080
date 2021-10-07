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

    load_test(cpu.memory, filename);
    cpu.pc = 0x100;

    while (1) {
        cpu_execute(&cpu, 0);
    }
}

int main(void) {
    test("tests/8080PRE.COM"); 
    test("tests/8080EXM.COM");
}
