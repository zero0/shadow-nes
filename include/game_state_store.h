#ifndef GAME_STATE_STORE_H
#define GAME_STATE_STORE_H

enum
{
    Store_0,
    Store_1,
    Store_2
};

void game_state_store_enter(void);

void game_state_store_leave(void);

void game_state_store_update(void);

#endif // GAME_STATE_STORE_H