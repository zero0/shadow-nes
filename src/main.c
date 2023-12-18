//static int d[20];
//#define i d[0]
//#define b d[1];
//static char i, b;

#include <nes.h>
#include <stdlib.h>

#include "types.h"
#include "macros.h"
#include "gamepad.h"
#include "ppu.h"
#include "subpixel.h"

#include "game_state_title.h"

#include "player.h"

//
//
//

#define GAME_STATE_INIT         (uint8_t)0
#define GAME_STATE_TITLE        (uint8_t)1
#define GAME_STATE_PLAYING      (uint8_t)2
#define GAME_STATE_GAME_OVER    (uint8_t)3

static uint8_t i, j;

static subpixel_t camera_x;
static subpixel_t camera_y;

static subpixel_t enemy_pos_x[32];
static subpixel_t enemy_pos_y[32];
static subpixel_t enemy_pos_dy[32];

static uint8_t game_state;
static uint8_t next_game_state;

void main(void)
{
    game_state = GAME_STATE_INIT;
    next_game_state = GAME_STATE_TITLE;

    subpixel_set( camera_x, 0, 0 );
    subpixel_set( camera_x, 0, 0 );

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
                init_player();
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
            update_player();

            ppu_set_scroll( make_scroll_subpixel( camera_x, camera_y ) );
            break;

        case GAME_STATE_GAME_OVER:
            break;
        }


        ppu_update();
    }
}
