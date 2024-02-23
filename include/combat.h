#ifndef COMBAT_H
#define COMBAT_H

#include "types.h"
#include "flags.h"
#include "subpixel.h"

//
//
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

#define DAMAGE_TYPE_ATTR_NONE   (uint8_t)0
#define DAMAGE_TYPE_ATTR_HEAL   (uint8_t)( 1 << 4 ) // dmg should be treakted as a heal
#define DAMAGE_TYPE_ATTR_MISS   (uint8_t)( 1 << 5 ) // dmg should be zeroed out
#define DAMAGE_TYPE_ATTR_CRIT   (uint8_t)( 1 << 6 ) // dmg should be crit (doubled)
#define DAMAGE_TYPE_ATTR_WEAK   (uint8_t)( 1 << 7 ) // dmg should be weak (halved)

#define DAMAGE_TYPE_MASK(dt)        ( (dt) & (uint8_t)0x0F )
#define DAMAGE_TYPE_ATTR_MASK(dt)   ( (dt) & (uint8_t)0xF0 )

#define MAKE_DAMAGE_TYPE(da, dt)    ( DAMAGE_TYPE_ATTR_MASK(da) | DAMAGE_TYPE_MASK(dt) )

#define MOD_INCOMING_DAMAGE_FROM_ATTR(dmg)                                  \
do                                                                          \
{                                                                           \
    ASSERT( !( (dmg).damage_type & DAMAGE_TYPE_ATTR_HEAL ) );               \
    if( (dmg).damage_type & DAMAGE_TYPE_ATTR_MISS ) (dmg).damage = 0;       \
    if( (dmg).damage_type & DAMAGE_TYPE_ATTR_CRIT ) (dmg).damage <<= 1;     \
    if( (dmg).damage_type & DAMAGE_TYPE_ATTR_WEAK ) (dmg).damage >>= 1;     \
} while( 0 )

#define MOD_INCOMING_HEALING_FROM_ATTR(heal)                                \
do                                                                          \
{                                                                           \
    ASSERT( ( (heal).damage_type & DAMAGE_TYPE_ATTR_HEAL ) );               \
    if( (heal).damage_type & DAMAGE_TYPE_ATTR_MISS ) (heal).damage = 0;     \
    if( (heal).damage_type & DAMAGE_TYPE_ATTR_CRIT ) (heal).damage <<= 1;   \
    if( (heal).damage_type & DAMAGE_TYPE_ATTR_WEAK ) (heal).damage >>= 1;   \
} while( 0 )

typedef uint8_t damage_type_t;

typedef struct {
    damage_type_t damage_type;
    uint8_t damage;
} damage_t;

//
//
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

