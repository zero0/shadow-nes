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

#define SPRITE_0                (char)0
#define SPRITE_1                (char)1

#define PALETTE_SPRITE_0        (char)0
#define PALETTE_SPRITE_1        (char)1
#define PALETTE_SPRITE_2        (char)2
#define PALETTE_SPRITE_3        (char)3

#define NAMETABLE_ROWS          (char)30
#define NAMETABLE_COLS          (char)32

#define SCREEN_WIDTH            (char)( NAMETABLE_COLS * 8 )
#define SCREEN_HEIGH            (char)( NAMETABLE_ROWS * 8 )


#define MAKE_SCROLL( x, y )     ( ( (char)(x) << 8 ) | (char)(y) )

extern uint8_t OAM_ARGS[4];
#pragma zpsym("OAM_ARGS");

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

#define oam_sprit_full( px, py, chr, pal, bg, fh, fv, spr )                         \
    OAM_ARGS[0] = (py) - 1;                                                         \
    OAM_ARGS[1] = ( (spr) << 1 ) | ( 0x01 & (chr) );                                \
    OAM_ARGS[2] = ( (fv) << 7 ) | ( (fh) << 6 ) | ( (bg) << 5 ) | ( 0x03 & (pal) ); \
    OAM_ARGS[3] = (px);                                                             \
    ppu_oam_sprite()

#define oam_sprite( px, py, chr, pal, spr ) oam_sprit_full( px, py, chr, pal, 0, 0, 0, spr )

void __fastcall__ ppu_oam_sprite();

#endif // PPU_H
