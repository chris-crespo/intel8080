#include <stdlib.h>
#include "cpu.h"

void cpu_init(CPU *cpu, void (*in)(CPU*), void (*out)(CPU*, u8)) {
    cpu->regs.a  = 0;
    cpu->regs.bc = 0;
    cpu->regs.de = 0;
    cpu->regs.hl = 0;

    cpu->flags.sign      = 0;
    cpu->flags.zero      = 0;
    cpu->flags.aux_carry = 0;
    cpu->flags.parity    = 0;
    cpu->flags.carry     = 0;
    
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
    printf("HL %04x\n", cpu->regs.hl);
}

u8 read_byte(CPU *cpu, u16 addr) {
    return cpu->memory[addr];
}

void write_byte(CPU *cpu, u16 addr, u8 value) {
    cpu->memory[addr] = value;
}

static u16 read_word(CPU *cpu, u16 addr) {
    u8 lo  = read_byte(cpu, addr);
    u16 hi = read_byte(cpu, addr+1);

    return lo | (hi << 8);
}

static void write_word(CPU *cpu, u16 addr, u16 word) {
    write_byte(cpu, addr, word & 0xff);
    write_byte(cpu, addr+1, word >> 8);
}

static inline u8 next_byte(CPU *cpu) {
    return read_byte(cpu, cpu->pc++);
}

static inline u16 next_word(CPU *cpu) {
    u16 word = read_word(cpu, cpu->pc);
    cpu->pc += 2;
    return word;
}

static u8 parity(u8 value) {
    u8 bits = 0;

    while (value) {
        bits += value & 0x1;
        value >>= 1;
    }

    return (bits & 0x1) == 0;
}

static inline void set_zsp(CPU *cpu, u8 value) {
    cpu->flags.zero   = (value == 0);
    cpu->flags.sign   = value >> 7;
    cpu->flags.parity = parity(value);
}

static inline void set_carry(CPU *cpu, bool carry) {
    cpu->flags.carry = carry;
}

static inline void set_aux(CPU *cpu, bool carry) {
    cpu->flags.aux_carry = carry;
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
    set_aux(cpu, !((value & 0xf) == 0xf));

    return value;
}

static inline void dad(CPU *cpu, u16 value) {
    cpu->regs.hl += value;
    set_carry(cpu, cpu->regs.hl < value);
}

static inline void rlc(CPU *cpu) {
    set_carry(cpu, cpu->regs.a >> 7);
    cpu->regs.a = (cpu->regs.a << 1) | cpu->flags.carry;
}

static inline void rrc(CPU *cpu) {
    set_carry(cpu, cpu->regs.a & 0x1);
    cpu->regs.a = (cpu->regs.a >> 1) | (cpu->flags.carry << 7);
}

static inline void shld(CPU *cpu, u16 addr) {
    write_word(cpu, addr, cpu->regs.hl);
}

static inline void lhld(CPU *cpu, u16 addr) {
    cpu->regs.hl = read_word(cpu, addr);
}

static inline void add(CPU *cpu, u8 value, bool carry) {
    u16 result = cpu->regs.a + value + carry;

    set_zsp(cpu, result);
    set_carry(cpu, result >> 8);
    set_aux(cpu, (cpu->regs.a ^ value ^ result) & 0x10);

    cpu->regs.a = result;
}

static inline void sub(CPU *cpu, u8 value, bool carry) {
    add(cpu, ~value, !carry);
    set_carry(cpu, !cpu->flags.carry);
}

