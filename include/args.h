#ifndef ARGS_H
#define ARGS_H

extern uint8_t ARGS[8];
#pragma zpsym("ARGS");

#define args_call_0( f )    \
f()

#define args_call_1( f, p0 )    \
ARGS[0] = (p0);                 \
f()

#define args_call_2( f, p0, p1 )    \
ARGS[0] = (p0);                     \
ARGS[1] = (p1);                     \
f()

#define args_call_3( f, p0, p1, p2 )\
ARGS[0] = (p0);                     \
ARGS[1] = (p1);                     \
ARGS[2] = (p2);                     \
f()

#define args_call_4( f, p0, p1, p2, p3 )\
ARGS[0] = (p0);                     \
ARGS[1] = (p1);                     \
ARGS[2] = (p2);                     \
ARGS[3] = (p3);                     \
f()


#endif // ARGS_H
