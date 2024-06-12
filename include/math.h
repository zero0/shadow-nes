#ifndef MATH_H
#define MATH_H

#include "macros.h"

// equal to lh = max( 0, lh - rh );
#define sub_clamp0_uint8( lh, rh )      \
do                                      \
{                                       \
    __asm__("lda %v", lh);              \
    __asm__("sec");                     \
    __asm__("sbc %v", rh);              \
    __asm__("bcc @store_%s", __LINE__); \
    __asm__("lda #0");                  \
    __asm__("@store_%s:", __LINE__);    \
    __asm__("sta %v", lh);              \
} while( 0 )

// equal to lh = max( 0, lh - (rh+o) );
#define sub_clamp0_uint8_o( lh, rh, o ) \
do                                      \
{                                       \
    __asm__("lda %v", lh);              \
    __asm__("sec");                     \
    __asm__("sbc %v+%b", rh, o);        \
    __asm__("bcc @store_%s", __LINE__); \
    __asm__("lda #0");                  \
    __asm__("@store_%s:", __LINE__);    \
    __asm__("sta %v", lh);              \
} while( 0 )

//
//
//

extern void rand_seed8(uint8_t seed);

extern void rand_seed_mask8(uint8_t seed, uint8_t mask);

extern uint8_t rand_next8(void);

#endif // MATH_H
