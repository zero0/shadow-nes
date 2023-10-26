#ifndef PPU_H
#define PPU_H

#include "types.h"
#include "macros.h"

#define NAMETABLE_0             (char)0
#define NAMETABLE_1             (char)1
#define NAMETABLE_2             (char)2
#define NAMETABLE_3             (char)3

#define PALETTE_BACKGROUND_0    (char)0
#define PALETTE_BACKGROUND_1    (char)1
#define PALETTE_BACKGROUND_2    (char)2
#define PALETTE_BACKGROUND_3    (char)3

#define PALETTE_SPRITE_0        (char)0
#define PALETTE_SPRITE_1        (char)1
#define PALETTE_SPRITE_2        (char)2
#define PALETTE_SPRITE_3        (char)3

#define MAKE_SCROLL( x, y )     ( ( (char)(x) << 8 ) | (char)(y) )

unsigned char __fastcall__ ppu_frame_index(void);

void __fastcall__ ppu_update(void);

void __fastcall__ ppu_off(void);

void __fastcall__ ppu_skip(void);

void __fastcall__ ppu_set_scroll( int xy );

void __fastcall__ ppu_address_tile( char x, char y );

void __fastcall__ ppu_update_tile( char x, char y, char t );

void __fastcall__ ppu_update_byte( char x, char y, char b );

void __fastcall__ ppu_clear_nametable( ptr_t tableAddress );

void __fastcall__ ppu_clear_palette();

void __fastcall__ ppu_fill_nametable( ptr_t tableAddress, char tile );

void __fastcall__ ppu_fill_nametable_attr( ptr_t tableAddress );

void __fastcall__ ppu_fill_nametable_attr_only( ptr_t tableAddress, char attr );


#endif // PPU_H
