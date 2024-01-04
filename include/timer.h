#ifndef TIMER_H
#define TIMER_H

#include "types.h"

typedef struct
{
    uint8_t frames;
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;    
} timer_t;

void __fastcall__ timer_reset( timer_t& t )
{
    t.frames = 0;
    t.seconds = 0;
    t.minutes = 0;
    t.hours = 0;
}

void __fastcall__ timer_tick( timer_t& t )
{
    ++t.frames;
    if( t.frames == 60 )
    {
        t.frames = 0;
        ++t.seconds;
        if( t.seconds == 60 )
        {
            t.seconds = 0;
            ++t.minutes;
            if( t.minutes == 60 )
            {
                t.minutes = 0;
                ++t.hours;
            }
        }
    }
}

#endif // TIMER_H
