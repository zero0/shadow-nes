#include "text.h"
#include "ppu.h"
#include "subpixel.h"

static char c;
static uint8_t i, x, y;

#define CHAR_SPACE          76
#define CHAR_NEW_LINE       77
#define CHAR_CARAGE_RETURN  78
#define CHAR_TAB            79

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
            ppu_push_tile_batch(0); // push empty tile
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
            ppu_push_tile_batch(0); // push empty tile
            ppu_push_tile_batch(0); // push empty tile
            ppu_push_tile_batch(0); // push empty tile
            x += 3;
            continue;
        }

        ppu_push_tile_batch( c );
        ++x;
    }

    ppu_end_tile_batch();
}