#define DAMAGE_STATUS_STUNNED               (uint8_t)(1 << DAMAGE_STATUS_TYPE_STUNNED )
#define DAMAGE_STATUS_BURN                  (uint8_t)(1 << DAMAGE_STATUS_TYPE_BURN )
#define DAMAGE_STATUS_FROZEN                (uint8_t)(1 << DAMAGE_STATUS_TYPE_FROZEN )
#define DAMAGE_STATUS_POISONED              (uint8_t)(1 << DAMAGE_STATUS_TYPE_POISONED )
#define DAMAGE_STATUS_BLINDED               (uint8_t)(1 << DAMAGE_STATUS_TYPE_BLINDED )
#define DAMAGE_STATUS_WITHERD               (uint8_t)(1 << DAMAGE_STATUS_TYPE_WITHERD )
#define DAMAGE_STATUS_MANABURN              (uint8_t)(1 << DAMAGE_STATUS_TYPE_MANABURN )
#define DAMAGE_STATUS_EXHAUSTION            (uint8_t)(1 << DAMAGE_STATUS_TYPE_EXHAUSTION )

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
    if( (st) & DAMAGE_STATUS_BURN && DAMAGE_TYPE_MASK( (dmg).damage_type ) == DAMAGE_TYPE_PHYSICAL ) (dmg).damage += 10;    \
    if( (st) & DAMAGE_STATUS_MANABURN && DAMAGE_TYPE_MASK( (dmg).damage_type ) == DAMAGE_TYPE_MAGIC ) (dmg).damage <<= 1;    \
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
        case DAMAGE_TYPE_PHYSICAL  : if( (dmg).damage > (res)[ DAMAGE_TYPE_PHYSICAL  ] ) (dmg).damage -= (res)[ DAMAGE_TYPE_FLAT      ]; else (dmg).damage = 0; break;  \
        case DAMAGE_TYPE_FIRE      : if( (dmg).damage > (res)[ DAMAGE_TYPE_FIRE      ] ) (dmg).damage -= (res)[ DAMAGE_TYPE_PHYSICAL  ]; else (dmg).damage = 0; break;  \
        case DAMAGE_TYPE_COLD      : if( (dmg).damage > (res)[ DAMAGE_TYPE_COLD      ] ) (dmg).damage -= (res)[ DAMAGE_TYPE_FIRE      ]; else (dmg).damage = 0; break;  \
        case DAMAGE_TYPE_POISON    : if( (dmg).damage > (res)[ DAMAGE_TYPE_POISON    ] ) (dmg).damage -= (res)[ DAMAGE_TYPE_COLD      ]; else (dmg).damage = 0; break;  \
        case DAMAGE_TYPE_RADIANT   : if( (dmg).damage > (res)[ DAMAGE_TYPE_RADIANT   ] ) (dmg).damage -= (res)[ DAMAGE_TYPE_POISON    ]; else (dmg).damage = 0; break;  \
        case DAMAGE_TYPE_SHADOW    : if( (dmg).damage > (res)[ DAMAGE_TYPE_SHADOW    ] ) (dmg).damage -= (res)[ DAMAGE_TYPE_RADIANT   ]; else (dmg).damage = 0; break;  \
        case DAMAGE_TYPE_MAGIC     : if( (dmg).damage > (res)[ DAMAGE_TYPE_MAGIC     ] ) (dmg).damage -= (res)[ DAMAGE_TYPE_SHADOW    ]; else (dmg).damage = 0; break;  \
        case DAMAGE_TYPE_FATIGUE   : if( (dmg).damage > (res)[ DAMAGE_TYPE_FATIGUE   ] ) (dmg).damage -= (res)[ DAMAGE_TYPE_MAGIC     ]; else (dmg).damage = 0; break;  \
        default: break;                                     \
    }                                                       \
} while( 0 )

#define MOD_STAMINA_REGEN_TIME(t, st)                       \
do                                                          \
{                                                           \
    if( (st) & ( DAMAGE_STATUS_FROZEN | DAMAGE_STATUS_TYPE_EXHAUSTION ) ) (t) <<= 1;    \
} while( 0 )

#define BUILDUP_MAX     (uint8_t)127

// build up starts at 0 and goes to BUILDUP_MAX, >=BUILDUP_MAX == trigger effect, dec by res + base value
#define RESET_BUILDUP(bu)                 \
do                                                          \
{                                                           \
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
        default: break; \
    }                                                       \
} while( 0 )

