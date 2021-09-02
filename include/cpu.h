#ifndef CPU_H
#define CPU_H

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
    u8 zero      : 1;
    u8 sign      : 1;
    u8 parity    : 1;
    u8 carry     : 1;
    u8 aux_carry : 1;
    u8 pad       : 3;
};

struct CPU {
    Registers regs;
    Flags flags;
    u16 pc;
    u16 sp;

};

#endif
