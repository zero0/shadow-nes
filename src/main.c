#include <nes.h>
#include <stdlib.h>

#include "types.h"
#include "ppu.h"
#include "timer.h"

#include "game_state.h"
#include "game_state_title.h"
#include "game_state_playing.h"
#include "game_state_cutscene.h"
#include "game_state_checkpoint.h"
#include "game_state_store.h"

//
//
//

typedef void (*game_state_func)(void);

static void __fastcall__ game_state_noop(void){}

static const game_state_func game_state_leave_func[] = {
    game_state_noop,
    game_state_title_leave,
    game_state_playing_leave,
    game_state_noop,
    game_state_cutscene_leave,
    game_state_store_leave,
    game_state_noop,
    game_state_noop,
};
static const game_state_func game_state_enter_func[] = {
    game_state_noop,
    game_state_title_enter,
    game_state_playing_enter,
    game_state_noop,
    game_state_cutscene_enter,
    game_state_store_enter,
    game_state_checkpoint_enter,
    game_state_noop,
};
static const game_state_func game_state_update_func[] = {
    game_state_noop,
    game_state_title_update,
    game_state_playing_update,
    game_state_noop,
    game_state_cutscene_update,
    game_state_store_update,
    game_state_checkpoint_update,
    game_state_noop,
};
STATIC_ASSERT( ARRAY_SIZE( game_state_leave_func ) == _GAME_STATE_COUNT );
STATIC_ASSERT( ARRAY_SIZE( game_state_enter_func ) == _GAME_STATE_COUNT );
STATIC_ASSERT( ARRAY_SIZE( game_state_update_func ) == _GAME_STATE_COUNT );

//
//
//

void main(void)
{
    game_state = GAME_STATE_INIT;
    set_next_game_state( GAME_STATE_TITLE );

    rt_timer_reset( game_rt_timer );

    while( 1 )
    {
        rt_timer_tick( game_rt_timer );

        // transition states
        if( game_state != next_game_state )
        {
            ppu_wait_vblank();

            ppu_off();

            game_state_leave_func[game_state]();

            game_state = next_game_state;

            game_state_enter_func[game_state]();

            ppu_wait_vblank();

            ppu_on();

            continue;
        }

        // call state update
        game_state_update_func[game_state]();

        // wait for ppu
        ppu_update();
    }
}
