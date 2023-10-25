//static int d[20];
//#define i d[0]
//#define b d[1];
//static char i, b;

#include <nes.h>
#include <stdlib.h>

#include "gamepad.h"
#include "ppu.h"

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

static char cursor_x;
static char cursor_y;

static ptr_t ptr;
static char i;

void main(void)
{
    num_players = 2;
cursor_x = 0;
cursor_y = 0;
        ptr = 0;
    while( 1 )
    {
        for( i = 0; i < num_players; ++i )
        {
            gamepad_poll( i );
        }

        current_gamepad_state = gamepad_state(0);

        if( GAMEPAD_BTN_DONW( current_gamepad_state, GAMEPAD_R))
        {
            ++cursor_x;
        }
        else if( GAMEPAD_BTN_DONW( current_gamepad_state, GAMEPAD_L))
        {
            --cursor_x;
        }
        
        if( GAMEPAD_BTN_DONW( current_gamepad_state, GAMEPAD_U))
        {
            --cursor_y;
        }
        else if( GAMEPAD_BTN_DONW( current_gamepad_state, GAMEPAD_D))
        {
            ++cursor_y;
        }

        ppu_off();


        ppu_set_scroll( MAKE_SCROLL( cursor_x, cursor_y ) );
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
