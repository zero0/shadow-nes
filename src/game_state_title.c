#include "types.h"
#include "macros.h"
#include "gamepad.h"
#include "subpixel.h"
#include "text.h"
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

extern ptr_t shadow_font;

static uint8_t arrow_sprite;
static timer_handle_t anim_title_timer;
static timer_handle_t anim_text_timer;

#define CHR_SPRITE(chr, sprite)     ((chr) + (sprite))

enum MenuState
{
    NewGame,
    Continue,
    Options,

    Count,
};

void __fastcall__ game_state_title_enter(void)
{
    arrow_sprite = ppu_request_sprite();

    // turn off ppu
    ppu_disable();

    // mapper update
    mapper_reset();
    mapper_reset_irq();

    mapper_set_chr_bank_0(SHADOW_FONT_EN_PNG_FONT_CHR_ROM);
    mapper_set_chr_bank_1(HUD_PNG_SPRITE_CHR_ROM);

    mapper_set_prg_bank(0);

    //ppu_disable_scope()
    {
        ppu_set_scroll( 0, 0 );
        ppu_clear_palette();
        ppu_clear_oam();

        // clear ppu to known state
        ppu_clear_nametable( NAMETABLE_A, 0xFF, 0 );
        ppu_clear_nametable( NAMETABLE_B, 0xFF, 0 );
        ppu_clear_nametable( NAMETABLE_C, 0xFF, 0 );
        ppu_clear_nametable( NAMETABLE_D, 0xFF, 0 );

        // CHR RAM does not work
        //ppu_upload_chr_ram( shadow_font, MAKE_CHR_PTR(SPRITE_0,0,0), 16*4+13 );

        // write "clear" tile
        //ppu_begin_write_chr_ram_index( TILE_SPRITE_ADDR, 0x33 );
        //ppu_write_chr_ram( 0,0,0,0, 0,0,0,0); //0x55,0x55,0x55,0x55, 0x55,0x55,0x55,0x55);
        //ppu_write_chr_ram( 0,0,0,0, 0,0,0,0);
        //ppu_end_write_chr_ram();

        ppu_tint_reset();

        ppu_set_palette_background( 0x0F );
        ppu_set_palette( PALETTE_BACKGROUND_0, 0x15, 0x26, 0x37 );
        ppu_set_palette( PALETTE_BACKGROUND_1, 0x2D, 0x3D, 0x20 );
        ppu_set_palette( PALETTE_BACKGROUND_2, 0x15, 0x26, 0x37 );
        ppu_set_palette( PALETTE_SPRITE_0, 0x15, 0x26, 0x37 );
        ppu_set_palette( PALETTE_SPRITE_1, 0x00, 0x10, 0x20 );
        ppu_set_palette( PALETTE_SPRITE_2, 0x00, 0x10, 0x20 );

        // draw title screen
        //text_draw_string_delay( ALIGN_SCREEN_WIDTH_CENTER(tr_game_title_width), ALIGN_SCREEN_HEIGHT_TOP(5), PALETTE_BACKGROUND_2, tr_game_title );

        ppu_set_nametable_attr( NAMETABLE_A_ATTR,  0, ALIGN_SCREEN_HEIGHT_TOP(5),  PALETTE_BACKGROUND_2, PALETTE_BACKGROUND_2, PALETTE_BACKGROUND_2, PALETTE_BACKGROUND_2,  TILE_TO_ATTR(SCREEN_WIDTH) );

        text_draw_string( ALIGN_SCREEN_WIDTH_CENTER(tr_new_game_width), ALIGN_SCREEN_HEIGHT_TOP(13 + (NewGame * 3)),  PALETTE_BACKGROUND_0, tr_new_game );
        text_draw_string( ALIGN_SCREEN_WIDTH_CENTER(tr_continue_width), ALIGN_SCREEN_HEIGHT_TOP(13 + (Continue * 3)), PALETTE_BACKGROUND_0, tr_continue );
        text_draw_string( ALIGN_SCREEN_WIDTH_CENTER(tr_options_width),  ALIGN_SCREEN_HEIGHT_TOP(13 + (Options * 3)),  PALETTE_BACKGROUND_0, tr_options );

        // NOTE: these strings are generated, no way to get length properly, hardcoded for now
#if DEMO_BUILD
        text_draw_string( ALIGN_SCREEN_WIDTH_CENTER(tr_demo_width), ALIGN_SCREEN_HEIGHT_BOTTOM(4), PALETTE_BACKGROUND_1, tr_demo );
#endif // DEMO_BUILD
        text_draw_string( ALIGN_SCREEN_WIDTH_CENTER(14), ALIGN_SCREEN_HEIGHT_BOTTOM(3), PALETTE_BACKGROUND_1, tr_version );
        text_draw_string( ALIGN_SCREEN_WIDTH_CENTER(8), ALIGN_SCREEN_HEIGHT_BOTTOM(2), PALETTE_BACKGROUND_1, tr_copyright );

        ppu_update_sprite_full( arrow_sprite, TILE_TO_PIXEL(10), TILE_TO_PIXEL(13), PALETTE_SPRITE_0, 0, 0, 0, CHR_SPRITE(HUD_PNG_SPRITE, SPRITE_POINTER_0) );
    }

    // turn on ppu
    ppu_enable();

    // reset game flow
    game_flow_reset();

    b = 0;
    game_state_internal = 0;

    anim_title_timer = request_timer( 10 );
    anim_text_timer = request_timer( 15 );

    text_delay_start();
}

