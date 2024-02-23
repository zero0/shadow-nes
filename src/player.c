#include "player.h"
#include "ppu.h"
#include "gamepad.h"
#include "subpixel.h"
#include "macros.h"
#include "args.h"
#include "globals.h"
#include "flags.h"
#include "timer.h"
#include "combat.h"
#include "game_state.h"
#include "game_data.h"

#define PLAYER_HEALTH_LIMIT                     (uint8_t)224
#define PLAYER_STAMINA_LIMIT                    (uint8_t)112

#define PLAYER_FLASK_COOLDOWN_TIME              (uint8_t)180
#define PLAYER_DODGE_COOLDOWN_TIME              (uint8_t)15
#define PLAYER_ATTACK0_COOLDOWN_TIME            (uint8_t)15
#define PLAYER_ATTACK1_COOLDOWN_TIME            (uint8_t)20

#define PLAYER_DODGE_INV_TIMER                  (uint8_t)40
#define PLAYER_STAMINA_DELAY_TIMER_NORMAL       (uint8_t)20
#define PLAYER_STAMINA_DELAY_TIMER_OVERDRAW     (uint8_t)60

#define PLAYER_MAX_LEVEL                        (uint8_t)4

#define PLAYER_MOVE_DIRECTION_NONE              (uint8_t)0
#define PLAYER_MOVE_DIRECTION_WEST              (uint8_t)( 1 << 0 )
#define PLAYER_MOVE_DIRECTION_EAST              (uint8_t)( 1 << 1 )
#define PLAYER_MOVE_DIRECTION_NORTH             (uint8_t)( 1 << 2 )
#define PLAYER_MOVE_DIRECTION_SOUTH             (uint8_t)( 1 << 3 )

#define PLAYER_STATE_IDLE                       (uint8_t)0
#define PLAYER_STATE_ATTACKING                  (uint8_t)1
#define PLAYER_STATE_DODGING                    (uint8_t)2
#define PLAYER_STATE_USING_FLASH                (uint8_t)3
#define PLAYER_STATE_HIT                        (uint8_t)4
#define PLAYER_STATE_DEAD                       (uint8_t)5

#define PLAYER_DAMAGE_MAX_QUEUE_LENGTH          (uint8_t)4
STATIC_ASSERT(IS_POW2(PLAYER_DAMAGE_MAX_QUEUE_LENGTH));

#define PLAYER_ACTION_MAX_QUEUE_LENGTH           (uint8_t)8
#define PLAYER_ACTION_MAX_QUEUE_LENGTH_MASK      (uint8_t)(PLAYER_ACTION_MAX_QUEUE_LENGTH - 1)
STATIC_ASSERT(IS_POW2(PLAYER_ACTION_MAX_QUEUE_LENGTH));

#define PLAYER_ANIMATION_NONE                   (uint8_t)0xFF

#define PLAYER_CAN_PERFORM_ACTION_NONE          (uint8_t)0
#define PLAYER_CAN_PERFORM_ACTION_MOVE          (uint8_t)( 1 << 0 )
#define PLAYER_CAN_PERFORM_ACTION_DODGE         (uint8_t)( 1 << 1 )
#define PLAYER_CAN_PERFORM_ACTION_FLASK         (uint8_t)( 1 << 2 )
#define PLAYER_CAN_PERFORM_ACTION_ATTACK0       (uint8_t)( 1 << 3 )
#define PLAYER_CAN_PERFORM_ACTION_ATTACK1       (uint8_t)( 1 << 4 )
#define PLAYER_CAN_PERFORM_ACTION_KNOCKBACK     (uint8_t)( 1 << 5 )
#define PLAYER_CAN_PERFORM_ACTION_ALL           (uint8_t)~0

enum
{
    Player_Action_Dodge,
    Player_Action_Flask,
    Player_Action_Attack0,
    Player_Action_Attack1,
};

extern ptr_t knight_sprite_0;
extern ptr_t knight_sprite_1;
extern ptr_t knight_sprite_2;

extern ptr_t progress_bar;

static uint8_t player_state;
static uint8_t player_next_state;

static flags8_t player_can_perform_action_flags;

static flags8_t player_changed_flags;
static uint8_t player_health;
static uint8_t player_stamina;
static uint8_t player_flasks;
static damage_status_t player_damage_status;
static uint8_t player_combat_position;

