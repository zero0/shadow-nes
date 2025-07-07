#include "text.h"
#include "ppu.h"
#include "globals.h"
#include "gametext.h"

#define EMPTY_TILE          0xFF



// draw string directly to nametable while nmi is off
void __fastcall__ text_draw_string_direct_impl(void)
{
    x = ARGS[0];
    y = ARGS[1];

    text_begin_str_at(ARGS[3]);
    for( i = 0, imax = text_strlen() ; i < imax ; ++i )
    {
        c = text_str_at( i );
        if( c == FONT_CHAR_SPACE )
        {
            ppu_push_tile_batch(EMPTY_TILE); // push empty tile
            ++x;
            continue;
        }
        if( c == FONT_CHAR_NEW_LINE )
        {
            ppu_end_tile_batch();

            x = ARGS[0];
            ++y;

            ppu_begin_tile_batch( x, y );
            continue;
        }
        if( c == FONT_CHAR_CARAGE_RETURN )
        {
            ppu_end_tile_batch();

            x = ARGS[0];
            y = ARGS[1];
            ++y;

            ppu_begin_tile_batch( x, y );
            continue;
        }
        if( c == FONT_CHAR_TAB )
        {
            ppu_push_tile_batch(EMPTY_TILE); // push empty tile
            ppu_push_tile_batch(EMPTY_TILE); // push empty tile
            ppu_push_tile_batch(EMPTY_TILE); // push empty tile
            x += 3;
            continue;
        }

        ppu_push_tile_batch( c );
        ++x;
    }
}

void __fastcall__ text_draw_string_impl(void)
{
    x = ARGS[0];
    y = ARGS[1];
    ppu_begin_tile_batch( x, y );

    text_begin_str_at(ARGS[3]);
    for( i = 0, imax = text_strlen() ; i < imax ; ++i )
    {
        c = text_str_at( i );
        if( c == FONT_CHAR_SPACE )
        {
            ppu_push_tile_batch(EMPTY_TILE); // push empty tile
            ++x;
            continue;
        }
        if( c == FONT_CHAR_NEW_LINE )
        {
            ppu_end_tile_batch();

            x = ARGS[0];
            ++y;

            ppu_begin_tile_batch( x, y );
            continue;
        }
        if( c == FONT_CHAR_CARAGE_RETURN )
        {
            ppu_end_tile_batch();

            x = ARGS[0];
            y = ARGS[1];
            ++y;

            ppu_begin_tile_batch( x, y );
            continue;
        }
        if( c == FONT_CHAR_TAB )
        {
            ppu_push_tile_batch(EMPTY_TILE); // push empty tile
            ppu_push_tile_batch(EMPTY_TILE); // push empty tile
            ppu_push_tile_batch(EMPTY_TILE); // push empty tile
            x += 3;
            continue;
        }

        ppu_push_tile_batch( c );
        ++x;
    }

    ppu_end_tile_batch();
}

#define BDC_ONES_INDEX              (uint8_t)0
#define BDC_TENS_INDEX              (uint8_t)0
#define BDC_HUNDREDS_INDEX          (uint8_t)1
#define BDC_THOUSANDS_INDEX         (uint8_t)1
#define BDC_TEN_THOUSANDS_INDEX     (uint8_t)2

#define BDC_ONES_MASK(v)            (uint8_t)( (v)[BDC_ONES_INDEX           ] & 0x0F )
#define BDC_TENS_MASK(v)            (uint8_t)( (v)[BDC_TENS_INDEX           ] & 0xF0 )
#define BDC_HUNDREDS_MASK(v)        (uint8_t)( (v)[BDC_HUNDREDS_INDEX       ] & 0x0F )
#define BDC_THOUSANDS_MASK(v)       (uint8_t)( (v)[BDC_THOUSANDS_INDEX      ] & 0xF0 )
#define BDC_TEN_THOUSANDS_MASK(v)   (uint8_t)( (v)[BDC_TEN_THOUSANDS_INDEX  ] & 0x0F )

#define BDC_ONES_VALUE(v)           (uint8_t)( ( (v)[BDC_ONES_INDEX           ] & 0x0F ) >> 0 )
#define BDC_TENS_VALUE(v)           (uint8_t)( ( (v)[BDC_TENS_INDEX           ] & 0xF0 ) >> 4 )
#define BDC_HUNDREDS_VALUE(v)       (uint8_t)( ( (v)[BDC_HUNDREDS_INDEX       ] & 0x0F ) >> 0 )
#define BDC_THOUSANDS_VALUE(v)      (uint8_t)( ( (v)[BDC_THOUSANDS_INDEX      ] & 0xF0 ) >> 4 )
#define BDC_TEN_THOUSANDS_VALUE(v)  (uint8_t)( ( (v)[BDC_TEN_THOUSANDS_INDEX  ] & 0x0F ) >> 0 )

#define BDC_ONES_SHIFT(v)           (uint8_t)( ( (v) << 0 ) & 0x0F )
#define BDC_TENS_SHIFT(v)           (uint8_t)( ( (v) << 4 ) & 0xF0 )
#define BDC_HUNDREDS_SHIFT(v)       BDC_ONES_SHIFT(v)
#define BDC_THOUSANDS_SHIFT(v)      BDC_TENS_SHIFT(v)
#define BDC_TEN_THOUSANDS_SHIFT(v)  BDC_ONES_SHIFT(v)

static uint8_t _bcd[5];

void __fastcall__ text_draw_uint8_impl(void)
{
    x = ARGS[0];
    y = ARGS[1];
    c = ARGS[3];

    _bcd[0] = 0;
    _bcd[1] = 0;

    // Double Dabble algorithm
    for( i = 0; i < 8; ++i)
    {
        if( BDC_ONES_MASK(_bcd) >= BDC_ONES_SHIFT(5) )
        {
            _bcd[BDC_ONES_INDEX] += BDC_ONES_SHIFT(3);
        }
        if( BDC_TENS_MASK(_bcd) >= BDC_TENS_SHIFT(5) )
        {
            _bcd[BDC_TENS_INDEX] += BDC_TENS_SHIFT(3);
        }
        if( BDC_HUNDREDS_MASK(_bcd) >= BDC_HUNDREDS_SHIFT(5) )
        {
            _bcd[BDC_HUNDREDS_INDEX] += BDC_HUNDREDS_SHIFT(3);
        }

        // clear carry
        __asm__("clc");

        // roll c -> carry
        __asm__("rol %v", c);

        // roll carry to ones -> tens -> hundreds
        __asm__("rol %v+0", _bcd);
        __asm__("rol %v+1", _bcd);
    }

    ppu_begin_tile_batch(ARGS[0], ARGS[1]);
    if( BDC_HUNDREDS_VALUE(_bcd) > 0 )
    {
        ppu_push_tile_batch(BDC_HUNDREDS_VALUE(_bcd));
    }
    if( BDC_TENS_VALUE(_bcd) > 0 )
    {
        ppu_push_tile_batch(BDC_TENS_VALUE(_bcd));
    }
    ppu_push_tile_batch(BDC_ONES_VALUE(_bcd));
    ppu_end_tile_batch();
}

void __fastcall__ text_draw_uint8_x2_impl(void)
{
    ppu_begin_tile_batch(ARGS[0], ARGS[1]);
    ppu_push_tile_batch(0x0F & (ARGS[3] >> 4));
    ppu_push_tile_batch(0x0F & (ARGS[3] >> 0));
    ppu_end_tile_batch();
}