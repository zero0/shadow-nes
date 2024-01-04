#include "text.h"
#include "ppu.h"
#include "subpixel.h"
#include "globals.h"

#define EMPTY_TILE          0xFF

#define CHAR_SPACE          77
#define CHAR_NEW_LINE       78
#define CHAR_CARAGE_RETURN  79
#define CHAR_TAB            80

void __fastcall__ text_draw_string_impl(void)
{
    text_begin_str_at(ARGS[3]);

    x = ARGS[0];
    y = ARGS[1];
    ppu_begin_tile_batch( x, y );

    i = 0;
    for( ; ; ++i )
    {
        c = text_str_at( i );
        if( c == '\0' ) break;
        if( c == CHAR_SPACE )
        {
            ppu_push_tile_batch(EMPTY_TILE); // push empty tile
            ++x;
            continue;
        }
        if( c == CHAR_NEW_LINE )
        {
            ppu_end_tile_batch();

            x = ARGS[0];
            ++y;

            ppu_begin_tile_batch( x, y );
            continue;
        }
        if( c == CHAR_CARAGE_RETURN )
        {
            ppu_end_tile_batch();

            x = ARGS[0];
            y = ARGS[1];

            ppu_begin_tile_batch( x, y );
            continue;
        }
        if( c == CHAR_TAB )
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

void __fastcall__ text_draw_uint8_impl(void)
{
    x = ARGS[0];
    y = ARGS[1];
    i = ARGS[3];

    if( i >= 10 ) ++x;
    if( i >= 100 ) ++x;

    c = i % 10;
    ppu_update_tile( x, y, c );

    i /= 10;
    if( i )
    {
        --x;
        c = i % 10;
        ppu_update_tile( x, y, c );

        i /= 10;
        if( i )
        {
            --x;
            c = i % 10;
            ppu_update_tile( x, y, c );
        }
    }
}

void __fastcall__ text_draw_uint8_x2_impl(void)
{
    ppu_begin_tile_batch(ARGS[0], ARGS[1]);
    ppu_push_tile_batch(0x0F & (ARGS[3] >> 4));
    ppu_push_tile_batch(0x0F & (ARGS[3] >> 0));
    ppu_end_tile_batch();
}