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

//
//
//

uint8_t boss_is_ready(void);

void boss_init( uint8_t bossIndex );

void boss_update(void);

#endif // BOSS_H
