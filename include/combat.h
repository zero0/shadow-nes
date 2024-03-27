#ifndef COMBAT_H
#define COMBAT_H

#include "types.h"
#include "flags.h"
#include "subpixel.h"
#include "collision.h"

//
// Damage Type
//

enum
{
    DAMAGE_TYPE_FLAT,                   // flag dmg
    DAMAGE_TYPE_PHYSICAL,               // applies stun
    DAMAGE_TYPE_FIRE,                   // applies burn
    DAMAGE_TYPE_COLD,                   // applies frozen
    DAMAGE_TYPE_POISON,                 // applies poison
    DAMAGE_TYPE_RADIANT,                // applies blinded
    DAMAGE_TYPE_SHADOW,                 // applies withered
    DAMAGE_TYPE_MAGIC,                  // applies manaburn
    DAMAGE_TYPE_FATIGUE,                // applies exhaustion, damage stamina instead of health
    _DAMAGE_TYPE_COUNT,
};
STATIC_ASSERT(_DAMAGE_TYPE_COUNT <= 0x0F );

enum
{
    DAMAGE_TYPE_ATTR_NONE = 0,
    DAMAGE_TYPE_ATTR_HEAL = 1 << 4, // dmg should be treakted as a heal
    DAMAGE_TYPE_ATTR_MISS = 1 << 5, // dmg should be zeroed out
    DAMAGE_TYPE_ATTR_CRIT = 1 << 6, // dmg should be crit (doubled)
    DAMAGE_TYPE_ATTR_WEAK = 1 << 7, // dmg should be weak (halved)
};

#define DAMAGE_TYPE_MASK(dt)        ( (uint8_t)(dt) & (uint8_t)0x0F )
#define DAMAGE_TYPE_ATTR_MASK(dt)   ( (uint8_t)(dt) & (uint8_t)0xF0 )

#define MAKE_DAMAGE_TYPE_ATTR(da, dt)   ( DAMAGE_TYPE_ATTR_MASK(da) | DAMAGE_TYPE_MASK(dt) )
#define MAKE_DAMAGE_TYPE(dt)            MAKE_DAMAGE_TYPE_ATTR( DAMAGE_TYPE_ATTR_NONE, (dt) )

#define MOD_INCOMING_DAMAGE_FROM_ATTR(dmg)                                  \
do                                                                          \
{                                                                           \
    ASSERT( !( (dmg).damage_type & DAMAGE_TYPE_ATTR_HEAL ) );               \
    if( (dmg).damage_type & DAMAGE_TYPE_ATTR_CRIT ) (dmg).damage <<= 1;     \
    if( (dmg).damage_type & DAMAGE_TYPE_ATTR_WEAK ) (dmg).damage >>= 1;     \
    if( (dmg).damage_type & DAMAGE_TYPE_ATTR_MISS ) (dmg).damage = 0;       \
} while( 0 )

#define MOD_INCOMING_HEALING_FROM_ATTR(heal)                                \
do                                                                          \
{                                                                           \
    ASSERT( ( (heal).damage_type & DAMAGE_TYPE_ATTR_HEAL ) );               \
    if( (heal).damage_type & DAMAGE_TYPE_ATTR_CRIT ) (heal).damage <<= 1;   \
    if( (heal).damage_type & DAMAGE_TYPE_ATTR_WEAK ) (heal).damage >>= 1;   \
    if( (heal).damage_type & DAMAGE_TYPE_ATTR_MISS ) (heal).damage = 0;     \
} while( 0 )

typedef uint8_t damage_type_t;

typedef struct {
    damage_type_t damage_type;
    uint8_t damage;
} damage_t;

//
// Damage Status
//

enum
{
    DAMAGE_STATUS_TYPE_STUNNED,             // stop stamina regen, reset stamina on apply
    DAMAGE_STATUS_TYPE_BURN,                // fire dmg per tick, reduce phys armor
    DAMAGE_STATUS_TYPE_FROZEN,              // slowed stamina regen, reduce fatigue armor
    DAMAGE_STATUS_TYPE_POISONED,            // poison dmg per tick, reduces hp no lower than 1
    DAMAGE_STATUS_TYPE_BLINDED,             // reduce outgoing dmg to 0 (miss)
    DAMAGE_STATUS_TYPE_WITHERD,             // lowered incoming healing
    DAMAGE_STATUS_TYPE_MANABURN,            // 2x dmg from magic
    DAMAGE_STATUS_TYPE_EXHAUSTION,          // slowed all regen, reduced dmg output
    _DAMAGE_STATUS_TYPE_COUNT,
};
STATIC_ASSERT(_DAMAGE_STATUS_TYPE_COUNT <= 8);

