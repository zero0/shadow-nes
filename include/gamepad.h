#ifndef GAMEPAD_H
#define GAMEPAD_H

#include "macros.h"
#include "types.h"

void __fastcall__ gamepad_poll( uint8_t pad );

uint8_t __fastcall__ gamepad_state( uint8_t pad );

uint8_t __fastcall__ gamepad_prev_state( uint8_t pad );

#define GAMEPAD_A       (uint8_t)0x80
#define GAMEPAD_B       (uint8_t)0x40
#define GAMEPAD_SELECT  (uint8_t)0x20
#define GAMEPAD_START   (uint8_t)0x10
#define GAMEPAD_U       (uint8_t)0x08
#define GAMEPAD_D       (uint8_t)0x04
#define GAMEPAD_L       (uint8_t)0x02
#define GAMEPAD_R       (uint8_t)0x01
STATIC_ASSERT( sizeof( GAMEPAD_A ) == sizeof( uint8_t ) );

// true if the button is pressed
#define GAMEPAD_BTN(v, b)               ( ( (v) & (b) ) == (b) )
#define GAMEPAD_BTN_DONW(c, b)          ( GAMEPAD_BTN( c, b ) )
#define GAMEPAD_BTN_UP(c, b)            ( !GAMEPAD_BTN( c, b ) )
#define GAMEPAD_BTN_PRESSED(p, c, b)    ( !GAMEPAD_BTN(p, b) && GAMEPAD_BTN(c, b) )
#define GAMEPAD_BTN_RELEASED(p, c, b)   ( GAMEPAD_BTN(p, b) && !GAMEPAD_BTN(c, b) )


#endif // GAMEPAD_H