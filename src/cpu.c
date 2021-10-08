#include <stdlib.h>
#include "cpu.h"

void cpu_init(CPU *cpu, void (*in)(CPU*), void (*out)(CPU*, u8)) {
    cpu->regs.a  = 0;
    cpu->regs.bc = 0;
    cpu->regs.de = 0;
    cpu->regs.hl = 0;
    
    cpu->pc = 0;
    cpu->sp = 0;

    cpu->interrupts_enabled = 0;

    cpu->memory = calloc(0x10000, sizeof(u8));
    if (!cpu->memory) {
        fprintf(stderr, "CPU: Out of memory.\n");
        exit(1);
    }

    cpu->in  = in;
    cpu->out = out;
}

void cpu_reset(CPU *cpu) {
    void (*in)(CPU*) = cpu->in;
    void (*out)(CPU*, u8) = cpu->out;

    free(cpu->memory);
    cpu->memory = 0;

    cpu_init(cpu, in, out);
}

static void not_implemented(u8 opcode) {
    fprintf(stderr, "Opcode 0x%02x not implemented.\n", opcode);
    exit(1);
}

static void print_state(CPU *cpu, u8 opcode) {
    printf("PC %04x  ", cpu->pc);
    printf("Opcode %02x  ", opcode);
    printf("SP %04x  ", cpu->sp);
    printf("BC %04x  ", cpu->regs.bc);
    printf("DE %04x  ", cpu->regs.de);
    printf("HL %04x  ", cpu->regs.hl);
    printf("A %02x  ", cpu->regs.a);
    printf("ZSPC %d%d%d%d\n", cpu->flags.zero, cpu->flags.sign, cpu->flags.parity, cpu->flags.carry);
}

static inline u8 memory_read(CPU *cpu, u16 addr) {
    return cpu->memory[addr];
}

static inline void memory_write(CPU *cpu, u16 addr, u8 value) {
    cpu->memory[addr] = value;
}

static inline u8 next_byte(CPU *cpu) {
    return memory_read(cpu, cpu->pc++);
}

static inline u16 next_word(CPU *cpu) {
    return next_byte(cpu) | ((u16)next_byte(cpu) << 8);
}

static u8 parity(u8 value) {
    int bits = 0;

    while (value) {
        bits += value & 0x1;
        value >>= 1;
    }

    return (bits & 0x1) == 0;
}

static inline void set_zsp(CPU *cpu, u8 value) {
    cpu->flags.zero   = (value == 0);
    cpu->flags.sign   = (value & 0x80) >> 7;
    cpu->flags.parity = parity(value);
}

static inline void set_carry(CPU *cpu, u8 value) {
    cpu->flags.carry = value;
}

static inline void set_aux(CPU *cpu, u8 value) {
    cpu->flags.aux_carry = value;
}

static inline u8 inr(CPU *cpu, u8 reg) {
    u8 value = reg + 1;

    set_zsp(cpu, value);
    set_aux(cpu, (value & 0xf) == 0);

    return value;
}

static inline u8 dcr(CPU *cpu, u8 reg) {
    u8 value = reg - 1;

    set_zsp(cpu, value);
    set_aux(cpu, (value & 0xf) == 0xf);

    return value;
}

static inline void dad(CPU *cpu, u16 value) {
    cpu->regs.hl += value;
    set_carry(cpu, cpu->regs.hl < value);
}

static inline void rrc(CPU *cpu) {
    cpu->flags.carry = cpu->regs.a & 0x1;
    cpu->regs.a = (cpu->regs.a >> 1) | (cpu->flags.carry << 7);
}

static inline void ana(CPU *cpu, u8 value) {
    cpu->regs.a &= value;
    set_carry(cpu, 0);
    set_zsp(cpu, cpu->regs.a);
}

static inline void xra(CPU *cpu, u8 value) {
    cpu->regs.a ^= value;
    set_zsp(cpu, cpu->regs.a);
    set_carry(cpu, 0);
    set_aux(cpu, 0);
}

static inline void push(CPU *cpu, u16 value) {
    memory_write(cpu, --cpu->sp, value >> 8);
    memory_write(cpu, --cpu->sp, value & 0xff);
}

static inline u16 pop(CPU *cpu) {
    u8 low  = memory_read(cpu, cpu->sp++);
    u16 high = memory_read(cpu, cpu->sp++);

    return low | (high << 8);
}

static inline void push_psw(CPU *cpu) {
    u16 af = cpu->regs.a << 8; 

    af |= cpu->flags.sign << 7;
    af |= cpu->flags.zero << 6;
    af |= cpu->flags.aux_carry << 4;
    af |= cpu->flags.parity << 2;
    af |= 0x1 << 1;
    af |= cpu->flags.carry;

    push(cpu, af);
}

