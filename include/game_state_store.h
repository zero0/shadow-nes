#ifndef GAME_STATE_STORE_H
#define GAME_STATE_STORE_H

enum
{
    STORE_0,
    STORE_1,
    STORE_2,
    _STORE_COUNT,
};

void game_state_store_enter(void);

void game_state_store_leave(void);

void game_state_store_update(void);

#endif // GAME_STATE_STORE_H