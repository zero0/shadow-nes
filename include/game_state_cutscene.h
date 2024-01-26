#ifndef GAME_STATE_CUTSCENE_H
#define GAME_STATE_CUTSCENE_H

enum
{
    Cutscene_000_Intro,

    Cutscene_100_Boss0_Preroll,
    Cutscene_110_Boss0_Postroll,

    Cutscene_999_Credits,
};

void game_state_cutscene_enter(void);

void game_state_cutscene_leave(void);

void game_state_cutscene_update(void);

#endif // GAME_STATE_CUTSCENE_H
