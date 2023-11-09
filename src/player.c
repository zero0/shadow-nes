#include "player.h"
#include "ppu.h"
#include "gamepad.h"
#include "subpixel.h"
#include "macros.h"

static uint8_t i;
static uint8_t j;

static uint8_t current_gamepad_state;
static uint8_t l, r;
static uint8_t anim_frame;
static uint8_t flip;


#define PLAYER_STATE_FACING_LEFT    0x01
#define PLAYER_STATE_HIT            0x02

#define PLAYER_STATE_WALKING        0x10
#define PLAYER_STATE_RUNNING        0x20
#define PLAYER_STATE_ATTACKING      0x40



static uint8_t player_state;
static uint8_t player_health;
static uint8_t player_stamina;
static uint8_t player_anim;

static subpixel_t player_pos_x;
static subpixel_t player_pos_y;
static subpixel_t player_pos_dy;

static subpixel_t temp_subpixel;
static subpixel_t temp_subpixel_0;
static subpixel_t temp_subpixel_1;

static uint8_t anim_player_walk_frames[] = {
    4, 5,
    6, 7,
    8, 9
};

static uint8_t f0;
static uint8_t f1;

void __fastcall__ init_player()
{
    STATE_RESET( player_state );
    player_health = 100;
    player_stamina = 50;

    subpixel_set( player_pos_x, 40, 0 );
    subpixel_set( player_pos_y, 40, 0 );
    subpixel_set_zero( player_pos_dy );
}

void __fastcall__ update_player()
{
    current_gamepad_state = gamepad_state(0);

    if( GAMEPAD_BTN_DONW( current_gamepad_state, GAMEPAD_A ) )
    {
        subpixel_set( temp_subpixel, 2, 128 );
    }
    else
    {
        subpixel_set( temp_subpixel, 1, 100 );
    }

    STATE_CLEAR( player_state, PLAYER_STATE_WALKING );

    if( GAMEPAD_BTN_DONW( current_gamepad_state, GAMEPAD_B ) )
    {
        STATE_SET( player_state, PLAYER_STATE_ATTACKING );
    }

    if( GAMEPAD_BTN_DONW( current_gamepad_state, GAMEPAD_R))
    {
        STATE_CLEAR( player_state, PLAYER_STATE_FACING_LEFT );
        STATE_SET( player_state, PLAYER_STATE_WALKING );
        subpixel_add( player_pos_x, player_pos_x, temp_subpixel );
    }
    else if( GAMEPAD_BTN_DONW( current_gamepad_state, GAMEPAD_L))
    {
        STATE_SET( player_state, PLAYER_STATE_FACING_LEFT );
        STATE_SET( player_state, PLAYER_STATE_WALKING );
        subpixel_sub( player_pos_x, player_pos_x, temp_subpixel );
    }
    
    flip = STATE_IS_SET( player_state, PLAYER_STATE_FACING_LEFT );

    i = 10;
    j = 2;
    l = flip ? 0 : 8;
    r = flip ? 8 : 0;

    // head
    oam_sprite_full( subpixel_to_pixel(player_pos_x)-l, subpixel_to_pixel(player_pos_y)-i, SPRITE_1, PALETTE_SPRITE_0, 0, flip, 0, 0 );
    oam_sprite_full( subpixel_to_pixel(player_pos_x)-r, subpixel_to_pixel(player_pos_y)-i, SPRITE_1, PALETTE_SPRITE_0, 0, flip, 0, 1 );

    oam_sprite_full( subpixel_to_pixel(player_pos_x)-l, subpixel_to_pixel(player_pos_y)-j , SPRITE_1, PALETTE_SPRITE_0, 0, flip, 0, 2 );
    oam_sprite_full( subpixel_to_pixel(player_pos_x)-r, subpixel_to_pixel(player_pos_y)-j , SPRITE_1, PALETTE_SPRITE_0, 0, flip, 0, 3 );

    if( STATE_IS_SET( player_state, PLAYER_STATE_WALKING ) )
    {
        anim_frame = ppu_frame_index() & 0x3;
        f0 = anim_frame * 2 + 6;
        f1 = anim_frame * 2 + 7;
    }
    else
    {
        f0 = 4; //anim_player_walk_frames[0];
        f1 = 5; //anim_player_walk_frames[1];
    }

    // body
    oam_sprite_full( subpixel_to_pixel(player_pos_x)-l, subpixel_to_pixel(player_pos_y)-0, SPRITE_1, PALETTE_SPRITE_0, 0, flip, 0, f0 );
    oam_sprite_full( subpixel_to_pixel(player_pos_x)-r, subpixel_to_pixel(player_pos_y)-0, SPRITE_1, PALETTE_SPRITE_0, 0, flip, 0, f1 );

    // generic head
    //oam_sprite_full( subpixel_to_pixel(player_pos_x)-l, subpixel_to_pixel(player_pos_y)-8, SPRITE_1, PALETTE_SPRITE_0, 0, flip, 0, 10 );
    //oam_sprite_full( subpixel_to_pixel(player_pos_x)-r, subpixel_to_pixel(player_pos_y)-8, SPRITE_1, PALETTE_SPRITE_0, 0, flip, 0, 11 );
     
}