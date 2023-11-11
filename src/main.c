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

#include "player.h"

//
//
//

#define GAME_STATE_INIT         (uint8_t)0
#define GAME_STATE_TITLE        (uint8_t)1
#define GAME_STATE_PLAYING      (uint8_t)2
#define GAME_STATE_GAME_OVER    (uint8_t)3

static uint8_t frame_index;
static uint8_t i, j;

static subpixel_t camera_x;
static subpixel_t camera_y;

static subpixel_t enemy_pos_x[32];
static subpixel_t enemy_pos_y[32];
static subpixel_t enemy_pos_dy[32];

static uint8_t game_state;

void main(void)
{
    init_player();
    
    game_state = GAME_STATE_INIT;

    subpixel_set( camera_x, 0, 0 );
    subpixel_set( camera_x, 0, 0 );

    while( 1 )
    {
        frame_index = ppu_frame_index();

        switch( game_state )
        {
        case GAME_STATE_INIT:
            game_state = GAME_STATE_TITLE;
            break;

        case GAME_STATE_TITLE:
            break;

        case GAME_STATE_PLAYING:
            break;

        case GAME_STATE_GAME_OVER:
            break;
        }

        update_player();

        ppu_set_scroll( make_scroll_subpixel( camera_x, camera_y ) );

        ppu_update();
    }
}
