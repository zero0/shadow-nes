#include "macros.h"
#include "boss.h"
#include "timer.h"
#include "combat.h"
#include "globals.h"
#include "subpixel.h"
#include "collision.h"
#include "ppu.h"

//
//
//

enum
{
    BOSS_CHANGED_NONE =     0,
    BOSS_CHANGED_HEALTH =   1 << 0,
    BOSS_CHANGED_STAMINA =  1 << 1,
    BOSS_CHANGED_STATUS =   1 << 2,
    BOSS_CHANGED_POSITION = 1 << 3,
    BOSS_CHANGED_ALL =      ~0,
};

enum
{
    BOSS_FLAG_NONE =       0,
    BOSS_FLAG_DEAD =       1 << 0,
    BOSS_FLAG_PHASE_TWO =  1 << 1,
    BOSS_FLAG_UNDEF_1 =    1 << 2,
    BOSS_FLAG_UNDEF_2 =    1 << 3,
    BOSS_FLAG_UNDEF_3 =    1 << 4,
    BOSS_FLAG_UNDEF_4 =    1 << 5,
    BOSS_FLAG_UNDEF_5 =    1 << 6,
    BOSS_FLAG_INV_FRAMES = 1 << 7,
};

enum
{
    BOSS_STATE_IDLE,
    BOSS_STATE_INTRO,
    BOSS_STATE_MOVE,
    BOSS_STATE_DODGE,
    BOSS_STATE_DEATH,
    BOSS_STATE_HIT,
    BOSS_STATE_ATTACK0,
    BOSS_STATE_ATTACK1,
    BOSS_STATE_ATTACK2,
    BOSS_STATE_ATTACK3,
};

static flags8_t boss_flags;
static flags8_t boss_changed_flags;
static uint8_t boss_index;
static uint8_t boss_stamina;
static damage_status_t boss_damage_status;
static uint16_t boss_health;

static uint8_t boss_state;
static uint8_t boss_next_state;

static timer_t boss_inv_frame_timer;
static timer_t boss_stamina_regen_timer;
static timer_t boss_state_timer;

static ext_aabb_t boss_hit_box;

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

//
// Boss Implementations
//

#include "boss_0_impl.inl"

//
//
//

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
    BOSS_STAMINA_REGEN_TIMER,
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

enum
{
    BOSS_COOLDOWN_TIMER_INV_FRAMES,
    _BOSS_COOLDOWN_TIMER_COUNT
};

