#ifndef MAPPER_H
#define MAPPER_H

#include "types.h"

#define PRG_BANK_STACK_SIZE     (uint8_t)8

extern uint8_t PRG_BANK_STACK_LEVEL;
#pragma zpsym("PRG_BANK_STACK_LEVEL");

extern uint8_t PRG_BANK_STACK[PRG_BANK_STACK_SIZE];
#pragma zpsym("PRG_BANK_STACK");

void __fastcall__ mapper_reset(void);

void __fastcall__ mapper_reset_irq(void);

void __fastcall__ mapper_set_scanline_irq(uint8_t scanline);

void __fastcall__ mapper_set_mirroring(uint8_t mirror);

void __fastcall__ mapper_set_chr_bank_0(uint8_t chrBank);

void __fastcall__ mapper_set_chr_bank_1(uint8_t chrBank);

void __fastcall__ mapper_set_prg_bank(uint8_t prgBank);


#endif // MAPPER_H