static timer_t player_flash_cooldown_timer;
static timer_t player_dodge_cooldown_timer;
static timer_t player_attack_cooldown_timer;
static timer_t player_attack_combo_timer;

static timer_t player_inv_frame_timer;
static timer_t player_animation_frame_timer;
static timer_t player_stamina_regen_timer;
static timer_t player_stamina_delay_timer;

static uint8_t player_playing_animation;

static subpixel_t player_pos_x;
static subpixel_t player_pos_y;
static subpixel_diff_t player_pos_dx;
static subpixel_diff_t player_pos_dy;

static damage_t player_damage_queue[PLAYER_DAMAGE_MAX_QUEUE_LENGTH];
STATIC_ASSERT(ARRAY_SIZE(player_damage_queue) == PLAYER_DAMAGE_MAX_QUEUE_LENGTH);
static uint8_t player_damage_queue_length;

static uint8_t player_action_queue[PLAYER_ACTION_MAX_QUEUE_LENGTH];
STATIC_ASSERT(ARRAY_SIZE(player_action_queue) == PLAYER_ACTION_MAX_QUEUE_LENGTH);
static uint8_t player_action_queue_length;
static uint8_t player_action_queue_index;

static damage_t _temp_dmg;

#ifdef DEBUG
static uint8_t debug_player_draw_debug;
#endif

//
// Per-Level Data
//

static const uint8_t player_max_health_per_level[] = {
    64,
    80,
    112,
    176,
};
STATIC_ASSERT(ARRAY_SIZE(player_max_health_per_level) == PLAYER_MAX_LEVEL);

static const uint8_t player_max_stamina_per_level[] = {
    32,
    48,
    64,
    80,
};
STATIC_ASSERT(ARRAY_SIZE(player_max_stamina_per_level) == PLAYER_MAX_LEVEL);

static const uint8_t player_max_flasks_per_level[] = {
    2,
    3,
    4,
    5,
};
STATIC_ASSERT(ARRAY_SIZE(player_max_flasks_per_level) == PLAYER_MAX_LEVEL);

static const uint8_t player_flask_heal_per_level[] = {
    40,
    45,
    50,
    55,
};
STATIC_ASSERT(ARRAY_SIZE(player_flask_heal_per_level) == PLAYER_MAX_LEVEL);

static const uint8_t player_stamina_regen_time_per_level[] = {
    5,
    6,
    7,
    8,
};
STATIC_ASSERT(ARRAY_SIZE(player_stamina_regen_time_per_level) == PLAYER_MAX_LEVEL);

static const uint8_t player_stamina_regen_amount_per_level[] = {
    2,
    4,
    6,
    8,
};
STATIC_ASSERT(ARRAY_SIZE(player_stamina_regen_amount_per_level) == PLAYER_MAX_LEVEL);

//
// Damage
//

static uint8_t player_damage_resistance_modifiers[_DAMAGE_TYPE_COUNT];
STATIC_ASSERT(ARRAY_SIZE(player_damage_resistance_modifiers) == _DAMAGE_TYPE_COUNT);

static uint8_t player_damage_status_buildup[_DAMAGE_STATUS_TYPE_COUNT];
STATIC_ASSERT(ARRAY_SIZE(player_damage_status_buildup) == _DAMAGE_STATUS_TYPE_COUNT);

static timer_t player_damage_status_timers[ _DAMAGE_STATUS_TYPE_COUNT ];
STATIC_ASSERT(ARRAY_SIZE(player_damage_status_timers) == _DAMAGE_STATUS_TYPE_COUNT);


//
//
//

flags8_t __fastcall__ get_player_changed_flags()
{
    return player_changed_flags;
}

uint8_t __fastcall__ get_player_current_health()
{
    return player_health;
}

uint8_t __fastcall__ get_player_max_health()
{
    return player_max_health_per_level[ g_current_game_data.player_level ];
}

uint8_t __fastcall__ get_player_current_stamina()
{
    return player_stamina;
}

uint8_t __fastcall__ get_player_max_stamina()
{
    return player_max_stamina_per_level[ g_current_game_data.player_level ];
}

