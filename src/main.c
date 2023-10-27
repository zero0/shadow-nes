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

static char current_gamepad_state;
static char num_players;

static ptr_t ptr;
static char i;
static char j;

static subpixel_t camera_x;
static subpixel_t camera_y;

static subpixel_t player_pos_x;
static subpixel_t player_pos_y;

static subpixel_t player2_pos_x;
static subpixel_t player2_pos_y;
static subpixel_t player2_pos_dy;

static subpixel_t enemy_pos_x[32];
static subpixel_t enemy_pos_y[32];
static subpixel_t enemy_pos_dy[32];

static subpixel_t temp_subpixel;
static subpixel_t temp_subpixel_0;
static subpixel_t temp_subpixel_1;

void main(void)
{
    num_players = 2;
    subpixel_set( camera_x, 128, 0 );
    subpixel_set( camera_x, 128, 0 );
    ptr = 0;

    subpixel_set( player_pos_x, 16, 0 );
    subpixel_set( player_pos_y, 15, 0 );

    subpixel_set( player2_pos_x, 30, 0 );
    subpixel_set( player2_pos_y, 30, 0 );
    subpixel_set( player2_pos_dy, 0, 0 );

    for( i = 0, j = 0; i < 32; i++, j += 8 )
    {
        subpixel_set( enemy_pos_x[i], j, 0 );
        subpixel_set( enemy_pos_y[i], j, 0 );
        subpixel_set( enemy_pos_dy[i], 0, 0 );
    }

    while( 1 )
    {
        for( i = 0; i < num_players; ++i )
        {
            gamepad_poll( i );
        }

        current_gamepad_state = gamepad_state(0);

        if( GAMEPAD_BTN_DONW( current_gamepad_state, GAMEPAD_A ) )
        {
            subpixel_set( temp_subpixel, 1, 200 );
        }
        else
        {
            subpixel_set( temp_subpixel, 0, 100 );
        }

        if( GAMEPAD_BTN_DONW( current_gamepad_state, GAMEPAD_B ) )
        {
            subpixel_inc( player_pos_x, temp_subpixel );
        }

        if( GAMEPAD_BTN_DONW( current_gamepad_state, GAMEPAD_R))
        {
            subpixel_inc( camera_x, temp_subpixel );
        }
        else if( GAMEPAD_BTN_DONW( current_gamepad_state, GAMEPAD_L))
        {
            subpixel_dec( camera_x, temp_subpixel );
        }
        
        if( GAMEPAD_BTN_DONW( current_gamepad_state, GAMEPAD_U))
        {
            subpixel_dec( camera_y, temp_subpixel );
        }
        else if( GAMEPAD_BTN_DONW( current_gamepad_state, GAMEPAD_D))
        {
            subpixel_inc( camera_y, temp_subpixel );
        }

        //ppu_off();

        //subpixel_set( temp_pos, 0, 8 );
        //subpixel_add( player_pos, player_pos, temp_pos );

        subpixel_set( temp_subpixel, 0, 1 );

        for( i = 0; i < 32; i++ )
        {
            subpixel_move( temp_subpixel_0, enemy_pos_dy[i]);
            subpixel_move( temp_subpixel_1, enemy_pos_y[i]);

            subpixel_inc( temp_subpixel_0, temp_subpixel );

            subpixel_inc( temp_subpixel_1, temp_subpixel_0 );

            if( enemy_pos_y[i].pix > (SCREEN_HEIGH-8) )
            {
                subpixel_set( temp_subpixel_0, -1, 0 );
            }

            subpixel_move( enemy_pos_dy[i], temp_subpixel_0 );
            subpixel_move( enemy_pos_y[i], temp_subpixel_1);

            oam_sprite( subpixel_to_pixel(enemy_pos_x[i]), subpixel_to_pixel(enemy_pos_y[i]), SPRITE_0, PALETTE_SPRITE_0 + (i & 0x03), 80 );
        }

        //oam_sprite( subpixel_to_pixel(player2_pos_x), subpixel_to_pixel(player2_pos_y), SPRITE_0, PALETTE_SPRITE_2, 80 );

        ppu_set_scroll( MAKE_SCROLL( subpixel_to_pixel( camera_x ), subpixel_to_pixel( camera_y ) ) );
        //ARGS.ppu_fill_nametable_attr_args.address = 0x2000;
        //ARGS.ppu_fill_nametable_attr_args.tile = 65;
        //ARGS.ppu_fill_nametable_attr_args.attr = 44;
        //ptr = 0x2400;
        //ppu_fill_nametable_attr( ptr );
//
        //ARGS.ppu_fill_nametable_attr_args.tile = 0;
        //ARGS.ppu_fill_nametable_attr_args.attr = 0;
        //ptr = 0x2000;
        //ppu_fill_nametable_attr( ptr );

        //ppu_clear_nametable( 0x2000 );
        
        //ppu_address_tile( 1, 2 );

        //ppu_update_tile(cursor_x, cursor_y, 34);

        ppu_update();
    }
}
