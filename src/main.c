//static int d[20];
//#define i d[0]
//#define b d[1];
//static char i, b;

#include "gamepad.h"
#include <nes.h>
#include <stdlib.h>

typedef unsigned int ptr_t;
typedef unsigned char uint8_t;

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

void __fastcall__ ppu_update(void);

void __fastcall__ ppu_off(void);

void __fastcall__ ppu_skip(void);

void __fastcall__ ppu_address_tile( char x, char y );

void __fastcall__ ppu_update_tile( char x, char y, char t );

void __fastcall__ ppu_update_byte( char x, char y, char b );

void __fastcall__ ppu_clear_nametable( ptr_t tableAddress );

void __fastcall__ ppu_fill_nametable( ptr_t tableAddress, char tile );

void __fastcall__ ppu_fill_nametable_attr( ptr_t tableAddress );

void __fastcall__ ppu_fill_nametable_attr_only( ptr_t tableAddress, char attr );


extern args_t ARGS;
#pragma zpsym("ARGS");

//
//
//

static char current_gamepad_state[4];
static char prev_gamepad_state[4];
static char gamepad_index;
static char num_players;

static char cursor_x;
static char cursor_y;

static ptr_t ptr;
static char i;

void main(void)
{
    gamepad_index = 0;
    num_players = 2;
cursor_x = 1;
cursor_y = 2;
        ptr = 0;
    while( 1 )
    {
        for( i = 0; i < num_players; ++i )
        {
            gamepad_poll( i );
            current_gamepad_state[i] = gamepad_state(i);
            prev_gamepad_state[i] = gamepad_prev_state(i);
        }

        ppu_off();

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
