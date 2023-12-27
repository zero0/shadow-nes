#include "player.h"
#include "ppu.h"
#include "gamepad.h"
#include "subpixel.h"
#include "macros.h"
#include "args.h"
#include "globals.h"

static uint8_t current_gamepad_state;
static uint8_t prev_gamepad_state;
static uint8_t anim_frame;
static uint8_t flip_h, flip_v;

#define PLAYER_MOVEMENT_STATE_FACING_LEFT    0x01
#define PLAYER_MOVEMENT_STATE_GROUNDED       0x02
#define PLAYER_MOVEMENT_STATE_CLIMBING       0x04
#define PLAYER_MOVEMENT_STATE_WALKING        0x08
#define PLAYER_MOVEMENT_STATE_RUNNING        0x10
#define PLAYER_MOVEMENT_STATE_JUMPING        0x20
#define PLAYER_MOVEMENT_STATE_DEATH          0x80

#define PLAYER_ATTACK_STATE_ATTACKING      0x01
#define PLAYER_ATTACK_STATE_BLOCKING       0x02
#define PLAYER_ATTACK_STATE_HIT            0x04
#define PLAYER_ATTACK_STATE_STUN           0x08

typedef struct
{
    uint8_t timer;
    uint8_t set_index;
    uint8_t frame_index;
} anim_control_t;

static uint8_t player_movement_state_next;
static uint8_t player_movement_state;
static uint8_t player_attack_state;

static uint8_t player_health;
static uint8_t player_stamina;
static anim_control_t player_anim_head;
static anim_control_t player_anim_body;

static subpixel_t player_pos_x;
static subpixel_t player_pos_y;
static subpixel_diff_t player_pos_dx;
static subpixel_diff_t player_pos_dy;

static const subpixel_diff_t player_gravity = { 1, 0 };
static const subpixel_diff_t player_jump = { 5, 0 };
static const subpixel_diff_t player_air_speed = { 1, 128 };
static const subpixel_diff_t player_walking_speed = { 1, 128 };
static const subpixel_diff_t player_running_speed = { 2, 0 };
static const subpixel_diff_t player_ground_friction = { 1, 0 };

#define ANIM_SET_ATTR_NONE      0x00
#define ANIM_SET_ATTR_REPEAT    0x01

/*
typedef struct
{
    uint8_t start_frame;
    uint8_t end_frame;
    uint8_t attr;           // repeat, etc
} anim_set_t;

static anim_set_t temp_anim_set;
*/

#define PLAYER_HEAD_ANIM_IDLE   0
#define PLAYER_HEAD_ANIM_WALK   2

#define PLAYER_BODY_ANIM_IDLE   8
#define PLAYER_BODY_ANIM_WALK   9


#define ANIMATION_SET_IDLE    0
#define ANIMATION_SET_WALK    1
#define ANIMATION_SET_RUN     2
#define ANIMATION_SET_JUMP    3
#define ANIMATION_SET_BLOCK   4
#define ANIMATION_SET_ATTACK  5
#define ANIMATION_SET_HIT     6
#define ANIMATION_SET_DEATH   7
#define ANIMATION_SET__LENGTH 8

// head anim set
static const uint8_t anim_head_set_start_frame[] = {
    PLAYER_HEAD_ANIM_IDLE, // idle
    PLAYER_HEAD_ANIM_WALK, // walk
    PLAYER_HEAD_ANIM_WALK, // run
    PLAYER_HEAD_ANIM_IDLE, // jump / falling
    PLAYER_HEAD_ANIM_IDLE, // block
    PLAYER_HEAD_ANIM_IDLE, // attack
    PLAYER_HEAD_ANIM_IDLE, // hit / knockback
    PLAYER_HEAD_ANIM_IDLE, // death
};
STATIC_ASSERT( ARRAY_SIZE( anim_head_set_start_frame ) == ANIMATION_SET__LENGTH );

