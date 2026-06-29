#include "types.h"
#include "macros.h"
#include "globals.h"
#include "text.h"
#include "gamepad.h"
#include "ppu.h"
#include "timer.h"
#include "game_state.h"
#include "game_flow.h"
#include "mapper.h"
#include "game_state.h"
#include "game_flow.h"
#include "game_data.h"
#include "chr_rom.h"
#include "hud.png.sprite.h"
#include <gametext.h>

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

#define MAKE_CUTSCENE_ATTR( t, a, c )                   (t) //(uint8_t)( ( (t) & 0x03 ) << 6 ) | ( ( (a) & 0x03 ) << 4 ) | ( ((c - 1) & 0x03) << 2 ) // need to subtract 1 since it's base 0
#define MAKE_CUTSCENE_PALETTE_4( p0, p1, p2, p3 )       (uint8_t)( ( (p3) & 0x03 ) << 6 | ( (p2) & 0x03 ) << 4 | ( (p1) & 0x03 ) << 2 | ( (p0) & 0x03 ) )
#define MAKE_CUTSCENE_PALETTE_2_DIALOG( c, d )          MAKE_CUTSCENE_PALETTE_4( c, d, c, d )
#define MAKE_CUTSCENE_PALETTE( p )                      MAKE_CUTSCENE_PALETTE_4( p, p, p, p )

#define GET_CUTSCENE_ATTR_TYPE( a )                     (a) //(uint8_t)( ( (a) >> 6 ) & 0x03 )
#define GET_CUTSCENE_ATTR_ALIGNMENT( a )                (uint8_t)( ( (a) >> 4 ) & 0x03 )
#define GET_CUTSCENE_ATTR_COUNT( a )                    (uint8_t)( ( ( (a) >> 2 ) & 0x03 ) + 1 ) // need to add 1 since it's base 0

#define GET_CUTSCENE_PALLETE( p, i )                    (uint8_t)( ( (p) >> ( (i) << 2 ) ) & 0x03 )

typedef struct
{
    uint8_t attr;
    uint8_t palettes;
    ptr_t t[MAX_CUTSCENE_TEXTS];
} cutscene_desc_t;

typedef struct
{
    const ptr_t t[MAX_CUTSCENE_TEXTS];
} cutscene_text_t;

static uint8_t all_cutscenes_attr[] = {
    MAKE_CUTSCENE_ATTR( CUTSCENE_TYPE_TEXT, CUTSCENE_V_ALIGN_MIDDLE, 2 ),
};
static uint8_t all_cutscenes_palettes[] = {
    MAKE_CUTSCENE_PALETTE( PALETTE_BACKGROUND_0 )
};
static const cutscene_text_t all_cutscenes_text[] = {
    {
        {
            (ptr_t)&tr_cutscene_intro_0,
            (ptr_t)&tr_cutscene_intro_1,
            (ptr_t)&tr_cutscene_intro_2,
            (ptr_t)&tr_cutscene_intro_3,
        }
    }
};

#define CHR_SPRITE(chr, sprite)     ((chr) + (sprite))

#if 0
STATIC_ASSERT(ARRAY_SIZE(all_cutscenes) == _CUTSCENE_COUNT);
#endif

//
//
//

extern ptr_t shadow_font;

static uint8_t current_cutscene_index;

static uint8_t arrow_sprite;

