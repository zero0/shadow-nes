#ifndef PPU_H
#define PPU_H

#include "types.h"
#include "macros.h"

#define NAMETABLE_0             (uint8_t)0
#define NAMETABLE_1             (uint8_t)1
#define NAMETABLE_2             (uint8_t)2
#define NAMETABLE_3             (uint8_t)3

#define PALETTE_BACKGROUND_0    (uint8_t)0
#define PALETTE_BACKGROUND_1    (uint8_t)1
#define PALETTE_BACKGROUND_2    (uint8_t)2
#define PALETTE_BACKGROUND_3    (uint8_t)3

#define SPRITE_0                (uint8_t)0
#define SPRITE_1                (uint8_t)1

#define PALETTE_SPRITE_0        (uint8_t)0
#define PALETTE_SPRITE_1        (uint8_t)1
#define PALETTE_SPRITE_2        (uint8_t)2
#define PALETTE_SPRITE_3        (uint8_t)3

#define NAMETABLE_ROWS          (uint8_t)30
#define NAMETABLE_COLS          (uint8_t)32

#define SCREEN_WIDTH            (uint8_t)( NAMETABLE_COLS * 8 )
#define SCREEN_HEIGH            (uint8_t)( NAMETABLE_ROWS * 8 )

#define make_scroll( x, y )     ( ( (uint16_t)(x) << 8 ) | (uint16_t)(y) )

extern uint8_t OAM_ARGS[4];
#pragma zpsym("OAM_ARGS");

uint8_t __fastcall__ ppu_frame_index(void);

void __fastcall__ ppu_update(void);

void __fastcall__ ppu_off(void);

void __fastcall__ ppu_skip(void);

void __fastcall__ ppu_set_scroll( uint16_t xy );

void __fastcall__ ppu_address_tile( uint8_t x, uint8_t y );

void __fastcall__ ppu_update_tile( uint8_t x, uint8_t y, uint8_t t );

void __fastcall__ ppu_update_byte( uint8_t x, uint8_t y, uint8_t b );

void __fastcall__ ppu_clear_nametable( ptr_t tableAddress );

void __fastcall__ ppu_clear_palette();

void __fastcall__ ppu_fill_nametable( ptr_t tableAddress, uint8_t tile );

void __fastcall__ ppu_fill_nametable_attr( ptr_t tableAddress );

void __fastcall__ ppu_fill_nametable_attr_only( ptr_t tableAddress, uint8_t attr );

#define oam_sprite_full( px, py, chr, pal, bg, fh, fv, spr )                        \
    OAM_ARGS[0] = (py) - 1;                                                         \
    OAM_ARGS[1] = ( (spr) );                                                        \
    OAM_ARGS[2] = ( (fv) << 7 ) | ( (fh) << 6 ) | ( (bg) << 5 ) | ( 0x03 & (pal) ); \
    OAM_ARGS[3] = (px);                                                             \
    ppu_oam_sprite()

#define oam_sprite( px, py, chr, pal, spr ) oam_sprite_full( px, py, chr, pal, 0, 0, 0, spr )

void __fastcall__ ppu_oam_clear();

void __fastcall__ ppu_oam_sprite();

#endif // PPU_H
