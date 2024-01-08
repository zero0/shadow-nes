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

#define PLAYER_HEALTH_LIMIT                     (uint8_t)224
#define PLAYER_STAMINA_LIMIT                    (uint8_t)112

#define PLAYER_FLASK_REFRESH_TIMER              (uint8_t)180

#define PLAYER_STAMINA_DELAY_TIMER_NORMAL       (uint8_t)20
#define PLAYER_STAMINA_DELAY_TIMER_OVERDRAW     (uint8_t)60

#define PLAYER_MAX_LEVEL                        (uint8_t)4

static uint8_t player_movement_state_next;
static uint8_t player_movement_state;
static uint8_t player_attack_state;

static flags8_t player_changed_flags;
static uint8_t player_level;
static uint8_t player_health;
static uint8_t player_stamina;
static uint8_t player_flasks;
static damage_status_t player_damage_status;
static uint8_t player_combat_position;

static timer_t player_flask_timer;
static timer_t player_inv_frame_timer;
static timer_t player_stamina_regen_timer;
static timer_t player_stamina_delay_timer;

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

static damage_t player_damage_queue[4];
static uint8_t player_damage_queue_length;

static uint8_t player_input_queue[4];
static uint8_t player_input_queue_length;

static damage_t _temp_dmg;

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

static const uint8_t player_damage_resistance_modifiers[] = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};
STATIC_ASSERT(ARRAY_SIZE(player_damage_resistance_modifiers) == _DAMAGE_TYPE_COUNT);

static uint8_t player_damage_status_buildup[] = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};
STATIC_ASSERT(ARRAY_SIZE(player_damage_status_buildup) == _DAMAGE_STATUS_TYPE_COUNT);

static timer_t player_damage_status_timers[] = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};
STATIC_ASSERT(ARRAY_SIZE(player_damage_status_timers) == _DAMAGE_STATUS_TYPE_COUNT);

static const uint8_t player_attack_0_combo_base_damage[] = {
    10, 10, 12, 15
};
static const uint8_t player_attack_1_combo_base_damage[] = {
    14, 16, 20
};

//
//
//

uint8_t __fastcall__ get_player_changed_flags()
{
    return player_changed_flags;
}

uint8_t __fastcall__ get_player_current_health()
{
    return player_health;
}

uint8_t __fastcall__ get_player_max_health()
{
    return player_max_health_per_level[ player_level ];
}

uint8_t __fastcall__ get_player_current_stamina()
{
    return player_stamina;
}

uint8_t __fastcall__ get_player_max_stamina()
{
    return player_max_stamina_per_level[ player_level ];
}


void __fastcall__ init_player(void)
{
    STATE_RESET( player_movement_state );
    player_level = 0;
    player_health = 10;
    player_stamina = player_max_stamina_per_level[player_level];
    player_flasks = player_max_flasks_per_level[player_level];
    player_damage_queue_length = 0;
    player_input_queue_length = 0;

    timer_set( player_inv_frame_timer, 0 );
    timer_set( player_flask_timer, 60 );
    timer_set( player_stamina_delay_timer, 1 );
    timer_set( player_stamina_regen_timer, player_stamina_regen_time_per_level[player_level] );

    player_changed_flags = 0xFF;

    subpixel_set( player_pos_x, 0, 0 );
    subpixel_set( player_pos_y, 10, 0 );
    subpixel_set_zero( player_pos_dx );
    subpixel_set_zero( player_pos_dy );
}

static void __fastcall__ player_update_physics(void)
{
}

void __fastcall__ player_render()
{
}

void __fastcall__ player_render_debug()
{
}

