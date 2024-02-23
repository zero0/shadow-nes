#ifndef BOSS_H
#define BOSS_H

#include "types.h"
#include "text.h"
#include "flags.h"

enum
{
    Boss_0,
    Boss_1,
    Boss_2,
    Boss_3,
    Boss_4,
    Boss_5,
    Boss_6,
    Boss_7,
    Boss_8,
    Boss_9,
    _Boss_Count,
};

#define BOSS_CHANGED_HEALTH     (uint8_t)( 1 << 0 )
#define BOSS_CHANGED_POSITION   (uint8_t)( 1 << 0 )

uint8_t get_boss_health_per_block_log2(void);

uint16_t get_boss_current_health(void);

uint16_t get_boss_max_health(void);

str_t get_boss_name(void);

str_t get_boss_location(void);

flags8_t get_boss_changed_flags(void);

void init_boss( uint8_t bossIndex );

void update_boss(void);

#endif // BOSS_H
