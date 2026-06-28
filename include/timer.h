#ifndef TIMER_H
#define TIMER_H

#include "types.h"

typedef struct
{
    uint8_t frames;
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
} rt_timer_t;

#define rt_timer_reset( t )         \
do                                  \
{                                   \
    (t).frames = 0;                 \
    (t).seconds = 0;                \
    (t).minutes = 0;                \
    (t).hours = 0;                  \
} while( 0 )

#define rt_timer_tick( t )          \
do                                  \
{                                   \
    ++(t).frames;                   \
    if( (t).frames == 60 )          \
    {                               \
        (t).frames = 0;             \
        ++(t).seconds;              \
        if( (t).seconds == 60 )     \
        {                           \
            (t).seconds = 0;        \
            ++(t).minutes;          \
            if( (t).minutes == 60 ) \
            {                       \
                (t).minutes = 0;    \
                ++(t).hours;        \
            }                       \
        }                           \
    }                               \
} while( 0 )

//
//
//

typedef uint8_t timer_t;

extern timer_t g_timers[8];
#pragma zpsym("g_timers");

typedef uint8_t timer_handle_t;

extern void init_timers(void);

extern void tick_timers(void);

extern timer_handle_t request_timer(uint8_t ticks);

extern void release_timer(timer_handle_t handle);

#define is_timer_done(handle)       (g_timers[handle] == 0)

#define set_timer(handle, ticks) do {   \
    g_timers[handle] = ticks;           \
} while( 0 )

#endif // TIMER_H
