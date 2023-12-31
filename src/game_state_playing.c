#include "types.h"
#include "macros.h"
#include "gamepad.h"
#include "subpixel.h"
#include "player.h"
#include "ppu.h"
#include "game_state.h"

extern ptr_t knight;
extern ptr_t knight_sprite_0;

void __fastcall__ game_state_playing_enter()
{
    ppu_clear_nametable( NAMETABLE_0_ADDR, 0xFF, 0 );
    //ppu_upload_chr_ram( shadow_font, 5, 0x00 );
    ppu_upload_chr_ram( knight, 2, 0x10 );

    ppu_set_scroll( 0, 0 );
    ppu_clear_palette();
    ppu_clear_oam();

    ppu_set_palette_background( 0x0F );
    ppu_set_palette( PALETTE_BACKGROUND_0, 0x15, 0x26, 0x37 );
    ppu_set_palette( PALETTE_BACKGROUND_1, 0x2D, 0x3D, 0x20 );
    ppu_set_palette( PALETTE_SPRITE_0, 0x0A, 0x1A, 0x2A );

    // top
    ppu_begin_tile_batch(0,0);
    ppu_repeat_tile_batch(0, SCREEN_WIDTH * 2);
    ppu_end_tile_batch();

    ppu_begin_tile_batch(0,2);
    ppu_repeat_tile_batch(0, SCREEN_WIDTH * 2);
    ppu_end_tile_batch();

    // bottom
    ppu_begin_tile_batch(0,SCREEN_HEIGH-8);
    ppu_repeat_tile_batch(0, SCREEN_WIDTH * 2);
    ppu_end_tile_batch();

    ppu_begin_tile_batch(0,SCREEN_HEIGH-6);
    ppu_repeat_tile_batch(0, SCREEN_WIDTH * 3);
    ppu_end_tile_batch();

    ppu_begin_tile_batch(0,SCREEN_HEIGH-3);
    ppu_repeat_tile_batch(0, SCREEN_WIDTH * 3);
    ppu_end_tile_batch();

    // start player
    init_player();
}

void __fastcall__ game_state_playing_leave()
{

}

void __fastcall__ game_state_playing_update()
{
    //update_player();
    ppu_add_meta_sprite( TILE_TO_PIXEL(3), TILE_TO_PIXEL(14), PALETTE_SPRITE_0, 0 );
}