// tick build up by (res + d), clearing status effects when build up is 0
#define TICK_BUILDUP(bu, st, res, d)                        \
do                                                          \
{                                                           \
    if( (bu)[ DAMAGE_TYPE_PHYSICAL ] > 0 ) { if( (bu)[ DAMAGE_TYPE_PHYSICAL ] < ( (res)[DAMAGE_TYPE_PHYSICAL ] + d ) ) (bu)[ DAMAGE_TYPE_PHYSICAL ] = 0; else (bu)[ DAMAGE_TYPE_PHYSICAL ] -= ( (res)[ DAMAGE_TYPE_PHYSICAL ] + d ); if( (bu)[ DAMAGE_TYPE_PHYSICAL ] == 0 ) (st) &= ~DAMAGE_STATUS_TYPE_STUNNED; } \
    if( (bu)[ DAMAGE_TYPE_FIRE     ] > 0 ) { if( (bu)[ DAMAGE_TYPE_FIRE     ] < ( (res)[DAMAGE_TYPE_FIRE     ] + d ) ) (bu)[ DAMAGE_TYPE_FIRE     ] = 0; else (bu)[ DAMAGE_TYPE_FIRE     ] -= ( (res)[ DAMAGE_TYPE_FIRE     ] + d ); if( (bu)[ DAMAGE_TYPE_FIRE     ] == 0 ) (st) &= ~DAMAGE_STATUS_TYPE_BURN; } \
    if( (bu)[ DAMAGE_TYPE_COLD     ] > 0 ) { if( (bu)[ DAMAGE_TYPE_COLD     ] < ( (res)[DAMAGE_TYPE_COLD     ] + d ) ) (bu)[ DAMAGE_TYPE_COLD     ] = 0; else (bu)[ DAMAGE_TYPE_COLD     ] -= ( (res)[ DAMAGE_TYPE_COLD     ] + d ); if( (bu)[ DAMAGE_TYPE_COLD     ] == 0 ) (st) &= ~DAMAGE_STATUS_TYPE_FROZEN; } \
    if( (bu)[ DAMAGE_TYPE_POISON   ] > 0 ) { if( (bu)[ DAMAGE_TYPE_POISON   ] < ( (res)[DAMAGE_TYPE_POISON   ] + d ) ) (bu)[ DAMAGE_TYPE_POISON   ] = 0; else (bu)[ DAMAGE_TYPE_POISON   ] -= ( (res)[ DAMAGE_TYPE_POISON   ] + d ); if( (bu)[ DAMAGE_TYPE_POISON   ] == 0 ) (st) &= ~DAMAGE_STATUS_TYPE_POISONED; } \
    if( (bu)[ DAMAGE_TYPE_RADIANT  ] > 0 ) { if( (bu)[ DAMAGE_TYPE_RADIANT  ] < ( (res)[DAMAGE_TYPE_RADIANT  ] + d ) ) (bu)[ DAMAGE_TYPE_RADIANT  ] = 0; else (bu)[ DAMAGE_TYPE_RADIANT  ] -= ( (res)[ DAMAGE_TYPE_RADIANT  ] + d ); if( (bu)[ DAMAGE_TYPE_RADIANT  ] == 0 ) (st) &= ~DAMAGE_STATUS_TYPE_BLINDED; } \
    if( (bu)[ DAMAGE_TYPE_SHADOW   ] > 0 ) { if( (bu)[ DAMAGE_TYPE_SHADOW   ] < ( (res)[DAMAGE_TYPE_SHADOW   ] + d ) ) (bu)[ DAMAGE_TYPE_SHADOW   ] = 0; else (bu)[ DAMAGE_TYPE_SHADOW   ] -= ( (res)[ DAMAGE_TYPE_SHADOW   ] + d ); if( (bu)[ DAMAGE_TYPE_SHADOW   ] == 0 ) (st) &= ~DAMAGE_STATUS_TYPE_WITHERD; } \
    if( (bu)[ DAMAGE_TYPE_MAGIC    ] > 0 ) { if( (bu)[ DAMAGE_TYPE_MAGIC    ] < ( (res)[DAMAGE_TYPE_MAGIC    ] + d ) ) (bu)[ DAMAGE_TYPE_MAGIC    ] = 0; else (bu)[ DAMAGE_TYPE_MAGIC    ] -= ( (res)[ DAMAGE_TYPE_MAGIC    ] + d ); if( (bu)[ DAMAGE_TYPE_MAGIC    ] == 0 ) (st) &= ~DAMAGE_STATUS_TYPE_MANABURN; } \
    if( (bu)[ DAMAGE_TYPE_FATIGUE  ] > 0 ) { if( (bu)[ DAMAGE_TYPE_FATIGUE  ] < ( (res)[DAMAGE_TYPE_FATIGUE  ] + d ) ) (bu)[ DAMAGE_TYPE_FATIGUE  ] = 0; else (bu)[ DAMAGE_TYPE_FATIGUE  ] -= ( (res)[ DAMAGE_TYPE_FATIGUE  ] + d ); if( (bu)[ DAMAGE_TYPE_FATIGUE  ] == 0 ) (st) &= ~DAMAGE_STATUS_TYPE_EXHAUSTION; } \
} while( 0 )

//
//
//

