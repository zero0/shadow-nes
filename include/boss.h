#ifndef BOSS_H
#define BOSS_H

#include "types.h"
#include "text.h"

#define BOSS_0  (uint8_t)0

#define BOSS_CHANGED_HEALTH     (uint8_t)( 1 << 0 )

uint8_t get_boss_health_per_block_log2(void);

uint16_t get_boss_current_health(void);

uint16_t get_boss_max_health(void);

str_t get_boss_name(void);

str_t get_boss_location(void);

uint8_t get_boss_changed_flags(void);

void boss_take_damage(uint8_t dmg);

void init_boss( uint8_t bossIndex );

void update_boss(void);

#endif // BOSS_H
