#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

void load_rom(const char *rom);

u8 memory_read(u16 addr);
void memory_write(u16 addr, u8 value);

#endif
