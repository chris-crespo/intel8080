#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

void load_rom(const char *rom);

u8 memory_fetch(u16 addr);

#endif
