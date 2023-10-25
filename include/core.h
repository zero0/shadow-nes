#ifndef CORE_H
#define CORE_H

#define CONCAT_(x, y)   x ## y
#define CONCAT(x, y)    CONCAT_(x, y)

#define STATIC_ASSERT(t)    static void CONCAT(sa, __LINE__)( char [ (t) ? 1 : -1 ] )

typedef unsigned int ptr_t;
typedef unsigned char uint8_t;


#endif // CORE_H