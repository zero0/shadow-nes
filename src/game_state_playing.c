#include "types.h"
#include "macros.h"
#include "gamepad.h"
#include "subpixel.h"
#include "player.h"
#include "boss.h"
#include "ppu.h"
#include "game_state.h"
#include "globals.h"

enum
{
    GAME_STATE_PLAYING_INTRO,
    GAME_STATE_PLAYING_PLAYING,
    GAME_STATE_PLAYING_PAUSED,
    GAME_STATE_PLAYING_TALKING,
    _GAME_STATE_PLAYING_COUNT,
};

extern ptr_t progress_bar;
extern ptr_t shadow_font;

void __fastcall__ game_state_playing_enter()
{
    ppu_clear_nametable( NAMETABLE_A, 0xFF, 0 );
    ppu_upload_chr_ram( shadow_font, MAKE_CHR_PTR(0,0,0), 16*4+13 );

    // upload progress bar
    ppu_upload_chr_ram( progress_bar, MAKE_CHR_PTR(0, 8, 0), 9);

    ppu_set_scroll( 0, 0 );
    ppu_clear_palette();
    ppu_clear_oam();

    ppu_set_palette_background( 0x0F );
    ppu_set_palette( PALETTE_BACKGROUND_0, 0x15, 0x26, 0x37 );
    ppu_set_palette( PALETTE_BACKGROUND_1, 0x05, 0x15, 0x30 ); // red, light red, white
    ppu_set_palette( PALETTE_BACKGROUND_2, 0x1A, 0x2A, 0x30 ); // green, light green, white
    ppu_set_palette( PALETTE_SPRITE_0, 0x0A, 0x1A, 0x2A );

    // top
    ppu_begin_tile_batch(0,0);
    ppu_repeat_tile_batch(0, SCREEN_WIDTH * 2);
    ppu_end_tile_batch();

    ppu_begin_tile_batch(0,2);
    ppu_repeat_tile_batch(0, SCREEN_WIDTH * 3);
    ppu_end_tile_batch();

    // bottom
    ppu_begin_tile_batch(0,SCREEN_HEIGH-7);
    ppu_repeat_tile_batch(0, SCREEN_WIDTH * 2);
    ppu_end_tile_batch();

    ppu_begin_tile_batch(0,SCREEN_HEIGH-6);
    ppu_repeat_tile_batch(0, SCREEN_WIDTH * 3);
    ppu_end_tile_batch();

    ppu_begin_tile_batch(0,SCREEN_HEIGH-3);
    ppu_repeat_tile_batch(0, SCREEN_WIDTH * 3);
    ppu_end_tile_batch();

    game_state_internal = GAME_STATE_PLAYING_INTRO;

    // health and stamina colors
    ppu_set_nametable_attr( NAMETABLE_A_ATTR, 0, 0,  0, 0, 2, 2,  4 );

    // start player
    player_init();

    // start boss
    boss_init(next_game_state_arg);
}

void __fastcall__ game_state_playing_leave()
{

}

void __fastcall__ game_state_playing_update()
{
    gamepad_poll(0);

    switch( game_state_internal )
    {
        case GAME_STATE_PLAYING_INTRO:
        {
            boss_update();

            if( boss_is_ready() )
            {
                game_state_internal = GAME_STATE_PLAYING_PLAYING;
            }
        }
            break;

        case GAME_STATE_PLAYING_PLAYING:
        {
            if( GAMEPAD_PRESSED(0, GAMEPAD_START) )
            {
                game_state_internal = GAME_STATE_PLAYING_PAUSED;
                return;
            }

            player_update();

            boss_update();
        }
            break;

        case GAME_STATE_PLAYING_PAUSED:
        {
            if( GAMEPAD_PRESSED(0, GAMEPAD_START) )
            {
                game_state_internal = GAME_STATE_PLAYING_PLAYING;
                return;
            }
        }
            break;
    }
}

void __fastcall__ game_state_playing_set_pause(uint8_t isPaused)
{
    UNUSED(isPaused);
}

uint8_t __fastcall__ game_state_playing_is_pause(void)
{
    return game_state_internal == GAME_STATE_PLAYING_PAUSED;
}
