#ifndef SUBPIXEL_H
#define SUBPIXEL_H

#include "types.h"

#define SUBPIXEL_MASK   (char)0x0F

typedef struct
{
    pixel_t pix;
    char sub;
} subpixel_t;
STATIC_ASSERT( sizeof( subpixel_t ) == sizeof( uint16_t ) );

#define subpixel_to_pixel( s )          (s).pix

// increases pixel if sub >= 8
#define subpixel_round_to_pixel( s )    ( (s).pix + ( 0x01 & ( (s).sub >> 3 ) ) )

#define subpixel_set( s, p, h )         (s).pix = (p); (s).sub = SUBPIXEL_MASK & (h)

#define subpixel_add( s, rh, lh )       (s).sub = (rh).sub + (lh).sub; (s).pix = (rh).pix + (lh).pix + ( (s).sub >> 4 ); (s).sub &= SUBPIXEL_MASK

#define subpixel_sub( s, rh, lh )       (s).sub = (rh).sub - (lh).sub; (s).pix = (rh).pix - (lh).pix - ( (s).sub >> 4 ); (s).sub &= SUBPIXEL_MASK

#endif // SUBPIXEL_H