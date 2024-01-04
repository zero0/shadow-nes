#ifndef SUBPIXEL_H
#define SUBPIXEL_H

#include "types.h"
#include "macros.h"

typedef union
{
    struct
    {
        pixel_t pix;
        uint8_t sub;
    };
    uint16_t v;
} subpixel_t;
STATIC_ASSERT( sizeof( subpixel_t ) == sizeof( uint16_t ) );

typedef union
{
    struct
    {
        int8_t pix;
        uint8_t sub;
    };
    int16_t v;
} subpixel_diff_t;
STATIC_ASSERT( sizeof( subpixel_diff_t ) == sizeof( int16_t ) );

// convert subpixel to pixel
#define subpixel_to_pixel( s )          (pixel_t)( (s).pix )

// increases pixel if sub >= 8
#define subpixel_round_to_pixel( s )    ( (s).pix + ( (s).sub & 0x80 ? 1 : 0 ) )

#define subpixel_make( p, pp )          (uint16_t)( ( (uint16_t)(p) << 8) | (uint16_t)(pp) )

// set pixel and subpixel
#define subpixel_set( s, p, pp )         (s).pix = (p); (s).sub = (pp)

// set subpixel to 0
#define subpixel_set_zero( s )          subpixel_set( s, 0, 0 )

#define subpixel_negate( s )            (s).pix ^= 0x80

#define subpixel_abs( s )               (s).pix &= 0x7F

// rh = lh
#define subpixel_move( rh, lh )         (rh).pix = (lh).pix; (rh).sub = (lh).sub

#define subpixel_cmp( rh, lh )          (int8_t)( (rh).pix == (lh).pix ? (rh).sub - (lh).sub : (rh).pix - (lh).pix )

#define subpixel_cmp_zero( rh )         (int8_t)( (rh).pix == 0 ? (int8_t)( (rh).sub ) : (int8_t)( (rh).pix ) )

#define subpixel_eq( rh, lh )           (int8_t)( (rh).v == (lh).v )

//
//
//

// add subpixels s = rh + lh (asm to use carry bit)
#define subpixel_add( s, rh, lh )       (s).v = (rh).v + (lh).v
//__asm__ ( "lda %v+1", rh );             \
//__asm__ ( "clc" );                      \
//__asm__ ( "adc %v+1", lh );             \
//__asm__ ( "sta %v+1", s );              \
//__asm__ ( "lda %v", rh );               \
//__asm__ ( "adc %v", lh );               \
//__asm__ ( "sta %v", s )

#define subpixel_add_pixel( s, rh, p )      (s).pix = (rh).pix + (p)
//__asm__ ( "lda %v", rh );               \
//__asm__ ( "adc %v", p );                \
//__asm__ ( "sta %v", s )

#define subpixel_add_direct( s, rh, p, pp )     (s).v = (rh).v + subpixel_make( p, pp )
//__asm__ ( "lda %v+1", rh );                 \
//__asm__ ( "clc" );                          \
//__asm__ ( "adc %v", pp );                   \
//__asm__ ( "sta %v+1", s );                  \
//__asm__ ( "lda %v", rh );                   \
//__asm__ ( "adc %v", p );                    \
//__asm__ ( "sta %v", s )


// equal to rh += lh;
#define subpixel_inc( rh, lh )              subpixel_add( rh, rh, lh )

#define subpixel_inc_pixel( rh, p )         subpixel_add_direct( rh, rh, p, 0 )
//__asm__ ( "lda %b", p );                    \
//__asm__ ( "clc" );                          \
//__asm__ ( "adc %v", rh )

#define subpixel_inc_direct( rh, p, pp )    subpixel_inc_direct( rh, rh, p, pp )
//__asm__ ( "lda %b", pp );                   \
//__asm__ ( "clc" );                          \
//__asm__ ( "adc %v+1", rh );                 \
//__asm__ ( "lda %b", p );                    \
//__asm__ ( "adc %v", rh );                   \

//
//
//

// subtract subpixels s = rh - lh (asm to use carry/negative bits)
#define subpixel_sub( s, rh, lh )   (s).v = (rh).v - (lh).v
//__asm__ ( "lda %v+1", rh );         \
//__asm__ ( "sec" );                  \
//__asm__ ( "sbc %v+1", lh );         \
//__asm__ ( "jsr %v", negate );       \
//__asm__ ( "sta %v+1", s );          \
//__asm__ ( "lda %v", rh );           \
//__asm__ ( "sbc %v", lh );           \
//__asm__ ( "sta %v", s )

#define subpixel_sub_direct( s, rh, p, pp )     (s).v = (rh).v - subpixel_make( p, pp )

#define subpixel_sub_pixel( s, rh, p )          subpixel_sub_direct( s, rh, p, 0 )
//__asm__ ( "lda %v", rh );               \
//__asm__ ( "sec" );                      \
//__asm__ ( "sbc %v", p );                \
//__asm__ ( "sta %v", s )

// equal to rh -= lh
#define subpixel_dec( rh, lh )          subpixel_sub( rh, rh, lh )

#define subpixel_dec_pixel( rh, p )     subpixel_sub_pixel( rh, rh, p )

#endif // SUBPIXEL_H