static const uint8_t anim_head_set_end_frame[] = {
    PLAYER_HEAD_ANIM_IDLE + 1, // idle
    PLAYER_HEAD_ANIM_WALK + 3, // walk
    PLAYER_HEAD_ANIM_WALK + 3, // run
    PLAYER_HEAD_ANIM_IDLE + 0, // jump / falling
    PLAYER_HEAD_ANIM_IDLE + 0, // block
    PLAYER_HEAD_ANIM_IDLE + 0, // attack
    PLAYER_HEAD_ANIM_IDLE + 0, // hit / knockback
    PLAYER_HEAD_ANIM_IDLE + 0, // death
};
STATIC_ASSERT( ARRAY_SIZE( anim_head_set_end_frame ) == ANIMATION_SET__LENGTH );

static const uint8_t anim_head_set_attr[] = {
    ANIM_SET_ATTR_REPEAT, // idle
    ANIM_SET_ATTR_REPEAT, // walk
    ANIM_SET_ATTR_REPEAT, // run
    ANIM_SET_ATTR_NONE, // jump / falling
    ANIM_SET_ATTR_NONE, // block
    ANIM_SET_ATTR_NONE, // attack
    ANIM_SET_ATTR_NONE, // hit / knockback
    ANIM_SET_ATTR_NONE, // death
};
STATIC_ASSERT( ARRAY_SIZE( anim_head_set_attr ) == ANIMATION_SET__LENGTH );

// body anim set
static const uint8_t anim_body_set_start_frame[] = {
    PLAYER_BODY_ANIM_IDLE, // idle
    PLAYER_BODY_ANIM_WALK, // walk
    PLAYER_BODY_ANIM_WALK, // run
    PLAYER_BODY_ANIM_IDLE, // jump / falling
    PLAYER_BODY_ANIM_IDLE, // block
    PLAYER_BODY_ANIM_IDLE, // attack
    PLAYER_BODY_ANIM_IDLE, // hit / knockback
    PLAYER_BODY_ANIM_IDLE, // death
};
STATIC_ASSERT( ARRAY_SIZE( anim_body_set_start_frame ) == ANIMATION_SET__LENGTH );

static const uint8_t anim_body_set_end_frame[] = {
    PLAYER_BODY_ANIM_IDLE + 0, // idle
    PLAYER_BODY_ANIM_WALK + 2, // walk
    PLAYER_BODY_ANIM_WALK + 2, // run
    PLAYER_BODY_ANIM_IDLE + 0, // jump / falling
    PLAYER_BODY_ANIM_IDLE + 0, // block
    PLAYER_BODY_ANIM_IDLE + 0, // attack
    PLAYER_BODY_ANIM_IDLE + 0, // hit / knockback
    PLAYER_BODY_ANIM_IDLE + 0, // death
};
STATIC_ASSERT( ARRAY_SIZE( anim_body_set_end_frame ) == ANIMATION_SET__LENGTH );

static const uint8_t anim_body_set_attr[] = {
    ANIM_SET_ATTR_REPEAT, // idle
    ANIM_SET_ATTR_REPEAT, // walk
    ANIM_SET_ATTR_REPEAT, // run
    ANIM_SET_ATTR_NONE, // jump / falling
    ANIM_SET_ATTR_NONE, // block
    ANIM_SET_ATTR_NONE, // attack
    ANIM_SET_ATTR_NONE, // hit / knockback
    ANIM_SET_ATTR_NONE, // death
};
STATIC_ASSERT( ARRAY_SIZE( anim_body_set_attr ) == ANIMATION_SET__LENGTH );