void __fastcall__ game_state_title_leave(void)
{
    ppu_release_sprite( arrow_sprite );
    release_timer(anim_title_timer);
    release_timer(anim_text_timer);
}

void __fastcall__ game_state_title_update(void)
{
    if( is_timer_done( anim_text_timer ) )
    {
        text_delay_advance();
        set_timer( anim_text_timer, 15 );
    }

    text_draw_string_delay( ALIGN_SCREEN_WIDTH_CENTER(tr_game_title_width), ALIGN_SCREEN_HEIGHT_TOP(5), PALETTE_BACKGROUND_2, tr_game_title );

    if( is_timer_done( anim_title_timer ) )
    {
        set_timer( anim_title_timer, 10 );
        ++b;

        switch( b )
        {
            case 0:
                ppu_set_palette( PALETTE_BACKGROUND_2, 0x15, 0x26, 0x37 );
                ppu_update_sprite_sprite( arrow_sprite, CHR_SPRITE(HUD_PNG_SPRITE, SPRITE_POINTER_0) );
                break;
            case 1:
                ppu_set_palette( PALETTE_BACKGROUND_2, 0x37, 0x15, 0x26 );
                ppu_update_sprite_sprite( arrow_sprite, CHR_SPRITE(HUD_PNG_SPRITE, SPRITE_POINTER_2) );
                break;
            case 2:
                ppu_set_palette( PALETTE_BACKGROUND_2, 0x26, 0x37, 0x15 );
                ppu_update_sprite_sprite( arrow_sprite, CHR_SPRITE(HUD_PNG_SPRITE, SPRITE_POINTER_4) );
                b = -1;
                break;
        }
    }

    if(GAMEPAD_PRESSED(0, GAMEPAD_L))
    {
        apu_play_sfx(0);
    }

    if(GAMEPAD_PRESSED(0, GAMEPAD_R))
    {
        text_delay_display_full();
    }

    switch( game_state_internal )
    {
        case NewGame:
            ppu_update_sprite_pos( arrow_sprite, TILE_TO_PIXEL(10), TILE_TO_PIXEL(13 + (NewGame * 3)) );
            if( GAMEPAD_PRESSED(0, GAMEPAD_D) )
            {
                game_state_internal = Continue;
            }
            else if( GAMEPAD_PRESSED(0, GAMEPAD_START) )
            {
                game_data_load_for_new_game();

                game_flow_advance();
            }
            break;

        case Continue:
            ppu_update_sprite_pos( arrow_sprite, TILE_TO_PIXEL(10), TILE_TO_PIXEL(13 + (Continue * 3)) );
            if( GAMEPAD_PRESSED(0, GAMEPAD_U) )
            {
                game_state_internal = NewGame;
            }
            else if( GAMEPAD_PRESSED(0, GAMEPAD_D) )
            {
                game_state_internal = Options;
            }
            else if( GAMEPAD_PRESSED(0, GAMEPAD_START) )
            {
                set_next_game_state( GAME_STATE_CONTINUE );
            }
            break;

        case Options:
            ppu_update_sprite_pos( arrow_sprite, TILE_TO_PIXEL(10), TILE_TO_PIXEL(13 + (Options * 3)) );
            if( GAMEPAD_PRESSED(0, GAMEPAD_U))
            {
                game_state_internal = Continue;
            }
            else if( GAMEPAD_PRESSED(0, GAMEPAD_START) )
            {
                set_next_game_state( GAME_STATE_OPTIONS );
            }
            break;
    }
}
