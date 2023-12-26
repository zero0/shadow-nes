#ifndef SUBPIXEL_H
#define SUBPIXEL_H

#include "types.h"
#include "macros.h"

typedef struct
{
    pixel_t pix;
    uint8_t sub;
} subpixel_t;
STATIC_ASSERT( sizeof( subpixel_t ) == sizeof( uint16_t ) );

typedef struct
{
    int8_t pix;
    uint8_t sub;
} subpixel_diff_t;
STATIC_ASSERT( sizeof( subpixel_diff_t ) == sizeof( uint16_t ) );

// make scroll from a subpixel
#define make_scroll_subpixel( sx, sy )  make_scroll( subpixel_to_pixel(sx), subpixel_to_pixel(sy) )

// convert subpixel to pixel
#define subpixel_to_pixel( s )          (pixel_t)( (s).pix )

// increases pixel if sub >= 8
#define subpixel_round_to_pixel( s )    ( (s).pix + ( (s).sub >=  ? 1 : 0 ) )

// set pixel and subpixel
#define subpixel_set( s, p, h )         (s).pix = (p); (s).sub = (h)

// set subpixel to 0
#define subpixel_set_zero( s )          subpixel_set( s, 0, 0 )

#define subpixel_negate( s )            (s).pix ^= 0x80

// rh = lh
#define subpixel_move( rh, lh )         (rh).pix = (lh).pix; (rh).sub = (lh).sub

#define subpixel_cmp( rh, lh )          (int8_t)( (rh).pix == (lh).pix ? (rh).sub - (lh).sub : (rh).pix - (lh).pix )

#define subpixel_cmp_zero( rh )         (int8_t)( (rh).pix == 0 ? (rh).sub : (rh).pix )

#define subpixel_eq( rh, lh )           ( subpixel_cmp( rh, lh ) == 0 )

//
//
//

// add subpixels s = rh + lh (asm to use carry bit)
#define subpixel_add( s, rh, lh )       \
__asm__ ( "lda %v+1", rh );             \
__asm__ ( "clc" );                      \
__asm__ ( "adc %v+1", lh );             \
__asm__ ( "sta %v+1", s );              \
__asm__ ( "lda %v", rh );               \
__asm__ ( "adc %v", lh );               \
__asm__ ( "sta %v", s )

#define subpixel_add_pixel( s, rh, p )  \
__asm__ ( "lda %v", rh );               \
__asm__ ( "adc %v", p );                \
__asm__ ( "sta %v", s )

#define subpixel_add_direct( s, rh, p, pp ) \
__asm__ ( "lda %v+1", rh );                 \
__asm__ ( "clc" );                          \
__asm__ ( "adc %v", pp );                   \
__asm__ ( "sta %v+1", s );                  \
__asm__ ( "lda %v", rh );                   \
__asm__ ( "adc %v", p );                    \
__asm__ ( "sta %v", s )


// equal to rh += lh;
#define subpixel_inc( rh, lh )              subpixel_add( rh, rh, lh )

#define subpixel_inc_pixel( rh, p )         \
__asm__ ( "lda %b", p );                    \
__asm__ ( "clc" );                          \
__asm__ ( "adc %v", rh )

#define subpixel_inc_direct( rh, p, pp )    \
__asm__ ( "lda %b", pp );                   \
__asm__ ( "clc" );                          \
__asm__ ( "adc %v+1", rh );                 \
__asm__ ( "lda %b", p );                    \
__asm__ ( "adc %v", rh );                   \

//
//
//

static void __fastcall__ negate()
{
    __asm__ ( "bpl %g", pos );
    __asm__ ( "eor #$FF" );
    __asm__ ( "adc #1" );
pos:
    return;
}

// subtract subpixels s = rh - lh (asm to use carry/negative bits)
#define subpixel_sub( s, rh, lh )   \
__asm__ ( "lda %v+1", rh );         \
__asm__ ( "sec" );                  \
__asm__ ( "sbc %v+1", lh );         \
__asm__ ( "jsr %v", negate );       \
__asm__ ( "sta %v+1", s );          \
__asm__ ( "lda %v", rh );           \
__asm__ ( "sbc %v", lh );           \
__asm__ ( "sta %v", s )

#define subpixel_sub_pixel( s, rh, p )  \
__asm__ ( "lda %v", rh );               \
__asm__ ( "sec" );                      \
__asm__ ( "sbc %v", p );                \
__asm__ ( "sta %v", s )

// equal to rh -= lh
#define subpixel_dec( rh, lh )          subpixel_sub( rh, rh, lh )

#define subpixel_dec_pixel( rh, p )     subpixel_sub_pixel( rh, rh, p )

#endif // SUBPIXEL_H