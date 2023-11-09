#ifndef MACROS_H
#define MACROS_H

#define CONCAT_(x, y)       x ## y
#define CONCAT(x, y)        CONCAT_(x, y)

#define T_(x)               #x
#define T(x)                T_(x)

#define STATIC_ASSERT(t)    static void CONCAT(sa, __LINE__)( char [ (t) ? 1 : -1 ] )

#define STATE_RESET( s )     (s) = 0
#define STATE_SET( s, f )    (s) |= (f)
#define STATE_CLEAR( s, f )  (s) &= ~(f)
#define STATE_TOGGLE( s, f ) (s) ^= (f)
#define STATE_IS_SET( s, f ) ( ( (s) & (f) ) == (f) )

#endif // MACROS_H