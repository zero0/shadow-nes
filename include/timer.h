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

typedef uint8_t timer_t;

#define timer_reset( t )        timer_set( t, 0 )
#define timer_set( t, f )       (t) = (f)
#define timer_tick( t )             \
do                                  \
{                                   \
    if( (t) > 0 ) --(t);  \
} while( 0 )
#define timer_tick_unchecked( t )   --(t)
#define timer_is_done( t )      ( (t) == 0 )

#endif // TIMER_H
