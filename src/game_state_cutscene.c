#include "types.h"
#include "macros.h"
#include "globals.h"
#include "text.h"
#include "gamepad.h"
#include "ppu.h"
#include "timer.h"
#include "game_state.h"
#include "game_flow.h"

enum
{
    HOLD_TO_SKIP_ENTIRE_CUTSCENE_TIME_FRAMES = 2 * 60,
    MAX_CUTSCENE_TEXTS = 4,
    END_CUTSCENE = 0xFF,
};

enum
{
    CUTSCENE_TYPE_TEXT,
    CUTSCENE_TYPE_DIALOG,
};

enum
{
    CUTSCENE_V_ALIGN_MIDDLE,
    CUTSCENE_V_ALIGN_TOP,
    CUTSCENE_V_ALIGN_BOTTOM,
};

#define MAKE_CUTSCENE_ATTR( t, a, c )                   (uint8_t)( ( (t) & 0x03 ) << 6 ) | ( ( (a) & 0x03 ) << 4 ) | ( ((c - 1) & 0x03) << 2 ) // need to subtract 1 since it's base 0
#define MAKE_CUTSCENE_PALETTE_4( p0, p1, p2, p3 )       (uint8_t)( ( (p3) & 0x03 ) << 6 | ( (p2) & 0x03 ) << 4 | ( (p1) & 0x03 ) << 2 | ( (p0) & 0x03 ) )
#define MAKE_CUTSCENE_PALETTE_2_DIALOG( c, d )          MAKE_CUTSCENE_PALETTE_4( c, d, c, d )
#define MAKE_CUTSCENE_PALETTE( p )                      MAKE_CUTSCENE_PALETTE_4( p, p, p, p )

#define GET_CUTSCENE_ATTR_TYPE( a )                     (uint8_t)( ( (a) >> 6 ) & 0x03 )
#define GET_CUTSCENE_ATTR_ALIGNMENT( a )                (uint8_t)( ( (a) >> 4 ) & 0x03 )
#define GET_CUTSCENE_ATTR_COUNT( a )                    (uint8_t)( ( ( (a) >> 2 ) & 0x03 ) + 1 ) // need to add 1 since it's base 0

#define GET_CUTSCENE_PALLETE( p, i )                    (uint8_t)( ( (p) >> ( (i) << 2 ) ) & 0x03 )

typedef struct
{
    uint8_t attr;
    uint8_t palettes;
    str_t t[MAX_CUTSCENE_TEXTS];
} cutscene_desc_t;

static const cutscene_desc_t all_cutscenes[] = {
    { MAKE_CUTSCENE_ATTR( CUTSCENE_TYPE_TEXT, CUTSCENE_V_ALIGN_MIDDLE, 2 ), MAKE_CUTSCENE_PALETTE( PALETTE_BACKGROUND_0 ), tr_cutscene_intro_0, tr_cutscene_intro_1, tr_cutscene_intro_2, tr_cutscene_intro_3 },

    { MAKE_CUTSCENE_ATTR( CUTSCENE_TYPE_TEXT, CUTSCENE_V_ALIGN_MIDDLE, 2 ), MAKE_CUTSCENE_PALETTE( PALETTE_BACKGROUND_0 ), tr_cutscene_intro_0, tr_cutscene_intro_1, tr_cutscene_intro_2, tr_cutscene_intro_3 },
    { MAKE_CUTSCENE_ATTR( CUTSCENE_TYPE_TEXT, CUTSCENE_V_ALIGN_MIDDLE, 2 ), MAKE_CUTSCENE_PALETTE( PALETTE_BACKGROUND_0 ), tr_cutscene_intro_0, tr_cutscene_intro_1, tr_cutscene_intro_2, tr_cutscene_intro_3 },

    { MAKE_CUTSCENE_ATTR( CUTSCENE_TYPE_TEXT, CUTSCENE_V_ALIGN_MIDDLE, 3 ), MAKE_CUTSCENE_PALETTE( PALETTE_BACKGROUND_0 ), tr_cutscene_intro_0, tr_cutscene_intro_1, tr_cutscene_intro_2, tr_cutscene_intro_3 },
};
STATIC_ASSERT(ARRAY_SIZE(all_cutscenes) == _CUTSCENE_COUNT);

//
//
//

extern ptr_t shadow_font;

static uint8_t current_cutscene_index;

