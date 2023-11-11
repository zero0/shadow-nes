#ifndef TYPES_H
#define TYPES_H

typedef unsigned int ptr_t;
typedef signed int ptrdiff_t;

typedef unsigned char uint8_t;
typedef unsigned int uint16_t;

typedef signed char int8_t;
typedef signed int int16_t;

typedef unsigned char pixel_t;

typedef struct
{
    pixel_t px;
    pixel_t py;
} pixel2d_t;

#endif // TYPES_H