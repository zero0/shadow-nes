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

typedef struct
{
    void* address;
    char tile;
} nametable_tile_t;

typedef struct
{
    void* address;
    char attr;
}  nametable_attribute_t;

typedef struct
{
    ptr_t address;
    char tile;
    char attr;
} nametable_fill_t;

typedef struct{
    ptr_t address;
    char tile;
    char attr;
} ppu_fill_nametable_attr_args_t;

typedef union
{
    ppu_fill_nametable_attr_args_t ppu_fill_nametable_attr_args;
} args_t;

extern args_t ARGS;
#pragma zpsym("ARGS");

//
//
//

static uint8_t frame_index;
static uint8_t i, j;

static subpixel_t camera_x;
static subpixel_t camera_y;

static subpixel_t enemy_pos_x[32];
static subpixel_t enemy_pos_y[32];
static subpixel_t enemy_pos_dy[32];

void main(void)
{
    init_player();
    
    subpixel_set( camera_x, 128, 0 );
    subpixel_set( camera_x, 128, 0 );

    for( i = 0, j = 0; i < 32; i++, j += 8 )
    {
        subpixel_set( enemy_pos_x[i], j, 0 );
        subpixel_set( enemy_pos_y[i], j, 0 );
        subpixel_set( enemy_pos_dy[i], 0, 0 );
    }

    ppu_oam_clear();

    ppu_update_tile( 0, 20, 0xE );

    while( 1 )
    {
        frame_index = ppu_frame_index();

        gamepad_poll( 0 );

        //current_gamepad_state = gamepad_state(0);

        //if( GAMEPAD_BTN_DONW( current_gamepad_state, GAMEPAD_A ) )
        //{
        //    subpixel_set( temp_subpixel, 1, 200 );
        //}
        //else
        //{
        //    subpixel_set( temp_subpixel, 0, 100 );
        //}
//
        //if( GAMEPAD_BTN_DONW( current_gamepad_state, GAMEPAD_R))
        //{
        //    subpixel_inc( camera_x, temp_subpixel );
        //}
        //else if( GAMEPAD_BTN_DONW( current_gamepad_state, GAMEPAD_L))
        //{
        //    subpixel_dec( camera_x, temp_subpixel );
        //}
        //
        //if( GAMEPAD_BTN_DONW( current_gamepad_state, GAMEPAD_U))
        //{
        //    subpixel_dec( camera_y, temp_subpixel );
        //}
        //else if( GAMEPAD_BTN_DONW( current_gamepad_state, GAMEPAD_D))
        //{
        //    subpixel_inc( camera_y, temp_subpixel );
        //}

        //ppu_off();

        //subpixel_set( temp_pos, 0, 8 );
        //subpixel_add( player_pos, player_pos, temp_pos );

        //subpixel_set( temp_subpixel, 0, 1 );

        //for( i = 0; i < 32; i++ )
        //{
        //    subpixel_move( temp_subpixel_0, enemy_pos_dy[i]);
        //    subpixel_move( temp_subpixel_1, enemy_pos_y[i]);
//
        //    subpixel_inc( temp_subpixel_0, temp_subpixel );
//
        //    subpixel_inc( temp_subpixel_1, temp_subpixel_0 );
//
        //    if( enemy_pos_y[i].pix > (SCREEN_HEIGH-8) )
        //    {
        //        subpixel_set( temp_subpixel_0, -1, 0 );
        //    }
//
        //    subpixel_move( enemy_pos_dy[i], temp_subpixel_0 );
        //    subpixel_move( enemy_pos_y[i], temp_subpixel_1);
//
        //    oam_sprite( subpixel_to_pixel(enemy_pos_x[i]), subpixel_to_pixel(enemy_pos_y[i]), SPRITE_0, PALETTE_SPRITE_0 + (i & 0x03), 80 );
        //}
//
        //if( subpixel_cmp( camera_x, camera_y ) == 0 )
        //{
//
        //}
//
        //if( subpixel_cmp( camera_x, camera_y ) < 0 )
        //{
//
        //}


        update_player();

        ppu_set_scroll( make_scroll_subpixel( camera_x, camera_y ) );

        ppu_update();
    }
}
