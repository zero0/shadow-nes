#include "types.h"
#include "macros.h"
#include "gamepad.h"
#include "subpixel.h"
#include "player.h"
#include "ppu.h"
#include "game_state.h"

void __fastcall__ game_state_playing_enter()
{
    ppu_clear_nametable( NAMETABLE_0_ADDR, 0xFF, 0 );
    //ppu_upload_chr_ram( shadow_font, 5, 0x00 );

    ppu_set_scroll( 0, 0 );
    ppu_clear_palette();
    ppu_clear_oam();

    ppu_set_palette_background( 0x0F );
    ppu_set_palette( PALETTE_BACKGROUND_0, 0x15, 0x26, 0x37 );
    ppu_set_palette( PALETTE_BACKGROUND_1, 0x2D, 0x3D, 0x20 );
    ppu_set_palette( PALETTE_SPRITE_0, 0x00, 0x10, 0x20 );

    ppu_begin_tile_batch(0,0);
    ppu_repeat_tile_batch(0, SCREEN_WIDTH * 3);
    ppu_end_tile_batch();

    init_player();
}

void __fastcall__ game_state_playing_leave()
{

}

void __fastcall__ game_state_playing_update()
{
    //update_player();
}
