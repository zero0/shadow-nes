#ifndef BOSS_H
#define BOSS_H

#include "types.h"
#include "text.h"

#define BOSS_0          (uint8_t)0
#define BOSS_1          (uint8_t)1
#define BOSS_2          (uint8_t)2
#define BOSS_3          (uint8_t)3
#define BOSS_4          (uint8_t)4
#define BOSS_5          (uint8_t)5
#define BOSS_6          (uint8_t)6
#define BOSS_7          (uint8_t)7
#define _BOSS_COUNT     (uint8_t)8

#define BOSS_CHANGED_HEALTH     (uint8_t)( 1 << 0 )

uint8_t get_boss_health_per_block_log2(void);

uint16_t get_boss_current_health(void);

uint16_t get_boss_max_health(void);

str_t get_boss_name(void);

str_t get_boss_location(void);

uint8_t get_boss_changed_flags(void);

void init_boss( uint8_t bossIndex );

void update_boss(void);

#endif // BOSS_H
