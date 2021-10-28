#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "types.h"
#include "cpu.h"

static void println(char *msg) {
    printf("%s%c", msg, '\n');
}

static void not_implemented(u8 opcode) {
    fprintf(stderr, "Opcode 0x%02x not implemented.\n", opcode);
    exit(1);
}

void disassemble_opcode(CPU *cpu, u8 opcode) {
    printf("%04x\t", cpu->pc - 1);
    switch (opcode) {
        // NOP
        case 0x00: println("NOP"); break;

        // LXI
        case 0x01: println("LXI BC"); break;
        case 0x11: println("LXI DE"); break;
        case 0x21: println("LXI HL"); break;
        case 0x31: println("LXI SP"); break;

        // INX
        case 0x03: println("INX BC"); break;
        case 0x13: println("INX DE"); break;
        case 0x23: println("INX HL"); break;
        case 0x33: println("INX SP"); break;
        
        // INR
        case 0x3c: println("INR A"); break;
        case 0x04: println("INR B"); break;
        case 0x0c: println("INR C"); break;
        case 0x14: println("INR D"); break;
        case 0x1c: println("INR E"); break;
        case 0x24: println("INR H"); break;
        case 0x2c: println("INR L"); break;
        case 0x34: println("INR M"); break;

        // DCX
        case 0x0b: println("DCX BC"); break;
        case 0x1b: println("DCX DE"); break;
        case 0x2b: println("DCX HL"); break;
        case 0x3b: println("DCX SP"); break;

        // DCR
        case 0x3d: println("DCR A"); break; 
        case 0x05: println("DCR B"); break;
        case 0x0d: println("DCR C"); break;
        case 0x15: println("DCR D"); break;
        case 0x1d: println("DCR E"); break;
        case 0x25: println("DCR H"); break;
        case 0x2d: println("DCR L"); break;
        case 0x35: println("DCR M"); break;

        // MVI
        case 0x3e: println("MVI A"); break;
        case 0x06: println("MVI D"); break;
        case 0x0e: println("MVI C"); break;
        case 0x16: println("MVI D"); break;
        case 0x1e: println("MVI E"); break;
        case 0x26: println("MVI H"); break;
        case 0x2e: println("MVI L"); break;
        case 0x36: println("MVI M"); break;

        case 0x27: println("DAA"); break; // DAA 
        case 0x2f: println("CMA"); break; // CMA 
        case 0x37: println("STC"); break; // STC 
        case 0x3f: println("CMC"); break; // CMC 

        // DAD
        case 0x09: println("DAD BC"); break;
        case 0x19: println("DAD DE"); break;
        case 0x29: println("DAD HL"); break;
        case 0x39: println("DAD SP"); break;

        // STAX
        case 0x02: println("STAX BC"); break;
        case 0x12: println("STAX DE"); break;
        case 0x32: println("STA"); break; // STA

        // LDAX
        case 0x0a: println("LDAX BC"); break;
        case 0x1a: println("LDAX DE"); break;
        case 0x3a: println("LDA"); break; // LDA

        case 0x07: println("RLC"); break; // RLC
        case 0x0f: println("RRC"); break; // RRC
        case 0x17: println("RAL"); break; // RAL
        case 0x1f: println("RAR"); break; // RAR

        case 0x22: println("SHLD"); break; // SHLD
        case 0x2a: println("LHLD"); break; // LHLD

        // MOV
        case 0x7f: println("MOV A A"); break;
        case 0x78: println("MOV A B"); break;
        case 0x79: println("MOV A C"); break;
        case 0x7a: println("MOV A D"); break;
        case 0x7b: println("MOV A E"); break;
        case 0x7c: println("MOV A H"); break;
        case 0x7d: println("MOV A L"); break;
        case 0x7e: println("MOV A M"); break;

        case 0x47: println("MOV B A"); break;
        case 0x40: println("MOV B B"); break;
        case 0x41: println("MOV B C"); break;
        case 0x42: println("MOV B D"); break;
        case 0x43: println("MOV B E"); break;
        case 0x44: println("MOV B H"); break;
        case 0x45: println("MOV B L"); break;
        case 0x46: println("MOV B M"); break;

        case 0x4f: println("MOV C A"); break;
        case 0x48: println("MOV C B"); break;
        case 0x49: println("MOV C C"); break;
        case 0x4a: println("MOV C D"); break;
        case 0x4b: println("MOV C E"); break;
        case 0x4c: println("MOV C H"); break;
        case 0x4d: println("MOV C L"); break;
        case 0x4e: println("MOV C M"); break;

        case 0x57: println("MOV D A"); break;
        case 0x50: println("MOV D B"); break;
        case 0x51: println("MOV D C"); break;
        case 0x52: println("MOV D D"); break;
        case 0x53: println("MOV D E"); break;
        case 0x54: println("MOV D H"); break;
        case 0x55: println("MOV D L"); break;
        case 0x56: println("MOV D M"); break;
        
        case 0x5f: println("MOV E A"); break;
        case 0x58: println("MOV E B"); break;
        case 0x59: println("MOV E C"); break;
        case 0x5a: println("MOV E D"); break;
        case 0x5b: println("MOV E E"); break;
        case 0x5c: println("MOV E H"); break;
        case 0x5d: println("MOV E L"); break;
        case 0x5e: println("MOV E M"); break;

        case 0x67: println("MOV H A"); break;
        case 0x60: println("MOV H B"); break;
        case 0x61: println("MOV H C"); break;
        case 0x62: println("MOV H D"); break;
        case 0x63: println("MOV H E"); break;
        case 0x64: println("MOV H H"); break;
        case 0x65: println("MOV H L"); break; 
        case 0x66: println("MOV H M"); break;

        case 0x6f: println("MOV L A"); break;
        case 0x68: println("MOV L B"); break;
        case 0x69: println("MOV L C"); break;
        case 0x6a: println("MOV L D"); break;
        case 0x6b: println("MOV L E"); break;
        case 0x6c: println("MOV L H"); break;
        case 0x6d: println("MOV L L"); break;
        case 0x6e: println("MOV L M"); break;
        
        case 0x77: println("MOV M A"); break;
        case 0x70: println("MOV M B"); break;
        case 0x71: println("MOV M C"); break;
        case 0x72: println("MOV M D"); break;
        case 0x73: println("MOV M E"); break;
        case 0x74: println("MOV M H"); break;
        case 0x75: println("MOV M L"); break;
        case 0x76: println("MOV M M"); break;

        // ADD
        case 0x87: println("ADD A"); break;
        case 0x80: println("ADD B"); break;
        case 0x81: println("ADD C"); break;
        case 0x82: println("ADD D"); break;
        case 0x83: println("ADD E"); break;
        case 0x84: println("ADD H"); break;
        case 0x85: println("ADD L"); break;
        case 0x86: println("ADD M"); break;
        case 0xc6: println("ADD byte"); break;

        // ADC
        case 0x8f: println("ADC A"); break;
        case 0x88: println("ADC B"); break;
        case 0x89: println("ADC C"); break;
        case 0x8a: println("ADC D"); break;
        case 0x8b: println("ADC E"); break;
        case 0x8c: println("ADC H"); break;
        case 0x8d: println("ADC L"); break;
        case 0x8e: println("ADC M"); break;
        case 0xce: println("ADC byte"); break;

        // SUB
        case 0x97: println("SUB A"); break;
        case 0x90: println("SUB B"); break;
        case 0x91: println("SUB C"); break;
        case 0x92: println("SUB D"); break;
        case 0x93: println("SUB E"); break;
        case 0x94: println("SUB H"); break;
        case 0x95: println("SUB L"); break;
        case 0x96: println("SUB M"); break;
        case 0xd6: println("SUB byte"); break;

        // SBB
        case 0x9f: println("SBB A"); break;
        case 0x98: println("SBB B"); break;
        case 0x99: println("SBB C"); break;
        case 0x9a: println("SBB D"); break;
        case 0x9b: println("SBB E"); break;
        case 0x9c: println("SBB H"); break;
        case 0x9d: println("SBB L"); break;
        case 0x9e: println("SBB M"); break;
        case 0xde: println("SBB byte"); break;

        // ANA
        case 0xa7: println("ANA A"); break;
        case 0xa0: println("ANA B"); break;
        case 0xa1: println("ANA C"); break;
        case 0xa2: println("ANA D"); break;
        case 0xa3: println("ANA E"); break;
        case 0xa4: println("ANA H"); break;
        case 0xa5: println("ANA L"); break;
        case 0xa6: println("ANA M"); break;
        case 0xe6: println("ANA byte"); break;

        // XRA
        case 0xaf: println("XRA A"); break;
        case 0xa8: println("XRA B"); break;
        case 0xa9: println("XRA C"); break;
        case 0xaa: println("XRA D"); break;
        case 0xab: println("XRA E"); break;
        case 0xac: println("XRA H"); break;
        case 0xad: println("XRA L"); break;
        case 0xae: println("XRA M"); break;
        case 0xee: println("XRA byte"); break;

        // ORA
        case 0xb7: println("ORA A"); break;
        case 0xb0: println("ORA B"); break;
        case 0xb1: println("ORA C"); break;
        case 0xb2: println("ORA D"); break;
        case 0xb3: println("ORA E"); break;
        case 0xb4: println("ORA H"); break;
        case 0xb5: println("ORA L"); break;
        case 0xb6: println("ORA M"); break;
        case 0xf6: println("ORA byte"); break;

        // PUSH
        case 0xc5: println("PUSH BC"); break;
        case 0xd5: println("PUSH DE"); break;
        case 0xe5: println("PUSH HL"); break;
        case 0xf5: println("PUSH PSW"); break;

        // POP
        case 0xc1: println("POP BC"); break;
        case 0xd1: println("POP DE"); break;
        case 0xe1: println("POP HL"); break;
        case 0xf1: println("POP PSW"); break;

        // PCHL
        case 0xe9: println("PCHL"); break;

        // JMP
        case 0xc3: println("JMP"); break;
        case 0xc2: println("JNZ"); break;
        case 0xca: println("JZ"); break;
        case 0xd2: println("JNC"); break;
        case 0xda: println("JC"); break;
        case 0xe2: println("JPO"); break;
        case 0xea: println("JPE"); break;
        case 0xf2: println("JP"); break;
        case 0xfa: println("JM"); break;

        // RET
        case 0xc9: println("RET"); break;
        case 0xc0: println("RNZ"); break;
        case 0xc8: println("RZ"); break;
        case 0xd0: println("RNC"); break;
        case 0xd8: println("RC"); break;
        case 0xe0: println("RPO"); break;
        case 0xe8: println("RPE"); break;
        case 0xf0: println("RP"); break;
        case 0xf8: println("RM"); break;

        // CALL
        case 0xcd: println("CALL"); break;
        case 0xc4: println("CNZ"); break;
        case 0xcc: println("CZ"); break;
        case 0xd4: println("CNC"); break;
        case 0xdc: println("CC"); break;
        case 0xe4: println("CPO"); break;
        case 0xec: println("CPE"); break;
        case 0xf4: println("CP"); break;
        case 0xfc: println("CM"); break;

        // OUT
        case 0xd3: println("OUT"); break;

        // XCHG
        case 0xeb: println("XCHG"); break;
        
        // SPHL
        case 0xf9: println("SPHL"); break;

        case 0xfb: println("EI"); break; // EI
        case 0xf3: println("DI"); break; // DI 

        // CMP
        case 0xbf: println("CMP A"); break;
        case 0xb8: println("CMP B"); break;
        case 0xb9: println("CMP C"); break;
        case 0xba: println("CMP D"); break;
        case 0xbb: println("CMP E"); break;
        case 0xbc: println("CMP H"); break;
        case 0xbd: println("CMP L"); break;
        case 0xbe: println("CMP M"); break;
        case 0xfe: println("CMP byte"); break;

        /// RST
        //case 0xff: rst(cpu, 0x38); break;

        // Undocumented opcodes
        //case 0x18: break;
        //case 0x20: break;
        
        default: not_implemented(opcode);
    }
}
