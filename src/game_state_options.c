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
    Options_Music,
    Options_SFX,

    Options_Back,

    _Count,
};

enum
{
    Options_Cursor_X = 3,
    Options_Label_X = 5,
    Options_Value_X = 10,

    Options_Y = 10,

    Options_Spacing = 3,

    Options_Music_Y = Options_Y + (Options_Music * Options_Spacing),
    Options_SFX_Y = Options_Y + (Options_SFX * Options_Spacing),

    Options_Back_Y = Options_Y + (Options_Back * Options_Spacing),

#if DEMO_BUILD
    Enable_Demo_Mode_Frame_Counter = 60,
    Enable_Demo_Mode_Hold_Input = GAMEPAD_SELECT,
    Enable_Demo_Mode_Hold_Option = Options_SFX,
#endif // DEMO_BUILD
};

#define CHR_SPRITE(chr, sprite)     ((chr) + (sprite))

static uint8_t arrow_sprite, music_sprite, sfx_sprite;

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

        text_draw_string( ALIGN_SCREEN_WIDTH_LEFT(Options_Label_X), ALIGN_SCREEN_HEIGHT_TOP(Options_Music_Y),  PALETTE_BACKGROUND_0, tr_options_music );

        text_draw_string( ALIGN_SCREEN_WIDTH_LEFT(Options_Label_X), ALIGN_SCREEN_HEIGHT_TOP(Options_SFX_Y),  PALETTE_BACKGROUND_0, tr_options_sfx );

        text_draw_string( ALIGN_SCREEN_WIDTH_LEFT(Options_Label_X), ALIGN_SCREEN_HEIGHT_TOP(Options_Back_Y),  PALETTE_BACKGROUND_0, tr_back );
    }

    // turn on ppu
    ppu_enable();

    arrow_sprite = ppu_request_sprite();
    music_sprite = ppu_request_sprite();
    sfx_sprite = ppu_request_sprite();

    ppu_update_sprite_full( arrow_sprite, TILE_TO_PIXEL(Options_Cursor_X), TILE_TO_PIXEL(Options_Music_Y), PALETTE_SPRITE_0, 0, 0, 0, CHR_SPRITE(CHR_ROM_02_HUD_PNG_SPRITE, SPRITE_POINTER_0) );
    ppu_update_sprite_full( music_sprite, TILE_TO_PIXEL(ALIGN_SCREEN_WIDTH_RIGHT(Options_Value_X)), TILE_TO_PIXEL(Options_Music_Y), PALETTE_SPRITE_0, 0, 0, 0, game_settings_is_music_enabled() ? CHR_SPRITE(CHR_ROM_02_HUD_PNG_SPRITE, SPRITE_CHECK_BOX_SELECTED) : CHR_SPRITE(CHR_ROM_02_HUD_PNG_SPRITE, SPRITE_CHECK_BOX_CLEAR) );
    ppu_update_sprite_full( sfx_sprite,   TILE_TO_PIXEL(ALIGN_SCREEN_WIDTH_RIGHT(Options_Value_X)), TILE_TO_PIXEL(Options_SFX_Y), PALETTE_SPRITE_0, 0, 0, 0, game_settings_is_sfx_enabled() ? CHR_SPRITE(CHR_ROM_02_HUD_PNG_SPRITE, SPRITE_CHECK_BOX_SELECTED) : CHR_SPRITE(CHR_ROM_02_HUD_PNG_SPRITE, SPRITE_CHECK_BOX_CLEAR) );

    game_state_internal = Options_Music;
    b = 0;

#if DEMO_BUILD
    c = Enable_Demo_Mode_Frame_Counter;
#endif // DEMO_BUILD
}

void __fastcall__ game_state_options_leave(void)
{
    ppu_release_sprite(arrow_sprite);
    ppu_release_sprite(music_sprite);
    ppu_release_sprite(sfx_sprite);
}

