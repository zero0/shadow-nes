#ifndef ANIMATION_H
#define ANIMATION_H

#define ANIM_SET_ATTR_NONE      0x00
#define ANIM_SET_ATTR_REPEAT    0x01

#define ANIM_KEYFRAME_ATTR_NONE                 0x00
#define ANIM_KEYFRAME_ATTR_FLIP_H               0x01
#define ANIM_KEYFRAME_ATTR_FLIP_V               0x02
#define ANIM_KEYFRAME_ATTR_OVERRIDE_PALETTE     0x08
#define ANIM_KEYFRAME_ATTR_OVERRIDE_PALETTE_0   0x10
#define ANIM_KEYFRAME_ATTR_OVERRIDE_PALETTE_1   0x20
#define ANIM_KEYFRAME_ATTR_OVERRIDE_PALETTE_2   0x30
#define ANIM_KEYFRAME_ATTR_OVERRIDE_PALETTE_3   0x40

typedef struct
{
    uint8_t timer;
    uint8_t set_index;
    uint8_t keyframe_index;
} anim_control_t;

typedef struct
{
    uint8_t start_keyframe;
    uint8_t end_keyframe;
    uint8_t attr;       // attribute flags
} anim_set_t;

typedef struct
{
    uint8_t meta_sprite;
    uint8_t frames;
    uint8_t x_offset;
    uint8_t y_offset;
    uint8_t attr;       // include flip, maybe pallete override too
} anim_keyframe_t;

#endif // ANIMATION_H