/*
  front r
l 0 1 2 i
e 3 4 5 g
f 6 7 8 h
t  back t
*/
#define DAMAGE_LOCATION_COLUMN_LEFT     (uint8_t)( 1 << 0 )
#define DAMAGE_LOCATION_COLUMN_CENTER   (uint8_t)( 1 << 1 )
#define DAMAGE_LOCATION_COLUMN_RIGHT    (uint8_t)( 1 << 2 )
#define DAMAGE_LOCATION_ROW_FRONT       (uint8_t)( 1 << 3 )
#define DAMAGE_LOCATION_ROW_MIDDLE      (uint8_t)( 1 << 4 )
#define DAMAGE_LOCATION_ROW_BACK        (uint8_t)( 1 << 5 )
#define DAMAGE_LOCATION_FORWARD_SLASH   (uint8_t)( 1 << 6 )  // "/" shape, lower left to upper right
#define DAMAGE_LOCATION_BACK_SLASH      (uint8_t)( 1 << 7 )  // "\" shape, top left to lower right

#define MAKE_COMBAT_POSITION(x,y)       ( ( 0x03 & (y) ) << 3 ) | ( 0x03 & (x) )
#define COMBAT_POSITION_X(p)            ( 0x03 & ( (p) >> 0 ) )
#define COMBAT_POSITION_Y(p)            ( 0x03 & ( (p) >> 3 ) )

#define COMBAT_POSITION_0x0             MAKE_COMBAT_POSITION( 0, 0 )
#define COMBAT_POSITION_1x0             MAKE_COMBAT_POSITION( 1, 0 )
#define COMBAT_POSITION_2x0             MAKE_COMBAT_POSITION( 2, 0 )

#define COMBAT_POSITION_0x1             MAKE_COMBAT_POSITION( 0, 1 )
#define COMBAT_POSITION_1x1             MAKE_COMBAT_POSITION( 1, 1 )
#define COMBAT_POSITION_2x1             MAKE_COMBAT_POSITION( 2, 1 )

#define COMBAT_POSITION_0x2             MAKE_COMBAT_POSITION( 0, 2 )
#define COMBAT_POSITION_1x2             MAKE_COMBAT_POSITION( 1, 2 )
#define COMBAT_POSITION_2x2             MAKE_COMBAT_POSITION( 2, 2 )

#define TEST_COMBAT_POSITION(pos, atk)          \
do                                              \
{                                               \
    switch( pos )                               \
    {                                           \
        case COMBAT_POSITION_0x0: return ( (atk) & (DAMAGE_LOCATION_COLUMN_LEFT | DAMAGE_LOCATION_ROW_FRONT | DAMAGE_LOCATION_BACK_SLASH) );                                     \
        case COMBAT_POSITION_1x0: return ( (atk) & (DAMAGE_LOCATION_COLUMN_CENTER | DAMAGE_LOCATION_ROW_FRONT) );                                                                \
        case COMBAT_POSITION_2x0: return ( (atk) & (DAMAGE_LOCATION_COLUMN_RIGHT | DAMAGE_LOCATION_ROW_FRONT | DAMAGE_LOCATION_FORWARD_SLASH) );                                 \
        case COMBAT_POSITION_0x1: return ( (atk) & (DAMAGE_LOCATION_COLUMN_LEFT | DAMAGE_LOCATION_ROW_MIDDLE ) );                                                                \
        case COMBAT_POSITION_1x1: return ( (atk) & (DAMAGE_LOCATION_COLUMN_CENTER | DAMAGE_LOCATION_ROW_MIDDLE | DAMAGE_LOCATION_BACK_SLASH | DAMAGE_LOCATION_FORWARD_SLASH ) ); \
        case COMBAT_POSITION_2x1: return ( (atk) & (DAMAGE_LOCATION_COLUMN_RIGHT | DAMAGE_LOCATION_ROW_MIDDLE ) );                                                               \
        case COMBAT_POSITION_0x2: return ( (atk) & (DAMAGE_LOCATION_COLUMN_LEFT | DAMAGE_LOCATION_ROW_BACK | DAMAGE_LOCATION_FORWARD_SLASH) );                                   \
        case COMBAT_POSITION_1x2: return ( (atk) & (DAMAGE_LOCATION_COLUMN_CENTER | DAMAGE_LOCATION_ROW_BACK ) );                                                                \
        case COMBAT_POSITION_2x2: return ( (atk) & (DAMAGE_LOCATION_COLUMN_RIGHT | DAMAGE_LOCATION_ROW_BACK | DAMAGE_LOCATION_BACK_SLASH) );                                     \
    }                                           \
} while( 0 )

