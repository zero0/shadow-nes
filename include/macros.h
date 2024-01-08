#ifndef MACROS_H
#define MACROS_H

#ifndef DISTRO_BUILD
#define USE_ASSERTS
#endif // DISTRO_BUILD

#define CONCAT_(x, y)       x ## y
#define CONCAT(x, y)        CONCAT_(x, y)

#define T_(x)               #x
#define T(x)                T_(x)

#define STATIC_ASSERT(t)    static void CONCAT(sa, __LINE__)( char [ (t) ? 1 : -1 ] )

#ifdef USE_ASSERTS
#define ASSERT(t)           do { if( (t) ) {} else { __asm__("brk"); } } while( 0 )
#else
#define ASSERT(t)           (void)0
#endif

#define STATE_RESET( s )     (s) = 0
#define STATE_SET( s, f )    (s) |= (f)
#define STATE_CLEAR( s, f )  (s) &= ~(f)
#define STATE_TOGGLE( s, f ) (s) ^= (f)
#define STATE_IS_SET( s, f ) ( ( (s) & (f) ) == (f) )

#define ARRAY_SIZE( arr )    ( sizeof(arr) / sizeof(arr[0]) )

#ifndef offsetof
#define offsetof( t, m )     ((uint8_t)&(((t *)0)->m))
#endif // offsetof

#endif // MACROS_H