void __fastcall__ player_init(void)
{
    player_health = player_max_health_per_level[g_current_game_data.player_level];
    player_stamina = player_max_stamina_per_level[g_current_game_data.player_level];
    player_flasks = player_max_flasks_per_level[g_current_game_data.player_level];
    player_damage_queue_length = 0;
    player_action_queue_length = 0;
    player_action_queue_index = 0;
    player_state = PLAYER_STATE_IDLE;
    player_next_state = PLAYER_STATE_IDLE;

    timer_set( player_inv_frame_timer, 0 );
    timer_set( player_flash_cooldown_timer, 0 );
    timer_set( player_dodge_cooldown_timer, 0 );
    timer_set( player_stamina_delay_timer, 1 );
    timer_set( player_stamina_regen_timer, player_stamina_regen_time_per_level[g_current_game_data.player_level] );

    flags_mark( player_changed_flags, PLAYER_CHANGED_ALL );

    // start in the center of the play space
    subpixel_set( player_pos_x, COMBAT_PLAYFIELD_WIDTH.pix >> 1, 0 );
    subpixel_set( player_pos_y, COMBAT_PLAYFIELD_HEIGHT.pix >> 1, 0 );
    subpixel_set_zero( player_pos_dx );
    subpixel_set_zero( player_pos_dy );

    // knite sprite
    ppu_upload_meta_sprite_chr_ram( knight_sprite_0, 0x10 );
    ppu_upload_meta_sprite_chr_ram( knight_sprite_1, 0x12 );
    ppu_upload_meta_sprite_chr_ram( knight_sprite_2, 0x14 );

#ifdef DEBUG
    debug_player_draw_debug = 1;
#endif
}

static void __fastcall__ player_update_physics(void)
{
}

#define PLAYER_HEALTH_PER_TILE_LOG2     (uint8_t)3

static void __fastcall__ player_render_status_bars(void)
{
    // update health when changed
    if( get_player_changed_flags() & PLAYER_CHANGED_HEALTH )
    {
        x = get_player_current_health();
        y = get_player_max_health();

        // player health bar
        ppu_begin_tile_batch(2,1);

        // full tiles
        for( i = 0, imax = (y >> PLAYER_HEALTH_PER_TILE_LOG2), j = 8; i < imax && x >= j; ++i, j += (1 << PLAYER_HEALTH_PER_TILE_LOG2) )
        {
            ppu_push_tile_batch(0x80 + 8);
        }

        // partial tile
        if( i < imax )
        {
            j -= (1 << PLAYER_HEALTH_PER_TILE_LOG2);
            ppu_push_tile_batch(0x80 + ( x - j ) );
            ++i;
        }

        // empty tiles
        for( ; i < imax; ++i )
        {
            ppu_push_tile_batch( 0x80 );
        }

        ppu_end_tile_batch();
    }

    // update stamina when changed
    if( get_player_changed_flags() & PLAYER_CHANGED_STAMINA )
    {
        x = get_player_current_stamina();
        y = get_player_max_stamina();

        // player stamina bar
        ppu_begin_tile_batch(2,2);

        // full tiles
        for( i = 0, imax = (y >> PLAYER_HEALTH_PER_TILE_LOG2), j = (1 << PLAYER_HEALTH_PER_TILE_LOG2); i < imax && x >= j; ++i, j += (1 << PLAYER_HEALTH_PER_TILE_LOG2) )
        {
            ppu_push_tile_batch(0x80 + 8);
        }

        // partial tile
        if( i < imax )
        {
            j -= (1 << PLAYER_HEALTH_PER_TILE_LOG2);
            ppu_push_tile_batch(0x80 + ( x - j ) );
            ++i;
        }

        // empty tiles
        for( ; i < imax; ++i )
        {
            ppu_push_tile_batch( 0x80 );
        }

        ppu_end_tile_batch();
    }
}

static void __fastcall__ player_render_character(void)
{
    switch(t)
    {
        case 0:
            ppu_add_meta_sprite( knight_sprite_0, TILE_TO_PIXEL(3), TILE_TO_PIXEL(14), 0x00 );
            break;

        case 1:
            ppu_add_meta_sprite( knight_sprite_1, TILE_TO_PIXEL(3), TILE_TO_PIXEL(14), 0x20 );
            break;

        case 2:
            ppu_add_meta_sprite( knight_sprite_2, TILE_TO_PIXEL(3), TILE_TO_PIXEL(14), 0x40 );
            break;
    }
}

#ifdef DEBUG
static void __fastcall__ player_render_debug(void)
{
}
#endif

