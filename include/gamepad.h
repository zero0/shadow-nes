#ifndef GAMEPAD_H
#define GAMEPAD_H

#include "macros.h"
#include "types.h"

enum
{
    GAMEPAD_1,
    GAMEPAD_2,
    GAMEPAD_3,
    GAMEPAD_4,
};

void __fastcall__ gamepad_poll( uint8_t pad );

uint8_t __fastcall__ gamepad_state( uint8_t pad );

uint8_t __fastcall__ gamepad_prev_state( uint8_t pad );

void __fastcall__ gamepad_clear_states( uint8_t pad );

uint8_t __fastcall__ gamepad_layer(void);

uint8_t __fastcall__ gampad_push_layer(void);

void __fastcall__ gamepad_pop_layer(void);

void __fastcall__ gamepad_reset_layer(void);

#define GAMEPAD_A       (uint8_t)0x80
#define GAMEPAD_B       (uint8_t)0x40
#define GAMEPAD_SELECT  (uint8_t)0x20
#define GAMEPAD_START   (uint8_t)0x10
#define GAMEPAD_U       (uint8_t)0x08
#define GAMEPAD_D       (uint8_t)0x04
#define GAMEPAD_L       (uint8_t)0x02
#define GAMEPAD_R       (uint8_t)0x01
STATIC_ASSERT( sizeof( GAMEPAD_A ) == sizeof( uint8_t ) );

// true if the button is down
#define GAMEPAD_BTN(v, b)               ( ( (v) & (b) ) == (b) )

#define GAMEPAD_BTN_DOWN(c, b)          ( GAMEPAD_BTN( c, b ) )
#define GAMEPAD_BTN_UP(c, b)            ( !GAMEPAD_BTN( c, b ) )
#define GAMEPAD_BTN_PRESSED(p, c, b)    ( GAMEPAD_BTN_UP(p, b) && GAMEPAD_BTN_DOWN(c, b) )
#define GAMEPAD_BTN_RELEASED(p, c, b)   ( GAMEPAD_BTN_DOWN(p, b) && GAMEPAD_BTN_UP(c, b) )
#define GAMEPAD_BTN_HELD(p, c, b)       ( GAMEPAD_BTN_DOWN(p, b) && GAMEPAD_BTN_DOWN(c, b) )

#define GAMEPAD_DOWN(pad, b)            ( GAMEPAD_BTN_DOWN( gamepad_state(pad), (b) ) )
#define GAMEPAD_UP(pad, b)              ( GAMEPAD_BTN_UP( gamepad_state(pad), (b) ) )
#define GAMEPAD_PRESSED(pad, b)         ( GAMEPAD_BTN_PRESSED( gamepad_prev_state(pad), gamepad_state(pad), (b) ) )
#define GAMEPAD_RELEASED(pad, b)        ( GAMEPAD_BTN_RELEASED( gamepad_prev_state(pad), gamepad_state(pad), (b) ) )
#define GAMEPAD_HELD(pad, b)            ( GAMEPAD_BTN_HELD( gamepad_prev_state(pad), gamepad_state(pad), (b) ) )


#endif // GAMEPAD_H