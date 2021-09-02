#include "cpu.h"
#include "memory.h"
#include "screen.h"

static CPU cpu;

static void not_implemented(u8 opcode) {
    fprintf(stderr, "Opcode 0x%02x not implemented.\n", opcode);
    screen_quit();
}

void cpu_tick(void) {
    u8 opcode = memory_fetch(cpu.pc++);
    switch (opcode) {
        default: {
            not_implemented(opcode);
        }
    }
}