static void __fastcall__ draw_cutscene_part(void)
{
    // TODO: maybe fade out/in while clearing
    ppu_disable();

    {
        ppu_set_scroll( 0, 0 );
        //ppu_clear_palette();
        //ppu_clear_oam();

        //ppu_upload_chr_ram( shadow_font, MAKE_CHR_PTR(0,0,0), 16*4+13 );

        // clear ppu to known state
        ppu_clear_nametable( NAMETABLE_A, 0xFF, 0 );
        ppu_clear_nametable( NAMETABLE_B, 0xFF, 0 );
        ppu_clear_nametable( NAMETABLE_C, 0xFF, 0 );
        ppu_clear_nametable( NAMETABLE_D, 0xFF, 0 );

        //ppu_set_palette_background( 0x0F );
        //ppu_set_palette( PALETTE_BACKGROUND_0, 0x15, 0x26, 0x37 );
        //ppu_set_palette( PALETTE_BACKGROUND_1, 0x05, 0x15, 0x30 ); // red, light red, white
        //ppu_set_palette( PALETTE_BACKGROUND_2, 0x1A, 0x2A, 0x30 ); // green, light green, white
        //ppu_set_palette( PALETTE_SPRITE_0, 0x0A, 0x1A, 0x2A );
        //ppu_set_palette( PALETTE_SPRITE_1, 0x0A, 0x1A, 0x2A );
        //ppu_set_palette( PALETTE_SPRITE_2, 0x0A, 0x1A, 0x2A );

        // draw
        switch( GET_CUTSCENE_ATTR_TYPE( all_cutscenes_attr[ current_cutscene_index ] ) )
        {
            case CUTSCENE_TYPE_TEXT:
                //text_draw_string_v( 0, 0, GET_CUTSCENE_PALLETE( all_cutscenes_palettes[ current_cutscene_index ], game_state_internal ), all_cutscenes_text[ current_cutscene_index ].t[ game_state_internal ] );
                text_draw_string_v( 0, ALIGN_SCREEN_HEIGHT_CENTER(4), 0, all_cutscenes_text[ current_cutscene_index ].t[ game_state_internal] ); //all_text[0]); // all_text[all_cutscenes_text[current_cutscene_index] + game_state_internal] ); //all_cutscenes_text[0].t[0]);
                break;

            case CUTSCENE_TYPE_DIALOG:
                //text_draw_string( 0, 0, GET_CUTSCENE_PALLETE( all_cutscenes[ current_cutscene_index ].palettes, game_state_internal + 0 ), all_cutscenes[ current_cutscene_index ].t[ game_state_internal + 0 ] );
                //text_draw_string( 0, 2, GET_CUTSCENE_PALLETE( all_cutscenes[ current_cutscene_index ].palettes, game_state_internal + 1 ), all_cutscenes[ current_cutscene_index ].t[ game_state_internal + 1 ] );
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
    // mapper update
    mapper_reset();
    mapper_reset_irq();

    mapper_set_chr_bank_0(SHADOW_FONT_EN_PNG_FONT_CHR_ROM);
    mapper_set_chr_bank_1(HUD_PNG_SPRITE_CHR_ROM);

    mapper_set_prg_bank(0);

    current_cutscene_index = 0;// next_game_state_arg;

    game_state_internal = 0;
    game_state_timer = request_timer( HOLD_TO_SKIP_ENTIRE_CUTSCENE_TIME_FRAMES );

    arrow_sprite = ppu_request_sprite();

    //ppu_update_sprite_sprite( arrow_sprite, CHR_SPRITE(HUD_PNG_SPRITE, SPRITE_DIALOG_ARROR) );

    ppu_update_sprite_full( arrow_sprite, 0, 0, PALETTE_SPRITE_0, 0, 0, 0, CHR_SPRITE(HUD_PNG_SPRITE, SPRITE_DIALOG_ARROR) );

    // draw the first cutscene
    draw_cutscene_part();
}

void __fastcall__ game_state_cutscene_leave(void)
{
    release_timer( game_state_timer );
    ppu_release_sprite( arrow_sprite );
}

#define end_cutscene() do { \
    game_flow_advance();    \
} while( 0 )

static void __fastcall__ advance_cutscene(void)
{
    // increment cutscene step (twice for dialog)
    switch( GET_CUTSCENE_ATTR_TYPE( all_cutscenes_attr[ current_cutscene_index ] ) )
    {
        case CUTSCENE_TYPE_DIALOG:
            ++game_state_internal;
            // fall through

        case CUTSCENE_TYPE_TEXT:
            ++game_state_internal;
            break;

        default:
            INVALID_CODE_PATH;
            break;
    }

    // if it's the last step, end the cutscene
    if( game_state_internal >= MAX_CUTSCENE_TEXTS )
    {
        end_cutscene();
    }
    // if there's no more text, end the cutscene
    else if( all_cutscenes_text[ current_cutscene_index ].t[ game_state_internal ] == 0 )
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
    if( GAMEPAD_HELD(0, GAMEPAD_START) )
    {
        if( is_timer_done( game_state_timer ) )
        {
            end_cutscene();
            return;
        }
    }
    // otherwise, reset timer
    else
    {
        set_timer( game_state_timer, HOLD_TO_SKIP_ENTIRE_CUTSCENE_TIME_FRAMES );
    }

    // advance cutscene on A press
    if( GAMEPAD_PRESSED(0, GAMEPAD_A) )
    {
        advance_cutscene();
        return;
    }

    ppu_update_sprite_pos( arrow_sprite, TILE_TO_PIXEL(ALIGN_SCREEN_WIDTH_RIGHT(1)), TILE_TO_PIXEL(ALIGN_SCREEN_HEIGHT_BOTTOM(1)) );
}