#define TEST_COMBAT_POSITION_VALUE(r, pos, atk) \
do                                              \
{                                               \
    switch( pos )                               \
    {                                           \
        case COMBAT_POSITION_0x0: (r) = ( (atk) & (DAMAGE_LOCATION_COLUMN_LEFT | DAMAGE_LOCATION_ROW_FRONT | DAMAGE_LOCATION_BACK_SLASH) );                                      break; \
        case COMBAT_POSITION_1x0: (r) = ( (atk) & (DAMAGE_LOCATION_COLUMN_CENTER | DAMAGE_LOCATION_ROW_FRONT) );                                                                 break; \
        case COMBAT_POSITION_2x0: (r) = ( (atk) & (DAMAGE_LOCATION_COLUMN_RIGHT | DAMAGE_LOCATION_ROW_FRONT | DAMAGE_LOCATION_FORWARD_SLASH) );                                  break; \
        case COMBAT_POSITION_0x1: (r) = ( (atk) & (DAMAGE_LOCATION_COLUMN_LEFT | DAMAGE_LOCATION_ROW_MIDDLE ) );                                                                 break; \
        case COMBAT_POSITION_1x1: (r) = ( (atk) & (DAMAGE_LOCATION_COLUMN_CENTER | DAMAGE_LOCATION_ROW_MIDDLE | DAMAGE_LOCATION_BACK_SLASH | DAMAGE_LOCATION_FORWARD_SLASH ) );  break; \
        case COMBAT_POSITION_2x1: (r) = ( (atk) & (DAMAGE_LOCATION_COLUMN_RIGHT | DAMAGE_LOCATION_ROW_MIDDLE ) );                                                                break; \
        case COMBAT_POSITION_0x2: (r) = ( (atk) & (DAMAGE_LOCATION_COLUMN_LEFT | DAMAGE_LOCATION_ROW_BACK | DAMAGE_LOCATION_FORWARD_SLASH) );                                    break; \
        case COMBAT_POSITION_1x2: (r) = ( (atk) & (DAMAGE_LOCATION_COLUMN_CENTER | DAMAGE_LOCATION_ROW_BACK ) );                                                                 break; \
        case COMBAT_POSITION_2x2: (r) = ( (atk) & (DAMAGE_LOCATION_COLUMN_RIGHT | DAMAGE_LOCATION_ROW_BACK | DAMAGE_LOCATION_BACK_SLASH) );                                      break; \
    }                                           \
} while( 0 )

//
//
//

static const subpixel_t COMBAT_PLAYFIELD_WIDTH = { 12, 0 };
static const subpixel_t COMBAT_PLAYFIELD_HEIGHT = { 12, 0 };

#define clamp_subpixel_to_combat_playfield( sx, sy )    \
do                                                      \
{                                                       \
    if( sx.v >= COMBAT_PLAYFIELD_WIDTH.v )              \
    {                                                   \
        sx.v = COMBAT_PLAYFIELD_WIDTH.v;                \
    }                                                   \
    if( sy.v >= COMBAT_PLAYFIELD_HEIGHT.v )             \
    {                                                   \
        sy.v = COMBAT_PLAYFIELD_HEIGHT.v;               \
    }                                                   \
}                                                       \
while( 0 )

#define convert_subpixel_to_combat_position( sx, sy )   MAKE_COMBAT_POSITION( subpixel_to_pixel( sx ) >> 2, subpixel_to_pixel( sy ) >> 2 );

//
// Defined in player.c
//

uint8_t test_attack_hits_player( uint8_t attack_position );

void queue_damage_player( uint8_t damage_type, uint8_t damage );

//
// Defined in boss.c
//

uint8_t test_attack_hits_boss( uint8_t attack_position );

void queue_damage_boss( uint8_t damage_type, uint8_t damage );

#endif // COMBAT_H