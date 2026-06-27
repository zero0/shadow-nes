#include "types.h"
#include "macros.h"
#include "globals.h"
#include "text.h"
#include "gamepad.h"
#include "mapper.h"
#include "ppu.h"
#include "timer.h"
#include "game_state.h"
#include "game_flow.h"
#include "game_data.h"
#include "chr_rom.h"
#include <gametext.h>

static uint8_t arrow_sprite;

enum
{
    SaveSlot_Label_X = 5,
    SaveSlot_Label_Y = 10,
    SaveSlot_Label_Y_Spacing = 5,

    SaveSlot_Arrow_X = 3,
    SaveSlot_Arrow_Y = SaveSlot_Label_Y,
    SaveSlot_Arrow_Y_Spacing = SaveSlot_Label_Y_Spacing,
};

enum
{
    SaveSlot_0,
    SaveSlot_1,
    SaveSlot_2,

    SaveSlot_Count,
};

STATIC_ASSERT(_SAVE_SLOT_COUNT == SaveSlot_Count);

void __fastcall__ game_state_continue_enter(void)
{
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

        ppu_tint_reset();

        ppu_set_palette_background( 0x0F );
        ppu_set_palette( PALETTE_BACKGROUND_0, 0x15, 0x26, 0x37 );
        ppu_set_palette( PALETTE_BACKGROUND_1, 0x2D, 0x3D, 0x20 );
        ppu_set_palette( PALETTE_BACKGROUND_2, 0x15, 0x26, 0x37 );
        ppu_set_palette( PALETTE_SPRITE_0, 0x00, 0x10, 0x20 );
        ppu_set_palette( PALETTE_SPRITE_1, 0x00, 0x10, 0x20 );
        ppu_set_palette( PALETTE_SPRITE_2, 0x00, 0x10, 0x20 );

        // draw continue screen
        text_draw_string( ALIGN_SCREEN_WIDTH_CENTER(tr_continue_width), ALIGN_SCREEN_HEIGHT_TOP(5), PALETTE_BACKGROUND_0, tr_continue );

        // save slot 0
        game_data_peek_from_save_slot( SaveSlot_0 );

        if( g_current_game_data.version == 0 )
        {
            text_draw_string( ALIGN_SCREEN_WIDTH_LEFT(SaveSlot_Label_X), ALIGN_SCREEN_HEIGHT_TOP(SaveSlot_Label_Y + (SaveSlot_0 * SaveSlot_Label_Y_Spacing)), PALETTE_BACKGROUND_2, tr_char_unknown );
        }
        else
        {
            text_draw_string( ALIGN_SCREEN_WIDTH_LEFT(SaveSlot_Label_X), ALIGN_SCREEN_HEIGHT_TOP(SaveSlot_Label_Y + (SaveSlot_0 * SaveSlot_Label_Y_Spacing)), PALETTE_BACKGROUND_2, tr_char_shadowborn );
        }
    }

    // turn on ppu
    ppu_enable();


    arrow_sprite = ppu_request_sprite();
    ppu_update_sprite_full( arrow_sprite, TILE_TO_PIXEL(4), TILE_TO_PIXEL(10), PALETTE_SPRITE_0, 0, 0, 0, 0x33 );

    game_state_internal = SaveSlot_0;
}

void __fastcall__ game_state_continue_leave(void)
{
    ppu_release_sprite( arrow_sprite );
}

void __fastcall__ game_state_continue_update(void)
{
    // go back to title screen
    if( GAMEPAD_PRESSED(0, GAMEPAD_B) )
    {
        set_next_game_state_dir( GAME_STATE_TITLE, GAME_STATE_DIRECTION_BACKWARD );
        return;
    }

    // load selected save slot
    if( GAMEPAD_PRESSED(0, GAMEPAD_A) )
    {
        game_data_peek_from_save_slot( game_state_internal );

        if( g_current_game_data.version != 0 )
        {
            game_data_load_from_save_slot( game_state_internal );

            //game_flow_advance();
        }
        return;
    }

    if( GAMEPAD_PRESSED(0, GAMEPAD_U) )
    {
        if( game_state_internal > 0 ) --game_state_internal;
    }

    if( GAMEPAD_PRESSED(0, GAMEPAD_D) )
    {
        if( game_state_internal < _SAVE_SLOT_COUNT ) ++game_state_internal;
    }

    switch( game_state_internal )
    {
        case SaveSlot_0:
            ppu_update_sprite_pos( arrow_sprite, TILE_TO_PIXEL(SaveSlot_Arrow_X), TILE_TO_PIXEL(SaveSlot_Arrow_Y + (SaveSlot_0 * SaveSlot_Arrow_Y_Spacing)) );
            break;
        case SaveSlot_1:
            ppu_update_sprite_pos( arrow_sprite, TILE_TO_PIXEL(SaveSlot_Arrow_X), TILE_TO_PIXEL(SaveSlot_Arrow_Y + (SaveSlot_1 * SaveSlot_Arrow_Y_Spacing)) );
            break;
        case SaveSlot_2:
            ppu_update_sprite_pos( arrow_sprite, TILE_TO_PIXEL(SaveSlot_Arrow_X), TILE_TO_PIXEL(SaveSlot_Arrow_Y + (SaveSlot_2 * SaveSlot_Arrow_Y_Spacing)) );
            break;
        default:
            INVALID_CODE_PATH;
            break;
    }
}
