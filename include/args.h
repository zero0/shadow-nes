#ifndef ARGS_H
#define ARGS_H

#include "macros.h"

extern uint8_t ARGS[4];
#pragma zpsym("ARGS")

extern void* ARGS_PTR[2];
#pragma zpsym("ARGS_PTR")

extern uint16_t ARGS_UINT16[2];
#pragma zpsym("ARGS_UINT16")

extern uint32_t ARGS_UINT32;
#pragma zpsym("ARGS_UINT32")

#define args_pack_ax_uint16( a, x )     ( ( (uint16_t)(x) << 8 ) | (uint16_t)(a) )

#define args_call_0( f ) do {       \
    f();                            \
} while( 0 )

#define args_call_1( f, a0 ) do {   \
    ARGS[0] = (a0);                 \
    f();                            \
} while( 0 )

#define args_call_2( f, a0, a1 ) do {   \
    ARGS[0] = (a0);                     \
    ARGS[1] = (a1);                     \
    f();                                \
} while( 0 )

#define args_call_3( f, a0, a1, a2 ) do {   \
    ARGS[0] = (a0);                         \
    ARGS[1] = (a1);                         \
    ARGS[2] = (a2);                         \
    f();                                    \
} while( 0 )

#define args_call_4( f, a0, a1, a2, a3 ) do {   \
    ARGS[0] = (a0);                             \
    ARGS[1] = (a1);                             \
    ARGS[2] = (a2);                             \
    ARGS[3] = (a3);                             \
    f();                                        \
} while( 0 )

#define args_call_4_3a_1p( f, a0, a1, a2, p0 ) do { \
    ARGS[0] = (a0);                                 \
    ARGS[1] = (a1);                                 \
    ARGS[2] = (a2);                                 \
    ARGS_PTR[0] = &(p0);                            \
    f();                                            \
} while( 0 )

#define args_call_4_3a_1s( f, a0, a1, a2, s0 ) do { \
    ARGS[0] = (a0);                                 \
    ARGS[1] = (a1);                                 \
    ARGS[2] = (a2);                                 \
    ARGS_UINT16[0] = (s0);                          \
    f();                                            \
} while( 0 )

#define args_call_4_3a_1i( f, a0, a1, a2, i0 ) do { \
    ARGS[0] = (a0);                                 \
    ARGS[1] = (a1);                                 \
    ARGS[2] = (a2);                                 \
    ARGS_UINT32 = (i0);                             \
    f();                                            \
} while( 0 )

#define lda_arg( a )    asm("lda %v,%b", ARGS, a)
#define ldx_arg( a )    asm("ldx %v,%b", ARGS, a)
#define ldy_arg( a )    asm("ldy %v,%b", ARGS, a)

#endif // ARGS_H
