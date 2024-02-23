#ifndef ANIMATION_H
#define ANIMATION_H

#include "flags.h"
#include "timer.h"

#define ANIM_SET_ATTR_NONE                      (uint8_t)0x00
#define ANIM_SET_ATTR_REPEAT                    (uint8_t)0x01

#define ANIM_KEYFRAME_ATTR_NONE                 (uint8_t)0x00
#define ANIM_KEYFRAME_ATTR_FLIP_H               (uint8_t)0x01
#define ANIM_KEYFRAME_ATTR_FLIP_V               (uint8_t)0x02
#define ANIM_KEYFRAME_ATTR_OVERRIDE_PALETTE     (uint8_t)0x08
#define ANIM_KEYFRAME_ATTR_OVERRIDE_PALETTE_0   (uint8_t)0x10
#define ANIM_KEYFRAME_ATTR_OVERRIDE_PALETTE_1   (uint8_t)0x20
#define ANIM_KEYFRAME_ATTR_OVERRIDE_PALETTE_2   (uint8_t)0x30
#define ANIM_KEYFRAME_ATTR_OVERRIDE_PALETTE_3   (uint8_t)0x40

typedef struct
{
    timer_t timer;
    uint8_t set_index;
    uint8_t set_keyframe_index;
} anim_control_t;

typedef struct
{
    uint8_t start_keyframe;
    uint8_t end_keyframe;
    flags8_t attr;       // attribute flags
} anim_set_t;

typedef struct
{
    uint8_t meta_sprite;
    uint8_t frame_count;
    uint8_t x_offset;
    uint8_t y_offset;
    flags8_t attr;       // attribute flags
} anim_keyframe_t;

#define lerp_template_uint8( v, t, tmax )  v lerp_uint8_##t( uint8_t a, uint8_t b ) { return ( (tamx) - (t) )}
typedef uint8_t (*lerp_uint8_func)( uint8_t a, uint8_t b );



#endif // ANIMATION_H