static void __fastcall__ player_update_stamina(void)
{
    if( player_stamina_delay_timer > 0 )
    {
        --player_stamina_delay_timer;
    }
    else if( player_stamina < player_max_stamina_per_level[g_current_game_data.player_level] )
    {
        if( player_stamina_regen_timer > 0 )
        {
            --player_stamina_regen_timer;

            if( player_stamina_regen_timer == 0 )
            {
                player_stamina += player_stamina_regen_amount_per_level[g_current_game_data.player_level];
                player_stamina_regen_timer = player_stamina_regen_time_per_level[g_current_game_data.player_level];

                MOD_STAMINA_REGEN_TIME(player_stamina_regen_timer, player_damage_status);

                flags_mark( player_changed_flags, PLAYER_CHANGED_STAMINA );
            }
        }
    }
}

static void __fastcall__ player_heal(void)
{
    // damage type should be flat
    ASSERT(DAMAGE_TYPE_MASK(_temp_dmg.damage_type) == DAMAGE_TYPE_FLAT);
    if( DAMAGE_TYPE_MASK(_temp_dmg.damage_type) != DAMAGE_TYPE_FLAT)
    {
        return;
    }

    // player already at full health
    if( player_health == player_max_health_per_level[g_current_game_data.player_level] )
    {
        return;
    }

    // modify healing by attributes
    MOD_INCOMING_HEALING_FROM_ATTR(_temp_dmg);

    // if no healing left, return
    if( _temp_dmg.damage == 0 )
    {
        return;
    }

    // modify healing by status effects
    MOD_INCOMING_HEALING_FROM_STATUS(_temp_dmg, player_damage_status);

    // if no healing left, return
    if( _temp_dmg.damage == 0 )
    {
        return;
    }

    // heal
    flags_mark( player_changed_flags, PLAYER_CHANGED_HEALTH );

    // player health + heal would go past max health, just set to max health
    if( player_health > ( player_max_health_per_level[g_current_game_data.player_level] - _temp_dmg.damage ) )
    {
        player_health = player_max_health_per_level[g_current_game_data.player_level];
    }
    else
    {
        player_health += _temp_dmg.damage;
    }
}

static void __fastcall__ player_death(void)
{

}

static void __fastcall__ player_take_damage(void)
{
    // if player is in I frames, return
    if( !timer_is_done( player_inv_frame_timer ) )
    {
        return;
    }

    // modify dmage by attributes
    MOD_INCOMING_DAMAGE_FROM_ATTR(_temp_dmg);

    // if no healing left, return
    if( _temp_dmg.damage == 0 )
    {
        return;
    }

    // modify damage from status effects
    MOD_INCOMING_DAMAGE_FROM_STATUS(_temp_dmg, player_damage_status);

    // no damage left, return
    if( _temp_dmg.damage == 0 )
    {
        return;
    }

    // modify damage for resistences
    MOD_INCOMING_DAMAGE_FROM_RESISTANCE(_temp_dmg, player_damage_resistance_modifiers);

    // no damage left, return
    if( _temp_dmg.damage == 0 )
    {
        return;
    }

    // build up damage
    b = player_damage_status;
    BUILDUP_DAMAGE(_temp_dmg, player_damage_status_buildup, player_damage_status);
    if( b != player_damage_status )
    {
        flags_mark( player_changed_flags, PLAYER_CHANGED_STATUS );
    }

    // take damage
    flags_mark( player_changed_flags, PLAYER_CHANGED_HEALTH );

    if( player_health < _temp_dmg.damage )
    {
        player_health = 0;
    }
    else
    {
        player_health -= _temp_dmg.damage;
    }
}

static void __fastcall__ player_process_damage_queue(void)
{
    for( i = 0; i < player_damage_queue_length; ++i )
    {
        _temp_dmg = player_damage_queue[i];
        if( _temp_dmg.damage_type & DAMAGE_TYPE_ATTR_HEAL )
        {
            player_heal();
        }
        else
        {
            player_take_damage();
        }
    }

    // clear damage queue
    player_damage_queue_length = 0;

    // if the player has no more health once all damage/healing is processed, player is dead
    if( player_health == 0 )
    {
        player_death();
    }
}

static void __fastcall__ player_use_stamina(uint8_t stamina)
{
    if( player_stamina < stamina )
    {
        player_stamina = 0;
        timer_set( player_stamina_delay_timer, PLAYER_STAMINA_DELAY_TIMER_OVERDRAW );
    }
    else
    {
        player_stamina -= stamina;
        timer_set( player_stamina_delay_timer, PLAYER_STAMINA_DELAY_TIMER_NORMAL );
    }

    timer_set( player_stamina_regen_timer, player_stamina_regen_time_per_level[g_current_game_data.player_level] );

    MOD_STAMINA_REGEN_TIME(player_stamina_regen_timer, player_damage_status);

    flags_mark( player_changed_flags, PLAYER_CHANGED_STAMINA );
}

