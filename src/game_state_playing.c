#include "types.h"
#include "macros.h"
#include "gamepad.h"
#include "subpixel.h"
#include "player.h"

void __fastcall__ game_state_playing_enter()
{
    init_player();
}

void __fastcall__ game_state_playing_leave()
{

}

void __fastcall__ game_state_playing_update()
{
    update_player();
}
