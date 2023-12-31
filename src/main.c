#include <nes.h>
#include <stdlib.h>

#include "types.h"
#include "ppu.h"

#include "game_state.h"
#include "game_state_title.h"
#include "game_state_playing.h"

//
//
//

void main(void)
{
    game_state = GAME_STATE_INIT;
    next_game_state = GAME_STATE_TITLE;

    while( 1 )
    {
        if( game_state != next_game_state )
        {
            // leave current state
            switch( game_state )
            {
            case GAME_STATE_INIT:
                break;

            case GAME_STATE_TITLE:
                game_state_title_leave();
                break;

            case GAME_STATE_PLAYING:
                game_state_playing_leave();
                break;

            case GAME_STATE_GAME_OVER:
                break;
            }

            game_state = next_game_state;

            // enter current state
            switch( game_state )
            {
            case GAME_STATE_INIT:
                break;

            case GAME_STATE_TITLE:
                game_state_title_enter();
                break;

            case GAME_STATE_PLAYING:
                game_state_playing_enter();
                break;

            case GAME_STATE_GAME_OVER:
                break;
            }
        }

        switch( game_state )
        {
        case GAME_STATE_INIT:
            break;

        case GAME_STATE_TITLE:
            game_state_title_update();
            break;

        case GAME_STATE_PLAYING:
            game_state_playing_update();
            break;

        case GAME_STATE_GAME_OVER:
            break;
        }

        ppu_update();
    }
}
