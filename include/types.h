#ifndef TYPES_H
#define TYPES_H

#include "macros.h"

typedef unsigned int    ptr_t;
typedef signed int      ptrdiff_t;
STATIC_ASSERT(sizeof(ptr_t) == sizeof(void*));

typedef unsigned char   uint8_t;
typedef unsigned int    uint16_t;
typedef unsigned long   uint32_t;

typedef signed char     int8_t;
typedef signed int      int16_t;
typedef signed long     int32_t;

typedef unsigned char   pixel_t;

typedef struct
{
    pixel_t px;
    pixel_t py;
} pixel2d_t;

#endif // TYPES_H