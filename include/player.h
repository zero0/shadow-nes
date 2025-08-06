#ifndef PLAYER_H
#define PLAYER_H

#include "types.h"
#include "flags.h"

#define PLAYER_CHANGED_NONE     (uint8_t)0
#define PLAYER_CHANGED_HEALTH   (uint8_t)( 1 << 0 )
#define PLAYER_CHANGED_STAMINA  (uint8_t)( 1 << 1 )
#define PLAYER_CHANGED_STATUS   (uint8_t)( 1 << 2 )
#define PLAYER_CHANGED_FLASKS   (uint8_t)( 1 << 3 )
#define PLAYER_CHANGED_POSITION (uint8_t)( 1 << 4 )
#define PLAYER_CHANGED_ALL      (uint8_t)~0

uint8_t player_is_dead(void);

void player_init(void);

void player_update(void);

void player_render(void);

#endif // PLAYER_H
