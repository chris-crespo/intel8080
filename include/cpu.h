#ifndef CPU_H
#define CPU_H

#include <stdbool.h>
#include "types.h"

struct Registers {
    union {
        u16 bc;
        struct {
            u8 c;
            u8 b;
        };
    };

    union {
        u16 de;
        struct {
            u8 e;
            u8 d;
        };
    };

    union {
        u16 hl;
        struct {
            u8 l;
            u8 h;
        };
    };

    u8 a;
};

struct Flags {
    bool sign      : 1;
    bool zero      : 1;
    bool aux_carry : 1;
    bool parity    : 1;
    bool carry     : 1;
};

struct CPU {
    Registers regs;
    Flags flags;

    union {
        u16 sp;
        struct {
            u8 sp_low;
            u8 sp_hi;
        };
    };

    union {
        u16 pc;
        struct {
            u8 pc_low;
            u8 pc_hi;
        };
    };

    bool interrupts_enabled;
    u8 interrupt_vector;

    u8 *memory;

    void (*in)(CPU *cpu, u8 port); 
    void (*out)(CPU *cpu, u8 port);
};

void cpu_init(CPU *cpu, void (*in)(CPU *cpu, u8 port), void (*out)(CPU *cpu, u8 port));
void cpu_reset(CPU *cpu);
void cpu_execute(CPU *cpu, u8 opcode);

u8 read_byte(CPU *cpu, u16 addr);
void write_byte(CPU *cpu, u16 addr, u8 value);

#endif