static inline void pop_psw(CPU *cpu) {
    u16 af = pop(cpu);

    cpu->regs.a = af >> 8;
    u8 psw = af & 0xff;

    cpu->flags.sign      = psw >> 7;
    cpu->flags.zero      = psw >> 6 & 0x1;
    cpu->flags.aux_carry = psw >> 4 & 0x1;
    cpu->flags.parity    = psw >> 2 & 0x1;
    cpu->flags.carry     = psw & 0x1;
}

static inline void jmp(CPU *cpu, u8 condition) {
    u16 addr = next_word(cpu);
    if (condition)
        cpu->pc = addr;
}

static inline void add(CPU *cpu, u8 *reg, u8 value) {
    *reg += value;

    set_zsp(cpu, *reg);
    set_carry(cpu, *reg < value);
    set_aux(cpu, (*reg & 0xf) < (value & 0xf));
}

static inline void ret(CPU *cpu, u8 cond) {
    if (cond) {
        cpu->pc = pop(cpu);
    }
}

static inline void call(CPU *cpu, u8 cond) {
    u16 addr = next_word(cpu);
    if (cond) {
        push(cpu, cpu->pc);
        cpu->pc = addr;
    }
}
/**
static inline void out(void) {
    u8 port = next_byte(cpu);
    switch (port) {
        case 0: screen_quit();
        case 1: {
            u8 operation = cpu->regs.c;
            if (operation == 9) {
                u16 addr = cpu->regs.de;

                u8 c;
                while ((c = memory_read(addr++)) != '$') {
                    printf("%c", c);
                }

                printf("\n");
                screen_quit();
            }

            break;
        }
        default: fprintf(stderr, "Port %d not handled.\n", port); break;
    }
}
*/

static inline void xchg(CPU *cpu) {
    u16 temp = cpu->regs.de; 
    cpu->regs.de = cpu->regs.hl;
    cpu->regs.hl = temp;
}

static inline void cmp(CPU *cpu, u8 value) {
    u16 comp = cpu->regs.a - value;
    set_zsp(cpu, comp);
    set_carry(cpu, comp >> 8);
}

static inline void rst(CPU *cpu, u8 addr) {
    push(cpu, cpu->pc);
    cpu->pc = addr;
}

