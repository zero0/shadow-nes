#ifndef PPU_H
#define PPU_H

#include "types.h"
#include "macros.h"

#define NAMETABLE_0             (uint8_t)0
#define NAMETABLE_1             (uint8_t)1
#define NAMETABLE_2             (uint8_t)2
#define NAMETABLE_3             (uint8_t)3

#define NAMETABLE_0_ADDR        (ptr_t)0x2000
#define NAMETABLE_1_ADDR        (ptr_t)0x2400
#define NAMETABLE_2_ADDR        (ptr_t)0x2800
#define NAMETABLE_3_ADDR        (ptr_t)0x2C00

#define PALETTE_BACKGROUND_0    (uint8_t)0
#define PALETTE_BACKGROUND_1    (uint8_t)1
#define PALETTE_BACKGROUND_2    (uint8_t)2
#define PALETTE_BACKGROUND_3    (uint8_t)3

#define PALETTE_SPRITE_0        (uint8_t)4
#define PALETTE_SPRITE_1        (uint8_t)5
#define PALETTE_SPRITE_2        (uint8_t)6
#define PALETTE_SPRITE_3        (uint8_t)7

#define SPRITE_0                (uint8_t)0
#define SPRITE_1                (uint8_t)1

#define NAMETABLE_ROWS          (uint8_t)30
#define NAMETABLE_COLS          (uint8_t)32

#define SCREEN_WIDTH_PIXELS     (uint8_t)( NAMETABLE_COLS * 8 )
#define SCREEN_HEIGH_PIXELS     (uint8_t)( NAMETABLE_ROWS * 8 )

#define SCREEN_WIDTH            (uint8_t)( NAMETABLE_COLS )
#define SCREEN_HEIGH            (uint8_t)( NAMETABLE_ROWS )

#define TILE_TO_PIXEL( x )      (uint8_t)( (x) * 8 )

#define TILE_TO_ADDRESS_ARGS(arg, base, x, y)       \
    (arg)[0] = (uint8_t)(base) | (uint8_t)( (y) );  \
    (arg)[1] = (uint8_t)( (x) << 4 )

#define TILE_INDEX_TO_ADDRESS_ARGS(arg, base, idx)  \
    (arg)[0] = (uint8_t)(base) | (uint8_t)( (idx) >> 4 );  \
    (arg)[1] = 0xFF & (uint8_t)( (idx) << 4 )

#define PTR_TO_ARGS(arg, ptr)                   \
    (arg)[0] = 0xFF & ((ptr) >> 8);             \
    (arg)[1] = 0xFF & (ptr)

#define ADDRESS_TO_ARGS(arg, ptr)               \
    (arg)[0] = 0xFF & (ptr_t)&(ptr);            \
    (arg)[1] = 0xFF & ((ptr_t)&(ptr) >> 8)

extern ptr_t PPU_DATA;

extern uint8_t PPU_ARGS[8];
#pragma zpsym("PPU_ARGS");

uint8_t __fastcall__ ppu_frame_index(void);

void __fastcall__ ppu_update(void);

void __fastcall__ ppu_off(void);

void __fastcall__ ppu_skip(void);

#define ppu_set_scroll( x, y )  \
    PPU_ARGS[0] = (x);          \
    PPU_ARGS[1] = (y);          \
    ppu_set_scroll_internal()

void __fastcall__ ppu_set_scroll_internal(void);

void __fastcall__ ppu_address_tile( uint8_t x, uint8_t y );

#define ppu_update_tile( px, py, t )    \
    PPU_ARGS[0] = (px);                 \
    PPU_ARGS[1] = (py);                 \
    PPU_ARGS[2] = (t);                  \
    ppu_update_tile_internal()

void __fastcall__ ppu_update_tile_internal(void);

//
//
//

#define ppu_begin_tile_batch( px, py )  \
    PPU_ARGS[0] = (px);                 \
    PPU_ARGS[1] = (py);                 \
    ppu_begin_tile_batch_internal()

void __fastcall__ ppu_begin_tile_batch_internal(void);

#define ppu_push_tile_batch( t )    \
    PPU_ARGS[0] = (t);              \
    ppu_push_tile_batch_internal()

void __fastcall__ ppu_push_tile_batch_internal(void);

#define ppu_repeat_tile_batch( t, c )   \
    PPU_ARGS[0] = (t);                  \
    PPU_ARGS[1] = (c);                  \
    ppu_repeat_tile_batch_internal()

void __fastcall__ ppu_repeat_tile_batch_internal(void);

#define ppu_end_tile_batch()        \
    ppu_end_tile_batch_internal()

void __fastcall__ ppu_end_tile_batch_internal(void);

//
//
//

void __fastcall__ ppu_update_byte( uint8_t x, uint8_t y, uint8_t b );