#define ANIM_KEYFRAME_ATTR_NONE                 0x00
#define ANIM_KEYFRAME_ATTR_FLIP_H               0x01
#define ANIM_KEYFRAME_ATTR_FLIP_V               0x02
#define ANIM_KEYFRAME_ATTR_OVERRIDE_PALETTE     0x08
#define ANIM_KEYFRAME_ATTR_OVERRIDE_PALETTE_0   0x10
#define ANIM_KEYFRAME_ATTR_OVERRIDE_PALETTE_1   0x20
#define ANIM_KEYFRAME_ATTR_OVERRIDE_PALETTE_2   0x30
#define ANIM_KEYFRAME_ATTR_OVERRIDE_PALETTE_3   0x40
/*
typedef struct
{
    uint8_t sprite0;
    uint8_t sprite1;
    uint8_t frames;
    uint8_t x_offset;
    uint8_t y_offset;
    uint8_t attr;       // include flip, maybe pallete override too
} anim_keyframe_t;

static anim_keyframe_t temp_anim_keyframe;

static anim_keyframe_t anim_keyframes[] = {
    // head top
    // idle
    { 0, 1, 255, 0, 0, ANIM_KEYFRAME_ATTR_NONE },
    { 0, 1, 128, 0, 1, ANIM_KEYFRAME_ATTR_NONE },

    // head bottom
    // idle
    { 2, 3, 255, 0, 0, ANIM_KEYFRAME_ATTR_NONE },
    { 2, 3, 128, 0, 1, ANIM_KEYFRAME_ATTR_NONE },

    // body
    // idle
    { 4, 5, 255, 0, 0, ANIM_KEYFRAME_ATTR_NONE },
    // walk
    { 6, 7,  10, 0, 0, ANIM_KEYFRAME_ATTR_NONE },
    { 8, 9,  10, 0, 0, ANIM_KEYFRAME_ATTR_NONE },
    { 4, 5,  10, 0, 0, ANIM_KEYFRAME_ATTR_NONE },
};
*/
static const uint8_t anim_keyframes_sprite_0[] = {
    // head top
    // idle
    0,
    0,
    // walk
    0,
    0,
    0,
    0,

    // head botton
    // idle
    2,
    2,

    // body
    // idle
    4,
    // walk
    6,
    8,
    4,
};

static const uint8_t anim_keyframes_sprite_1[] = {
    // head top
    // idle
    1,
    1,
    // walk
    1,
    1,
    1,
    1,

    // head botton
    // idle
    3,
    3,

    // body
    // idle
    5,
    // walk
    7,
    9,
    5,
};

static const uint8_t anim_keyframes_frames[] = {
    // head top
    // idle
    60,
    30,
    // walk
    11,
    10,
    11,
    10,

    // head botton
    // idle
    255,
    128,

    // body
    // idle
    255,
    // walk
    10,
    10,
    10,
};

static const uint8_t anim_keyframes_x_offset[] = {
    // head top
    // idle
    0,
    -1,
    // walk
    0,
    -1,
    0,
    1,

    // head botton
    // idel
    0,
    0,

    // body
    // idle
    0,
    // walk
    0,
    0,
    0,
};

static const uint8_t anim_keyframes_y_offset[] = {
    // head top
    // idle
    0,
    0,
    // walk
    0,
    0,
    0,
    0,

    // head botton
    // idel
    0,
    1,

    // body
    // idle
    0,
    // walk
    0,
    0,
    0,
};

static const uint8_t anim_keyframes_attr[] = {
    // head top
    // idle
    ANIM_KEYFRAME_ATTR_NONE,
    ANIM_KEYFRAME_ATTR_FLIP_H,
    // walk
    ANIM_KEYFRAME_ATTR_NONE,
    ANIM_KEYFRAME_ATTR_NONE,
    ANIM_KEYFRAME_ATTR_NONE,
    ANIM_KEYFRAME_ATTR_NONE,

    // head botton
    // idel
    ANIM_KEYFRAME_ATTR_NONE,
    ANIM_KEYFRAME_ATTR_NONE,

    // body
    // idle
    ANIM_KEYFRAME_ATTR_NONE,
    // walk
    ANIM_KEYFRAME_ATTR_NONE,
    ANIM_KEYFRAME_ATTR_NONE,
    ANIM_KEYFRAME_ATTR_NONE,
};

