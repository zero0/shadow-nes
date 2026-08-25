#include "text.h"
#include "ppu.h"
#include "globals.h"
#include "gametext.h"

extern uint8_t text_delay_position;
#pragma zpsym("text_delay_position");

extern uint8_t text_delay_start_x;
#pragma zpsym("text_delay_start_x");

extern uint8_t text_delay_start_y;
#pragma zpsym("text_delay_start_y")

extern uint8_t text_delay_x;
#pragma zpsym("text_delay_x");

extern uint8_t text_delay_y;
#pragma zpsym("text_delay_y")

extern uint8_t* text_delay_c_ptr;
#pragma zpsym("text_delay_c_ptr");

#define EMPTY_TILE          0xFF
#define TAB_COUNT           3

// clear a block for text
//  ARGS[0] = x
//  ARGS[1] = y
//  ARGS[2] = count
void __fastcall__ text_clear_impl(void)
{
    ppu_begin_tile_batch( ARGS[0], ARGS[1] );

    ppu_push_repeat_tile_batch( EMPTY_TILE, ARGS[2] );

    ppu_end_tile_batch();
}

// draw string
//  ARGS[0] = x
//  ARGS[1] = y
//  ARGS[2] = palette (TODO: update to allow for palette changes)
//  ARGS_PTR[0] = string ptr
void __fastcall__ text_draw_string_impl(void)
{
    x = ARGS[0];
    y = ARGS[1];
    ppu_begin_tile_batch( x, y );

    c_ptr = ARGS_PTR[0];

    for( i = 0, j = 1, imax = c_ptr[0]; i != imax; ++i, ++j )
    {
        c = c_ptr[j];
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
            ++y;

            ppu_begin_tile_batch( x, y );
            continue;
        }
        if( c == FONT_CHAR_TAB )
        {
#if TAB_COUNT > 1
            ppu_push_tile_batch(EMPTY_TILE); // push empty tile
#endif // TAB_COUNT > 1
#if TAB_COUNT > 2
            ppu_push_tile_batch(EMPTY_TILE); // push empty tile
#endif // TAB_COUNT > 2
#if TAB_COUNT > 3
            ppu_push_tile_batch(EMPTY_TILE); // push empty tile
#endif // TAB_COUNT > 3
#if TAB_COUNT > 4
            ppu_push_tile_batch(EMPTY_TILE); // push empty tile
#endif // TAB_COUNT > 4

            x += TAB_COUNT;
            continue;
        }

        ppu_push_tile_batch( c );
        ++x;
    }

    ppu_end_tile_batch();
}

#define MAX_DELAY_CHAR_COUNT    (2)

// draw string
//  ARGS[0] = x
//  ARGS[1] = y
//  ARGS[2] = palette (TODO: update to allow for palette changes)
//  ARGS_PTR[0] = string ptr
void __fastcall__ text_delay_start_impl(void)
{
    text_delay_start_x = text_delay_x = ARGS[0];
    text_delay_start_y = text_delay_y = ARGS[1];
    text_delay_c_ptr = ARGS_PTR[0];
    text_delay_position = 0;
}

// advance delay string
void __fastcall__ text_delay_advance_impl(void)
{
    if(text_delay_position == 0xFF)
    {
        return;
    }

    imax = text_delay_c_ptr[0];
    if(text_delay_position >= imax)
    {
        text_delay_position = 0xFF;
        return;
    }

    x = text_delay_x;
    y = text_delay_y;
    i = j = text_delay_position;
    ++j;
    w = 0;

    ppu_begin_tile_batch( x, y );

    for( ; i != imax && w < ARGS[0]; ++i, ++j )
    {
        c = text_delay_c_ptr[j];
        if( c == FONT_CHAR_SPACE )
        {
            c = EMPTY_TILE;
        }
        else if( c == FONT_CHAR_NEW_LINE )
        {
            ppu_end_tile_batch();

            x = text_delay_start_x;
            ++y;

            ppu_begin_tile_batch( x, y );
            continue;
        }
        else if( c == FONT_CHAR_CARAGE_RETURN )
        {
            ppu_end_tile_batch();

            x = text_delay_start_x;
            ++y;

            ppu_begin_tile_batch( x, y );
            continue;
        }
        else if( c == FONT_CHAR_TAB )
        {
#if TAB_COUNT > 0
            ppu_push_tile_batch(EMPTY_TILE); // push empty tile
#endif // TAB_COUNT > 0
#if TAB_COUNT > 1
            ppu_push_tile_batch(EMPTY_TILE); // push empty tile
#endif // TAB_COUNT > 1
#if TAB_COUNT > 2
            ppu_push_tile_batch(EMPTY_TILE); // push empty tile
#endif // TAB_COUNT > 2
#if TAB_COUNT > 3
            ppu_push_tile_batch(EMPTY_TILE); // push empty tile
#endif // TAB_COUNT > 3

            x += TAB_COUNT;
            continue;
        }

        ppu_push_tile_batch( c );
        ++x;
        ++w;
    }

    ppu_end_tile_batch();

    // store updated text delay values
    text_delay_x = x;
    text_delay_y = y;
    text_delay_position = i;
}

void __fastcall__ text_delay_display_full(void)
{
    text_delay_advance(0xFF);
}

extern uint8_t bcd[5];
extern uint8_t itoa_input[4];
#pragma zpsym("itoa_input")

#define ITOA_START_LEFT_JUSTIFIED(i) do {                       \
if (1 || (bcd[i] & 0x0F) > 0) ppu_push_tile_batch((bcd[i] & 0x0F));        \
} while( 0 )

