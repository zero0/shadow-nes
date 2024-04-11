#include "macros.h"
#include "boss.h"
#include "timer.h"
#include "combat.h"
#include "globals.h"
#include "subpixel.h"
#include "collision.h"

#include "boss_0_impl.inl"

static const uint8_t all_boss_health_per_block_log2[_BOSS_COUNT] = {
    BOSS_0_HEALTH_PER_BLOCK_LOG2,
};

enum
{
    BOSS_HEALTH_MAX,
    BOSS_HEALTH_PHASE_TRANSITION,
    _BOSS_HEALTH_COUNT,
};

static const uint16_t all_boss_healths[_BOSS_COUNT][_BOSS_HEALTH_COUNT] = {
    BOSS_0_HEALTH,
    {},
    {},
    {},
    {},
    {},
    {},
    {},

    {},
    {},
};
STATIC_ASSERT(ARRAY_SIZE(all_boss_healths) == _BOSS_COUNT);

enum
{
    BOSS_STAMINA_MAX,
    BOSS_STAMINA_REGEN,
    _BOSS_STAMINA_COUNT,
};

static const uint8_t all_boss_staminas[_BOSS_COUNT][_BOSS_STAMINA_COUNT] = {
    BOSS_0_STAMINA,
};
STATIC_ASSERT(ARRAY_SIZE(all_boss_staminas) == _BOSS_COUNT);

static const uint8_t all_boss_resistances[_BOSS_COUNT][_DAMAGE_TYPE_COUNT] = {
    BOSS_0_RESISTANCE,
    {},
    {},
    {},
    {},
    {},
    {},
    {},

    {},
    {},
};
STATIC_ASSERT(ARRAY_SIZE(all_boss_resistances) == _BOSS_COUNT);

static const str_t all_boss_names[] = {
    BOSS_0_NAME,
    tr_boss_1_name,
    tr_boss_2_name,
    tr_boss_3_name,
    tr_boss_4_name,
    tr_boss_5_name,
    tr_boss_6_name,
    tr_boss_7_name,

    tr_boss_8_name,
    tr_boss_9_name,
};
STATIC_ASSERT(ARRAY_SIZE(all_boss_names) == _BOSS_COUNT);

static const str_t all_boss_location_names[] = {
    BOSS_0_LOCATION,
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
STATIC_ASSERT(ARRAY_SIZE(all_boss_location_names) == _BOSS_COUNT);

enum
{
    BOSS_MUSIC_PHASE_INTRO,
    BOSS_MUSIC_PHASE_ONE,
    BOSS_MUSIC_PHASE_TRANSITION,
    BOSS_MUSIC_PHASE_TWO,
    BOSS_MUSIC_PHASE_OUTRO,
    _BOSS_MUSIC_PHASE_COUNT,
};

static const uint8_t all_boss_music[_BOSS_COUNT][_BOSS_MUSIC_PHASE_COUNT] = {
    BOSS_0_MUSIC,
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
STATIC_ASSERT(ARRAY_SIZE(all_boss_music) == _BOSS_COUNT);

//
//
//

enum
{
    BOSS_STATE_NONE =       0,
    BOSS_STATE_DEAD =       1 << 0,
    BOSS_STATE_PHASE_TWO =  1 << 1,
    BOSS_STATE_UNDEF_1 =    1 << 2,
    BOSS_STATE_UNDEF_2 =    1 << 3,
    BOSS_STATE_UNDEF_3 =    1 << 4,
    BOSS_STATE_UNDEF_4 =    1 << 5,
    BOSS_STATE_UNDEF_5 =    1 << 6,
    BOSS_STATE_INV_FRAMES = 1 << 7,
};

static flags8_t boss_state;
static flags8_t boss_changed_flags;
static uint8_t boss_index;
static uint16_t boss_health;
static uint8_t boss_stamina;
static timer_t boss_inv_frame_timer;
static damage_status_t boss_damage_status;

static subpixel_t boss_position_x;
static subpixel_t boss_position_y;

static aabb_t boss_hit_box;
static aabb_t boss_damage_box;

static uint8_t boss_damage_resistance_modifiers[_DAMAGE_TYPE_COUNT];
STATIC_ASSERT(ARRAY_SIZE(boss_damage_resistance_modifiers) == _DAMAGE_TYPE_COUNT);

static uint8_t boss_damage_status_buildup[_DAMAGE_STATUS_TYPE_COUNT];
STATIC_ASSERT(ARRAY_SIZE(boss_damage_status_buildup) == _DAMAGE_STATUS_TYPE_COUNT);

static timer_t boss_damage_status_timers[ _DAMAGE_STATUS_TYPE_COUNT ];
STATIC_ASSERT(ARRAY_SIZE(boss_damage_status_timers) == _DAMAGE_STATUS_TYPE_COUNT);


enum
{
    BOSS_DAMAGE_QUEUE_MAX_LENGTH = 4,
};
static damage_t boss_damage_queue[BOSS_DAMAGE_QUEUE_MAX_LENGTH];
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
    return all_boss_healths[ boss_index ][ BOSS_HEALTH_MAX ];
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

static void __fastcall__ boss_enter_phase_2(void)
{
    flags_mark( boss_state, BOSS_STATE_PHASE_TWO );
}

static void __fastcall__ boss_death(void)
{
    flags_mark( boss_state, BOSS_STATE_DEAD );
}

static void __fastcall__ boss_heal(void)
{
}

static void __fastcall__ boss_take_damage(void)
{
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
    MOD_INCOMING_DAMAGE_FROM_RESISTANCE(_temp_dmg, all_boss_resistances[boss_index]);

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
        boss_changed_flags |= BOSS_CHANGED_STATUS;
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

    // enter phase 2
    if( !flags_is_set( boss_state, BOSS_STATE_PHASE_TWO ) && ( boss_health < all_boss_healths[boss_index][BOSS_HEALTH_PHASE_TRANSITION] ) )
    {
        boss_enter_phase_2();
    }
    else if( boss_health == 0 )
    {
        boss_death();
    }
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

void __fastcall__ boss_init(uint8_t bossIndex)
{
    boss_index = bossIndex;
    boss_state = BOSS_STATE_NONE;

    flags_mark( boss_changed_flags, BOSS_CHANGED_ALL );
    boss_damage_queue_length = 0;

    boss_health = all_boss_healths[ boss_index ][ BOSS_HEALTH_MAX ];
    boss_stamina = all_boss_staminas[ boss_index ][ BOSS_STAMINA_MAX ];
}

void __fastcall__ boss_update(void)
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
    UNUSED(attack_location);

    // if the boss is in Iframes, return no hit
    if( boss_inv_frame_timer > 0 ) return 0;

    return 0;
}

void __fastcall__ queue_damage_boss( uint8_t damage_type, uint8_t damage )
{
    ASSERT(boss_damage_queue_length < ARRAY_SIZE(boss_damage_queue));
    boss_damage_queue[boss_damage_queue_length].damage_type = damage_type;
    boss_damage_queue[boss_damage_queue_length].damage = damage;
    ++boss_damage_queue_length;
}