void __fastcall__ game_state_options_update(void)
{
    // go back to title screen
    if( GAMEPAD_PRESSED(0, GAMEPAD_B) )
    {
        set_next_game_state_dir( GAME_STATE_TITLE, GAME_STATE_DIRECTION_BACKWARD );
        return;
    }

    if( GAMEPAD_PRESSED(0, GAMEPAD_U) )
    {
        if(game_state_internal > 0) --game_state_internal;
        b = 1;
    }
    else if( GAMEPAD_PRESSED(0, GAMEPAD_D) )
    {
        if(game_state_internal < _Count) ++game_state_internal;
        b = 1;
    }

    if( b )
    {
        b = 0;
        switch( game_state_internal )
        {
            case Options_Music:
                ppu_update_sprite_pos( arrow_sprite, TILE_TO_PIXEL(Options_Cursor_X), TILE_TO_PIXEL(Options_Music_Y) );
                break;

            case Options_SFX:
                ppu_update_sprite_pos( arrow_sprite, TILE_TO_PIXEL(Options_Cursor_X), TILE_TO_PIXEL(Options_SFX_Y) );
                break;

            case Options_Back:
                ppu_update_sprite_pos( arrow_sprite, TILE_TO_PIXEL(Options_Cursor_X), TILE_TO_PIXEL(Options_Back_Y) );
                break;
        }
    }

    ppu_update_sprite_sprite( music_sprite, game_settings_is_music_enabled() ? CHR_SPRITE(CHR_ROM_02_HUD_PNG_SPRITE, SPRITE_CHECK_BOX_SELECTED) : CHR_SPRITE(CHR_ROM_02_HUD_PNG_SPRITE, SPRITE_CHECK_BOX_CLEAR) );
    ppu_update_sprite_sprite( sfx_sprite,   game_settings_is_sfx_enabled() ? CHR_SPRITE(CHR_ROM_02_HUD_PNG_SPRITE, SPRITE_CHECK_BOX_SELECTED) : CHR_SPRITE(CHR_ROM_02_HUD_PNG_SPRITE, SPRITE_CHECK_BOX_CLEAR) );

    //text_clear(ALIGN_SCREEN_WIDTH_RIGHT(Options_Value_X), ALIGN_SCREEN_HEIGHT_TOP(Options_Y + (Options_Music * Options_Spacing)), 3);
    //text_draw_string( ALIGN_SCREEN_WIDTH_RIGHT(Options_Value_X), ALIGN_SCREEN_HEIGHT_TOP(Options_Y + (Options_Music * Options_Spacing)),  PALETTE_BACKGROUND_0, tr_on);

    //text_clear(ALIGN_SCREEN_WIDTH_RIGHT(Options_Value_X), ALIGN_SCREEN_HEIGHT_TOP(Options_Y + (Options_SFX * Options_Spacing)), 3);
    //text_draw_string( ALIGN_SCREEN_WIDTH_RIGHT(Options_Value_X), ALIGN_SCwREEN_HEIGHT_TOP(Options_Y + (Options_SFX * Options_Spacing)),  PALETTE_BACKGROUND_0,   tr_on);

    if( GAMEPAD_PRESSED(0, GAMEPAD_A) )
    {
        switch( game_state_internal)
        {
            case Options_Music:
                {
                    game_settings_toggle_music_enabled();
                }
                break;

            case Options_SFX:
                {
                    game_settings_toggle_sfx_enabled();
                }
                break;

            case Options_Back:
                {
                    set_next_game_state_dir( GAME_STATE_TITLE, GAME_STATE_DIRECTION_BACKWARD );
                }
                return;

        }
    }

#if DEMO_BUILD
    if( game_state_internal == Enable_Demo_Mode_Hold_Option && GAMEPAD_HELD(0, Enable_Demo_Mode_Hold_Input) )
    {
        c--;
        if( c == 0 )
        {
            game_settings_enable_demo_mode();
            set_next_game_state_dir( GAME_STATE_TITLE, GAME_STATE_DIRECTION_FORWARD );
            return;
        }
    }
    else
    {
        c = Enable_Demo_Mode_Frame_Counter;
    }
#endif // DEMO_BUILD
}
