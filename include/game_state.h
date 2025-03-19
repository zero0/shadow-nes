#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "timer.h"
#include "flags.h"
#include "types.h"
#include "macros.h"

#include "game_state_title.h"
#include "game_state_continue.h"
#include "game_state_playing.h"
#include "game_state_cutscene.h"
#include "game_state_store.h"
#include "game_state_checkpoint.h"

enum
{
    GAME_STATE_INIT,
    GAME_STATE_TITLE,
    GAME_STATE_CONTINUE,
    GAME_STATE_PLAYING,
    GAME_STATE_CUTSCENE,
    GAME_STATE_STORE,
    GAME_STATE_CHECKPOINT,
    GAME_STATE_GAME_OVER,
    _GAME_STATE_COUNT,
};

enum
{
    GAME_STATE_DIRECTION_BACKWARD = -1,
    GAME_STATE_DIRECTION_FORWARD =   1,
};

extern uint8_t game_state;
#pragma zpsym("game_state")

extern uint8_t next_game_state;
#pragma zpsym("next_game_state")
extern uint8_t next_game_state_arg;
#pragma zpsym("next_game_state_arg")
extern int8_t next_game_state_direction;
#pragma zpsym("next_game_state_direction")

extern uint8_t game_state_internal;
#pragma zpsym("game_state_internal")
extern timer_t game_state_timer;
#pragma zpsym("game_state_timer")

extern rt_timer_t game_rt_timer;
#pragma zpsym("game_rt_timer")

#define set_next_game_state_arg_dir(x,a,d)  do { next_game_state = (x); next_game_state_arg = (a); next_game_state_direction = (d); } while( 0 )
#define set_next_game_state_dir(x,d)        set_next_game_state_arg_dir(x, 0, d)
#define set_next_game_state_arg(x,a)        set_next_game_state_arg_dir(x, a, GAME_STATE_DIRECTION_FORWARD)
#define set_next_game_state(x)              set_next_game_state_arg_dir(x, 0, GAME_STATE_DIRECTION_FORWARD)


#endif // GAME_STATE_H