static void __fastcall__ update_player_stamina(void)
{
    if( player_stamina_delay_timer > 0 )
    {
        --player_stamina_delay_timer;
    }
    else if( player_stamina < player_max_stamina_per_level[player_level] )
    {
        if( player_stamina_regen_timer > 0 )
        {
            --player_stamina_regen_timer;

            if( player_stamina_regen_timer == 0 )
            {
                player_stamina += player_stamina_regen_amount_per_level[player_level];
                player_stamina_regen_timer = player_stamina_regen_time_per_level[player_level];

                MOD_STAMINA_REGEN_TIME(player_stamina_regen_timer, player_damage_status);

                player_changed_flags |= PLAYER_CHANGED_STAMINA;
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
    if( player_health == player_max_health_per_level[player_level] )
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
    player_changed_flags |= PLAYER_CHANGED_HEALTH;

    // player health + heal would go past max health, just set to max health
    if( player_health > ( player_max_health_per_level[player_level] - _temp_dmg.damage ) )
    {
        player_health = player_max_health_per_level[player_level];
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
        player_changed_flags |= PLAYER_CHANGED_STATUS;
    }

    // take damage
    player_changed_flags |= PLAYER_CHANGED_HEALTH;

    if( player_health < _temp_dmg.damage )
    {
        player_health = 0;
    }
    else
    {
        player_health -= _temp_dmg.damage;
    }

    if( player_health == 0 )
    {
        player_death();
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
}

static void __fastcall__ player_use_stamina(uint8_t stamina)
{
    if( player_stamina < stamina )
    {
        player_stamina = 0;
        timer_set( player_stamina_delay_timer, PLAYER_STAMINA_DELAY_TIMER_OVERDRAW );
        timer_set( player_stamina_regen_timer, player_stamina_regen_time_per_level[player_level] );

        MOD_STAMINA_REGEN_TIME(player_stamina_regen_timer, player_damage_status);

        player_changed_flags |= PLAYER_CHANGED_STAMINA;
    }
    else
    {
        player_stamina -= stamina;
        timer_set( player_stamina_delay_timer, PLAYER_STAMINA_DELAY_TIMER_NORMAL );
        timer_set( player_stamina_regen_timer, player_stamina_regen_time_per_level[player_level] );

        MOD_STAMINA_REGEN_TIME(player_stamina_regen_timer, player_damage_status);

        player_changed_flags |= PLAYER_CHANGED_STAMINA;
    }
}

#define can_perform_attack(atk) ( player_stamina > 0 && timer_is_done( player_inv_frame_timer ) )

static void __fastcall__ perform_attack(uint8_t attack_index)
{
    //if( test_attack_hits_boss( DAMAGE_LOCATION_COLUMN_CENTER ) )
    {
        // build damange
        _temp_dmg.damage_type = MAKE_DAMAGE_TYPE(0, DAMAGE_TYPE_PHYSICAL);
        _temp_dmg.damage = 10;

        // modify damage based on stats
        MOD_OUTGOING_DAMAGE_FROM_STATUS(_temp_dmg, player_damage_status);

        // queue boss damage
        queue_damage_boss( _temp_dmg.damage_type, _temp_dmg.damage );
    }
}

#define can_perform_dodge()     ( player_stamina > 0 && timer_is_done( player_inv_frame_timer ) )

static void __fastcall__ perform_dodge(void)
{

}

#define can_perform_flask()     ( player_flasks > 0 && timer_is_done( player_flask_timer ) )

static void __fastcall__ perform_flask(void)
{
    // enquque heal
    ASSERT(player_damage_queue_length < ARRAY_SIZE(player_damage_queue));
    player_damage_queue[player_damage_queue_length].damage_type = MAKE_DAMAGE_TYPE( DAMAGE_TYPE_ATTR_HEAL, DAMAGE_TYPE_FLAT );
    player_damage_queue[player_damage_queue_length].damage = player_flask_heal_per_level[player_level];
    ++player_damage_queue_length;

    timer_set( player_flask_timer, PLAYER_FLASK_REFRESH_TIMER );

    --player_flasks;

    // mark flask changed
    player_changed_flags |= PLAYER_CHANGED_FLASKS;
}

void __fastcall__ update_player(void)
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
        player_changed_flags |= PLAYER_CHANGED_STATUS;
    }

    // regen stamina
    update_player_stamina();

    // tick timers
    timer_tick( player_flask_timer );
    timer_tick( player_inv_frame_timer );

    // light attack
    if( GAMEPAD_PRESSED( 0, GAMEPAD_A ) )
    {
        if( can_perform_attack( 0 ) )
        {
            perform_attack( 0 );
        }
    }

    // heavy attack
    if( GAMEPAD_PRESSED( 0, GAMEPAD_B ) )
    {
        if( can_perform_attack( 1 ) )
        {
            perform_attack( 1 );
        }
    }

    // heal
    if( GAMEPAD_PRESSED( 0, GAMEPAD_SELECT ) )
    {
        if( can_perform_flask() )
        {
            perform_flask();
        }
    }

    // move left/right
    if( GAMEPAD_DOWN( 0, GAMEPAD_L ) )
    {

    }
    else if( GAMEPAD_DOWN( 0, GAMEPAD_R ) )
    {

    }

    // up/dow
    if( GAMEPAD_DOWN( 0, GAMEPAD_U ) )
    {

    }
    else if( GAMEPAD_DOWN( 0, GAMEPAD_D ) )
    {
        if( can_perform_dodge() )
        {
            perform_dodge();
        }
    }
}

//
// Combat interface
//

uint8_t __fastcall__ test_attack_hits_player( uint8_t attack_location )
{
    // if the player is in Iframes, return no hit
    if( player_inv_frame_timer > 0 ) return 0;

    // test combat position against attack location
    TEST_COMBAT_POSITION( player_combat_position, attack_location );

    // positino not found, this should not happen
    INVALID_CODE_PATH;

    return 0;
}

void __fastcall__ queue_damage_player( uint8_t damage_type, uint8_t damage )
{
    ASSERT(player_damage_queue_length < ARRAY_SIZE(player_damage_queue));
    player_damage_queue[player_damage_queue_length].damage_type = damage_type;
    player_damage_queue[player_damage_queue_length].damage = damage;
    ++player_damage_queue_length;
}
