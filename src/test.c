#include <stdio.h>
#include "cpu.h"
#include "memory.h"

static void test(const char *filename) {
    load_test(filename);
    cpu_set_pc(0x100);

    while (1) {
        cpu_tick();
    }
}

int main(void) {
    test("tests/8080PRE.COM"); 
}