// ensure all keyframes are the same size
STATIC_ASSERT( ARRAY_SIZE( anim_keyframes_sprite_0 ) == ARRAY_SIZE( anim_keyframes_sprite_1 ) );
STATIC_ASSERT( ARRAY_SIZE( anim_keyframes_sprite_1 ) == ARRAY_SIZE( anim_keyframes_frames ) );
STATIC_ASSERT( ARRAY_SIZE( anim_keyframes_frames ) == ARRAY_SIZE( anim_keyframes_x_offset ) );
STATIC_ASSERT( ARRAY_SIZE( anim_keyframes_x_offset ) == ARRAY_SIZE( anim_keyframes_y_offset ) );
STATIC_ASSERT( ARRAY_SIZE( anim_keyframes_y_offset ) == ARRAY_SIZE( anim_keyframes_attr ) );
STATIC_ASSERT( ARRAY_SIZE( anim_keyframes_attr ) == ARRAY_SIZE( anim_keyframes_sprite_0 ) );

static uint8_t f0;
static uint8_t f1;

void __fastcall__ init_player(void)
{
    STATE_RESET( player_movement_state );
    player_health = 100;
    player_stamina = 50;

    subpixel_set( player_pos_x, 0, 0 );
    subpixel_set( player_pos_y, 10, 0 );
    subpixel_set_zero( player_pos_dx );
    subpixel_set_zero( player_pos_dy );

    player_anim_head.set_index = ANIMATION_SET_IDLE;
    player_anim_head.frame_index = anim_head_set_start_frame[ player_anim_head.set_index ];
    player_anim_head.timer = anim_keyframes_frames[ player_anim_head.frame_index ];

    player_anim_body.set_index = ANIMATION_SET_IDLE;
    player_anim_body.frame_index = anim_body_set_start_frame[ player_anim_body.set_index ];
    player_anim_body.timer = anim_keyframes_frames[ player_anim_body.frame_index ];
}

static void __fastcall__ player_update_input(void)
{
    gamepad_poll(0);

    prev_gamepad_state = gamepad_prev_state(0);
    current_gamepad_state = gamepad_state(0);

    if( GAMEPAD_BTN_DONW( current_gamepad_state, GAMEPAD_A ) )
    {
        STATE_SET( player_movement_state_next, PLAYER_MOVEMENT_STATE_RUNNING );
    }
    else
    {
        STATE_CLEAR( player_movement_state_next, PLAYER_MOVEMENT_STATE_RUNNING );
    }

    if( GAMEPAD_BTN_DONW( current_gamepad_state, GAMEPAD_B ) )
    {
        //STATE_SET( player_movement_state_next, PLAYER_MOVEMENT_STATE_ATTACKING );
    }

    if( GAMEPAD_BTN_PRESSED( prev_gamepad_state, current_gamepad_state, GAMEPAD_U ) )
    {
        if( STATE_IS_SET( player_movement_state_next, PLAYER_MOVEMENT_STATE_GROUNDED ) )
        {
            subpixel_dec( player_pos_dy, player_jump );
        }
    }

    if( GAMEPAD_BTN_DONW( current_gamepad_state, GAMEPAD_R) )
    {
        //STATE_SET( player_movement_state_next, PLAYER_MOVEMENT_STATE_FACING_LEFT );
        if( 1 ) //&& STATE_IS_SET( player_movement_state_next, PLAYER_STATE_GROUNDED ) )
        {
            if( STATE_IS_SET( player_movement_state_next, PLAYER_MOVEMENT_STATE_RUNNING ) )
            {
                subpixel_inc( player_pos_dx, player_running_speed );
            }
            else
            {
                subpixel_inc( player_pos_dx, player_walking_speed );
            }
        }
        else
        {
            subpixel_inc( player_pos_dx, player_air_speed );
        }
    }
    else if( GAMEPAD_BTN_DONW( current_gamepad_state, GAMEPAD_L))
    {
        STATE_SET( player_movement_state_next, PLAYER_MOVEMENT_STATE_FACING_LEFT );
        if( 1 ) // && STATE_IS_SET( player_movement_state_next, PLAYER_STATE_GROUNDED ) )
        {
            if( STATE_IS_SET( player_movement_state_next, PLAYER_MOVEMENT_STATE_RUNNING ) )
            {
                subpixel_dec( player_pos_dx, player_running_speed );
            }
            else
            {
                subpixel_dec( player_pos_dx, player_walking_speed );
            }
        }
        else
        {
            subpixel_dec( player_pos_dx, player_air_speed );
        }
    }
}

