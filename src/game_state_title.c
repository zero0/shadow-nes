#include "types.h"
#include "macros.h"
#include "gamepad.h"
#include "subpixel.h"
#include "text.h"
#include "ppu.h"
#include "globals.h"
#include "game_state.h"

extern ptr_t shadow_font;

void __fastcall__ game_state_title_enter()
{
    ppu_clear_nametable( NAMETABLE_A, 0xFF, 0 );
    ppu_upload_chr_ram( shadow_font, MAKE_CHR_PTR(0,0,0), 16*4+13 );

    ppu_set_scroll( 0, 0 );
    ppu_clear_palette();
    ppu_clear_oam();

    ppu_begin_write_chr_ram_index( 0x10, 0x33 );
    ppu_write_chr_ram( 0x55,0x55,0x55,0x55, 0x55,0x55,0x55,0x55);
    ppu_write_chr_ram( 0,0,0,0, 0,0,0,0);
    ppu_end_write_chr_ram();

    ppu_set_palette_background( 0x0F );
    ppu_set_palette( PALETTE_BACKGROUND_0, 0x15, 0x26, 0x37 );
    ppu_set_palette( PALETTE_BACKGROUND_1, 0x2D, 0x3D, 0x20 );
    ppu_set_palette( PALETTE_BACKGROUND_2, 0x15, 0x26, 0x37 );
    ppu_set_palette( PALETTE_SPRITE_0, 0x00, 0x10, 0x20 );

    text_draw_string( ALIGN_SCREEN_WIDTH_CENTER(12), 5, PALETTE_BACKGROUND_2, tr_game_title );

    ppu_set_nametable_attr( NAMETABLE_A_ATTR, 0, 5,  2, 2, 2, 2,  TILE_TO_ATTR(SCREEN_WIDTH) );

    text_draw_string( ALIGN_SCREEN_WIDTH_CENTER(8), 13, PALETTE_BACKGROUND_0, tr_new_game );
    text_draw_string( ALIGN_SCREEN_WIDTH_CENTER(8), 16, PALETTE_BACKGROUND_0, tr_continue );

    text_draw_string( ALIGN_SCREEN_WIDTH_CENTER(12), (SCREEN_HEIGH - 3), PALETTE_BACKGROUND_1, tr_version );

    text_draw_string( ALIGN_SCREEN_WIDTH_CENTER(8), (SCREEN_HEIGH - 1), PALETTE_BACKGROUND_1, tr_copyright );

    b = 0;
    t = 0;
    game_state_internal = 0;
    timer_set( game_state_timer, 10 );
}

void __fastcall__ game_state_title_leave()
{
}

void __fastcall__ game_state_title_update()
{
    gamepad_poll( 0 );

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

    switch( t )
    {
        case 0:
            ppu_add_oam_sprite(TILE_TO_PIXEL(10), TILE_TO_PIXEL(13), PALETTE_SPRITE_0, 0x33);
            if( GAMEPAD_PRESSED(0, GAMEPAD_D) )
            {
                t = 1;
            }
            else if( GAMEPAD_PRESSED(0, GAMEPAD_START) )
            {
                set_next_game_state( GAME_STATE_PLAYING );
            }
            break;

        case 1:
            ppu_add_oam_sprite(TILE_TO_PIXEL(10), TILE_TO_PIXEL(16), PALETTE_SPRITE_0, 0x33);
            if( GAMEPAD_PRESSED(0, GAMEPAD_U))
            {
                t = 0;
            }
            else if( GAMEPAD_PRESSED(0, GAMEPAD_START) )
            {
                set_next_game_state( GAME_STATE_PLAYING );
            }
            break;
    }
}
