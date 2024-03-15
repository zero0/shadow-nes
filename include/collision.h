#ifndef COLLISION_H
#define COLLISION_H

#include "types.h"
#include "subpixel.h"

extern uint8_t test_collision_lh[4];
extern uint8_t test_collision_rh[4];

typedef struct
{
    subpixel_t x;
    subpixel_t y;
    uint8_t width;
    uint8_t height;
} aabb_t;

enum
{
    COLLISION_TEST_MISS,
    COLLISION_TEST_HIT,
};

extern uint8_t _test_collision_internal(void);

#define test_aabb_vs_subpixel(c, aabb, x, y)                                \
do                                                                          \
{                                                                           \
    test_collision_lh[0] = subpixel_to_pixel( (aabb).x );                   \
    test_collision_lh[1] = subpixel_to_pixel( (aabb).y );                   \
    test_collision_lh[2] = subpixel_to_pixel( (aabb).x ) + (aabb).width;    \
    test_collision_lh[3] = subpixel_to_pixel( (aabb).y ) + (aabb).height;   \
    test_collision_rh[0] = subpixel_to_pixel( x );                          \
    test_collision_rh[1] = subpixel_to_pixel( y );                          \
    test_collision_rh[2] = subpixel_to_pixel( x ) + 1;                      \
    test_collision_rh[3] = subpixel_to_pixel( y ) + 1;                      \
    (c) = test_collision_internal();                                        \
} while( false );

#define test_aabb_vs_point(c, aabb, x, y)                                   \
do                                                                          \
{                                                                           \
    test_collision_lh[0] = subpixel_to_pixel( (aabb).x );                   \
    test_collision_lh[1] = subpixel_to_pixel( (aabb).y );                   \
    test_collision_lh[2] = subpixel_to_pixel( (aabb).x ) + (aabb).width;    \
    test_collision_lh[3] = subpixel_to_pixel( (aabb).y ) + (aabb).height;   \
    test_collision_rh[0] = (x);                                             \
    test_collision_rh[1] = (y);                                             \
    test_collision_rh[2] = (x) + 1;                                         \
    test_collision_rh[3] = (y) + 1;                                         \
    (c) = test_collision_internal();                                        \
} while( false );

#define test_aabb_vs_aabb(c, aabb0, aabb1)                                  \
do                                                                          \
{                                                                           \
    test_collision_lh[0] = subpixel_to_pixel( (aabb0).x );                  \
    test_collision_lh[1] = subpixel_to_pixel( (aabb0).y );                  \
    test_collision_lh[2] = subpixel_to_pixel( (aabb0).x ) + (aabb0).width;  \
    test_collision_lh[3] = subpixel_to_pixel( (aabb0).y ) + (aabb0).height; \
    test_collision_rh[0] = subpixel_to_pixel( (aabb1).x );                  \
    test_collision_rh[1] = subpixel_to_pixel( (aabb1).y );                  \
    test_collision_rh[2] = subpixel_to_pixel( (aabb1).x ) + (aabb1).width;  \
    test_collision_rh[3] = subpixel_to_pixel( (aabb1).y ) + (aabb1).height; \
    (c) = test_collision_internal();                                        \
} while( false );

#endif // COLLISION_H
