#ifndef GAMEPAD_H
#define GAMEPAD_H

#include "core.h"

void __fastcall__ gamepad_poll( char pad );

char __fastcall__ gamepad_state( char pad );

char __fastcall__ gamepad_prev_state( char pad );

#define GAMEPAD_A       (char)0x01
#define GAMEPAD_B       (char)0x02
#define GAMEPAD_SELECT  (char)0x04
#define GAMEPAD_START   (char)0x08
#define GAMEPAD_U       (char)0x10
#define GAMEPAD_D       (char)0x20
#define GAMEPAD_L       (char)0x40
#define GAMEPAD_R       (char)0x80
STATIC_ASSERT( sizeof( GAMEPAD_A ) == 1 );

// true if the button is pressed
#define GAMEPAD_BTN(v, b)               ( ( (v) & (b) ) == (b) )
#define GAMEPAD_BTN_DONW(p, c, b)       ( GAMEPAD_BTN( c, b ) )
#define GAMEPAD_BTN_UP(p, c, b)         ( !GAMEPAD_BTN( c, b ) )
#define GAMEPAD_BTN_PRESSED(p, c, b)    ( !GAMEPAD_BTN(p, b) && GAMEPAD_BTN(c, b) )
#define GAMEPAD_BTN_RELEASED(p, c, b)   ( GAMEPAD_BTN(p, b) && !GAMEPAD_BTN(c, b) )


#endif // GAMEPAD_H