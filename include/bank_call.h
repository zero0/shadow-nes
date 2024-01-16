#ifndef BANK_CALL_H
#define BANK_CALL_H

#include "macros.h"
#include "mapper.h"

#define PUSH_CODE_BANK(bnk)                         \
_Pragma("rodata-name (push, \"ROM_0" T(bnk) "\")")  \
_Pragma("code-name   (push, \"ROM_0" T(bnk) "\")")

#define POP_CODE_BANK()         \
_Pragma("rodata-name (pop)")    \
_Pragma("code-name   (pop)")

void push_prg_bank(uint8_t bnk);

void pop_prg_bank(void);

#define bank_call(bnk, fn)  \
do                          \
{                           \
    push_prg_bank(bnk);     \
    (*fn)();                \
    pop_prg_bank();         \
} while( 0 )

#endif // BANK_CALL_H