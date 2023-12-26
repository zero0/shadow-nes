#include "text.h"
#include "ppu.h"
#include "subpixel.h"

static char c;
static uint8_t i, x, y;

#define CHAR_SPACE          72
#define CHAR_NEW_LINE       73
#define CHAR_CARAGE_RETURN  74
#define CHAR_TAB            75

void __fastcall__ text_draw_string_impl(void)
{
    text_begin_str_at(ARGS[3]);

    x = ARGS[0];
    y = ARGS[1];
    i = 0;
    for( ; ; ++i, ++x )
    {
        c = text_str_at( i );
        if( c == '\0' ) break;
        if( c == CHAR_SPACE ) continue;
        if( c == CHAR_NEW_LINE )
        {
            x = ARGS[0];
            ++y;
        }
        if( c == CHAR_TAB )
        {
            x += 3;
            continue;
        }

        ppu_update_tile( x, y, c );
    }
}
