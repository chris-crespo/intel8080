#include "cpu.h"
#include "memory.h"
#include "screen.h"

static CPU cpu;

static void not_implemented(u8 opcode) {
    fprintf(stderr, "Opcode 0x%02x not implemented.\n", opcode);
    screen_quit();
}

static void print_state(u8 opcode) {
    printf("PC %04x  ", cpu.pc);
    printf("Opcode %02x  ", opcode);
    printf("SP %04x  ", cpu.sp);
    printf("BC %04x  ", cpu.regs.bc);
    printf("DE %04x  ", cpu.regs.de);
    printf("HL %04x  ", cpu.regs.hl);
    printf("A %02x  ", cpu.regs.a);
    printf("ZSPC %d%d%d%d\n", cpu.flags.zero, cpu.flags.sign, cpu.flags.parity,cpu.flags.carry);
}

static void stack_push(u8 value) {
    memory_write(--cpu.sp, value);
}

static u8 stack_pop(void) {
    return memory_fetch(cpu.sp++);
}

static inline u16 fetch_addr(void) {
    u8 low = memory_fetch(cpu.pc++);
    u16 hi = memory_fetch(cpu.pc++);
    return low | (hi << 8);
}

static u8 parity(u8 value) {
    int bits = 0;

    while (value) {
        bits += value & 0x1;
        value >>= 1;
    }

    return (bits & 0x1) == 0;
}

