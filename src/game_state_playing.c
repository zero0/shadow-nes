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

    init_player();
}

void __fastcall__ game_state_playing_leave()
{

}

void __fastcall__ game_state_playing_update()
{
    update_player();
}
