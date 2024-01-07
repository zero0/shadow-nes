#include "boss.h"

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

static uint8_t current_boss_state;
static uint8_t current_boss_changed_flags;
static uint8_t current_boss_index;
static uint16_t current_boss_health;

uint8_t __fastcall__ get_boss_health_per_block_log2(void)
{
    return all_boss_health_per_block_log2[ current_boss_index ];
}

uint16_t __fastcall__ get_boss_current_health(void)
{
    return current_boss_health;
}

uint16_t __fastcall__ get_boss_max_health(void)
{
    return all_boss_max_healths[ current_boss_index ];
}

str_t __fastcall__ get_boss_name(void)
{
    return all_boss_names[ current_boss_index ];
}

str_t __fastcall__ get_boss_location(void)
{
    return all_boss_location_names[ current_boss_index ];
}

uint8_t __fastcall__ get_boss_changed_flags(void)
{
    return current_boss_changed_flags;
}

void __fastcall__ boss_take_damage(uint8_t dmg)
{
    // if the boss is in I frames, don't take damage
    if( current_boss_state & BOSS_STATE_INV_FRAMES )
    {
        return;
    }

    // adjust incoming damage based on armor
    if( dmg < all_boss_armors[ current_boss_index ] )
    {
        dmg = 0;
    }
    else
    {
        dmg -= all_boss_armors[ current_boss_index ];
    }

    // take damage
    if( dmg > 0 )
    {
        current_boss_changed_flags |= BOSS_CHANGED_HEALTH;

        if( dmg >= current_boss_health )
        {
            current_boss_health -= dmg;
        }
        else
        {
            current_boss_health = 0;
        }

        if( current_boss_health == 0 )
        {
            current_boss_state |= BOSS_STATE_DEAD;
        }
    }
}

void __fastcall__ init_boss(uint8_t bossIndex)
{
    current_boss_index = 0;
    current_boss_state = 0;
    current_boss_changed_flags = 0xFF;

    current_boss_health = 1000;//all_boss_max_healths[ current_boss_index ];
}

void __fastcall__ update_boss(void)
{
    current_boss_changed_flags = 0;
}