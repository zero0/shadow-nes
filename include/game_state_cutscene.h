#ifndef GAME_STATE_CUTSCENE_H
#define GAME_STATE_CUTSCENE_H

enum
{
    CUTSCENE_000_INTRO,

    CUTSCENE_100_BOSS_0,
    CUTSCENE_110_BOSS_0,

    CUTSCENE_999_CREDITS,

    _CUTSCENE_COUNT,
};

void game_state_cutscene_enter(void);

void game_state_cutscene_leave(void);

void game_state_cutscene_update(void);

#endif // GAME_STATE_CUTSCENE_H
