#ifndef COLLISION_H
#define COLLISION_H

#include "types.h"
#include "macros.h"
#include "subpixel.h"

extern uint8_t test_collision_lh[4];
#pragma zpsym("test_collision_lh");
extern uint8_t test_collision_rh[4];
#pragma zpsym("test_collision_rh");

#define AABB_LEFT    0
#define AABB_TOP     1
#define AABB_RIGHT   2
#define AABB_BOTTOM  3

typedef struct
{
    subpixel_t x;
    subpixel_t y;
    uint8_t width;
    uint8_t height;
} aabb_t;

typedef struct
{
    subpixel_t center_x;
    subpixel_t center_y;
    uint8_t ext_x;
    uint8_t ext_y;
} ext_aabb_t;

typedef struct
{
    subpixel_t center_x;
    subpixel_t center_y;
    uint8_t ext_l;
    uint8_t ext_t;
    uint8_t ext_r;
    uint8_t ext_b;
} ltrb_aabb_t;

enum
{
    COLLISION_TEST_MISS,
    COLLISION_TEST_HIT,
};

uint8_t __fastcall__ perform_collision_test(void);

//
// Collision API
//

#define begin_collision_aabb(h, aabb)                                                           \
do                                                                                              \
{                                                                                               \
    CONCAT(test_collision_, h)[AABB_LEFT  ] = subpixel_to_pixel( (aabb).x );                    \
    CONCAT(test_collision_, h)[AABB_TOP   ] = subpixel_to_pixel( (aabb).y );                    \
    CONCAT(test_collision_, h)[AABB_RIGHT ] = subpixel_to_pixel( (aabb).x ) + (aabb).width;     \
    CONCAT(test_collision_, h)[AABB_BOTTOM] = subpixel_to_pixel( (aabb).y ) + (aabb).height;    \
} while( 0 )

#define begin_collision_subpixel(h, x, y)                                   \
do                                                                          \
{                                                                           \
    CONCAT(test_collision_, h)[AABB_LEFT  ] = subpixel_to_pixel( x );       \
    CONCAT(test_collision_, h)[AABB_TOP   ] = subpixel_to_pixel( y );       \
    CONCAT(test_collision_, h)[AABB_RIGHT ] = subpixel_to_pixel( x ) + 1;   \
    CONCAT(test_collision_, h)[AABB_BOTTOM] = subpixel_to_pixel( y ) + 1;   \
} while( 0 )

#define begin_collision_point(h, x, y)                  \
do                                                      \
{                                                       \
    CONCAT(test_collision_, h)[AABB_LEFT  ] = (x);      \
    CONCAT(test_collision_, h)[AABB_TOP   ] = (y);      \
    CONCAT(test_collision_, h)[AABB_RIGHT ] = (x) + 1;  \
    CONCAT(test_collision_, h)[AABB_BOTTOM] = (y) + 1;  \
} while( 0 )

#define begin_collision_ext_aabb(h, ext)                                                            \
do                                                                                                  \
{                                                                                                   \
    CONCAT(test_collision_, h)[AABB_LEFT  ] = subpixel_to_pixel( (ext).center_x ) - (ext).ext_x;    \
    CONCAT(test_collision_, h)[AABB_TOP   ] = subpixel_to_pixel( (ext).center_y ) - (ext).ext_y;    \
    CONCAT(test_collision_, h)[AABB_RIGHT ] = subpixel_to_pixel( (ext).center_x ) + (ext).ext_x;    \
    CONCAT(test_collision_, h)[AABB_BOTTOM] = subpixel_to_pixel( (ext).center_y ) + (ext).ext_y;    \
} while( 0 )

#define begin_collision_ltrb_aabb(h, ltrb)                                                          \
do                                                                                                  \
{                                                                                                   \
    CONCAT(test_collision_, h)[AABB_LEFT  ] = subpixel_to_pixel( (ltrb).center_x ) - (ltrb).ext_l;  \
    CONCAT(test_collision_, h)[AABB_TOP   ] = subpixel_to_pixel( (ltrb).center_y ) - (ltrb).ext_t;  \
    CONCAT(test_collision_, h)[AABB_RIGHT ] = subpixel_to_pixel( (ltrb).center_x ) + (ltrb).ext_r;  \
    CONCAT(test_collision_, h)[AABB_BOTTOM] = subpixel_to_pixel( (ltrb).center_y ) + (ltrb).ext_b;  \
} while( 0 )

#endif // COLLISION_H
