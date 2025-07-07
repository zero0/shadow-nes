#ifndef TYPES_H
#define TYPES_H

#include "macros.h"

typedef unsigned char   uint8_t;
typedef unsigned int    uint16_t;
typedef unsigned long   uint32_t;

typedef signed char     int8_t;
typedef signed int      int16_t;
typedef signed long     int32_t;

typedef unsigned char   pixel_t;

#if sizeof(int32_t) == sizeof(void*)
typedef uint32_t        ptr_t;
typedef int32_t         ptrdiff_t;
#elif sizeof(int16_t) == sizeof(void*)
typedef uint16_t        ptr_t;
typedef int16_t         ptrdiff_t;
#else
#error "Unknown pointer size"
#endif

typedef struct
{
    pixel_t px;
    pixel_t py;
} pixel2d_t;

#endif // TYPES_H