static void __fastcall__ player_update_current_anim()
{
    player_anim_head.frame_index = anim_head_set_start_frame[ player_anim_head.set_index ];
    player_anim_head.timer = anim_keyframes_frames[ player_anim_head.frame_index ];

    player_anim_body.frame_index = anim_body_set_start_frame[ player_anim_body.set_index ];
    player_anim_body.timer = anim_keyframes_frames[ player_anim_body.frame_index ];
}

static void __fastcall__ player_update_state(void)
{
    if( STATE_IS_SET( player_movement_state, PLAYER_MOVEMENT_STATE_GROUNDED ) )
    {
        if( STATE_IS_SET( player_movement_state, PLAYER_MOVEMENT_STATE_WALKING ) )
        {
            player_anim_head.set_index = ANIMATION_SET_WALK;
            player_anim_body.set_index = ANIMATION_SET_WALK;

            player_update_current_anim();
        }
        else if( STATE_IS_SET( player_movement_state, PLAYER_MOVEMENT_STATE_RUNNING ) )
        {
            player_anim_head.set_index = ANIMATION_SET_RUN;
            player_anim_body.set_index = ANIMATION_SET_RUN;

            player_update_current_anim();
        }
        else
        {
            player_anim_head.set_index = ANIMATION_SET_IDLE;
            player_anim_body.set_index = ANIMATION_SET_IDLE;

            player_update_current_anim();
        }
    }
    else if( STATE_IS_SET( player_movement_state, PLAYER_MOVEMENT_STATE_JUMPING ) )
    {
        player_anim_head.set_index = ANIMATION_SET_JUMP;
        player_anim_body.set_index = ANIMATION_SET_JUMP;

        player_update_current_anim();
    }


}

#define store_get_at_index( s, t, arr, idx, p )     \
__asm__ ( "ldy %v", idx );                          \
__asm__ ( "lda %v+%b,y", arr, offsetof(t, p));      \
__asm__ ( "sta %v", s )


#define memcpy_array_to_struct( dst_t, dst_v, dst_m, src_t, src_a, src_idx, src_m ) \
__asm__ ( "ldy %v", src_idx );                          \
__asm__ ( "lda %v+%b,y", src_a, offsetof(src_t, src_m));      \
__asm__ ( "sta %v+%b", dst_v, offsetof(dst_t, dst_m) )