#include "player_state_idle.inl"
#include "player_state_dodge.inl"
#include "player_state_move.inl"
#include "player_state_flask.inl"
#include "player_state_attack.inl"

static void __fastcall__ player_enqueu_action(uint8_t action)
{
    player_action_queue[ player_action_queue_length] = action;
    player_action_queue_length++;
    player_action_queue_length &= PLAYER_ACTION_MAX_QUEUE_LENGTH_MASK;
}

// update player input
static void __fastcall__ player_process_input(void)
{
    // b - prev
    // t - current

    // heal
    if( GAMEPAD_BTN_PRESSED( b, t, GAMEPAD_SELECT ) )
    {
        if( can_perform_flask() )
        {
            perform_flask();
        }
        else if( can_queue_flask() )
        {
            player_enqueu_action( (uint8_t)Player_Action_Flask );
        }
    }

    // pause menu
    if( GAMEPAD_BTN_PRESSED( b, t, GAMEPAD_START ) )
    {
        game_state_playing_set_pause(1);
    }

    // B down, modify input
    if( GAMEPAD_BTN_DOWN( t, GAMEPAD_B ) )
    {
        // heavy attack
        if( GAMEPAD_BTN_PRESSED( b, t, GAMEPAD_A ) )
        {
            if( can_perform_attack1() )
            {
                perform_attack1();
            }
            else if( can_queue_attack1() )
            {
                player_enqueu_action( (uint8_t)Player_Action_Attack1 );
            }
        }

        v = 0;

        // dodge left/right
        if( GAMEPAD_BTN_PRESSED( b, t, GAMEPAD_L ) )
        {
            v |= PLAYER_MOVE_DIRECTION_WEST;
        }
        else if( GAMEPAD_BTN_PRESSED( b, t, GAMEPAD_R ) )
        {
            v |= PLAYER_MOVE_DIRECTION_EAST;
        }

        // dodge up/dow
        if( GAMEPAD_BTN_PRESSED( b, t, GAMEPAD_U ) )
        {
            v |= PLAYER_MOVE_DIRECTION_NORTH;
        }
        else if( GAMEPAD_BTN_PRESSED( b, t, GAMEPAD_D ) )
        {
            v |= PLAYER_MOVE_DIRECTION_SOUTH;
        }

        // if a direction was pressed, dodge
        if( v )
        {
            // dodge in a direction
            if( can_perform_dodge() )
            {
                perform_dodge( v );
            }
            else if( can_queue_dodge() )
            {
                player_enqueu_action( (uint8_t)Player_Action_Dodge );
                player_enqueu_action( v );
            }
        }
    }
    else
    {
        // light attack
        if( GAMEPAD_BTN_PRESSED( b, t, GAMEPAD_A ) )
        {
            if( can_perform_attack0() )
            {
                perform_attack0();
            }
            else if( can_queue_attack0() )
            {
                player_enqueu_action( (uint8_t)Player_Action_Attack0 );
            }
        }

        // move in a direction
        if( can_perform_move() )
        {
            v = 0;

            // dodge left/right
            if( GAMEPAD_BTN_DOWN( t, GAMEPAD_L ) )
            {
                v |= PLAYER_MOVE_DIRECTION_WEST;
            }
            else if( GAMEPAD_BTN_DOWN( t, GAMEPAD_R ) )
            {
                v |= PLAYER_MOVE_DIRECTION_EAST;
            }

            // up/dow
            if( GAMEPAD_BTN_DOWN( t, GAMEPAD_U ) )
            {
                v |= PLAYER_MOVE_DIRECTION_NORTH;
            }
            else if( GAMEPAD_BTN_DOWN( t, GAMEPAD_D ) )
            {
                v |= PLAYER_MOVE_DIRECTION_SOUTH;
            }

            if( v )
            {
                perform_move( v );
            }
        }
    }
}

