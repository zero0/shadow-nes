#ifndef PLAYER_H
#define PLAYER_H

#include "types.h"

#define PLAYER_CHANGED_HEALTH   (uint8_t)( 1 << 0 )
#define PLAYER_CHANGED_STAMINA  (uint8_t)( 1 << 1 )

uint8_t get_player_current_health();

uint8_t get_player_max_health();

uint8_t get_player_current_stamina();

uint8_t get_player_max_stamina();

uint8_t get_player_changed_flags();

void init_player();

void update_player();

#endif // PLAYER_H