void cpu_tick(void) {
    static int ticks;


    u8 opcode = memory_fetch(cpu.pc++);
    switch (opcode) {
        case 0x00: {
            // NOP
            break;
        }
        case 0x01: {
            // LXI B
            cpu.regs.bc = fetch_addr();
            break;
        }
        case 0x05: {
            // DCR B
            cpu.regs.b--;
            
            cpu.flags.sign      = (cpu.regs.b & 0x80) != 0;
            cpu.flags.zero      = cpu.regs.b == 0;
            cpu.flags.parity    = parity(cpu.regs.b);
            cpu.flags.aux_carry = (cpu.regs.b & 0xf) == 0xf;

            break;
        }
        case 0x06: {
            // MVI B
            cpu.regs.b = memory_fetch(cpu.pc++);
            break;
        }
        case 0x09: {
            // DAD B
            cpu.regs.hl += cpu.regs.bc;
            cpu.flags.carry = cpu.regs.hl < cpu.regs.bc;

            break;
        }
        case 0x0e: {
            // MVI C
            cpu.regs.c = memory_fetch(cpu.pc++);
            break;
        }
        case 0x0f: {
            // RRC
            cpu.flags.carry = cpu.regs.a & 0x1;
            cpu.regs.a = (cpu.regs.a >> 1) | (cpu.flags.carry << 7);

            break;
        }
        case 0x11: {
            // LXI D
            cpu.regs.de = fetch_addr();
            break;
        }
        case 0x13: {
            // INX D
            cpu.regs.de++;
            break;
        }
        case 0x19: {
            // DAD D
            cpu.regs.hl += cpu.regs.de;
            cpu.flags.carry = cpu.regs.hl < cpu.regs.de;
            break;
        }
        case 0x1a: {
            // LDAX D
            cpu.regs.a = memory_fetch(cpu.regs.de);
            break;
        }
        case 0x20: {
            break;
        }
        case 0x21: {
            // LXI H
            cpu.regs.hl = fetch_addr();
            break;
        }
        case 0x23: {
            // INX H
            cpu.regs.hl++;
            break;
        }
        case 0x26: {
            // MVI H
            cpu.regs.h = memory_fetch(cpu.pc++);
            break;
        }
        case 0x29: {
            // DAD H
            u16 old = cpu.regs.hl;

            cpu.regs.hl += cpu.regs.hl;
            cpu.flags.carry = cpu.regs.hl < old;

            break;
        }
        case 0x31: {
            // LXI SP
            cpu.sp = fetch_addr();
            break;
        }
        case 0x32: {
            // STA
            memory_write(fetch_addr(), cpu.regs.a);
            break;
        }
        case 0x36: {
            // MVI M 
            memory_write(cpu.regs.hl, memory_fetch(cpu.pc++));
            break;
        }
        case 0x3a: {
            // LDA
            cpu.regs.a = memory_fetch(fetch_addr());
            break;
        }
        case 0x3d: {
            // DCR A
            cpu.regs.a--;

            cpu.flags.sign = (cpu.regs.a & 0x80) != 0;
            cpu.flags.zero = cpu.regs.a == 0;
            cpu.flags.parity = parity(cpu.regs.a);
            cpu.flags.aux_carry = (cpu.regs.a & 0xf) == 0xf;

            break;
        }
        case 0x3e: {
            // MVI A
            cpu.regs.a = memory_fetch(cpu.pc++);
            break;
        }
        case 0x56: {
            // MOV D, M
            cpu.regs.d = memory_fetch(cpu.regs.hl);
            break;
        }
        case 0x5c: {
            // MOV E, H
            cpu.regs.e = cpu.regs.h;
            break;
        }
        case 0x5e: {
            // MOV E, M
            cpu.regs.e = memory_fetch(cpu.regs.hl);
            break;
        }
        case 0x66: {
            // MOV H, M
            cpu.regs.h = memory_fetch(cpu.regs.hl);
            break;
        }
        case 0x6f: {
            // MOV L, A
            cpu.regs.l = cpu.regs.a;
            break;
        }
        case 0x77: {
            // MOV M, A
            memory_write(cpu.regs.hl, cpu.regs.a);
            break;
        }
        case 0x7a: {
            // MOV A, D
            cpu.regs.a = cpu.regs.d;
            break;
        }
        case 0x7c: {
            // MOV A, H
            cpu.regs.a = cpu.regs.h;
            break;
        }
        case 0x7e: {
            // MOV A, M
            cpu.regs.a = memory_fetch(cpu.regs.hl);
            break;
        }
        case 0xa7: {
            // ANA A
            cpu.flags.carry  = 0;
            cpu.flags.zero   = cpu.regs.a == 0;
            cpu.flags.sign   = (cpu.regs.a & 0x80) != 0;
            cpu.flags.parity = parity(cpu.regs.a);

            break;
        }
        case 0xc1: {
            // POP B
            cpu.regs.c = stack_pop();
            cpu.regs.b = stack_pop();

            break;
        }
        case 0xc2: {
            // JNZ
            u16 addr = fetch_addr();
            if (!cpu.flags.zero)
                cpu.pc = addr;

            break;
        }
        case 0xc3: {
            // JUMP
            cpu.pc = fetch_addr();
            break;
        }
        case 0xc5: {
            // PUSH B
            stack_push(cpu.regs.b);
            stack_push(cpu.regs.c);

            break;
        }
        case 0xc6: {
            // ADI
            u8 immediate = memory_fetch(cpu.pc++);
            cpu.regs.a += immediate;

            cpu.flags.carry     = cpu.regs.a < immediate;
            cpu.flags.sign      = (cpu.regs.a & 0x80) != 0;
            cpu.flags.zero      = cpu.regs.a == 0;
            cpu.flags.parity    = parity(cpu.regs.a);
            cpu.flags.aux_carry = (cpu.regs.a & 0xf) < (immediate & 0xf);

            break;
        }
        case 0xc9: {
            // RET
            cpu.pc_low = stack_pop();
            cpu.pc_hi  = stack_pop();
            
            break;
        }
        case 0xca: {
            // JZ
            u16 addr = fetch_addr();
            if (cpu.flags.zero)
                cpu.pc = addr;

            break;
        }
        case 0xcd: {
            // CALL
            stack_push(cpu.pc_hi);
            stack_push(cpu.pc_low);

            cpu.pc = fetch_addr();

            break;
        }
        case 0xd1: {
            // POP D
            cpu.regs.e = stack_pop();
            cpu.regs.d = stack_pop();

            break;
        }
        case 0xd3: {
            // OUT
            printf("OUT Device number: %d.\n", memory_fetch(cpu.pc++));
            break;
        }
        case 0xd5: {
            // PUSH D
            stack_push(cpu.regs.d);
            stack_push(cpu.regs.e);

            break;
        }
        case 0xe1: {
            // POP H
            cpu.regs.l = stack_pop();
            cpu.regs.h = stack_pop();

            break;
        }
        case 0xe5: {
            // PUSH H
            stack_push(cpu.regs.h);
            stack_push(cpu.regs.l);

            break;
        }
        case 0xe6: {
            // ANI
            cpu.regs.a &= memory_fetch(cpu.pc++);

            cpu.flags.carry  = 0;
            cpu.flags.zero   = cpu.regs.a == 0;
            cpu.flags.sign   = (cpu.regs.a & 0x80) != 0;
            cpu.flags.parity = parity(cpu.regs.a);

            break;
        }
        case 0xeb: {
            // XCHG
            u16 temp = cpu.regs.de;

            cpu.regs.de = cpu.regs.hl;
            cpu.regs.hl = temp;

            break;
        }
        case 0xf1: {
            // POP PSW
            u8 psw = stack_pop();
            cpu.regs.a = stack_pop();

            cpu.flags.sign      = (psw & 0x80) != 0;
            cpu.flags.zero      = (psw & 0x40) != 0;
            cpu.flags.aux_carry = (psw & 0x10) != 0;
            cpu.flags.parity    = (psw & 0x04) != 0;
            cpu.flags.carry     = (psw & 0x01) != 0;

            break;
        }
        case 0xf5: {
            // PUSH PSW
            stack_push(cpu.regs.a);
            stack_push((cpu.flags.sign << 7)
                | (cpu.flags.zero << 6)
                | (cpu.flags.aux_carry << 4)
                | (cpu.flags.parity << 2)
                | 0x2
                | cpu.flags.carry);

            break;
        }
        case 0xfb: {
            // EI
            cpu.interrupts_enabled = true;
            break;
        }
        case 0xfe: {
            // CPI
            u8 immediate = memory_fetch(cpu.pc++);
            u8 comp      = cpu.regs.a - immediate;

            cpu.flags.zero      = comp == 0;
            cpu.flags.sign      = (comp & 0x80) != 0;
            cpu.flags.parity    = parity(comp);
            cpu.flags.carry     = immediate <= cpu.regs.a;
            cpu.flags.aux_carry = (immediate & 0xf) <= (cpu.regs.a & 0xf);

            break;
        }
        case 0xff: {
            // RST
            stack_push(cpu.pc_hi);
            stack_push(cpu.pc_low);

            cpu.pc = 0x38;

            break;
        }
        default: {
            not_implemented(opcode);
        }
    }
    print_state(opcode);
}