static void __fastcall__ player_update_anim(void)
{
    // head
    if( player_anim_head.timer == 0 )
    {
        // increment frame index
        ++player_anim_head.frame_index;

        // if the current frame is past the last frame, reset
        if( player_anim_head.frame_index > anim_head_set_end_frame[ player_anim_head.set_index ] )
        {
            // if the set is marked as 'repaea', set frame index back to start_frame
            if( STATE_IS_SET( anim_head_set_attr[ player_anim_head.set_index ], ANIM_SET_ATTR_REPEAT ) )
            {
                player_anim_head.frame_index = anim_head_set_start_frame[ player_anim_head.set_index ];
            }
            else
            // otherwise, set to end frame
            {
                player_anim_head.frame_index = anim_head_set_end_frame[ player_anim_head.set_index ];
            }
        }

        // update timer to new frame
        player_anim_head.timer = anim_keyframes_frames[ player_anim_head.frame_index ];
    }
    else
    {
        --player_anim_head.timer;
    }

    // body
    if( player_anim_body.timer == 0 )
    {
        // increment frame index
        ++player_anim_body.frame_index;

        // if the current frame is past the last frame, reset
        if( player_anim_body.frame_index > anim_body_set_end_frame[ player_anim_body.set_index ] )
        {
            // if the set is marked as 'repaea', set frame index back to start_frame
            if( STATE_IS_SET( anim_body_set_attr[ player_anim_body.set_index ], ANIM_SET_ATTR_REPEAT ) )
            {
                player_anim_body.frame_index = anim_body_set_start_frame[ player_anim_body.set_index ];
            }
            else
            // otherwise, set to end frame
            {
                player_anim_body.frame_index = anim_body_set_end_frame[ player_anim_body.set_index ];
            }
        }

        // update timer to new frame
        player_anim_body.timer = anim_keyframes_frames[ player_anim_body.frame_index ];
    }
    else
    {
        --player_anim_body.timer;
    }
}

static void __fastcall__ uint8_to_oam()
{
    v = ARGS[2] & 0x0F;
    switch( v )
    {
        case 0x0: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, '0' ); break;
        case 0x1: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, '1' ); break;
        case 0x2: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, '2' ); break;
        case 0x3: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, '3' ); break;
        case 0x4: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, '4' ); break;
        case 0x5: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, '5' ); break;
        case 0x6: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, '6' ); break;
        case 0x7: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, '7' ); break;
        case 0x8: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, '8' ); break;
        case 0x9: v = '0' + v; break; // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, '9' ); break;
        case 0xa: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, 'a' ); break;
        case 0xb: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, 'b' ); break;
        case 0xc: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, 'c' ); break;
        case 0xd: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, 'd' ); break;
        case 0xe: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, 'e' ); break;
        case 0xf: v = 'a' + ( v - 10 ); break; // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, 'f' ); break;
        default:  v = 'x'; break;
    }
    ppu_add_oam_sprite( ARGS[0]+8, ARGS[1], PALETTE_SPRITE_0, v );

    v = (ARGS[2] >> 4) & 0x0F;
    switch( v )
    {
        case 0x0: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, '0' ); break;
        case 0x1: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, '1' ); break;
        case 0x2: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, '2' ); break;
        case 0x3: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, '3' ); break;
        case 0x4: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, '4' ); break;
        case 0x5: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, '5' ); break;
        case 0x6: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, '6' ); break;
        case 0x7: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, '7' ); break;
        case 0x8: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, '8' ); break;
        case 0x9: v = '0' + v; break; // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, '9' ); break;
        case 0xa: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, 'a' ); break;
        case 0xb: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, 'b' ); break;
        case 0xc: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, 'c' ); break;
        case 0xd: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, 'd' ); break;
        case 0xe: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, 'e' ); break;
        case 0xf: v = 'a' + ( v - 10 ); break; // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, 'f' ); break;
        default:  v = 'x'; break;
    }
    ppu_add_oam_sprite( ARGS[0], ARGS[1], PALETTE_SPRITE_0, v );
}

