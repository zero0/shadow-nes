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

uint8_t get_player_current_health();

uint8_t get_player_max_health();

uint8_t get_player_current_stamina();

uint8_t get_player_max_stamina();

flags8_t get_player_changed_flags();

void player_init(void);

void player_update();

#endif // PLAYER_H