void cpu_execute(CPU *cpu, u8 opcode) {
    // TODO: Change this
    opcode = next_byte(cpu);
    switch (opcode) {
        // NOP
        case 0x00: break;

        // LXI
        //case 0x01: cpu->regs.bc = next_word(cpu); break;
        case 0x11: cpu->regs.de = next_word(cpu); break;
        case 0x21: cpu->regs.hl = next_word(cpu); break;
        case 0x31: cpu->sp = next_word(cpu); break;

        // INX
        //case 0x03: cpu->regs.bc++; break;
        //case 0x13: cpu->regs.de++; break;
        case 0x23: cpu->regs.hl++; break;
        //case 0x33: cpu->sp++; break;
        
        // INR
        //case 0x14: cpu->regs.d = inr(cpu, cpu->regs.d); break;
        case 0x3c: cpu->regs.a = inr(cpu, cpu->regs.a); break;

        // DCR
        case 0x05: cpu->regs.b = dcr(cpu, cpu->regs.b); break;
        //case 0x0d: cpu->regs.c = dcr(cpu, cpu->regs.c); break;
        //case 0x15: cpu->regs.d = dcr(cpu, cpu->regs.d); break;
        //case 0x1d: cpu->regs.e = dcr(cpu, cpu->regs.e); break;
        //case 0x25: cpu->regs.h = dcr(cpu, cpu->regs.h); break;
        //case 0x2d: cpu->regs.l = dcr(cpu, cpu->regs.l); break;
        //case 0x35: memory_write(cpu, cpu->regs.hl, dcr(cpu, memory_read(cpu, cpu->regs.hl))); break;
        //case 0x3d: cpu->regs.a = dcr(cpu, cpu->regs.a); break; 

        // MVI
        case 0x3e: cpu->regs.a = next_byte(cpu); break;
        case 0x06: cpu->regs.b = next_byte(cpu); break;
        case 0x0e: cpu->regs.c = next_byte(cpu); break;
        //case 0x26: cpu->regs.h = next_byte(cpu); break;
        //case 0x36: memory_write(cpu, cpu->regs.hl, next_byte(cpu)); break;

        // DAD
        //case 0x09: dad(cpu, cpu->regs.bc); break;
        //case 0x19: dad(cpu, cpu->regs.de); break;
        //case 0x29: dad(cpu, cpu->regs.hl); break;

        // LDAX
        //case 0x1a: cpu->regs.a = memory_read(cpu, cpu->regs.de); break;

        // RRC
        case 0x0f: rrc(cpu); break;

        // STA
        //case 0x32: memory_write(cpu, next_word(cpu), cpu->regs.a); break;
                   
        // LDA
        case 0x3a: cpu->regs.a = memory_read(cpu, next_word(cpu)); break;

        // MOV
        //case 0x7f: cpu->regs.a = cpu->regs.a; break;
        case 0x78: cpu->regs.a = cpu->regs.b; break;
        case 0x79: cpu->regs.a = cpu->regs.c; break;
        //case 0x7a: cpu->regs.a = cpu->regs.d; break;
        //case 0x7b: cpu->regs.a = cpu->regs.e; break;
        case 0x7c: cpu->regs.a = cpu->regs.h; break;
        case 0x7d: cpu->regs.a = cpu->regs.l; break;
        case 0x7e: cpu->regs.a = memory_read(cpu, cpu->regs.hl); break;
        //case 0x47: cpu->regs.b = cpu->regs.a; break;
        //case 0x40: cpu->regs.b = cpu->regs.b; break;
        //case 0x41: cpu->regs.b = cpu->regs.c; break;
        //case 0x42: cpu->regs.b = cpu->regs.d; break;
        //case 0x43: cpu->regs.b = cpu->regs.e; break;
        //case 0x44: cpu->regs.b = cpu->regs.h; break;
        //case 0x45: cpu->regs.b = cpu->regs.l; break;
        //case 0x46: cpu->regs.b = memory_read(cpu, cpu->regs.hl); break;
        //case 0x4f: cpu->regs.c = cpu->regs.a; break;
        //case 0x48: cpu->regs.c = cpu->regs.b; break;
        //case 0x49: cpu->regs.c = cpu->regs.c; break;
        //case 0x4a: cpu->regs.c = cpu->regs.d; break;
        //case 0x4b: cpu->regs.c = cpu->regs.e; break;
        //case 0x4c: cpu->regs.c = cpu->regs.h; break;
        //case 0x4d: cpu->regs.c = cpu->regs.l; break;
        //case 0x4e: cpu->regs.c = memory_read(cpu, cpu->regs.hl); break;
        //case 0x57: cpu->regs.d = cpu->regs.a; break;
        //case 0x50: cpu->regs.d = cpu->regs.b; break;
        //case 0x51: cpu->regs.d = cpu->regs.c; break;
        //case 0x52: cpu->regs.d = cpu->regs.d; break;
        //case 0x53: cpu->regs.d = cpu->regs.e; break;
        //case 0x54: cpu->regs.d = cpu->regs.h; break;
        //case 0x55: cpu->regs.d = cpu->regs.l; break;
        //case 0x56: cpu->regs.d = memory_read(cpu, cpu->regs.hl); break;
        //case 0x5f: cpu->regs.e = cpu->regs.a; break;
        //case 0x58: cpu->regs.e = cpu->regs.b; break;
        //case 0x59: cpu->regs.e = cpu->regs.c; break;
        //case 0x5a: cpu->regs.e = cpu->regs.d; break;
        //case 0x5b: cpu->regs.e = cpu->regs.e; break;
        //case 0x5c: cpu->regs.e = cpu->regs.h; break;
        //case 0x5d: cpu->regs.e = cpu->regs.l; break;
        //case 0x5e: cpu->regs.e = memory_read(cpu, cpu->regs.hl); break;
        //case 0x67: cpu->regs.h = cpu->regs.a; break;
        //case 0x60: cpu->regs.h = cpu->regs.b; break;
        //case 0x61: cpu->regs.h = cpu->regs.c; break;
        //case 0x62: cpu->regs.h = cpu->regs.d; break;
        //case 0x63: cpu->regs.h = cpu->regs.e; break;
        //case 0x64: cpu->regs.h = cpu->regs.h; break;
        //case 0x65: cpu->regs.h = cpu->regs.l; break; 
        //case 0x66: cpu->regs.h = memory_read(cpu, cpu->regs.hl); break;
        //case 0x6f: cpu->regs.l = cpu->regs.a; break;
        //case 0x68: cpu->regs.l = cpu->regs.b; break;
        //case 0x69: cpu->regs.l = cpu->regs.c; break;
        //case 0x6a: cpu->regs.l = cpu->regs.d; break;
        //case 0x6b: cpu->regs.l = cpu->regs.e; break;
        //case 0x6c: cpu->regs.l = cpu->regs.h; break;
        //case 0x6d: cpu->regs.l = cpu->regs.l; break;
        //case 0x6e: cpu->regs.l = memory_read(cpu, cpu->regs.hl); break;
        //case 0x77: memory_write(cpu, cpu->regs.hl, cpu->regs.a); break;
        //case 0x70: memory_write(cpu, cpu->regs.hl, cpu->regs.b); break;
        //case 0x71: memory_write(cpu, cpu->regs.hl, cpu->regs.c); break;
        //case 0x72: memory_write(cpu, cpu->regs.hl, cpu->regs.d); break;
        //case 0x73: memory_write(cpu, cpu->regs.hl, cpu->regs.e); break;
        //case 0x74: memory_write(cpu, cpu->regs.hl, cpu->regs.h); break;
        //case 0x75: memory_write(cpu, cpu->regs.hl, cpu->regs.l); break;
        //case 0x76: memory_write(cpu, cpu->regs.hl, memory_read(cpu, cpu->regs.hl)); break;

        // ANA
        //case 0xa7: ana(cpu, cpu->regs.a); break;
        case 0xe6: ana(cpu, next_byte(cpu)); break;

        // XRA
        //case 0xaf: xra(cpu, cpu->regs.a); break;
        //case 0xa8: xra(cpu, cpu->regs.b); break;
        //case 0xa9: xra(cpu, cpu->regs.c); break;
        //case 0xaa: xra(cpu, cpu->regs.d); break;
        //case 0xab: xra(cpu, cpu->regs.e); break;
        //case 0xac: xra(cpu, cpu->regs.h); break;
        //case 0xad: xra(cpu, cpu->regs.l); break;
        //case 0xae: xra(cpu, memory_read(cpu, cpu->regs.hl)); break;
        //case 0xee: xra(cpu, next_byte(cpu)); break;

        // PUSH
        case 0xc5: push(cpu, cpu->regs.bc); break;
        case 0xd5: push(cpu, cpu->regs.de); break;
        case 0xe5: push(cpu, cpu->regs.hl); break;
        case 0xf5: push_psw(cpu); break;

        // POP
        case 0xc1: cpu->regs.bc = pop(cpu); break;
        case 0xd1: cpu->regs.de = pop(cpu); break;
        case 0xe1: cpu->regs.hl = pop(cpu); break;
        case 0xf1: pop_psw(cpu); break;

        // PCHL
        case 0xe9: cpu->pc = cpu->regs.hl; break;

        // JMP
        case 0xc2: jmp(cpu, !cpu->flags.zero); break;
        case 0xc3: jmp(cpu, 1); break;
        case 0xca: jmp(cpu, cpu->flags.zero); break;
        case 0xd2: jmp(cpu, !cpu->flags.carry); break;
        case 0xda: jmp(cpu, cpu->flags.carry); break;
        case 0xe2: jmp(cpu, !cpu->flags.parity); break;
        case 0xea: jmp(cpu, cpu->flags.parity); break;
        case 0xf2: jmp(cpu, !cpu->flags.sign); break;
        case 0xfa: jmp(cpu, cpu->flags.sign); break;

        // ADD
        //case 0xc6: add(cpu, &cpu->regs.a, next_byte(cpu)); break;

        // RET
        case 0xc0: ret(cpu, !cpu->flags.zero); break;
        case 0xc8: ret(cpu, cpu->flags.zero); break;
        case 0xc9: ret(cpu, 1); break;
        case 0xd0: ret(cpu, !cpu->flags.carry); break;
        case 0xd8: ret(cpu, cpu->flags.carry); break;
        case 0xe0: ret(cpu, !cpu->flags.parity); break;
        case 0xe8: ret(cpu, cpu->flags.parity); break;
        case 0xf0: ret(cpu, !cpu->flags.sign); break;
        case 0xf8: ret(cpu, cpu->flags.sign); break;

        // CALL
        case 0xc4: call(cpu, !cpu->flags.zero); break;
        case 0xcc: call(cpu, cpu->flags.zero); break;
        case 0xcd: call(cpu, 1); break;
        case 0xd4: call(cpu, !cpu->flags.carry); break;
        case 0xdc: call(cpu, cpu->flags.carry); break;
        case 0xe4: call(cpu, !cpu->flags.parity); break;
        case 0xec: call(cpu, cpu->flags.parity); break;
        case 0xf4: call(cpu, !cpu->flags.sign); break;
        case 0xfc: call(cpu, cpu->flags.sign); break;

        // OUT
        //case 0xd3: cpu->out(cpu, next_byte(cpu)); break;

        // XCHG
        //case 0xeb: xchg(cpu); break;

        // EI
        //case 0xfb: cpu->interrupts_enabled = true; break;

        // CPI
        case 0xfe: cmp(cpu, next_byte(cpu)); break;

        /// RST
        //case 0xff: rst(cpu, 0x38); break;

        // Undocumented opcodes
        //case 0x18: break;
        //case 0x20: break;
        
        default: not_implemented(opcode);
    }
}
