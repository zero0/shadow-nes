#include "macros.h"
#include "boss.h"
#include "timer.h"
#include "combat.h"
#include "globals.h"
#include "subpixel.h"

static const uint8_t all_boss_health_per_block_log2[] = {
    7,
};

static const uint16_t all_boss_max_healths[] = {
    (uint16_t)1792,
};

static const uint8_t all_boss_armors[] = {
    0,
};

static const str_t all_boss_names[] = {
    tr_boss_1_name,
};
static const str_t all_boss_location_names[] = {
    tr_boss_1_location,
};
static const uint8_t all_boss_music[] = {
    0,
};

#define BOSS_STATE_DEAD             (uint8_t)(1 << 0)
#define BOSS_STATE_UNDEF_0          (uint8_t)(1 << 1)
#define BOSS_STATE_UNDEF_1          (uint8_t)(1 << 2)
#define BOSS_STATE_UNDEF_2          (uint8_t)(1 << 3)
#define BOSS_STATE_UNDEF_3          (uint8_t)(1 << 4)
#define BOSS_STATE_UNDEF_4          (uint8_t)(1 << 5)
#define BOSS_STATE_UNDEF_5          (uint8_t)(1 << 6)
#define BOSS_STATE_INV_FRAMES       (uint8_t)(1 << 7)

static uint8_t boss_state;
static flags8_t boss_changed_flags;
static uint8_t boss_index;
static uint16_t boss_health;
static timer_t boss_inv_frame_timer;
static damage_status_t boss_damage_status;
static uint8_t boss_combat_position;

static subpixel_t boss_position_x;
static subpixel_t boss_position_y;

static damage_t boss_damage_queue[4];
static uint8_t boss_damage_queue_length;

static damage_t _temp_dmg;

uint8_t __fastcall__ get_boss_health_per_block_log2(void)
{
    return all_boss_health_per_block_log2[ boss_index ];
}

uint16_t __fastcall__ get_boss_current_health(void)
{
    return boss_health;
}

uint16_t __fastcall__ get_boss_max_health(void)
{
    return all_boss_max_healths[ boss_index ];
}

str_t __fastcall__ get_boss_name(void)
{
    return all_boss_names[ boss_index ];
}

str_t __fastcall__ get_boss_location(void)
{
    return all_boss_location_names[ boss_index ];
}

flags8_t __fastcall__ get_boss_changed_flags(void)
{
    return boss_changed_flags;
}

static void __fastcall__ boss_heal(void)
{
}

static void __fastcall__ boss_take_damage(void)
{
    x16 = _temp_dmg.damage;
    boss_health -= x16;

    flags_mark( boss_changed_flags, BOSS_CHANGED_HEALTH );

    #if 0
    // if boss is in I frames, return
    if( !timer_is_done( boss_inv_frame_timer ) )
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
    MOD_INCOMING_DAMAGE_FROM_STATUS(_temp_dmg, boss_damage_status);

    // no damage left, return
    if( _temp_dmg.damage == 0 )
    {
        return;
    }

    // modify damage for resistences
    MOD_INCOMING_DAMAGE_FROM_RESISTANCE(_temp_dmg, boss_damage_resistance_modifiers);

    // no damage left, return
    if( _temp_dmg.damage == 0 )
    {
        return;
    }

    // build up damage
    b = boss_damage_status;
    BUILDUP_DAMAGE(_temp_dmg, boss_damage_status_buildup, boss_damage_status);
    if( b != boss_damage_status )
    {
        boss_changed_flags |= PLAYER_CHANGED_STATUS;
    }

    // take damage
    boss_changed_flags |= BOSS_CHANGED_HEALTH;

    if( boss_health < _temp_dmg.damage )
    {
        boss_health = 0;
    }
    else
    {
        boss_health -= _temp_dmg.damage;
    }

    if( boss_health == 0 )
    {
        boss_death();
    }
    #endif
}

#define boss_move_direct( sx, sy )                          \
do {                                                        \
    boss_position_x = sx;                                   \
    boss_position_y = sy;                                   \
    flags_mark( boss_changed_flags, BOSS_CHANGED_POSITION ); \
}                                                           \
while( 0 )

#define boss_move_offset( dx, dy )                                          \
do {                                                                        \
    subpixel_inc( boss_position_x, dx );                                    \
    subpixel_inc( boss_position_y, dy );                                    \
    clamp_subpixel_to_combat_playfield( boss_position_x, boss_position_y ); \
    flags_mark( boss_changed_flags, BOSS_CHANGED_POSITION );                 \
}                                                                           \
while( 0 )

static void __fastcall__ boss_process_damage_queue(void)
{
    for( i = 0; i < boss_damage_queue_length; ++i )
    {
        _temp_dmg = boss_damage_queue[i];
        if( _temp_dmg.damage_type & DAMAGE_TYPE_ATTR_HEAL )
        {
            boss_heal();
        }
        else
        {
            boss_take_damage();
        }
    }

    // clear damage queue
    boss_damage_queue_length = 0;
}

void __fastcall__ init_boss(uint8_t bossIndex)
{
    boss_index = bossIndex;
    boss_state = 0;
    flags_mark( boss_changed_flags, 0xFF );
    boss_damage_queue_length = 0;

    boss_health = 1000;//all_boss_max_healths[ current_boss_index ];
}

void __fastcall__ update_boss(void)
{
    flags_reset( boss_changed_flags );

    // process damage queue
    if( boss_damage_queue_length > 0 )
    {
        boss_process_damage_queue();
    }
}


//
// Combat interface
//

uint8_t __fastcall__ test_attack_hits_boss( uint8_t attack_location )
{
    // if the boss is in Iframes, return no hit
    if( boss_inv_frame_timer > 0 ) return 0;

    // if the boss position changed, update combat position
    if( flags_is_set( boss_changed_flags, BOSS_CHANGED_POSITION ) )
    {
        boss_combat_position = convert_subpixel_to_combat_position( boss_position_x, boss_position_y );
    }

    // test combat position against attack location
    TEST_COMBAT_POSITION( boss_combat_position, attack_location );

    // positino not found, this should not happen
    INVALID_CODE_PATH;

    return 0;
}

void __fastcall__ queue_damage_boss( uint8_t damage_type, uint8_t damage )
{
    ASSERT(boss_damage_queue_length < ARRAY_SIZE(boss_damage_queue));
    boss_damage_queue[boss_damage_queue_length].damage_type = damage_type;
    boss_damage_queue[boss_damage_queue_length].damage = damage;
    ++boss_damage_queue_length;
}
