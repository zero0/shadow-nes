#include "types.h"
#include "macros.h"
#include "globals.h"
#include "text.h"
#include "gamepad.h"
#include "ppu.h"
#include "timer.h"
#include "game_state.h"
#include "game_flow.h"
#include "game_data.h"

static uint8_t arrow_sprite;

void __fastcall__ game_state_continue_enter(void)
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

        ppu_tint_reset();

        ppu_set_palette_background( 0x0F );
        ppu_set_palette( PALETTE_BACKGROUND_0, 0x15, 0x26, 0x37 );
        ppu_set_palette( PALETTE_BACKGROUND_1, 0x2D, 0x3D, 0x20 );
        ppu_set_palette( PALETTE_BACKGROUND_2, 0x15, 0x26, 0x37 );
        ppu_set_palette( PALETTE_SPRITE_0, 0x00, 0x10, 0x20 );
        ppu_set_palette( PALETTE_SPRITE_1, 0x00, 0x10, 0x20 );
        ppu_set_palette( PALETTE_SPRITE_2, 0x00, 0x10, 0x20 );

        // draw continue screen
        text_draw_string( ALIGN_SCREEN_WIDTH_CENTER(tr_continue_width), ALIGN_SCREEN_HEIGHT_TOP(5), PALETTE_BACKGROUND_2, tr_continue );

        // save slot 0
        for( j = 0, k = 10; j < _SAVE_SLOT_COUNT; ++j, k += 5 )
        {
            game_data_peek_from_save_slot( j );

            if( g_current_game_data.version == 0 )
            {
                text_draw_string( ALIGN_SCREEN_WIDTH_LEFT(5), ALIGN_SCREEN_HEIGHT_TOP(k), PALETTE_BACKGROUND_2, tr_char_unknown );
            }
            else
            {
                text_draw_string( ALIGN_SCREEN_WIDTH_LEFT(5), ALIGN_SCREEN_HEIGHT_TOP(k), PALETTE_BACKGROUND_2, tr_char_shadowborn );
            }
        }
    }

    // turn on ppu
    ppu_enable();

    ppu_update_sprite_full( arrow_sprite, TILE_TO_PIXEL(4), TILE_TO_PIXEL(10), PALETTE_SPRITE_0, 0, 0, 0, 0x33 );

    b = 1;
    game_state_internal = 0;
    timer_set( game_state_timer, 10 );
}

void __fastcall__ game_state_continue_leave(void)
{
    ppu_release_sprite( arrow_sprite );
}

void __fastcall__ game_state_continue_update(void)
{
    // go back to title screen
    if( GAMEPAD_PRESSED( GAMEPAD_1, GAMEPAD_B ) )
    {
        set_next_game_state_dir( GAME_STATE_TITLE, GAME_STATE_DIRECTION_BACKWARD );
    }

    // load selected save slot
    if( GAMEPAD_PRESSED( GAMEPAD_1, GAMEPAD_A ) )
    {
        game_data_peek_from_save_slot( game_state_internal );

        if( g_current_game_data.version != 0 )
        {
            game_data_load_from_save_slot( game_state_internal );

            advance_game_flow();
        }
        return;
    }

    if( GAMEPAD_PRESSED( GAMEPAD_1, GAMEPAD_U ) )
    {
        if( game_state_internal > 0 )
        {
            --game_state_internal;
            ++b;
        }
    }

    if( GAMEPAD_PRESSED( GAMEPAD_1, GAMEPAD_D ) )
    {
        if( game_state_internal < _SAVE_SLOT_COUNT-1 )
        {
            ++game_state_internal;
            ++b;
        }
    }

    if( b != 0 )
    {
        b = 0;
        switch( game_state_internal )
        {
            case 0:
                ppu_update_sprite_pos( arrow_sprite, TILE_TO_PIXEL(3), TILE_TO_PIXEL(10) );
                break;
            case 1:
                ppu_update_sprite_pos( arrow_sprite, TILE_TO_PIXEL(3), TILE_TO_PIXEL(15) );
                break;
            case 2:
                ppu_update_sprite_pos( arrow_sprite, TILE_TO_PIXEL(3), TILE_TO_PIXEL(20) );
                break;
            default:
                INVALID_CODE_PATH;
                break;
        }
    }
}