static void __fastcall__ draw_cutscene_part(void)
{
    // TODO: maybe fade out/in while clearing
    ppu_disable();

    {
        ppu_set_scroll( 0, 0 );
        ppu_clear_palette();
        ppu_clear_oam();

        //ppu_upload_chr_ram( shadow_font, MAKE_CHR_PTR(0,0,0), 16*4+13 );

        ppu_set_palette_background( 0x0F );
        ppu_set_palette( PALETTE_BACKGROUND_0, 0x15, 0x26, 0x37 );
        ppu_set_palette( PALETTE_BACKGROUND_1, 0x05, 0x15, 0x30 ); // red, light red, white
        ppu_set_palette( PALETTE_BACKGROUND_2, 0x1A, 0x2A, 0x30 ); // green, light green, white
        ppu_set_palette( PALETTE_SPRITE_0, 0x0A, 0x1A, 0x2A );
        ppu_set_palette( PALETTE_SPRITE_1, 0x0A, 0x1A, 0x2A );
        ppu_set_palette( PALETTE_SPRITE_2, 0x0A, 0x1A, 0x2A );

        // draw
        switch( GET_CUTSCENE_ATTR_TYPE( all_cutscenes[ current_cutscene_index ].attr ) )
        {
            case CUTSCENE_TYPE_TEXT:
                text_draw_string( 0, 0, GET_CUTSCENE_PALLETE( all_cutscenes[ current_cutscene_index ].palettes, game_state_internal ), all_cutscenes[ current_cutscene_index ].t[ game_state_internal ] );
                break;

            case CUTSCENE_TYPE_DIALOG:
                text_draw_string( 0, 0, GET_CUTSCENE_PALLETE( all_cutscenes[ current_cutscene_index ].palettes, game_state_internal + 0 ), all_cutscenes[ current_cutscene_index ].t[ game_state_internal + 0 ] );
                text_draw_string( 0, 2, GET_CUTSCENE_PALLETE( all_cutscenes[ current_cutscene_index ].palettes, game_state_internal + 1 ), all_cutscenes[ current_cutscene_index ].t[ game_state_internal + 1 ] );
                break;

            default:
                INVALID_CODE_PATH;
                break;
        }
    }

    ppu_enable();
}

void __fastcall__ game_state_cutscene_enter(void)
{
    current_cutscene_index = next_game_state_arg;

    r = HOLD_TO_SKIP_ENTIRE_CUTSCENE_TIME_FRAMES;
    game_state_internal = 0;
    timer_set( game_state_timer, 10 );

    // draw the first cutscene
    draw_cutscene_part();
}

void __fastcall__ game_state_cutscene_leave(void)
{
}

static void __fastcall__ end_cutscene(void)
{
    advance_game_flow();
}

static void __fastcall__ advance_cutscene(void)
{
    // increment cutscene step (twice for dialog)
    switch( GET_CUTSCENE_ATTR_TYPE( all_cutscenes[ current_cutscene_index ].attr ) )
    {
        case CUTSCENE_TYPE_DIALOG:
            ++game_state_internal;

        case CUTSCENE_TYPE_TEXT:
            ++game_state_internal;
            break;

        default:
            INVALID_CODE_PATH;
            break;
    }

    // if it's the last step, end the cutscene
    if( game_state_internal >= GET_CUTSCENE_ATTR_COUNT( all_cutscenes[ current_cutscene_index ].attr ) )
    {
        end_cutscene();
    }
    // if there's no more text, end the cutscene
    else if( all_cutscenes[ current_cutscene_index ].t[ game_state_internal ] == (uint8_t)END_CUTSCENE )
    {
        end_cutscene();
    }
    // otherwise, draw the new cutscene
    else
    {
        draw_cutscene_part();
    }
}

void __fastcall__ game_state_cutscene_update(void)
{
    // if START is heald for a number of frames, end the cutscene early
    if( GAMEPAD_HELD(GAMEPAD_1, GAMEPAD_START) )
    {
        --r;
        if( r == 0 )
        {
            end_cutscene();
        }
    }
    // otherwise, reset counter
    else
    {
        r = HOLD_TO_SKIP_ENTIRE_CUTSCENE_TIME_FRAMES;
    }

    // advance cutscene on A press
    if( GAMEPAD_RELEASED(GAMEPAD_1, GAMEPAD_A) )
    {
        advance_cutscene();
    }
}
