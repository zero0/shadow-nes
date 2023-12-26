#ifndef ARGS_H
#define ARGS_H

extern uint8_t ARGS[8];
#pragma zpsym("ARGS");

#define args_pack_ax_uint16( a, x )     ( ( (uint16_t)(x) << 8 ) | (uint16_t)(a) )

#define args_call_0( f )    \
f()

#define args_call_1( f, p0 )    \
ARGS[0] = (p0);                 \
f()

#define args_call_2( f, p0, p1 )    \
ARGS[0] = (p0);                     \
ARGS[1] = (p1);                     \
f()

#define args_call_3( f, p0, p1, p2 )\
ARGS[0] = (p0);                     \
ARGS[1] = (p1);                     \
ARGS[2] = (p2);                     \
f()

#define args_call_4( f, p0, p1, p2, p3 )\
ARGS[0] = (p0);                     \
ARGS[1] = (p1);                     \
ARGS[2] = (p2);                     \
ARGS[3] = (p3);                     \
f()

#define lda_arg( a )                \
asm("lda %v,%b", ARGS, a)

#define ldx_arg( a )                \
asm("ldx %v,%b", ARGS, a)

#define ldy_arg( a )                \
asm("ldy %v,%b", ARGS, a)

#endif // ARGS_H
