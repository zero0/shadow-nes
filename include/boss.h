#ifndef BOSS_H
#define BOSS_H

#include "types.h"
#include "text.h"
#include "flags.h"

enum
{
    BOSS_0,
    BOSS_1,
    BOSS_2,
    BOSS_3,
    BOSS_4,
    BOSS_5,
    BOSS_6,
    BOSS_7,

    //
    BOSS_8,
    BOSS_9,
    _BOSS_COUNT,
};

#define BOSS_CHANGED_HEALTH     (uint8_t)( 1 << 0 )
#define BOSS_CHANGED_POSITION   (uint8_t)( 1 << 0 )

uint8_t get_boss_health_per_block_log2(void);

uint16_t get_boss_current_health(void);

uint16_t get_boss_max_health(void);

str_t get_boss_name(void);

str_t get_boss_location(void);

flags8_t get_boss_changed_flags(void);

void boss_init( uint8_t bossIndex );

void boss_update(void);

#endif // BOSS_H