static void __fastcall__ uint8_to_chr()
{
    v = ARGS[2] & 0x0F;
    switch( v )
    {
        case 0x0: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, '0' ); break;
        case 0x1: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, '1' ); break;
        case 0x2: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, '2' ); break;
        case 0x3: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, '3' ); break;
        case 0x4: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, '4' ); break;
        case 0x5: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, '5' ); break;
        case 0x6: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, '6' ); break;
        case 0x7: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, '7' ); break;
        case 0x8: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, '8' ); break;
        case 0x9: v = '0' + v; break; // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, '9' ); break;
        case 0xa: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, 'a' ); break;
        case 0xb: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, 'b' ); break;
        case 0xc: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, 'c' ); break;
        case 0xd: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, 'd' ); break;
        case 0xe: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, 'e' ); break;
        case 0xf: v = 'a' + ( v - 10 ); break; // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, 'f' ); break;
        default:  v = 'x'; break;
    }
    ppu_update_tile( ARGS[0]+1, ARGS[1], v );

    v = (ARGS[2] >> 4) & 0x0F;
    switch( v )
    {
        case 0x0: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, '0' ); break;
        case 0x1: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, '1' ); break;
        case 0x2: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, '2' ); break;
        case 0x3: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, '3' ); break;
        case 0x4: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, '4' ); break;
        case 0x5: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, '5' ); break;
        case 0x6: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, '6' ); break;
        case 0x7: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, '7' ); break;
        case 0x8: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, '8' ); break;
        case 0x9: v = '0' + v; break; // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, '9' ); break;
        case 0xa: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, 'a' ); break;
        case 0xb: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, 'b' ); break;
        case 0xc: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, 'c' ); break;
        case 0xd: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, 'd' ); break;
        case 0xe: // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, 'e' ); break;
        case 0xf: v = 'a' + ( v - 10 ); break; // ppu_add_oam_sprite( x, y, SPRITE_0, PALETTE_SPRITE_0, 'f' ); break;
        default:  v = 'x'; break;
    }
    ppu_update_tile( ARGS[0], ARGS[1], v );
}

static void __fastcall__ player_update_physics(void)
{
    // horizontal movement
    subpixel_inc( player_pos_x, player_pos_dx );

    if( player_pos_dx.pix > 0 )
    {
        subpixel_dec( player_pos_dx, player_ground_friction );
        if( player_pos_dx.pix <= 0 )
        {
            subpixel_set_zero( player_pos_dx );
        }
        else
        {
            STATE_SET( player_movement_state_next, PLAYER_MOVEMENT_STATE_WALKING );
        }
    }
    else if( player_pos_dx.pix < 0 )
    {
        subpixel_inc( player_pos_dx, player_ground_friction );
        if( player_pos_dx.pix >= 0 )
        {
            subpixel_set_zero( player_pos_dx );
        }
        else
        {
            STATE_SET( player_movement_state_next, PLAYER_MOVEMENT_STATE_WALKING );
        }
    }
    else
    {
        subpixel_set_zero( player_pos_dx );
    }

    // gravity
    subpixel_inc( player_pos_y, player_pos_dy );

    if( player_pos_y.pix > 200 )
    {
        player_pos_y.pix = 200;
        player_pos_y.sub = 0;
        subpixel_set_zero( player_pos_dy );
        STATE_SET( player_movement_state_next, PLAYER_MOVEMENT_STATE_GROUNDED );
    }

    if( player_pos_y.pix == 200 )
    {
        STATE_SET( player_movement_state_next, PLAYER_MOVEMENT_STATE_GROUNDED );
        STATE_CLEAR( player_movement_state_next, PLAYER_MOVEMENT_STATE_JUMPING );
    }
    else
    {
        subpixel_inc( player_pos_dy, player_gravity);
        STATE_CLEAR( player_movement_state_next, PLAYER_MOVEMENT_STATE_GROUNDED );
        STATE_SET( player_movement_state_next, PLAYER_MOVEMENT_STATE_JUMPING );
    }

    //uint8_to_c( 2, 2, 55 );
}