static void __fastcall__ player_process_action_queue(void)
{
    while( player_action_queue_index != player_action_queue_length )
    {
        b = player_action_queue[ player_action_queue_index ];
        player_action_queue_index++;
        player_action_queue_index &= PLAYER_ACTION_MAX_QUEUE_LENGTH_MASK;

        switch(b)
        {
            case Player_Action_Dodge:
            {
                // pull dodge direction from queue
                v = player_action_queue[ player_action_queue_index ];
                player_action_queue_index++;
                player_action_queue_index &= PLAYER_ACTION_MAX_QUEUE_LENGTH_MASK;

                if( can_perform_dodge() )
                {
                    perform_dodge(v);
                }
            }
                break;
            case Player_Action_Flask:
            {
                if( can_perform_flask() )
                {
                    perform_flask();
                }
            }
                break;
            case Player_Action_Attack0:
            {
                if( can_perform_attack0() )
                {
                    perform_attack0();
                }
            }
                break;
            case Player_Action_Attack1:
            {
                if( can_perform_attack1() )
                {
                    perform_attack1();
                }
            }
                break;
        }

    }
}

void __fastcall__ player_update(void)
{
    player_changed_flags = 0;

    // process damage queue
    if( player_damage_queue_length > 0 )
    {
        player_process_damage_queue();
    }

    // tick build up
    b = player_damage_status;
    TICK_BUILDUP(player_damage_status_buildup, player_damage_status, player_damage_resistance_modifiers, 10);
    if( b != player_damage_status )
    {
        flags_mark( player_changed_flags, PLAYER_CHANGED_STATUS );
    }

    // regen stamina
    player_update_stamina();

    // tick timers
    timer_tick( player_flash_cooldown_timer );
    timer_tick( player_dodge_cooldown_timer );
    timer_tick( player_attack_cooldown_timer );
    timer_tick( player_inv_frame_timer );
    timer_tick( player_animation_frame_timer );
    timer_tick( player_attack_combo_timer );

    if( timer_is_done( player_attack_combo_timer ) )
    {
        player_current_attack_combo = 0xFF;
    }

    // process action queue
    player_process_action_queue();

    // perform state transitions
    if( player_state != player_next_state )
    {
        // leave current state
        switch( player_state )
        {
            case PLAYER_STATE_DODGING:
                player_state_dodge_leave();
                break;

            case PLAYER_STATE_USING_FLASH:
                player_state_flash_leave();
                break;
        }

        player_state = player_next_state;

        // enter new state
        switch( player_state )
        {
            case PLAYER_STATE_IDLE:
                player_state_idle_enter();
                break;

            case PLAYER_STATE_DODGING:
                player_state_dodge_enter();
                break;

            case PLAYER_STATE_USING_FLASH:
                player_state_flash_enter();
                break;
        }
    }

    // state update
    switch( player_state )
    {
        case PLAYER_STATE_DODGING:
            player_state_dodge_update();
            break;

        case PLAYER_STATE_USING_FLASH:
            player_state_flash_update();
            break;
    }

    // process input
    b = gamepad_prev_state(0);
    t = gamepad_state(0);
    player_process_input();

    // move player
    subpixel_inc( player_pos_x, player_pos_dx );
    subpixel_inc( player_pos_y, player_pos_dy );

    subpixel_diff_set_zero( player_pos_dx );
    subpixel_diff_set_zero( player_pos_dy );

    // draw status bars
    player_render_status_bars();

    // draw character
    //player_render_character();

#ifdef DEBUG
    if( debug_player_draw_debug )
    {
        player_render_debug();
    }
#endif
}

//
// Combat interface
//

uint8_t __fastcall__ test_attack_hits_player( uint8_t attack_location )
{
    // if the player is in Iframes, return no hit
    if( player_inv_frame_timer > 0 ) return 0;

    // update player combat position
    if( flags_is_set( player_changed_flags, PLAYER_CHANGED_POSITION ) )
    {
        player_combat_position = convert_subpixel_to_combat_position( player_pos_x, player_pos_y );
    }

    // test combat position against attack location
    TEST_COMBAT_POSITION( player_combat_position, attack_location );

    // positino not found, this should not happen
    INVALID_CODE_PATH;

    return 0;
}

void __fastcall__ queue_damage_player( uint8_t damage_type, uint8_t damage )
{
    ASSERT(player_damage_queue_length < PLAYER_DAMAGE_MAX_QUEUE_LENGTH);
    player_damage_queue[player_damage_queue_length].damage_type = damage_type;
    player_damage_queue[player_damage_queue_length].damage = damage;
    ++player_damage_queue_length;
}
