//static int d[20];
//#define i d[0]
//#define b d[1];
//static char i, b;

#include "gamepad.h"
#include <nes.h>
#include <stdlib.h>

typedef struct
{
    char x;
    char y;
    char tile;
} tile_update_t;

void __fastcall__ ppu_update(void);

void __fastcall__ ppu_off(void);

void __fastcall__ ppu_skip(void);

void __fastcall__ ppu_address_tile( char x, char y );

void __fastcall__ ppu_update_tile( char x, char y, char t );

void __fastcall__ ppu_update_tile_ptr( tile_update_t* t );


extern tile_update_t temp;
#pragma zpsym("temp");

//
//
//

static char current_gamepad_state[4];
static char prev_gamepad_state[4];
static char gamepad_index;
static char num_players;

static char cursor_x;
static char cursor_y;

static char i;
//static tile_update_t ttt;

void main(void)
{
    gamepad_index = 0;
    num_players = 2;
cursor_x = 1;
cursor_y = 2;
    while( 1 )
    {
        //for( i = 0; i < num_players; ++i )
        //{
        //    gamepad_poll( i );
        //    current_gamepad_state[i] = gamepad_state(i);
        //    prev_gamepad_state[i] = gamepad_prev_state(i);
        //}

        //ppu_off();

        //ppu_address_tile( 1, 2 );

        ppu_update_tile(cursor_x, cursor_y, 34);

        ppu_update();
    }
}