enum
{
    DAMAGE_STATUS_STUNNED =     1 << DAMAGE_STATUS_TYPE_STUNNED,
    DAMAGE_STATUS_BURN =        1 << DAMAGE_STATUS_TYPE_BURN,
    DAMAGE_STATUS_FROZEN =      1 << DAMAGE_STATUS_TYPE_FROZEN,
    DAMAGE_STATUS_POISONED =    1 << DAMAGE_STATUS_TYPE_POISONED,
    DAMAGE_STATUS_BLINDED =     1 << DAMAGE_STATUS_TYPE_BLINDED,
    DAMAGE_STATUS_WITHERD =     1 << DAMAGE_STATUS_TYPE_WITHERD,
    DAMAGE_STATUS_MANABURN =    1 << DAMAGE_STATUS_TYPE_MANABURN,
    DAMAGE_STATUS_EXHAUSTION =  1 << DAMAGE_STATUS_TYPE_EXHAUSTION,
};

typedef flags8_t damage_status_t;

#define MOD_INCOMING_HEALING_FROM_STATUS(heal, st)          \
do                                                          \
{                                                           \
    if( (st) & DAMAGE_STATUS_WITHERD ) (heal).damage >>= 1; \
} while( 0 )

#define MOD_OUTGOING_HEALING_FROM_STATUS(heal, st)          \
do                                                          \
{                                                           \
} while( 0 )

#define MOD_INCOMING_DAMAGE_FROM_STATUS(dmg, st)            \
do                                                          \
{                                                           \
    if( (st) & DAMAGE_STATUS_BURN       && DAMAGE_TYPE_MASK( (dmg).damage_type ) == DAMAGE_TYPE_PHYSICAL ) (dmg).damage += 10;    \
    if( (st) & DAMAGE_STATUS_MANABURN   && DAMAGE_TYPE_MASK( (dmg).damage_type ) == DAMAGE_TYPE_MAGIC )    (dmg).damage <<= 1;    \
} while( 0 )

#define MOD_OUTGOING_DAMAGE_FROM_STATUS(dmg, st)                                        \
do                                                                                      \
{                                                                                       \
    if( (st) & DAMAGE_STATUS_BLINDED )    (dmg).damage_type |= DAMAGE_TYPE_ATTR_MISS;   \
    if( (st) & DAMAGE_STATUS_EXHAUSTION ) (dmg).damage_type |= DAMAGE_TYPE_ATTR_WEAK;   \
} while( 0 )

#define MOD_INCOMING_DAMAGE_FROM_RESISTANCE(dmg, res)                                   \
do                                                          \
{                                                           \
    switch( DAMAGE_TYPE_MASK((dmg).damage_type) )           \
    {                                                       \
        case DAMAGE_TYPE_PHYSICAL  : if( (dmg).damage > (res)[ DAMAGE_TYPE_PHYSICAL ] ) (dmg).damage -= (res)[ DAMAGE_TYPE_PHYSICAL ]; else (dmg).damage = 0; break;  \
        case DAMAGE_TYPE_FIRE      : if( (dmg).damage > (res)[ DAMAGE_TYPE_FIRE     ] ) (dmg).damage -= (res)[ DAMAGE_TYPE_FIRE     ]; else (dmg).damage = 0; break;  \
        case DAMAGE_TYPE_COLD      : if( (dmg).damage > (res)[ DAMAGE_TYPE_COLD     ] ) (dmg).damage -= (res)[ DAMAGE_TYPE_COLD     ]; else (dmg).damage = 0; break;  \
        case DAMAGE_TYPE_POISON    : if( (dmg).damage > (res)[ DAMAGE_TYPE_POISON   ] ) (dmg).damage -= (res)[ DAMAGE_TYPE_POISON   ]; else (dmg).damage = 0; break;  \
        case DAMAGE_TYPE_RADIANT   : if( (dmg).damage > (res)[ DAMAGE_TYPE_RADIANT  ] ) (dmg).damage -= (res)[ DAMAGE_TYPE_RADIANT  ]; else (dmg).damage = 0; break;  \
        case DAMAGE_TYPE_SHADOW    : if( (dmg).damage > (res)[ DAMAGE_TYPE_SHADOW   ] ) (dmg).damage -= (res)[ DAMAGE_TYPE_SHADOW   ]; else (dmg).damage = 0; break;  \
        case DAMAGE_TYPE_MAGIC     : if( (dmg).damage > (res)[ DAMAGE_TYPE_MAGIC    ] ) (dmg).damage -= (res)[ DAMAGE_TYPE_MAGIC    ]; else (dmg).damage = 0; break;  \
        case DAMAGE_TYPE_FATIGUE   : if( (dmg).damage > (res)[ DAMAGE_TYPE_FATIGUE  ] ) (dmg).damage -= (res)[ DAMAGE_TYPE_FATIGUE  ]; else (dmg).damage = 0; break;  \
        default: break;                                     \
    }                                                       \
} while( 0 )

