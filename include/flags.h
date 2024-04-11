#ifndef FLAGS_H
#define FLAGS_H

#include "types.h"

typedef uint8_t flags8_t;

#define flags_reset( s )            (s) = 0
#define flags_set( s, f )           (s) = (f)
#define flags_mark( s, f )          (s) |= (f)
#define flags_unmark( s, f )        (s) &= ~(f)
#define flags_toggle( s, f )        (s) ^= (f)
#define flags_is_set( s, f )        ( ( (s) & (f) ) == (f) )
#define flags_is_any_set( s, f )    ( ( (s) & (f) ) != 0 )
#define flags_is_not_set( s, f )    ( ( (s) & (f) ) == 0 )

#endif // FLAGS_H
