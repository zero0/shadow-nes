#ifndef TEXT_H
#define TEXT_H

#include "types.h"
#include "args.h"

#define text_draw_string( x, y, p, t )      args_call_4_3a_1p( text_draw_string_impl, x, y, p, t )

#define text_draw_uint8( x, y, p, n )       args_call_4( text_draw_uint8_impl, x, y, p, n )
#define text_draw_uint8_x2( x, y, p, n )    args_call_4( text_draw_uint8_x2_impl, x, y, p, n )

#define text_draw_uint16( x, y, p, n )
#define text_draw_uint16_x2( x, y, p, n )

#define text_draw_string_delay(x, y, p, t)  args_call_4_3a_1p( text_draw_string_delay_impl, x, y, p, t )

void __fastcall__ text_delay_start(void);

void __fastcall__ text_delay_advance(void);

void __fastcall__ text_delay_display_full(void);

//
// Internal functions
//

void __fastcall__ text_draw_string_delay_impl(void);

void __fastcall__ text_draw_string_impl(void);

void __fastcall__ text_draw_uint8_impl(void);

void __fastcall__ text_draw_uint8_x2_impl(void);

#endif // TEXT_H
