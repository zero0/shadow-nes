#include "types.h"
#include "macros.h"
#include "gamepad.h"
#include "subpixel.h"
#include "text.h"
#include "flags.h"
#include "ppu.h"
#include "globals.h"
#include "game_state.h"
#include "game_flow.h"
#include "game_data.h"
#include "apu.h"
#include "timer.h"
#include "mapper.h"
#include "chr_rom.h"
#include "hud.png.sprite.h"
#include <gametext.h>

enum
{
    Option_Music,
    Option_SFX,

    Back,

    _Count,
};

enum
{
    Options_Cursor_X = 3,
    Options_Label_X = 5,
    Options_Value_X = 10,

    Options_Y = 10,

    Options_Spacing = 3,
};

#define CHR_SPRITE(chr, sprite)     ((chr) + (sprite))

static uint8_t arrow_sprite;

void __fastcall__ game_state_options_enter(void)
{
    // turn off ppu
    ppu_disable();

    // mapper update
    mapper_reset();
    mapper_reset_irq();

    mapper_set_chr_bank_0(CHR_ROM_00_SHADOW_FONT_EN_PNG_FONT_CHR_ROM);
    mapper_set_chr_bank_1(CHR_ROM_02_HUD_PNG_SPRITE_CHR_ROM);

    mapper_set_prg_bank(0);

    {
        ppu_set_scroll( 0, 0 );
        ppu_clear_palette();
        ppu_clear_oam();

        // clear ppu to known state
        ppu_clear_nametable( NAMETABLE_A, 0xFF, 0 );
        ppu_clear_nametable( NAMETABLE_B, 0xFF, 0 );
        ppu_clear_nametable( NAMETABLE_C, 0xFF, 0 );
        ppu_clear_nametable( NAMETABLE_D, 0xFF, 0 );

        // draw options
        text_draw_string( ALIGN_SCREEN_WIDTH_CENTER(tr_options_width), ALIGN_SCREEN_HEIGHT_TOP(5), PALETTE_BACKGROUND_0, tr_options );

        text_draw_string( ALIGN_SCREEN_WIDTH_LEFT(Options_Label_X), ALIGN_SCREEN_HEIGHT_TOP(Options_Y + (Option_Music * Options_Spacing)),  PALETTE_BACKGROUND_0, tr_options_music );
        //text_draw_string( ALIGN_SCREEN_WIDTH_RIGHT(Options_Value_X), ALIGN_SCREEN_HEIGHT_TOP(Options_Y + (Option_Music * Options_Spacing)),  PALETTE_BACKGROUND_0, tr_on );

        text_draw_string( ALIGN_SCREEN_WIDTH_LEFT(Options_Label_X), ALIGN_SCREEN_HEIGHT_TOP(Options_Y + (Option_SFX * Options_Spacing)),  PALETTE_BACKGROUND_0, tr_options_sfx );
        //text_draw_string( ALIGN_SCREEN_WIDTH_RIGHT(Options_Value_X), ALIGN_SCREEN_HEIGHT_TOP(Options_Y + (Option_SFX * Options_Spacing)),  PALETTE_BACKGROUND_0, tr_off );

        text_draw_string( ALIGN_SCREEN_WIDTH_LEFT(Options_Label_X), ALIGN_SCREEN_HEIGHT_TOP(Options_Y + (Back * Options_Spacing)),  PALETTE_BACKGROUND_0, tr_back );
    }

    // turn on ppu
    ppu_enable();

    arrow_sprite = ppu_request_sprite();

    ppu_update_sprite_full( arrow_sprite, 0, 0, PALETTE_SPRITE_0, 0, 0, 0, CHR_SPRITE(CHR_ROM_02_HUD_PNG_SPRITE, SPRITE_POINTER_0) );

    game_state_internal = Option_Music;
}

void __fastcall__ game_state_options_leave(void)
{
    ppu_release_sprite(arrow_sprite);
}

void __fastcall__ game_state_options_update(void)
{
    // go back to title screen
    if( GAMEPAD_PRESSED(0, GAMEPAD_B ) )
    {
        set_next_game_state_dir( GAME_STATE_TITLE, GAME_STATE_DIRECTION_BACKWARD );
        return;
    }

    if(GAMEPAD_PRESSED(0, GAMEPAD_U))
    {
        if(game_state_internal > 0) --game_state_internal;
    }
    if(GAMEPAD_PRESSED(0, GAMEPAD_D))
    {
        if(game_state_internal < _Count) ++game_state_internal;
    }

    if(flags_is_set(g_global_settings_data.flags0, GLOBAL_SETTINGS_0_MUSIC))
    {
    }
    else
    {
    }

    //text_clear(ALIGN_SCREEN_WIDTH_RIGHT(Options_Value_X), ALIGN_SCREEN_HEIGHT_TOP(Options_Y + (Option_Music * Options_Spacing)), 3);
    text_draw_string( ALIGN_SCREEN_WIDTH_RIGHT(Options_Value_X), ALIGN_SCREEN_HEIGHT_TOP(Options_Y + (Option_Music * Options_Spacing)),  PALETTE_BACKGROUND_0, tr_on);

    //text_clear(ALIGN_SCREEN_WIDTH_RIGHT(Options_Value_X), ALIGN_SCREEN_HEIGHT_TOP(Options_Y + (Option_SFX * Options_Spacing)), 3);
    text_draw_string( ALIGN_SCREEN_WIDTH_RIGHT(Options_Value_X), ALIGN_SCREEN_HEIGHT_TOP(Options_Y + (Option_SFX * Options_Spacing)),  PALETTE_BACKGROUND_0,   tr_on);

    switch( game_state_internal)
    {
        case Option_Music:
            ppu_update_sprite_pos( arrow_sprite, TILE_TO_PIXEL(Options_Cursor_X), TILE_TO_PIXEL(Options_Y + (Option_Music * Options_Spacing)) );
            if( GAMEPAD_PRESSED(0, GAMEPAD_A))
            {
                flags_toggle(g_global_settings_data.flags0, GLOBAL_SETTINGS_0_MUSIC);
            }
            break;

        case Option_SFX:
            ppu_update_sprite_pos( arrow_sprite, TILE_TO_PIXEL(Options_Cursor_X), TILE_TO_PIXEL(Options_Y + (Option_SFX * Options_Spacing)) );
            if( GAMEPAD_PRESSED(0, GAMEPAD_A))
            {
                flags_toggle(g_global_settings_data.flags0, GLOBAL_SETTINGS_0_SFX);
            }
            break;

        case Back:
            ppu_update_sprite_pos( arrow_sprite, TILE_TO_PIXEL(Options_Cursor_X), TILE_TO_PIXEL(Options_Y + (Back * Options_Spacing)) );
            if( GAMEPAD_PRESSED(0, GAMEPAD_A))
            {
                set_next_game_state_dir( GAME_STATE_TITLE, GAME_STATE_DIRECTION_BACKWARD );
            }
            break;
    }
}