#define ITOA_PART_LEFT_JUSTIFIED(i) do {                        \
if (1 || (bcd[i] & 0xF0) > 0) ppu_push_tile_batch((bcd[i] >> 4));          \
if (1 || (bcd[i] & 0x0F) > 0) ppu_push_tile_batch((bcd[i] & 0x0F));        \
} while( 0 )

#define ITOA_END_LEFT_JUSTIFIED(i) do {                         \
if (1 || (bcd[i] & 0xF0) > 0) ppu_push_tile_batch((bcd[i] >> 4));          \
ppu_push_tile_batch((bcd[i] & 0x0F));                           \
} while( 0 )

extern void __fastcall__ itoa_uint8_impl(void);
extern void __fastcall__ itoa_uint16_impl(void);
extern void __fastcall__ itoa_uint32_impl(void);

// draw an 8bit number in base 10 [0..255]
//  ARGS[0] = x
//  ARGS[1] = y
//  ARGS[2] = palette (TODO: implement)
//  ARGS[3] = 8bit number
void __fastcall__ text_draw_uint8_impl(void)
{
    itoa_input[0] = ARGS[3];

    itoa_uint8_impl();

    ppu_begin_tile_batch(ARGS[0], ARGS[1]);
    ITOA_START_LEFT_JUSTIFIED(1);
    ITOA_END_LEFT_JUSTIFIED(0);
    ppu_end_tile_batch();
}

// draw a 16bit number in base 10 [0..65535]
//  ARGS[0] = x
//  ARGS[1] = y
//  ARGS[2] = palette (TODO: implement)
//  ARGS_UINT16[0] = 16bit number
void __fastcall__ text_draw_uint16_impl(void)
{
    itoa_input[0] = (uint8_t)(ARGS_UINT16[0] >> 8);
    itoa_input[1] = (uint8_t)(ARGS_UINT16[0]);

    itoa_uint16_impl();

    ppu_begin_tile_batch(ARGS[0], ARGS[1]);
    ITOA_START_LEFT_JUSTIFIED(2);
    ITOA_PART_LEFT_JUSTIFIED(1);
    ITOA_END_LEFT_JUSTIFIED(0);
    ppu_end_tile_batch();
}

// draw a 32bit number in base 10 [0..4294967295]
//  ARGS[0] = x
//  ARGS[1] = y
//  ARGS[2] = palette (TODO: implement)
//  ARGS_UINT32 = 32bit number
void __fastcall__ text_draw_uint32_impl(void)
{
    itoa_input[0] = (uint8_t)(ARGS_UINT32 >> 24);
    itoa_input[1] = (uint8_t)(ARGS_UINT32 >> 16);
    itoa_input[2] = (uint8_t)(ARGS_UINT32 >> 8);
    itoa_input[3] = (uint8_t)(ARGS_UINT32);

    itoa_uint32_impl();

    ppu_begin_tile_batch(ARGS[0], ARGS[1]);
    ITOA_PART_LEFT_JUSTIFIED(4);
    ITOA_PART_LEFT_JUSTIFIED(3);
    ITOA_PART_LEFT_JUSTIFIED(2);
    ITOA_PART_LEFT_JUSTIFIED(1);
    ITOA_END_LEFT_JUSTIFIED(0);
    ppu_end_tile_batch();
}

// draw an 8bit number in base 16 [00..FF]
//  ARGS[0] = x
//  ARGS[1] = y
//  ARGS[2] = palette (TODO: implement)
//  ARGS[3] = 8bit number
void __fastcall__ text_draw_uint8_x2_impl(void)
{
    ppu_begin_tile_batch(ARGS[0], ARGS[1]);
    ppu_push_tile_batch(0x0F & (ARGS[3] >> 4));
    ppu_push_tile_batch(0x0F & (ARGS[3] >> 0));
    ppu_end_tile_batch();
}

// draw a 16bit number in base 16 [0000..FFFF]
//  ARGS[0] = x
//  ARGS[1] = y
//  ARGS[2] = palette (TODO: implement)
//  ARGS_UINT16[3] = 8bit number
void __fastcall__ text_draw_uint16_x2_impl(void)
{
    ppu_begin_tile_batch(ARGS[0], ARGS[1]);
    ppu_push_tile_batch(0x0F & (ARGS_UINT16[0] >> 12));
    ppu_push_tile_batch(0x0F & (ARGS_UINT16[0] >> 8));
    ppu_push_tile_batch(0x0F & (ARGS_UINT16[0] >> 4));
    ppu_push_tile_batch(0x0F & (ARGS_UINT16[0] >> 0));
    ppu_end_tile_batch();
}

// draw a 32bit number in base 16 [00000000..FFFFFFFF]
//  ARGS[0] = x
//  ARGS[1] = y
//  ARGS[2] = palette (TODO: implement)
//  ARGS_UINT16[3] = 8bit number
void __fastcall__ text_draw_uint32_x2_impl(void)
{
    ppu_begin_tile_batch(ARGS[0], ARGS[1]);
    ppu_push_tile_batch(0x0F & (ARGS_UINT32 >> 28));
    ppu_push_tile_batch(0x0F & (ARGS_UINT32 >> 24));
    ppu_push_tile_batch(0x0F & (ARGS_UINT32 >> 20));
    ppu_push_tile_batch(0x0F & (ARGS_UINT32 >> 16));
    ppu_push_tile_batch(0x0F & (ARGS_UINT32 >> 12));
    ppu_push_tile_batch(0x0F & (ARGS_UINT32 >> 8));
    ppu_push_tile_batch(0x0F & (ARGS_UINT32 >> 4));
    ppu_push_tile_batch(0x0F & (ARGS_UINT32 >> 0));
    ppu_end_tile_batch();
}