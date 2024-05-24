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

enum
{
    BOSS_CHANGED_NONE =     0,
    BOSS_CHANGED_HEALTH =   1 << 0,
    BOSS_CHANGED_STAMINA =  1 << 1,
    BOSS_CHANGED_STATUS =   1 << 2,
    BOSS_CHANGED_POSITION = 1 << 3,
    BOSS_CHANGED_ALL =      ~0,
};

uint8_t get_boss_health_per_block_log2(void);

uint16_t get_boss_current_health(void);

uint16_t get_boss_max_health(void);

str_t get_boss_name(void);

str_t get_boss_location(void);

flags8_t get_boss_changed_flags(void);

uint8_t boss_is_dead(void);

void boss_init( uint8_t bossIndex );

void boss_update(void);

#endif // BOSS_H
