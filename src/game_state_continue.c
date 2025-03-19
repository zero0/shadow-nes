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
        {
            game_data_peek_from_save_slot( 0 );

            if( g_current_game_data.version == 0 )
            {
                //text_draw_string( ALIGN_SCREEN_WIDTH_LEFT(5), ALIGN_SCREEN_HEIGHT_TOP(10)), PALETTE_BACKGROUND_2, tr_char_unknown );
            }
            else
            {
                //text_draw_string( ALIGN_SCREEN_WIDTH_LEFT(5), ALIGN_SCREEN_HEIGHT_TOP(10)), PALETTE_BACKGROUND_2, g_current_game_data.name );
            }
        }

    }

    // turn on ppu
    ppu_enable();

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
        //game_data_load_from_save_slot( game_state_internal );

        //set_next_game_state( GAME_STATE_PLAYING );
        return;
    }

    if( GAMEPAD_PRESSED( GAMEPAD_1, GAMEPAD_U ) )
    {
        if( game_state_internal > 0 )
        {
            --game_state_internal;
        }
    }

    if( GAMEPAD_PRESSED( GAMEPAD_1, GAMEPAD_D ) )
    {
        if( game_state_internal < _SAVE_SLOT_COUNT )
        {
            ++game_state_internal;
        }
    }
}