#define MOD_STAMINA_REGEN_TIME(t, st)                       \
do                                                          \
{                                                           \
    if( (st) & DAMAGE_STATUS_FROZEN )           (t) <<= 1;  \
    if( (st) & DAMAGE_STATUS_TYPE_EXHAUSTION )  (t) <<= 1;  \
} while( 0 )

#define BUILDUP_MAX     (uint8_t)127

// build up starts at 0 and goes to BUILDUP_MAX, >=BUILDUP_MAX == trigger effect, dec by base value
#define RESET_BUILDUP(bu)             \
do                                    \
{                                     \
    (bu)[ DAMAGE_TYPE_PHYSICAL ] = 0; \
    (bu)[ DAMAGE_TYPE_FIRE     ] = 0; \
    (bu)[ DAMAGE_TYPE_COLD     ] = 0; \
    (bu)[ DAMAGE_TYPE_POISON   ] = 0; \
    (bu)[ DAMAGE_TYPE_RADIANT  ] = 0; \
    (bu)[ DAMAGE_TYPE_SHADOW   ] = 0; \
    (bu)[ DAMAGE_TYPE_MAGIC    ] = 0; \
    (bu)[ DAMAGE_TYPE_FATIGUE  ] = 0; \
} while( 0 )

// build up status effects from damage if not already triggered
#define BUILDUP_DAMAGE(dmg, bu, st)                         \
do                                                          \
{                                                           \
    switch( DAMAGE_TYPE_MASK((dmg).damage_type) )           \
    {                                                       \
        case DAMAGE_TYPE_PHYSICAL  : if( ( (st) & DAMAGE_STATUS_TYPE_STUNNED ) == 0 )    { (bu)[ DAMAGE_TYPE_PHYSICAL ] += (dmg).damage; if( (bu)[ DAMAGE_TYPE_PHYSICAL ] >= BUILDUP_MAX ) { (st) |= DAMAGE_STATUS_TYPE_STUNNED; } } break; \
        case DAMAGE_TYPE_FIRE      : if( ( (st) & DAMAGE_STATUS_TYPE_BURN ) == 0 )       { (bu)[ DAMAGE_TYPE_FIRE     ] += (dmg).damage; if( (bu)[ DAMAGE_TYPE_FIRE     ] >= BUILDUP_MAX ) { (st) |= DAMAGE_STATUS_TYPE_BURN; } } break; \
        case DAMAGE_TYPE_COLD      : if( ( (st) & DAMAGE_STATUS_TYPE_FROZEN ) == 0 )     { (bu)[ DAMAGE_TYPE_COLD     ] += (dmg).damage; if( (bu)[ DAMAGE_TYPE_COLD     ] >= BUILDUP_MAX ) { (st) |= DAMAGE_STATUS_TYPE_FROZEN; } } break; \
        case DAMAGE_TYPE_POISON    : if( ( (st) & DAMAGE_STATUS_TYPE_POISONED ) == 0 )   { (bu)[ DAMAGE_TYPE_POISON   ] += (dmg).damage; if( (bu)[ DAMAGE_TYPE_POISON   ] >= BUILDUP_MAX ) { (st) |= DAMAGE_STATUS_TYPE_POISONED; } } break; \
        case DAMAGE_TYPE_RADIANT   : if( ( (st) & DAMAGE_STATUS_TYPE_BLINDED ) == 0 )    { (bu)[ DAMAGE_TYPE_RADIANT  ] += (dmg).damage; if( (bu)[ DAMAGE_TYPE_RADIANT  ] >= BUILDUP_MAX ) { (st) |= DAMAGE_STATUS_TYPE_BLINDED; } } break; \
        case DAMAGE_TYPE_SHADOW    : if( ( (st) & DAMAGE_STATUS_TYPE_WITHERD ) == 0 )    { (bu)[ DAMAGE_TYPE_SHADOW   ] += (dmg).damage; if( (bu)[ DAMAGE_TYPE_SHADOW   ] >= BUILDUP_MAX ) { (st) |= DAMAGE_STATUS_TYPE_WITHERD; } } break; \
        case DAMAGE_TYPE_MAGIC     : if( ( (st) & DAMAGE_STATUS_TYPE_MANABURN ) == 0 )   { (bu)[ DAMAGE_TYPE_MAGIC    ] += (dmg).damage; if( (bu)[ DAMAGE_TYPE_MAGIC    ] >= BUILDUP_MAX ) { (st) |= DAMAGE_STATUS_TYPE_MANABURN; } } break; \
        case DAMAGE_TYPE_FATIGUE   : if( ( (st) & DAMAGE_STATUS_TYPE_EXHAUSTION ) == 0 ) { (bu)[ DAMAGE_TYPE_FATIGUE  ] += (dmg).damage; if( (bu)[ DAMAGE_TYPE_FATIGUE  ] >= BUILDUP_MAX ) { (st) |= DAMAGE_STATUS_TYPE_EXHAUSTION; } } break; \
        default: break;                                     \
    }                                                       \
} while( 0 )

