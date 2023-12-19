#ifndef GAME_STATE_H
#define GAME_STATE_H

#define GAME_STATE_INIT         (uint8_t)0
#define GAME_STATE_TITLE        (uint8_t)1
#define GAME_STATE_PLAYING      (uint8_t)2
#define GAME_STATE_GAME_OVER    (uint8_t)3

static uint8_t game_state;
static uint8_t next_game_state;

#define set_next_game_state(x)      next_game_state = (x)

#endif // GAME_STATE_H