void __fastcall__ player_render()
{
    // head
    flip_h = STATE_IS_SET( player_movement_state, PLAYER_MOVEMENT_STATE_FACING_LEFT );
    if( STATE_IS_SET( anim_keyframes_attr[ player_anim_head.frame_index], ANIM_KEYFRAME_ATTR_FLIP_H ) )
    {
        flip_h = !flip_h;
    }

    l = flip_h ? 8 : 0;
    r = flip_h ? 0 : 8;

    ppu_add_oam_sprite_full( player_pos_x.pix + anim_keyframes_x_offset[player_anim_head.frame_index] + l, player_pos_y.pix + anim_keyframes_y_offset[player_anim_head.frame_index] - 18, PALETTE_SPRITE_2, 0, flip_h, 0, anim_keyframes_sprite_0[player_anim_head.frame_index] );
    ppu_add_oam_sprite_full( player_pos_x.pix + anim_keyframes_x_offset[player_anim_head.frame_index] + r, player_pos_y.pix + anim_keyframes_y_offset[player_anim_head.frame_index] - 18, PALETTE_SPRITE_2, 0, flip_h, 0, anim_keyframes_sprite_1[player_anim_head.frame_index] );

    ppu_add_oam_sprite_full( player_pos_x.pix + anim_keyframes_x_offset[player_anim_head.frame_index] + l, player_pos_y.pix + anim_keyframes_y_offset[player_anim_head.frame_index] - 10, PALETTE_SPRITE_2, 0, flip_h, 0, 2 );
    ppu_add_oam_sprite_full( player_pos_x.pix + anim_keyframes_x_offset[player_anim_head.frame_index] + r, player_pos_y.pix + anim_keyframes_y_offset[player_anim_head.frame_index] - 10, PALETTE_SPRITE_2, 0, flip_h, 0, 3 );

    // body
    flip_h = STATE_IS_SET( player_movement_state, PLAYER_MOVEMENT_STATE_FACING_LEFT );
    if( STATE_IS_SET( anim_keyframes_attr[ player_anim_body.frame_index], ANIM_KEYFRAME_ATTR_FLIP_H ) )
    {
        flip_h = !flip_h;
    }

    l = flip_h ? 8 : 0;
    r = flip_h ? 0 : 8;

    ppu_add_oam_sprite_full( player_pos_x.pix + anim_keyframes_x_offset[player_anim_body.frame_index] + l, player_pos_y.pix + anim_keyframes_y_offset[player_anim_body.frame_index] - 8, PALETTE_SPRITE_2, 0, flip_h, 0, anim_keyframes_sprite_0[player_anim_body.frame_index] );
    ppu_add_oam_sprite_full( player_pos_x.pix + anim_keyframes_x_offset[player_anim_body.frame_index] + r, player_pos_y.pix + anim_keyframes_y_offset[player_anim_body.frame_index] - 8, PALETTE_SPRITE_2, 0, flip_h, 0, anim_keyframes_sprite_1[player_anim_body.frame_index] );
}

void __fastcall__ player_render_debug()
{
    ppu_update_tile( 0, 0, 'x' );
    args_call_3( uint8_to_chr, 2, 0, player_pos_x.pix );
    args_call_3( uint8_to_chr, 4, 0, player_pos_x.sub );

    args_call_3( uint8_to_chr, 7, 0, player_pos_dx.pix );
    args_call_3( uint8_to_chr, 9, 0, player_pos_dx.sub );

    args_call_3( uint8_to_chr, 12, 0, player_anim_head.timer );
    args_call_3( uint8_to_chr, 15, 0, player_anim_head.frame_index );
    args_call_3( uint8_to_chr, 18, 0, player_movement_state );

    ppu_update_tile( 0, 1, 'y' );
    args_call_3( uint8_to_chr, 2, 1, player_pos_y.pix );
    args_call_3( uint8_to_chr, 4, 1, player_pos_y.sub );

    args_call_3( uint8_to_chr, 7, 1, player_pos_dy.pix );
    args_call_3( uint8_to_chr, 9, 1, player_pos_dy.sub );

    args_call_3( uint8_to_chr, 12, 1, player_anim_body.timer );
    args_call_3( uint8_to_chr, 15, 1, player_anim_body.frame_index );
}

void __fastcall__ update_player()
{
    player_render_debug();

    player_update_input();

    player_update_physics();

    if( player_movement_state != player_movement_state_next )
    {
        player_movement_state = player_movement_state_next;
        player_movement_state_next = 0;

        player_update_state();
    }

    player_update_anim();

    player_render();
}