#define _TICK_BUILDUP_DAMAGE_TYPE(dt, s, bu, st, res, d)\
if( (bu)[ dt ] > 0 )                                    \
{                                                       \
    if( (bu)[ dt ] <= (d) )                             \
    {                                                   \
        (bu)[ dt ] = 0;                                 \
        (st) &= ~(s);                                   \
    }                                                   \
    else                                                \
    {                                                   \
        (bu)[ dt ] -= (d);                              \
    }                                                   \
}


// tick build up by (d), clearing status effects when build up is 0
#define TICK_BUILDUP(bu, st, res, d)                                                                    \
do                                                                                                      \
{                                                                                                       \
    _TICK_BUILDUP_DAMAGE_TYPE( DAMAGE_TYPE_PHYSICAL, DAMAGE_STATUS_TYPE_STUNNED   , bu, st, res, d );   \
    _TICK_BUILDUP_DAMAGE_TYPE( DAMAGE_TYPE_FIRE    , DAMAGE_STATUS_TYPE_BURN      , bu, st, res, d );   \
    _TICK_BUILDUP_DAMAGE_TYPE( DAMAGE_TYPE_COLD    , DAMAGE_STATUS_TYPE_FROZEN    , bu, st, res, d );   \
    _TICK_BUILDUP_DAMAGE_TYPE( DAMAGE_TYPE_POISON  , DAMAGE_STATUS_TYPE_POISONED  , bu, st, res, d );   \
    _TICK_BUILDUP_DAMAGE_TYPE( DAMAGE_TYPE_RADIANT , DAMAGE_STATUS_TYPE_BLINDED   , bu, st, res, d );   \
    _TICK_BUILDUP_DAMAGE_TYPE( DAMAGE_TYPE_SHADOW  , DAMAGE_STATUS_TYPE_WITHERD   , bu, st, res, d );   \
    _TICK_BUILDUP_DAMAGE_TYPE( DAMAGE_TYPE_MAGIC   , DAMAGE_STATUS_TYPE_MANABURN  , bu, st, res, d );   \
    _TICK_BUILDUP_DAMAGE_TYPE( DAMAGE_TYPE_FATIGUE , DAMAGE_STATUS_TYPE_EXHAUSTION, bu, st, res, d );   \
} while( 0 )

//
//
//

extern union
{
    uint8_t _data[8];
    ltrb_aabb_t aabb;
} combat_damage_area;
#pragma zpsym("combat_damage_area");

//
// Defined in player.c
//

uint8_t test_attack_hits_player(void);

void queue_damage_player( uint8_t damage_type, uint8_t damage );

//
// Defined in boss.c
//

uint8_t test_attack_hits_boss( uint8_t attack_position );

void queue_damage_boss( uint8_t damage_type, uint8_t damage );

#endif // COMBAT_H