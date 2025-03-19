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

extern ptr_t shadow_font;

static uint8_t arrow_sprite;

void __fastcall__ game_state_title_enter(void)
{
    arrow_sprite = ppu_request_sprite();

    // turn off ppu
    ppu_disable();
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
        ppu_set_palette( PALETTE_SPRITE_0, 0x00, 0x10, 0x20 );
        ppu_set_palette( PALETTE_SPRITE_1, 0x00, 0x10, 0x20 );
        ppu_set_palette( PALETTE_SPRITE_2, 0x00, 0x10, 0x20 );

        // draw title screen
        text_draw_string( ALIGN_SCREEN_WIDTH_CENTER(tr_game_title_width), ALIGN_SCREEN_HEIGHT_TOP(5), PALETTE_BACKGROUND_2, tr_game_title );

        ppu_set_nametable_attr( NAMETABLE_A_ATTR,  0, ALIGN_SCREEN_HEIGHT_TOP(5),  PALETTE_BACKGROUND_2, PALETTE_BACKGROUND_2, PALETTE_BACKGROUND_2, PALETTE_BACKGROUND_2,  TILE_TO_ATTR(SCREEN_WIDTH) );

        text_draw_string( ALIGN_SCREEN_WIDTH_CENTER(tr_new_game_width), ALIGN_SCREEN_HEIGHT_TOP(13), PALETTE_BACKGROUND_0, tr_new_game );
        text_draw_string( ALIGN_SCREEN_WIDTH_CENTER(tr_continue_width), ALIGN_SCREEN_HEIGHT_TOP(16), PALETTE_BACKGROUND_0, tr_continue );

        // NOTE: these strings are generated, no way to get length properly, hardcoded for now
        text_draw_string( ALIGN_SCREEN_WIDTH_CENTER(14), ALIGN_SCREEN_HEIGHT_BOTTOM(3), PALETTE_BACKGROUND_1, tr_version );

        text_draw_string( ALIGN_SCREEN_WIDTH_CENTER(8), ALIGN_SCREEN_HEIGHT_BOTTOM(2), PALETTE_BACKGROUND_1, tr_copyright );

        ppu_update_sprite_full( arrow_sprite, TILE_TO_PIXEL(10), TILE_TO_PIXEL(13), PALETTE_SPRITE_0, 0, 0, 0, 0x33 );
    }

    // turn on ppu
    ppu_enable();

    // reset game flow
    reset_game_flow();

    b = 0;
    game_state_internal = 0;
    timer_set( game_state_timer, 10 );
}

void __fastcall__ game_state_title_leave(void)
{
    ppu_release_sprite( arrow_sprite );
}

void __fastcall__ game_state_title_update(void)
{
    timer_tick( game_state_timer );
    if( timer_is_done( game_state_timer ) )
    {
        timer_set( game_state_timer, 10 );
        ++b;

        switch( b )
        {
            case 0:
                ppu_set_palette( PALETTE_BACKGROUND_2, 0x15, 0x26, 0x37 );
                break;
            case 1:
                ppu_set_palette( PALETTE_BACKGROUND_2, 0x37, 0x15, 0x26 );
                break;
            case 2:
                ppu_set_palette( PALETTE_BACKGROUND_2, 0x26, 0x37, 0x15 );
                b = -1;
                break;
        }
    }

    if(GAMEPAD_PRESSED(0, GAMEPAD_L))
    {
        apu_play_sfx(0);
    }

    switch( game_state_internal )
    {
        case 0:
            //ppu_add_oam_sprite(TILE_TO_PIXEL(10), TILE_TO_PIXEL(13), PALETTE_SPRITE_0, 0x33);
            if( GAMEPAD_PRESSED(0, GAMEPAD_D) )
            {
                ppu_update_sprite_pos( arrow_sprite, TILE_TO_PIXEL(10), TILE_TO_PIXEL(16) );
                game_state_internal = 1;
            }
            else if( GAMEPAD_PRESSED(0, GAMEPAD_START) )
            {
                game_data_load_for_new_game();

                advance_game_flow();
            }
            break;

        case 1:
            //ppu_add_oam_sprite(TILE_TO_PIXEL(10), TILE_TO_PIXEL(16), PALETTE_SPRITE_0, 0x33);
            if( GAMEPAD_PRESSED(0, GAMEPAD_U))
            {
                ppu_update_sprite_pos( arrow_sprite, TILE_TO_PIXEL(10), TILE_TO_PIXEL(13) );
                game_state_internal = 0;
            }
            else if( GAMEPAD_PRESSED(0, GAMEPAD_START) )
            {
                set_next_game_state( GAME_STATE_CONTINUE );
            }
            break;
    }
}
