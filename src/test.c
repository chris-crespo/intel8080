#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"

static bool test_done = 0;

static void load_test(u8 *memory, const char *test) {
    File *file = fopen(test, "rb");
    if (!file) {
        fprintf(stderr, "Could not open %s.\n", test);
        exit(1);
    }

    fread(memory, 1, 0x10000, file);
    fclose(file);
}

static inline void out(CPU *cpu, u8 port) {
    switch (port) {
        case 0: test_done = 1; break;
        case 1: {
            u8 operation = cpu->regs.c;
            if (operation == 9) {
                u16 addr = cpu->regs.de;

                u8 c;
                while ((c = memory_read(cpu, addr++)) != '$') {
                    printf("%c", c);
                }

                printf("\n");
            }

            break;
        }
        default: fprintf(stderr, "Port %d not handled.\n", port); exit(1);
    }
}

static void test(const char *filename) {
    CPU cpu;
    cpu_init(&cpu, 0, out);

    load_test(&cpu.memory[0x100], filename);
    cpu.pc = 0x100;

    cpu.memory[0x0] = 0xd3;
    cpu.memory[0x1] = 0x00;

    cpu.memory[0x5] = 0xd3;
    cpu.memory[0x6] = 0x01;
    cpu.memory[0x7] = 0xc9;

    test_done = 0;
    while (!test_done) {
        cpu_execute(&cpu, 0);
    }
}

int main(void) {
    test("tests/8080PRE.COM"); 
    test("tests/8080EXM.COM");
}
