#ifndef TEXT_H
#define TEXT_H

#include "types.h"
#include "args.h"

typedef uint8_t str_t;

#include "../assets/gametext.en.h"

uint8_t __fastcall__ text_strlen(str_t str);

void __fastcall__ text_begin_str_at(str_t str);

char __fastcall__ text_str_at(uint8_t index);

#define text_draw_string( x, y, p, t )  args_call_4( text_draw_string_impl, x, y, p, t )

#define text_draw_uint8( x, y, p, n )

#define text_draw_uint16( x, y, p, n )

void __fastcall__ text_draw_string_impl(void);

#endif // TEXT_H
