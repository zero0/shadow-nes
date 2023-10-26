#ifndef SUBPIXEL_H
#define SUBPIXEL_H

#include "types.h"

typedef struct
{
    pixel_t pix;
    uint8_t sub;
} subpixel_t;
STATIC_ASSERT( sizeof( subpixel_t ) == sizeof( uint16_t ) );

#define subpixel_to_pixel( s )          (s).pix

// increases pixel if sub >= 8
#define subpixel_round_to_pixel( s )    ( (s).pix + ( (s).sub >=  ? 1 : 0 ) )

#define subpixel_set( s, p, h )         (s).pix = (p); (s).sub = (h)

// add subpixels s = rh + lh (asm to use carry bit)
#define subpixel_add( s, rh, lh )       \
__asm__ ( "lda %v+%b", rh, 1 ); \
__asm__ ( "clc" );              \
__asm__ ( "adc %v+%b", lh, 1 ); \
__asm__ ( "sta %v+%b", s, 1 );  \
__asm__ ( "lda %v", rh );       \
__asm__ ( "adc %v", lh );       \
__asm__ ( "sta %v", s )

// equal to rh += lh;
#define subpixel_inc( rh, lh )          subpixel_add( rh, rh, lh )

static void __fastcall__ negate()
{
    __asm__ ( "bpl %g", neg);
    __asm__ ( "eor #$FF" );
    __asm__ ( "adc #1" );
neg:
    return;
}

// subtract subpixels s = rh - lh (asm to use carry/negative bits)
#define subpixel_sub( s, rh, lh )       \
__asm__ ( "lda %v+%b", rh, 1 ); \
__asm__ ( "clc" );              \
__asm__ ( "sbc %v+%b", lh, 1 ); \
__asm__ ( "jsr %v", negate );   \
__asm__ ( "sta %v+%b", s, 1 );  \
__asm__ ( "lda %v", rh );       \
__asm__ ( "sbc %v", lh );       \
__asm__ ( "sta %v", s )

//__asm__ ( "bpl %g", neg ); \
//__asm__ ( "  eor #$FF" ); \
//__asm__ ( "  inc" ); \
//neg: \

// equal to rh -= lh
//#define subpixel_inc( rh, lh )          subpixel_sub( rh, rh, lh )

#endif // SUBPIXEL_H