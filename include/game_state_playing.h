#ifndef GAME_STATE_PLAYING_H
#define GAME_STATE_PLAYING_H

void game_state_playing_enter(void);

void game_state_playing_leave(void);

void game_state_playing_update(void);

//
//
//

void game_state_playing_set_pause(uint8_t isPaused);

uint8_t game_state_playing_is_pause(void);

#endif // GAME_STATE_PLAYING_H