static const uint8_t all_boss_cooldown_timers[_BOSS_COUNT][_BOSS_COOLDOWN_TIMER_COUNT] = {
    BOSS_0_COOLDOWN_TIMERS,
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

//
//
//

#define CALL_BOSS_FUNC(b, f)                \
do                                          \
{                                           \
    switch( (b) )                           \
    {                                       \
        case BOSS_0: boss_0_##f(); break;   \
        case BOSS_1: break;   \
        case BOSS_2: break;   \
        case BOSS_3: break;   \
        case BOSS_4: break;   \
        case BOSS_5: break;   \
        case BOSS_6: break;   \
        case BOSS_7: break;   \
        case BOSS_8: break;   \
        case BOSS_9: break;   \
        default: INVALID_CODE_PATH; break;  \
    }                                       \
} while( 0 )

//
//
//

static void __fastcall__ boss_enter_phase_2(void)
{
    flags_mark( boss_flags, BOSS_FLAG_PHASE_TWO );
}

static void __fastcall__ boss_death(void)
{
    flags_mark( boss_flags, BOSS_FLAG_DEAD );
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
        flags_mark( boss_changed_flags, BOSS_CHANGED_STATUS );
    }

    // take damage
    flags_mark( boss_changed_flags, BOSS_CHANGED_HEALTH );

    if( boss_health < _temp_dmg.damage )
    {
        boss_health = 0;
    }
    else
    {
        boss_health -= _temp_dmg.damage;
    }

    // set Iframes
    timer_set( boss_inv_frame_timer, all_boss_cooldown_timers[boss_index][BOSS_COOLDOWN_TIMER_INV_FRAMES] );

    // enter phase 2
    if( flags_is_not_set( boss_flags, BOSS_FLAG_PHASE_TWO ) && ( boss_health < all_boss_healths[boss_index][BOSS_HEALTH_PHASE_TRANSITION] ) )
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

uint8_t __fastcall__ boss_is_dead(void)
{
    return boss_health == 0;
}

static void __fastcall__ boss_update_stamina(void)
{
    if( boss_stamina < all_boss_staminas[boss_index][BOSS_STAMINA_MAX] )
    {
        timer_tick( boss_stamina_regen_timer );
        if( timer_is_done( boss_stamina_regen_timer ) )
        {
            boss_stamina += all_boss_staminas[boss_index][BOSS_STAMINA_REGEN];

            // cap stamina
            if( boss_stamina > all_boss_staminas[boss_index][BOSS_STAMINA_MAX] )
            {
                boss_stamina = all_boss_staminas[boss_index][BOSS_STAMINA_MAX];
            }

            // reset regen timer
            timer_set( boss_stamina_regen_timer, all_boss_staminas[boss_index][BOSS_STAMINA_REGEN_TIMER] );

            // modify regent timer based on status
            MOD_STAMINA_REGEN_TIME(boss_stamina_regen_timer, boss_damage_status);

            // mark stamina changed
            flags_mark( boss_changed_flags, BOSS_CHANGED_STAMINA );
        }
    }
}

static void __fastcall__ boss_render_status_bar(void)
{
    // boss health changed
    if( flags_is_set( boss_changed_flags, BOSS_CHANGED_HEALTH ) )
    {
        x16 = boss_health;
        y16 = all_boss_healths[boss_index][BOSS_HEALTH_MAX];
        z = all_boss_health_per_block_log2[boss_index];

        // boss health bar
        ppu_begin_tile_batch(2,27);

        // full tiles
        for( i16 = 0, imax16 = (y16 >> z), j16 = (1 << z); i16 < imax16 && x16 >= j16; ++i16, j16 += (1 << z) )
        {
            ppu_push_tile_batch(0x80 + 8);
        }

        // partial tile
        if( i16 < imax16 )
        {
            j16 -= (1 << z);
            ppu_push_tile_batch(0x80 + (0x07 & (uint8_t)( x16 - j16 ) ) );
            ++i16;
        }

        // empty tiles
        for( ; i16 < imax16 && i16 < 20; ++i16 )
        {
            ppu_push_tile_batch( 0x80 );
        }

        ppu_end_tile_batch();
    }
}

static void __fastcall__ boss_render(void)
{

}

#if DEBUG
static void __fastcall__ debug_boss_render(void)
{

}
#endif

//
// Public API
//

uint8_t __fastcall__ boss_is_ready(void)
{
    return boss_state != BOSS_STATE_INTRO;
}

void __fastcall__ boss_init(uint8_t bossIndex)
{
    boss_index = bossIndex;
    boss_flags = BOSS_FLAG_NONE;

    boss_state = BOSS_STATE_IDLE;
    boss_next_state = BOSS_STATE_INTRO;

    timer_reset( boss_inv_frame_timer );
    timer_reset( boss_stamina_regen_timer );
    timer_reset( boss_state_timer );

    flags_mark( boss_changed_flags, BOSS_CHANGED_ALL );
    boss_damage_queue_length = 0;

    boss_health = all_boss_healths[ boss_index ][ BOSS_HEALTH_MAX ];
    boss_stamina = all_boss_staminas[ boss_index ][ BOSS_STAMINA_MAX ];

    memset(boss_damage_resistance_modifiers, 0, ARRAY_SIZE(boss_damage_resistance_modifiers));
    memset(boss_damage_status_buildup, 0, ARRAY_SIZE(boss_damage_status_buildup));
    memset(boss_damage_status_timers, 0, ARRAY_SIZE(boss_damage_status_timers));

    // call boss specific init
    CALL_BOSS_FUNC( boss_index, init );
}

void __fastcall__ boss_update(void)
{
    flags_reset( boss_changed_flags );

    // process damage queue
    if( boss_damage_queue_length > 0 )
    {
        boss_process_damage_queue();
    }

    // tick build up
    b = boss_damage_status;
    TICK_BUILDUP(boss_damage_status_buildup, boss_damage_status, boss_damage_resistance_modifiers, 1);
    if( b != boss_damage_status )
    {
        flags_mark( boss_changed_flags, BOSS_CHANGED_STATUS );
    }

    // regen stamina
    boss_update_stamina();

    // tick timers
    timer_tick( boss_inv_frame_timer );
    timer_tick( boss_stamina_regen_timer );
    timer_tick( boss_state_timer );

    // call boss specific update
    CALL_BOSS_FUNC( boss_index, update );

    // update state
    if( boss_state != boss_next_state )
    {
        // leave current state
        switch( boss_state )
        {
            default:
                break;
        }

        boss_state = boss_next_state;

        // enter new state
        switch( boss_state )
        {
            case BOSS_STATE_INTRO:
                timer_set( boss_state_timer, 30 );
                break;

            default:
                break;
        }
    }

    // update state
    switch( boss_state )
    {
        case BOSS_STATE_INTRO:
        {
            if( timer_is_done( boss_state_timer ) )
            {
                boss_next_state = BOSS_STATE_IDLE;
            }
        }
            break;

        default:
            break;
    }

    // render status bar
    boss_render_status_bar();

    // render boss
    boss_render();

#if DEBUG
    debug_boss_render();
#endif
}

//
// Combat interface
//

uint8_t __fastcall__ test_attack_hits_boss(void)
{
    t = 0;

    // if the boss is in Iframes, return no hit
    if( boss_inv_frame_timer > 0 ) return t;

    // test the boss's hit box with the hurt box
    begin_collision_ext_aabb( lh, boss_hit_box );
    begin_collision_ltrb_aabb( rh, combat_damage_area.aabb );
    t = perform_collision_test();

    return 0;
}

void __fastcall__ queue_damage_boss( uint8_t damage_type, uint8_t damage )
{
    ASSERT(boss_damage_queue_length < ARRAY_SIZE(boss_damage_queue));
    boss_damage_queue[boss_damage_queue_length].damage_type = damage_type;
    boss_damage_queue[boss_damage_queue_length].damage = damage;
    ++boss_damage_queue_length;
}