#define ppu_clear_nametable( ptr, c, a )            \
    PTR_TO_ARGS(PPU_ARGS, ptr);                     \
    PPU_ARGS[2] = (c);                              \
    PPU_ARGS[3] = (a);                              \
    ppu_clear_nametable_internal()

void __fastcall__ ppu_clear_nametable_internal(void);

void __fastcall__ ppu_clear_palette();

#define ppu_set_palette_background( c0 )            \
    PPU_ARGS[0] = (c0);                             \
    ppu_set_palette_background_internal()

void __fastcall__ ppu_set_palette_background_internal(void);

#define ppu_set_palette( p, c1, c2, c3 )            \
    PPU_ARGS[0] = (p);                              \
    PPU_ARGS[1] = (c1);                             \
    PPU_ARGS[2] = (c2);                             \
    PPU_ARGS[3] = (c3);                             \
    ppu_set_palette_internal()

void __fastcall__ ppu_set_palette_internal(void);

void __fastcall__ ppu_fill_nametable( ptr_t tableAddress, uint8_t tile );

void __fastcall__ ppu_fill_nametable_attr( ptr_t tableAddress );

void __fastcall__ ppu_fill_nametable_attr_only( ptr_t tableAddress, uint8_t attr );

//
//
//

#define ppu_add_oam_sprite_full( px, py, pal, bg, fh, fv, spr )                     \
    PPU_ARGS[0] = (py) - 1;                                                         \
    PPU_ARGS[1] = (spr);                                                            \
    PPU_ARGS[2] = ( (fv) << 7 ) | ( (fh) << 6 ) | ( (bg) << 5 ) | ( 0x03 & (pal) ); \
    PPU_ARGS[3] = (px);                                                             \
    ppu_oam_sprite()

#define ppu_add_oam_sprite( px, py, pal, spr ) ppu_add_oam_sprite_full( px, py, pal, 0, 0, 0, spr )

void __fastcall__ ppu_clear_oam();

void __fastcall__ ppu_oam_sprite();

//
//
//

#define ppu_upload_meta_sprite_chr_ram(ptr, base)           \
    ADDRESS_TO_ARGS(PPU_ARGS, ptr);                         \
    PPU_ARGS[2] = (base);                                   \
    ppu_upload_meta_sprite_chr_ram_internal()

void __fastcall__ ppu_upload_meta_sprite_chr_ram_internal(void);

#define ppu_add_meta_sprite( ptr, px, py, off )                                     \
    ADDRESS_TO_ARGS(PPU_ARGS, ptr);                                                 \
    PPU_ARGS[2] = (px);                                                             \
    PPU_ARGS[3] = (py) - 1;                                                         \
    PPU_ARGS[4] = (off);                                                            \
    ppu_add_meta_sprite_internal()

void __fastcall__ ppu_add_meta_sprite_internal(void);

//
//
//

#define ppu_upload_chr_ram( ptr, c, dst )           \
    ADDRESS_TO_ARGS(PPU_ARGS, ptr);                 \
    PPU_ARGS[2] = (c);                              \
    PPU_ARGS[3] = (dst);                            \
    ppu_upload_chr_ram_internal()

void __fastcall__ ppu_upload_chr_ram_internal(void);

#define ppu_begin_write_chr_ram(p, tx, ty)          \
    TILE_TO_ADDRESS_ARGS(PPU_ARGS, p, tx, ty);      \
    ppu_begin_write_chr_ram_internal()

#define ppu_begin_write_chr_ram_index(p, idx)       \
    TILE_INDEX_TO_ADDRESS_ARGS(PPU_ARGS, p, idx);   \
    ppu_begin_write_chr_ram_internal()

void __fastcall__ ppu_begin_write_chr_ram_internal(void);


#define ppu_write_chr_ram(c0, c1, c2, c3, c4, c5, c6, c7)   \
    __asm__("lda #%b", c0);             \
    __asm__("sta %v", PPU_DATA);        \
    __asm__("lda #%b", c1);             \
    __asm__("sta %v", PPU_DATA);        \
    __asm__("lda #%b", c2);             \
    __asm__("sta %v", PPU_DATA);        \
    __asm__("lda #%b", c3);             \
    __asm__("sta %v", PPU_DATA);        \
    __asm__("lda #%b", c4);             \
    __asm__("sta %v", PPU_DATA);        \
    __asm__("lda #%b", c5);             \
    __asm__("sta %v", PPU_DATA);        \
    __asm__("lda #%b", c6);             \
    __asm__("sta %v", PPU_DATA);        \
    __asm__("lda #%b", c7);             \
    __asm__("sta %v", PPU_DATA)

void __fastcall__ ppu_write_chr_ram_internal(void);

#define ppu_end_write_chr_ram()   \
    ppu_end_write_chr_ram_internal()

void __fastcall__ ppu_end_write_chr_ram_internal(void);

#endif // PPU_H
