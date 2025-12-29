#ifndef MACROS_H
#define MACROS_H

#ifndef DISTRO_BUILD
#define USE_ASSERTS
#endif // DISTRO_BUILD

#define CONCAT_(x, y)           x ## y
#define CONCAT(x, y)            CONCAT_(x, y)

#define T_(x)                   #x
#define T(x)                    T_(x)

#define UNUSED(x)               (void)x

#define IS_POW2(t)              ( ( (t) & ( (t) - 1 ) ) == 0 )

#if 0
#define STATIC_ASSERT(x)        _Static_assert((x))
#else
#define STATIC_ASSERT(x)
#endif

#ifdef USE_ASSERTS
#define ASSERT(t)               ((t) ? (void)0 : __asm__("brk"))
#define INVALID_CODE_PATH       do { __asm__("brk"); } while(0)
#else
#define ASSERT(t)               (void)0
#define INVALID_CODE_PATH       (void)0
#endif // USE_ASSERTS

#define ARRAY_SIZE( arr )       ( sizeof(arr) / sizeof(arr[0]) )

#ifndef offsetof
#define offsetof( t, m )        ((uint8_t)&(((t *)0)->m))
#endif // offsetof

#ifndef memset
#define memset( p, v, s )           \
do {                                \
    __asm__("lda #%b", (v));        \
    __asm__("ldx #0");              \
    __asm__("@_" T(__LINE__) ":");  \
    __asm__("sta %v,x", p);         \
    __asm__("inx");                 \
    __asm__("cpx #%b", (s));        \
    __asm__("bne @_" T(__LINE__) ); \
} while( 0 )
#endif // memset

#endif // MACROS_H