static inline void ana(CPU *cpu, u8 value) {
    set_aux(cpu, ((cpu->regs.a | value) & 0x08) != 0);
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

static inline void ora(CPU *cpu, u8 value) {
    cpu->regs.a |= value;
    set_carry(cpu, 0);
    set_aux(cpu, 0);
    set_zsp(cpu, cpu->regs.a);
}

static inline void cmp(CPU *cpu, u8 value) {
    u16 comp = cpu->regs.a - value;
    set_zsp(cpu, comp);
    set_carry(cpu, comp >> 8);
    set_aux(cpu, ~(cpu->regs.a ^ comp ^ value) & 0x10);
}

static inline void push(CPU *cpu, u16 value) {
    cpu->sp -= 2;
    write_word(cpu, cpu->sp, value);
}

static inline u16 pop(CPU *cpu) {
    u16 value = read_word(cpu, cpu->sp);
    cpu->sp += 2;

    return value;
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

static inline void jmp(CPU *cpu, bool condition) {
    u16 addr = next_word(cpu);
    if (condition)
        cpu->pc = addr;
}

static inline void ret(CPU *cpu, bool cond) {
    if (cond) {
        cpu->pc = pop(cpu);
    }
}

static inline void call(CPU *cpu, bool cond) {
    u16 addr = next_word(cpu);
    if (cond) {
        push(cpu, cpu->pc);
        cpu->pc = addr;
    }
}

static inline void xchg(CPU *cpu) {
    u16 temp = cpu->regs.de; 
    cpu->regs.de = cpu->regs.hl;
    cpu->regs.hl = temp;
}

static inline void rst(CPU *cpu, u8 addr) {
    push(cpu, cpu->pc);
    cpu->pc = addr;
}

void cpu_execute(CPU *cpu) {
    // TODO: Change this
    u8 opcode = next_byte(cpu);
    switch (opcode) {
        // NOP
        case 0x00: break;

        // LXI
        case 0x01: cpu->regs.bc = next_word(cpu); break;
        case 0x11: cpu->regs.de = next_word(cpu); break;
        case 0x21: cpu->regs.hl = next_word(cpu); break;
        case 0x31: cpu->sp = next_word(cpu); break;

        // INX
        case 0x03: cpu->regs.bc += 1; break;
        case 0x13: cpu->regs.de += 1; break;
        case 0x23: cpu->regs.hl += 1; break;
        case 0x33: cpu->sp += 1; break;
        
        // INR
        case 0x3c: cpu->regs.a = inr(cpu, cpu->regs.a); break;
        case 0x14: cpu->regs.d = inr(cpu, cpu->regs.d); break;
        case 0x34: write_byte(cpu, cpu->regs.hl, inr(cpu, read_byte(cpu, cpu->regs.hl))); break;

        // DCX
        case 0x0b: cpu->regs.bc -= 1; break;
        case 0x2b: cpu->regs.hl -= 1; break;

        // DCR
        //case 0x3d: cpu->regs.a = dcr(cpu, cpu->regs.a); break; 
        case 0x05: cpu->regs.b = dcr(cpu, cpu->regs.b); break;
        case 0x0d: cpu->regs.c = dcr(cpu, cpu->regs.c); break;
        //case 0x15: cpu->regs.d = dcr(cpu, cpu->regs.d); break;
        //case 0x1d: cpu->regs.e = dcr(cpu, cpu->regs.e); break;
        //case 0x25: cpu->regs.h = dcr(cpu, cpu->regs.h); break;
        //case 0x2d: cpu->regs.l = dcr(cpu, cpu->regs.l); break;
        //case 0x35: write_byte(cpu, cpu->regs.hl, dcr(cpu, read_byte(cpu, cpu->regs.hl))); break;

        // MVI
        case 0x3e: cpu->regs.a = next_byte(cpu); break;
        case 0x06: cpu->regs.b = next_byte(cpu); break;
        case 0x0e: cpu->regs.c = next_byte(cpu); break;
        case 0x16: cpu->regs.d = next_byte(cpu); break;
        case 0x26: cpu->regs.h = next_byte(cpu); break;
        case 0x36: write_byte(cpu, cpu->regs.hl, next_byte(cpu)); break;

        // DAD
        case 0x09: dad(cpu, cpu->regs.bc); break;
        case 0x19: dad(cpu, cpu->regs.de); break;
        case 0x29: dad(cpu, cpu->regs.hl); break;
        case 0x39: dad(cpu, cpu->sp); break;

        // STAX
        case 0x12: write_byte(cpu, cpu->regs.de, cpu->regs.a); break;
        case 0x32: write_byte(cpu, next_word(cpu), cpu->regs.a); break; // STA

        // LDAX
        case 0x1a: cpu->regs.a = read_byte(cpu, cpu->regs.de); break;
        case 0x3a: cpu->regs.a = read_byte(cpu, next_word(cpu)); break; // LDA

        case 0x37: set_carry(cpu, 1); break; // STC

        case 0x07: rlc(cpu); break; // RLC
        case 0x0f: rrc(cpu); break; // RRC

        case 0x22: write_word(cpu, next_word(cpu), cpu->regs.hl); break; // SHLD 
        case 0x2a: cpu->regs.hl = read_word(cpu, next_word(cpu)); break; // LHLD

        // MOV
        //case 0x7f: cpu->regs.a = cpu->regs.a; break;
        case 0x78: cpu->regs.a = cpu->regs.b; break;
        case 0x79: cpu->regs.a = cpu->regs.c; break;
        case 0x7a: cpu->regs.a = cpu->regs.d; break;
        case 0x7b: cpu->regs.a = cpu->regs.e; break;
        case 0x7c: cpu->regs.a = cpu->regs.h; break;
        case 0x7d: cpu->regs.a = cpu->regs.l; break;
        case 0x7e: cpu->regs.a = read_byte(cpu, cpu->regs.hl); break;

        case 0x47: cpu->regs.b = cpu->regs.a; break;
        //case 0x40: cpu->regs.b = cpu->regs.b; break;
        //case 0x41: cpu->regs.b = cpu->regs.c; break;
        //case 0x42: cpu->regs.b = cpu->regs.d; break;
        //case 0x43: cpu->regs.b = cpu->regs.e; break;
        //case 0x44: cpu->regs.b = cpu->regs.h; break;
        //case 0x45: cpu->regs.b = cpu->regs.l; break;
        case 0x46: cpu->regs.b = read_byte(cpu, cpu->regs.hl); break;

        case 0x4f: cpu->regs.c = cpu->regs.a; break;
        //case 0x48: cpu->regs.c = cpu->regs.b; break;
        //case 0x49: cpu->regs.c = cpu->regs.c; break;
        //case 0x4a: cpu->regs.c = cpu->regs.d; break;
        //case 0x4b: cpu->regs.c = cpu->regs.e; break;
        //case 0x4c: cpu->regs.c = cpu->regs.h; break;
        //case 0x4d: cpu->regs.c = cpu->regs.l; break;
        case 0x4e: cpu->regs.c = read_byte(cpu, cpu->regs.hl); break;

        //case 0x57: cpu->regs.d = cpu->regs.a; break;
        //case 0x50: cpu->regs.d = cpu->regs.b; break;
        //case 0x51: cpu->regs.d = cpu->regs.c; break;
        //case 0x52: cpu->regs.d = cpu->regs.d; break;
        //case 0x53: cpu->regs.d = cpu->regs.e; break;
        case 0x54: cpu->regs.d = cpu->regs.h; break;
        //case 0x55: cpu->regs.d = cpu->regs.l; break;
        //case 0x56: cpu->regs.d = read_byte(cpu, cpu->regs.hl); break;
        
        case 0x5f: cpu->regs.e = cpu->regs.a; break;
        //case 0x58: cpu->regs.e = cpu->regs.b; break;
        //case 0x59: cpu->regs.e = cpu->regs.c; break;
        //case 0x5a: cpu->regs.e = cpu->regs.d; break;
        //case 0x5b: cpu->regs.e = cpu->regs.e; break;
        //case 0x5c: cpu->regs.e = cpu->regs.h; break;
        case 0x5d: cpu->regs.e = cpu->regs.l; break;
        case 0x5e: cpu->regs.e = read_byte(cpu, cpu->regs.hl); break;

        //case 0x67: cpu->regs.h = cpu->regs.a; break;
        //case 0x60: cpu->regs.h = cpu->regs.b; break;
        //case 0x61: cpu->regs.h = cpu->regs.c; break;
        //case 0x62: cpu->regs.h = cpu->regs.d; break;
        //case 0x63: cpu->regs.h = cpu->regs.e; break;
        //case 0x64: cpu->regs.h = cpu->regs.h; break;
        //case 0x65: cpu->regs.h = cpu->regs.l; break; 
        case 0x66: cpu->regs.h = read_byte(cpu, cpu->regs.hl); break;

        case 0x6f: cpu->regs.l = cpu->regs.a; break;
        //case 0x68: cpu->regs.l = cpu->regs.b; break;
        //case 0x69: cpu->regs.l = cpu->regs.c; break;
        //case 0x6a: cpu->regs.l = cpu->regs.d; break;
        //case 0x6b: cpu->regs.l = cpu->regs.e; break;
        //case 0x6c: cpu->regs.l = cpu->regs.h; break;
        //case 0x6d: cpu->regs.l = cpu->regs.l; break;
        //case 0x6e: cpu->regs.l = read_byte(cpu, cpu->regs.hl); break;
        
        case 0x77: write_byte(cpu, cpu->regs.hl, cpu->regs.a); break;
        //case 0x70: write_byte(cpu, cpu->regs.hl, cpu->regs.b); break;
        //case 0x71: write_byte(cpu, cpu->regs.hl, cpu->regs.c); break;
        //case 0x72: write_byte(cpu, cpu->regs.hl, cpu->regs.d); break;
        //case 0x73: write_byte(cpu, cpu->regs.hl, cpu->regs.e); break;
        //case 0x74: write_byte(cpu, cpu->regs.hl, cpu->regs.h); break;
        //case 0x75: write_byte(cpu, cpu->regs.hl, cpu->regs.l); break;
        //case 0x76: write_byte(cpu, cpu->regs.hl, read_byte(cpu, cpu->regs.hl)); break;

        // ADD
        case 0x87: add(cpu, cpu->regs.a, 0); break;
        case 0x80: add(cpu, cpu->regs.b, 0); break;
        case 0x81: add(cpu, cpu->regs.c, 0); break;
        case 0x82: add(cpu, cpu->regs.d, 0); break;
        case 0x83: add(cpu, cpu->regs.e, 0); break;
        case 0x84: add(cpu, cpu->regs.h, 0); break;
        case 0x85: add(cpu, cpu->regs.l, 0); break;
        case 0x86: add(cpu, read_byte(cpu, cpu->regs.hl), 0); break;
        case 0xc6: add(cpu, next_byte(cpu), 0); break;

        // ADC
        case 0x8f: add(cpu, cpu->regs.a, cpu->flags.carry); break;
        case 0x88: add(cpu, cpu->regs.b, cpu->flags.carry); break;
        case 0x89: add(cpu, cpu->regs.c, cpu->flags.carry); break;
        case 0x8a: add(cpu, cpu->regs.d, cpu->flags.carry); break;
        case 0x8b: add(cpu, cpu->regs.e, cpu->flags.carry); break;
        case 0x8c: add(cpu, cpu->regs.h, cpu->flags.carry); break;
        case 0x8d: add(cpu, cpu->regs.l, cpu->flags.carry); break;
        case 0x8e: add(cpu, read_byte(cpu, cpu->regs.hl), cpu->flags.carry); break;
        case 0xce: add(cpu, next_byte(cpu), cpu->flags.carry); break;

        // SUB
        case 0x97: sub(cpu, cpu->regs.a, 0); break;
        case 0x90: sub(cpu, cpu->regs.b, 0); break;
        case 0x91: sub(cpu, cpu->regs.c, 0); break;
        case 0x92: sub(cpu, cpu->regs.d, 0); break;
        case 0x93: sub(cpu, cpu->regs.e, 0); break;
        case 0x94: sub(cpu, cpu->regs.h, 0); break;
        case 0x95: sub(cpu, cpu->regs.l, 0); break;
        case 0x96: sub(cpu, read_byte(cpu, cpu->regs.hl), 0); break;
        case 0xd6: sub(cpu, next_byte(cpu), 0); break;

        // SBB
        case 0x9f: sub(cpu, cpu->regs.a, cpu->flags.carry); break;
        case 0x98: sub(cpu, cpu->regs.b, cpu->flags.carry); break;
        case 0x99: sub(cpu, cpu->regs.c, cpu->flags.carry); break;
        case 0x9a: sub(cpu, cpu->regs.d, cpu->flags.carry); break;
        case 0x9b: sub(cpu, cpu->regs.e, cpu->flags.carry); break;
        case 0x9c: sub(cpu, cpu->regs.h, cpu->flags.carry); break;
        case 0x9d: sub(cpu, cpu->regs.l, cpu->flags.carry); break;
        case 0x9e: sub(cpu, read_byte(cpu, cpu->regs.hl), cpu->flags.carry); break;
        case 0xde: sub(cpu, next_byte(cpu), cpu->flags.carry); break;

        // ANA
        case 0xa7: ana(cpu, cpu->regs.a); break;
        case 0xa0: ana(cpu, cpu->regs.b); break;
        case 0xa1: ana(cpu, cpu->regs.c); break;
        case 0xa2: ana(cpu, cpu->regs.d); break;
        case 0xa3: ana(cpu, cpu->regs.e); break;
        case 0xa4: ana(cpu, cpu->regs.h); break;
        case 0xa5: ana(cpu, cpu->regs.l); break;
        case 0xa6: ana(cpu, read_byte(cpu, cpu->regs.hl)); break;
        case 0xe6: ana(cpu, next_byte(cpu)); break;

        // XRA
        case 0xaf: xra(cpu, cpu->regs.a); break;
        case 0xa8: xra(cpu, cpu->regs.b); break;
        case 0xa9: xra(cpu, cpu->regs.c); break;
        case 0xaa: xra(cpu, cpu->regs.d); break;
        case 0xab: xra(cpu, cpu->regs.e); break;
        case 0xac: xra(cpu, cpu->regs.h); break;
        case 0xad: xra(cpu, cpu->regs.l); break;
        case 0xae: xra(cpu, read_byte(cpu, cpu->regs.hl)); break;
        case 0xee: xra(cpu, next_byte(cpu)); break;

        // ORA
        case 0xb7: ora(cpu, cpu->regs.a); break;
        case 0xb0: ora(cpu, cpu->regs.b); break;
        case 0xb1: ora(cpu, cpu->regs.c); break;
        case 0xb2: ora(cpu, cpu->regs.d); break;
        case 0xb3: ora(cpu, cpu->regs.e); break;
        case 0xb4: ora(cpu, cpu->regs.h); break;
        case 0xb5: ora(cpu, cpu->regs.l); break;
        case 0xb6: ora(cpu, read_byte(cpu, cpu->regs.hl)); break;
        case 0xf6: ora(cpu, next_byte(cpu)); break;

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
        case 0xc3: jmp(cpu, 1); break;
        case 0xc2: jmp(cpu, !cpu->flags.zero); break;
        case 0xca: jmp(cpu, cpu->flags.zero); break;
        case 0xd2: jmp(cpu, !cpu->flags.carry); break;
        case 0xda: jmp(cpu, cpu->flags.carry); break;
        case 0xe2: jmp(cpu, !cpu->flags.parity); break;
        case 0xea: jmp(cpu, cpu->flags.parity); break;
        case 0xf2: jmp(cpu, !cpu->flags.sign); break;
        case 0xfa: jmp(cpu, cpu->flags.sign); break;

        // RET
        case 0xc9: ret(cpu, 1); break;
        case 0xc0: ret(cpu, !cpu->flags.zero); break;
        case 0xc8: ret(cpu, cpu->flags.zero); break;
        case 0xd0: ret(cpu, !cpu->flags.carry); break;
        case 0xd8: ret(cpu, cpu->flags.carry); break;
        case 0xe0: ret(cpu, !cpu->flags.parity); break;
        case 0xe8: ret(cpu, cpu->flags.parity); break;
        case 0xf0: ret(cpu, !cpu->flags.sign); break;
        case 0xf8: ret(cpu, cpu->flags.sign); break;

        // CALL
        case 0xcd: call(cpu, 1); break;
        case 0xc4: call(cpu, !cpu->flags.zero); break;
        case 0xcc: call(cpu, cpu->flags.zero); break;
        case 0xd4: call(cpu, !cpu->flags.carry); break;
        case 0xdc: call(cpu, cpu->flags.carry); break;
        case 0xe4: call(cpu, !cpu->flags.parity); break;
        case 0xec: call(cpu, cpu->flags.parity); break;
        case 0xf4: call(cpu, !cpu->flags.sign); break;
        case 0xfc: call(cpu, cpu->flags.sign); break;

        // OUT
        case 0xd3: cpu->out(cpu, next_byte(cpu)); break;

        // XCHG
        case 0xeb: xchg(cpu); break;
        
        // SPHL
        case 0xf9: cpu->sp = cpu->regs.hl; break;

        case 0xfb: cpu->interrupts_enabled = true; break; // EI
        case 0xf3: cpu->interrupts_enabled = false; break; // DI 

        // CMP
        case 0xbf: cmp(cpu, cpu->regs.a); break;
        case 0xb8: cmp(cpu, cpu->regs.b); break;
        case 0xb9: cmp(cpu, cpu->regs.c); break;
        case 0xba: cmp(cpu, cpu->regs.d); break;
        case 0xbb: cmp(cpu, cpu->regs.e); break;
        case 0xbc: cmp(cpu, cpu->regs.h); break;
        case 0xbd: cmp(cpu, cpu->regs.l); break;
        case 0xbe: cmp(cpu, read_byte(cpu, cpu->regs.hl)); break;
        case 0xfe: cmp(cpu, next_byte(cpu)); break;

        /// RST
        //case 0xff: rst(cpu, 0x38); break;

        // Undocumented opcodes
        //case 0x18: break;
        //case 0x20: break;
        
        default: not_implemented(opcode);
    }
}
