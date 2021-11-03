#include "shift.h"

static u16 reg;
static u8  offset;

void shift_write(u8 value) {
    reg = (reg >> 8) | (value << 8);
}

void shift_offset(u8 value) {
    offset = value;
}

u8 shift_read(void) {
    return (reg & (0xff00 >> offset)) >> (8 - offset);
}
