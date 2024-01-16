#include "bank_call.h"

void __fastcall__ push_prg_bank( uint8_t bnk )
{
    PRG_BANK_STACK[ PRG_BANK_STACK_LEVEL ] = bnk;
    ++PRG_BANK_STACK_LEVEL;
    ASSERT( PRG_BANK_STACK_LEVEL < PRG_BANK_STACK_SIZE );
    mapper_set_prg_bank( bnk );
}

void __fastcall__ pop_prg_bank(void)
{
    ASSERT( PRG_BANK_STACK_LEVEL > 0 );
    --PRG_BANK_STACK_LEVEL;
    mapper_set_prg_bank( PRG_BANK_STACK[ PRG_BANK_STACK_LEVEL ] );
}
