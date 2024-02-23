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
    HOLD_TO_SKIP_ENTIRE_CUTSCENE_TIME_FRAMES = 3 * 60,
    MAX_CUTSCENE_TEXTS = 4,
};

enum
{
    CUTSCENE_TYPE_TEXT,
    CUTSCENE_TYPE_DIALOG,
};

#define MAKE_CUTSCENE_ATTR( t )                         (uint8_t)( ( (t) & 0x0F ) )
#define MAKE_CUTSCENE_PALETTE_4( p0, p1, p2, p3 )       (uint8_t)( ( (p3) & 0x03 ) << 6 | ( (p2) & 0x03 ) << 4 | ( (p1) & 0x03 ) << 2 | ( (p0) & 0x03 ) )
#define MAKE_CUTSCENE_PALETTE_2_DIALOG( c, d )          MAKE_CUTSCENE_PALETTE_4( c, d, c, d )
#define MAKE_CUTSCENE_PALETTE( p )                      MAKE_CUTSCENE_PALETTE_4( p, p, p, p )

#define GET_CUTSCENE_ATTR_TYPE( a )                     (uint8_t)( (a) & 0x0F )
#define GET_CUTSCENE_PALLETE( p, i )                    (uint8_t)( ( (p) >> ( (i) << 2 ) ) & 0x03 )

typedef struct
{
    uint8_t attr;
    uint8_t palettes;
    str_t t[MAX_CUTSCENE_TEXTS];
} cutscene_desc_t;

static const cutscene_desc_t all_cutscenes[] = {
    { MAKE_CUTSCENE_ATTR( CUTSCENE_TYPE_TEXT ), MAKE_CUTSCENE_PALETTE_2_DIALOG( PALETTE_BACKGROUND_0, PALETTE_BACKGROUND_1 ), tr_cutscene_intro_0, tr_cutscene_intro_1, tr_cutscene_intro_2, tr_cutscene_intro_3 },
};

extern ptr_t shadow_font;

static uint8_t current_cutscene_index;
static uint8_t current_cutscene_step;

static void __fastcall__ draw_cutscene_part(void)
{
    ppu_set_scroll( 0, 0 );

    // TODO: maybe fade out/in while clearing
    ppu_wait_vblank();
    ppu_off();
    ppu_clear_nametable( NAMETABLE_A, 0xFF, 0 );

    // draw
    switch( GET_CUTSCENE_ATTR_TYPE( all_cutscenes[ current_cutscene_index ].attr ) )
    {
        case CUTSCENE_TYPE_TEXT:
            text_draw_string( 0, 0, GET_CUTSCENE_PALLETE( all_cutscenes[ current_cutscene_index ].palettes, current_cutscene_step ), all_cutscenes[ current_cutscene_index ].t[ current_cutscene_step ] );
            break;

        case CUTSCENE_TYPE_DIALOG:
            text_draw_string( 0, 0, GET_CUTSCENE_PALLETE( all_cutscenes[ current_cutscene_index ].palettes, current_cutscene_step + 0 ), all_cutscenes[ current_cutscene_index ].t[ current_cutscene_step + 0 ] );
            text_draw_string( 0, 2, GET_CUTSCENE_PALLETE( all_cutscenes[ current_cutscene_index ].palettes, current_cutscene_step + 1 ), all_cutscenes[ current_cutscene_index ].t[ current_cutscene_step + 1 ] );
            break;

        default:
            INVALID_CODE_PATH;
            break;
    }

    ppu_wait_vblank();
    ppu_on();
}

void __fastcall__ game_state_cutscene_enter(void)
{
    ppu_clear_nametable( NAMETABLE_A, 0xFF, 0 );
    ppu_upload_chr_ram( shadow_font, MAKE_CHR_PTR(0,0,0), 16*4+13 );

    ppu_set_scroll( 0, 0 );
    ppu_clear_palette();
    ppu_clear_oam();

    ppu_set_palette_background( 0x0F );
    ppu_set_palette( PALETTE_BACKGROUND_0, 0x15, 0x26, 0x37 );
    ppu_set_palette( PALETTE_BACKGROUND_1, 0x05, 0x15, 0x30 ); // red, light red, white
    ppu_set_palette( PALETTE_BACKGROUND_2, 0x1A, 0x2A, 0x30 ); // green, light green, white
    ppu_set_palette( PALETTE_SPRITE_0, 0x0A, 0x1A, 0x2A );

    current_cutscene_index = next_game_state_arg;
    current_cutscene_step = 0;

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
            ++current_cutscene_step;

        case CUTSCENE_TYPE_TEXT:
            ++current_cutscene_step;
            break;

        default:
            INVALID_CODE_PATH;
            break;
    }

    // if it's the last step, end the cutscene
    if( current_cutscene_step >= (uint8_t)MAX_CUTSCENE_TEXTS )
    {
        end_cutscene();
    }
    // if there's no more text, end the cutscene
    else if( all_cutscenes[ current_cutscene_index ].t[ current_cutscene_step ] == 0xFF )
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
    gamepad_poll(0);

    // if START is heald for a number of frames, end the cutscene early
    if( GAMEPAD_HELD(0, GAMEPAD_START) )
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
    if( GAMEPAD_RELEASED(0, GAMEPAD_A) )
    {
        advance_cutscene();
    }
}
