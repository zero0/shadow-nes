
;
; PPU
;

.importzp TEMP
.importzp sp

.import apu_update_from_nmi

PPU_CTRL            =$2000
PPU_MASK            =$2001
PPU_STATUS          =$2002
PPU_OAM_ADDR        =$2003
PPU_OAM_DATA        =$2004
PPU_SCROLL          =$2005
PPU_ADDR            =$2006
PPU_DATA            =$2007
PPU_OAM_DMA         =$4014

.export _PPU_ADDR           =PPU_ADDR
.export _PPU_DATA           =PPU_DATA

; nametable addresses
.define NAMETABLE_ROWS                  #30
.define NAMETABLE_COLS                  #32
.define NAMETABLE_BYTE_COUNT            NAMETABLE_ROWS * NAMETABLE_COLS
.define NAMETABLE_ATTR_BYTE_COUNT       #64

.define NAMETABLE_A_BASE_ADDR           $2000
.define NAMETABLE_A_ATTR_ADDR           NAMETABLE_A_BASE_ADDR + NAMETABLE_BYTE_COUNT

; palette addresses
.define PALETTE_BASE_ADDR               $3f00
.define PALETTE_BYTE_COUNT              #32

.define PALETTE_BACKGROUND_BASE_ADDR    PALETTE_BASE_ADDR
.define PALETTE_BACKGROUND_0_ADDR       PALETTE_BACKGROUND_BASE_ADDR + $0
.define PALETTE_BACKGROUND_1_ADDR       PALETTE_BACKGROUND_BASE_ADDR + $4
.define PALETTE_BACKGROUND_2_ADDR       PALETTE_BACKGROUND_BASE_ADDR + $8
.define PALETTE_BACKGROUND_3_ADDR       PALETTE_BACKGROUND_BASE_ADDR + $C
.define PALETTE_BACKGROUND_BYTE_COUNT   #16

.define PALETTE_SPRITE_BASE_ADDR        PALETTE_BASE_ADDR + $10
.define PALETTE_SPRITE_0_ADDR           PALETTE_SPRITE_BASE_ADDR + $0
.define PALETTE_SPRITE_1_ADDR           PALETTE_SPRITE_BASE_ADDR + $4
.define PALETTE_SPRITE_2_ADDR           PALETTE_SPRITE_BASE_ADDR + $8
.define PALETTE_SPRITE_3_ADDR           PALETTE_SPRITE_BASE_ADDR + $C
.define PALETTE_SPRITE_BYTE_COUNT       #16

NAMETABLE_A         =$2000
NAMETABLE_B         =$2400
NAMETABLE_C         =$2800
NAMETABLE_D         =$2C00

NAMETABLE_A_ATTR    =$23C0
NAMETABLE_B_ATTR    =$27C0
NAMETABLE_C_ATTR    =$2BC0
NAMETABLE_D_ATTR    =$2FC0

_NAMETABLE_A         =NAMETABLE_A
_NAMETABLE_B         =NAMETABLE_B
_NAMETABLE_C         =NAMETABLE_C
_NAMETABLE_D         =NAMETABLE_D
_NAMETABLE_A_ATTR    =NAMETABLE_A_ATTR
_NAMETABLE_B_ATTR    =NAMETABLE_B_ATTR
_NAMETABLE_C_ATTR    =NAMETABLE_C_ATTR
_NAMETABLE_D_ATTR    =NAMETABLE_D_ATTR

.export _NAMETABLE_A
.export _NAMETABLE_A_ATTR
.export _NAMETABLE_B
.export _NAMETABLE_B_ATTR
.export _NAMETABLE_C
.export _NAMETABLE_C_ATTR
.export _NAMETABLE_D
.export _NAMETABLE_D_ATTR

.macro popa
    lda (sp), y
    iny
.endmacro

; push all registers to the stack
.macro push_reg
    ; push a
    pha
    ; push x
    txa
    pha
    ; push y
    tya
    pha
.endmacro

; pop all registers from the stack
.macro pop_reg
    ; pop y
    pla
    tay
    ; pop x
    pla
    tax
    ; pop a
    pla
.endmacro

; PPU_CTRL Modes
.define PPU_CTRL_BASE_NAMETABLE_ADDR_2000           $00 ; %00000000
.define PPU_CTRL_BASE_NAMETABLE_ADDR_2400           $01 ; %00000001
.define PPU_CTRL_BASE_NAMETABLE_ADDR_2800           $02 ; %00000010
.define PPU_CTRL_BASE_NAMETABLE_ADDR_2C00           $03 ; %00000011
.define PPU_CTRL_VRAM_ADDR_INC_1_ACROSS             $00 ; %00000000
.define PPU_CTRL_VRAM_ADDR_INC_32_DOWN              $04 ; %00000100
.define PPU_CTRL_SPRITE_PATTERN_TABLE_ADDR_0000     $00 ; %00000000
.define PPU_CTRL_SPRITE_PATTERN_TABLE_ADDR_1000     $08 ; %00001000
.define PPU_CTRL_BACKGROUND_PATTERN_TABLE_ADDR_0000 $00 ; %00000000
.define PPU_CTRL_BACKGROUND_PATTERN_TABLE_ADDR_1000 $10 ; %00010000
.define PPU_CTRL_SPRITE_SIZE_8x8                    $00 ; %00000000
.define PPU_CTRL_SPRITE_SIZE_8x16                   $20 ; %00100000
.define PPU_CTRL_READ_EXT                           $00 ; %00000000
.define PPU_CTRL_WRITE_EXT                          $40 ; %01000000
.define PPU_CTRL_GENERATE_NMI_OFF                   $00 ; %00000000
.define PPU_CTRL_GENERATE_NMI_ON                    $80 ; %10000000

; PPU_MASK Modes
.define PPU_MASK_GRAYSCALE_OFF                      $00 ; %00000000
.define PPU_MASK_GRAYSCALE_ON                       $01 ; %00000001
.define PPU_MASK_BACKGROUND_LEFTMOST_8x8_HIDE       $00 ; %00000000
.define PPU_MASK_BACKGROUND_LEFTMOST_8x8_SHOW       $02 ; %00000010
.define PPU_MASK_SPRITE_LEFTMOST_8x8_HIDE           $00 ; %00000000
.define PPU_MASK_SPRITE_LEFTMOST_8x8_SHOW           $04 ; %00000100
.define PPU_MASK_BACKGROUND_HIDE                    $00 ; %00000000
.define PPU_MASK_BACKGROUND_SHOW                    $08 ; %00001000
.define PPU_MASK_SPRITE_HIDE                        $00 ; %00000000
.define PPU_MASK_SPRITE_SHOW                        $10 ; %00010000
.define PPU_MASK_EMPHASIZE_RED_OFF                  $00 ; %00000000
.define PPU_MASK_EMPHASIZE_RED_ON                   $20 ; %00100000
.define PPU_MASK_EMPHASIZE_GREEN_OFF                $00 ; %00000000
.define PPU_MASK_EMPHASIZE_GREEN_ON                 $40 ; %01000000
.define PPU_MASK_EMPHASIZE_BLUE_OFF                 $00 ; %00000000
.define PPU_MASK_EMPHASIZE_BLUE_ON                  $80 ; %10000000
.define PPU_MASK_EMPHASIZE_ALL_ON                   PPU_MASK_EMPHASIZE_RED_ON | PPU_MASK_EMPHASIZE_GREEN_ON | PPU_MASK_EMPHASIZE_BLUE_ON

.define NMI_NAMETABLE_UPDATE_COUNT_MAX              #32

; Dirty upload mask
.define DIRTY_UPLOAD_MASK_OAM                       $80 ;
.define DIRTY_UPLOAD_MASK_OAM_CLEAR                 $7F ;

;
;
;

.segment "ZEROPAGE"

    NMI_LOCK:                   .res 1 ;
    NMI_COUNT:                  .res 1 ;
    NMI_READY:                  .res 1 ;
    NMI_NAMETABLE_UPDATE_COUNT: .res 1 ;
    PPU_CTRL_BUFFER:            .res 1 ;
    PPU_MASK_BUFFER:            .res 1 ;
    NAMETABLE_UPDATE_LEN:       .res 1 ;
    NAMETABLE_UPDATE_POS:       .res 1 ;
    PALETTE_UPDATE_LEN:         .res 1 ;
    OAM_UPDATE_LEN:             .res 1 ;
    OAM_SPRITE_LEN:             .res 1 ;
    OAM_SPRITE_FREE_LIST_LEN:   .res 1 ;
    DIRTY_UPLOAD_MASK:          .res 1 ;
    SCROLL_X:                   .res 1 ;
    SCROLL_Y:                   .res 1 ;
    TILE_BATCH_INDEX:           .res 1 ;
    META_SPRITE_LEN:            .res 1 ;
    META_SPRITE_ATTR:           .res 1 ;
    META_SPRITE_TILE:           .res 1 ;
    META_SPRITE_ADDR:           .res 2 ;
    CHR_UPLOAD_ADDR:            .res 2 ;
    PALETTE_TINT_OAM_PTR:       .res 2 ;
    PALETTE_TINT_BACKGROUND_PTR:.res 2 ;
    _PPU_ARGS:                  .res 8 ;

.export _PPU_ARGS

.segment "BSS"
    NAMETABLE_UPDATE:       .res 256 ;
    PALETTE_UPDATE:         .res 32 ;
    OAM_SPRITE_FREE_LIST:   .res 64 ;

.segment "OAM"
    OAM_UPDATE:             .res 256 ;

;
; ppu functions
;

.export _ppu_frame_index = ppu_frame_index

.export ppu_init
.export ppu_wait_vblank, _ppu_wait_vblank
.export ppu_enable_default

.export ppu_clear_nametable
.export ppu_clear_palette
.export ppu_clear_chr_ram
.export ppu_clear_oam

.export _ppu_set_scroll_internal

.export _ppu_update = ppu_update
.export _ppu_off, _ppu_on
.export _ppu_skip

.export _ppu_set_address_tile_internal
.export _ppu_update_tile_internal

.export _ppu_begin_tile_batch_internal
.export _ppu_push_tile_batch_internal
.export _ppu_repeat_tile_batch_internal
.export _ppu_end_tile_batch_internal

.export _ppu_update_byte
.export _ppu_clear_nametable_internal
.export _ppu_fill_nametable_attr
.export _ppu_set_nametable_attr_internal

.export _ppu_clear_palette
.export _ppu_set_palette_internal
.export _ppu_set_palette_background_internal

.export _ppu_clear_oam = ppu_clear_oam
.export _ppu_clear_sprites = ppu_clear_sprites
.export _ppu_request_sprite = ppu_request_sprite
.export _ppu_release_sprite = ppu_release_sprite
.export _ppu_update_sprite = ppu_update_sprite
.export _ppu_oam_sprite

.export _ppu_add_meta_sprite_internal
.export _ppu_upload_meta_sprite_chr_ram_internal

.export _ppu_clear_chr_ram_internal
.export _ppu_upload_chr_ram_internal
.export _ppu_begin_write_chr_ram_internal
.export _ppu_write_chr_ram_internal
.export _ppu_end_write_chr_ram_internal

.export _ppu_tint_palellete_oam_internal
.export _ppu_tint_palellete_background_internal
.export _ppu_tint_palelletes_internal
.export _ppu_tint_reset_internal

.segment "LOWCODE"

; ppu_frame_index: returns in A the frame count [0..255]
.proc ppu_frame_index

    lda NMI_COUNT
    rts
.endproc

; ppu_init: initialize PPU
.proc ppu_init

    lda #0

    ; clear internal buffers
    sta SCROLL_X
    sta SCROLL_Y

    sta PPU_CTRL_BUFFER
    sta PPU_MASK_BUFFER

    ; clear PPU values
    sta PPU_SCROLL
    sta PPU_SCROLL

    sta PPU_CTRL    ; disable NMI
    sta PPU_MASK    ; disable rendering

    ; clear palette tint pointers
    sta PALETTE_TINT_BACKGROUND_PTR+0
    sta PALETTE_TINT_BACKGROUND_PTR+1

    sta PALETTE_TINT_OAM_PTR+0
    sta PALETTE_TINT_OAM_PTR+1

    rts
.endproc

; ppu_wait_vblank: waits for the next vblank
ppu_wait_vblank:

.proc _ppu_wait_vblank
    bit PPU_STATUS
    :
        bit PPU_STATUS
        bpl :-
    rts
.endproc

ppu_enable_default:
    lda SCROLL_X
    sta PPU_SCROLL

    lda SCROLL_Y
    sta PPU_SCROLL

    lda #(PPU_CTRL_GENERATE_NMI_ON | PPU_CTRL_SPRITE_PATTERN_TABLE_ADDR_1000 | PPU_CTRL_BASE_NAMETABLE_ADDR_2000 | PPU_CTRL_BACKGROUND_PATTERN_TABLE_ADDR_0000)
    sta PPU_CTRL
    sta PPU_CTRL_BUFFER

    lda #(PPU_MASK_BACKGROUND_SHOW | PPU_MASK_SPRITE_SHOW | PPU_MASK_BACKGROUND_LEFTMOST_8x8_SHOW | PPU_MASK_SPRITE_LEFTMOST_8x8_SHOW)
    sta PPU_MASK
    sta PPU_MASK_BUFFER
    rts

; ppu_update: waits until next NMI, turns rendering on (if not already), uploads OAM, palette, and nametable update to PPU
.proc ppu_update

    ; check if there are oam updates
    lda OAM_SPRITE_LEN
    beq @no_oam_update

    ; convert index -> offset
    asl
    asl

    ; transfer index -> offset
    tax

    ; clear remaining OAM data
    lda #$FF
    :
        sta OAM_UPDATE, x
        inx
        bne :-

@no_oam_update:

    ; wait for NMI to be ready
    lda #1
    sta NMI_READY
    :
        lda NMI_READY
        bne :-
    rts
.endproc

; ppu_skip: waits until next NMI, does not update PPU
.proc _ppu_skip

    lda NMI_COUNT
    :
        cmp NMI_COUNT
        beq :-

    rts
.endproc

; ppu_off: waits until next NMI, turns rendering off (now safe to write PPU directly via $2007)
.proc _ppu_off

    lda #2
    sta NMI_READY
    :
        lda NMI_READY
        bne :-

    rts
.endproc

; ppu_on: waits until next NMI, turns rendering back on
.proc _ppu_on

    ; upload palette updates
    jsr _ppu_upload_palette

    ; upload nametable updates
    jsr _ppu_upload_nametable

    lda #3
    sta NMI_READY
    :
        lda NMI_READY
        bne :-

    rts
.endproc

; ppu_set_scroll: sets the (x,y) scroll position from PPU_ARGS 0 & 1
.proc _ppu_set_scroll_internal

    lda _PPU_ARGS+0
    sta SCROLL_X
    lda _PPU_ARGS+1
    sta SCROLL_Y

    rts
.endproc

; ppu_set_address_tile: use with rendering off, sets memory address to tile at X/Y, ready for a $2007 (PPU_DATA) write
;   Y =  0- 31 nametable $2000
;   Y = 32- 63 nametable $2400
;   Y = 64- 95 nametable $2800
;   Y = 96-127 nametable $2C00
.proc _ppu_set_address_tile_internal
    ; reset latch
    bit PPU_STATUS

    ; load Y
    lda _PPU_ARGS+1

    ; shift Y >> 3
    lsr
    lsr
    lsr

    ; high bits of ( Y >> 3 ) | $20
    ora #$20
    sta PPU_ADDR

    ; shift Y << 5
    lda _PPU_ARGS+1
    asl
    asl
    asl
    asl
    asl

    ; ( Y << 5 ) | X
    ora _PPU_ARGS+0

    ; low bits of ( Y << 5 ) | X
    sta PPU_ADDR
    rts
.endproc

; ppu_update_tile: can be used with rendering on, sets the tile at X/Y to tile A next time you call _ppu_update (see ppu_set_address_tile)
_ppu_update_tile_internal:

    ; load Y
    lda _PPU_ARGS+1

    ; shift Y >> 3
    lsr
    lsr
    lsr

    ; load length in X
    ldx NAMETABLE_UPDATE_LEN

    ; high bits of (Y >> 3) | $20 (nametable base)
    ora #$20
    sta NAMETABLE_UPDATE, x
    inx

    ; load Y
    lda _PPU_ARGS+1
    ; shift Y << 5
    asl
    asl
    asl
    asl
    asl

    ; ( Y << 5 ) | X
    ; recover X value
    ora _PPU_ARGS+0
    sta NAMETABLE_UPDATE, x
    inx

    ; tile count
    lda #1
    sta NAMETABLE_UPDATE, x
    inx

    ; recover A value
    lda _PPU_ARGS+2
    sta NAMETABLE_UPDATE, x
    inx

    ; store new length
    stx NAMETABLE_UPDATE_LEN

    rts

; ppu_update_byte: like ppu_update_tile, but X/Y makes the high/low bytes of the PPU address to write
;    this may be useful for updating attribute tiles
_ppu_update_byte:

    ; load length
    ldx NAMETABLE_UPDATE_LEN

    ; store high byte
    lda _PPU_ARGS+0
    sta NAMETABLE_UPDATE, x
    inx

    ; store low byte
    lda _PPU_ARGS+1
    sta NAMETABLE_UPDATE, x
    inx

    ; store byte
    lda _PPU_ARGS+2
    sta NAMETABLE_UPDATE, x
    inx

    ; store new length
    stx NAMETABLE_UPDATE_LEN

    rts

;
_ppu_begin_tile_batch_internal:

    ; load Y
    lda _PPU_ARGS+1

    ; shift Y >> 3
    lsr
    lsr
    lsr

    ; load length in X
    ldx NAMETABLE_UPDATE_LEN

    ; high bits of (Y >> 3) | $20 (nametable base)
    ora #$20
    sta NAMETABLE_UPDATE, x
    inx

    ; load Y
    lda _PPU_ARGS+1
    ; shift Y << 5
    asl
    asl
    asl
    asl
    asl

    ; ( Y << 5 ) | X
    ; recover X value
    ora _PPU_ARGS+0
    sta NAMETABLE_UPDATE, x
    inx

    ; tile count
    lda #0
    sta NAMETABLE_UPDATE, x

    ; store tile batch index to update
    stx TILE_BATCH_INDEX
    inx

    ; store new length
    stx NAMETABLE_UPDATE_LEN

    rts

;
.proc _ppu_push_tile_batch_internal

    ; load length
    ldx NAMETABLE_UPDATE_LEN

    ; store tile
    lda _PPU_ARGS+0
    sta NAMETABLE_UPDATE, x
    inx

    ; store new lenght
    stx NAMETABLE_UPDATE_LEN

    ; increment batch count
    ldx TILE_BATCH_INDEX
    inc NAMETABLE_UPDATE, x

    rts
.endproc

;
.proc _ppu_repeat_tile_batch_internal

    ; load length
    ldx NAMETABLE_UPDATE_LEN

    ; store tile
    lda _PPU_ARGS+0
    sta NAMETABLE_UPDATE, x
    inx

    ; store new lenght
    stx NAMETABLE_UPDATE_LEN

    ; increment batch count by specified number
    ldx TILE_BATCH_INDEX
    lda _PPU_ARGS+1
    clc
    adc NAMETABLE_UPDATE, x
    ora #$80
    sta NAMETABLE_UPDATE, x

    rts
.endproc

;
_ppu_end_tile_batch_internal:

    ; clear tile bach index
    lda #0
    sta TILE_BATCH_INDEX

    rts

; clears specific nametable at PPU_ARGS[0..1] to PPU_ARGS[2] with attr PPU_ARS[3]
ppu_clear_nametable:

.proc _ppu_clear_nametable_internal

    ;; disable rendering
    ;lda #0
    ;sta PPU_MASK

    ; reset latch
    bit PPU_STATUS

    ; store address
    lda _PPU_ARGS+0
    sta PPU_ADDR
    lda _PPU_ARGS+1
    sta PPU_ADDR

    ; clear valeu
    lda _PPU_ARGS+2

    ldy NAMETABLE_ROWS ; 30 rows
    :
        ldx NAMETABLE_COLS ; 32 columns
        :
            sta PPU_DATA
            dex
            bne :-
        dey
        bne :--

    ; attribue clear value
    lda _PPU_ARGS+3

    ; empty attribute table
    ldx #64 ; 64 bytes
    :
        sta PPU_DATA
        dex
        bne :-

    ;; reset ppu mask
    ;lda PPU_MASK_BUFFER
    ;sta PPU_MASK

    rts
.endproc

; ppu_clear_palette: clear all palettes
_ppu_clear_palette:

.proc ppu_clear_palette
    lda #0
    ldx PALETTE_BYTE_COUNT
    :
        sta PALETTE_UPDATE, x
        dex
        bne :-

    ; mark palette as dirty
    inc PALETTE_UPDATE_LEN

    rts
.endproc

; Updates
_ppu_set_palette_internal:

    ; convert palette index to memory location (mul 4)
    lda _PPU_ARGS+0;
    asl
    asl

    ; transfer to A -> X
    tax

    ; store colors into update buffer
    lda _PPU_ARGS+1
    sta PALETTE_UPDATE+1, x
    lda _PPU_ARGS+2
    sta PALETTE_UPDATE+2, x
    lda _PPU_ARGS+3
    sta PALETTE_UPDATE+3, x

    ; mark palette as dirty
    inc PALETTE_UPDATE_LEN

    rts

; Set the background for all palettes
_ppu_set_palette_background_internal:

.proc ppu_set_palette_background

    ; clear color
    lda _PPU_ARGS+0;

    ; nametable backgroupds
    sta PALETTE_UPDATE+0
    sta PALETTE_UPDATE+4
    sta PALETTE_UPDATE+8
    sta PALETTE_UPDATE+12

    ; sprite backgrouds
    sta PALETTE_UPDATE+16
    sta PALETTE_UPDATE+20
    sta PALETTE_UPDATE+24
    sta PALETTE_UPDATE+28

    ; mark palette as dirty
    inc PALETTE_UPDATE_LEN

    rts
.endproc

; fill nametable at $ARGS+0/ARGS+1 with table ARGS+2 and attr ARGS+3
_ppu_fill_nametable_attr:

.proc ppu_fill_nametable_attr

    ; reset latch
    bit PPU_STATUS

    ; store
    stx PPU_ADDR
    sta PPU_ADDR

    lda #0

    ; empty nametable
    ldy #30 ; 30 rows
    :
        ldx #32 ; 32 columns
        :
            sta PPU_DATA
            dex
            bne :-
        dey
        bne :--

    ; set all attributes to 0
    ldx #64 ; 64 bytes
    :
        sta PPU_DATA
        dex
        bne :-

    rts
.endproc

; fill attr byte in range
.proc _ppu_set_nametable_attr_internal

    ; load length
    ldx NAMETABLE_UPDATE_LEN

    ; store high byte
    lda _PPU_ARGS+0
    sta NAMETABLE_UPDATE, x
    inx

    ; store low byte
    lda _PPU_ARGS+1
    sta NAMETABLE_UPDATE, x
    inx

    ; store repeat count
    lda _PPU_ARGS+3
    ora #$80
    sta NAMETABLE_UPDATE, x
    inx

    ; store byte
    lda _PPU_ARGS+2
    sta NAMETABLE_UPDATE, x
    inx

    ; store new length
    stx NAMETABLE_UPDATE_LEN

    rts
.endproc

;
;
;

; clear oam buffer
.proc ppu_clear_oam

    ; clear oam buffer
    lda #$FF
    ldx #0
    :
        sta OAM_UPDATE, x
        inx
        bne :-

    rts
.endproc

; clear sprite lists
.proc ppu_clear_sprites

    ; reset lengths
    lda #0
    sta OAM_SPRITE_LEN
    sta OAM_SPRITE_FREE_LIST_LEN

    rts
.endproc

; request a sprite
.proc ppu_request_sprite

    ; load sprite free list length
    ldx OAM_SPRITE_FREE_LIST_LEN

    ; if there are sprites in free list, remove it from the list and return
    beq :+

        ; decrement length
        dex
        stx OAM_SPRITE_FREE_LIST_LEN

        ; load index from free list
        lda OAM_SPRITE_FREE_LIST, x

        rts
        :

    ; load sprite length
    ldx OAM_SPRITE_LEN

    ; store value to A
    txa

    ; increment and store sprite length
    inx
    stx OAM_SPRITE_LEN

    rts
.endproc

; release sprite A back to pool
.proc ppu_release_sprite

    ; load sprite free list length
    ldx OAM_SPRITE_FREE_LIST_LEN

    ; store sprite in free list
    sta OAM_SPRITE_FREE_LIST, x

    ; increment free list length
    inx
    stx OAM_SPRITE_FREE_LIST_LEN

    ; convert index -> offset
    asl
    asl

    ; transfer index -> offset
    tax

    ; load clear value
    lda #$FF

    ; store cleared oam updates
    sta OAM_UPDATE, x
    inx

    sta OAM_UPDATE, x
    inx

    sta OAM_UPDATE, x
    inx

    sta OAM_UPDATE, x

    ; mark dirty
    lda DIRTY_UPLOAD_MASK
    ora #(DIRTY_UPLOAD_MASK_OAM)
    sta DIRTY_UPLOAD_MASK

    rts
.endproc

; update a sprite
.proc ppu_update_sprite

    ; load sprite index (index -> offset)
    lda _PPU_ARGS+0
    asl
    asl

    ; transfer index -> offset
    tax

    ; store oam updates
    lda _PPU_ARGS+1
    sta OAM_UPDATE, x
    inx

    lda _PPU_ARGS+2
    sta OAM_UPDATE, x
    inx

    lda _PPU_ARGS+3
    sta OAM_UPDATE, x
    inx

    lda _PPU_ARGS+4
    sta OAM_UPDATE, x

    ; mark dirty
    lda DIRTY_UPLOAD_MASK
    ora #(DIRTY_UPLOAD_MASK_OAM)
    sta DIRTY_UPLOAD_MASK

    rts
.endproc

; ppu_oam_sprite: add a sprite from _OAM_ARGS to OAM_UPDATE
.proc _ppu_oam_sprite

    ; load OAM buffer length
    ldx OAM_UPDATE_LEN

    ; store OAM updates
    lda _PPU_ARGS+0
    sta OAM_UPDATE, x
    inx

    lda _PPU_ARGS+1
    sta OAM_UPDATE, x
    inx

    lda _PPU_ARGS+2
    sta OAM_UPDATE, x
    inx

    lda _PPU_ARGS+3
    sta OAM_UPDATE, x
    inx

    ; store updated length
    stx OAM_UPDATE_LEN

    rts
.endproc

; given a meta-sprite, upload it's sprites to CHR RAM
.proc _ppu_upload_meta_sprite_chr_ram_internal

    ; NOTE: needs to be reversed as offset expects [low, high] in bytes [0, 1]
    ; store metasprite address
    lda _PPU_ARGS+0
    sta META_SPRITE_ADDR+1
    lda _PPU_ARGS+1
    sta META_SPRITE_ADDR+0

    ; load chr upload address
    ldy #0
    lda (META_SPRITE_ADDR), y
    sta CHR_UPLOAD_ADDR+0
    iny
    lda (META_SPRITE_ADDR), y
    sta CHR_UPLOAD_ADDR+1
    iny

    ; load count
    lda (META_SPRITE_ADDR), y
    sta META_SPRITE_LEN
    iny

    ;; disable rendering
    ;lda PPU_MASK_BUFFER
    ;and %11101111
    ;sta PPU_MASK

    ; set base offset
    lda _PPU_ARGS+2
    sta PPU_ADDR
    lda #0
    sta PPU_ADDR

    @loop_each_tile:
        ;; TODO: actually use tile index, for now assume 0..n
        ; skip attribute
        iny

        ; load tile index
        lda (META_SPRITE_ADDR), y
        iny

        ; mask out palette
        and #$3F

        ; TODO: do something with index in A?

        ; store Y on the stack
        tya
        pha

        ;  load 16 bytes
        ldy #0

        .repeat 16
        lda (CHR_UPLOAD_ADDR), y
        sta PPU_DATA
        iny
        .endrepeat

        ; increment address by 16
        lda CHR_UPLOAD_ADDR+0
        ldy CHR_UPLOAD_ADDR+1

        clc
        adc #$10
        bcc :+
            iny
            :
        sta CHR_UPLOAD_ADDR+0
        sty CHR_UPLOAD_ADDR+1

        ; restore Y
        pla
        tay

        ; loop for each tile
        cpy META_SPRITE_LEN
        bne @loop_each_tile

    ;; reenable rendering
    ;lda PPU_MASK_BUFFER
    ;sta PPU_MASK

    rts
.endproc

;
.proc _ppu_add_meta_sprite_internal

    ; load OAM buffer length
    ldx OAM_UPDATE_LEN

    ; NOTE: needs to be reversed as offset expects [low, high] in bytes [0, 1]
    ; store metasprite address
    lda _PPU_ARGS+0
    sta META_SPRITE_ADDR+1
    lda _PPU_ARGS+1
    sta META_SPRITE_ADDR+0

    ; load metasprite length
    ldy #2
    lda (META_SPRITE_ADDR), y
    sta META_SPRITE_LEN
    iny

    :
        ; load metasprite attributes and tile
        lda (META_SPRITE_ADDR), y
        sta META_SPRITE_ATTR
        iny

        lda (META_SPRITE_ADDR), y
        sta META_SPRITE_TILE
        iny

        ; store OAM updates
        ; y
        ; get Y tile offset from metasprite attributes
        lda META_SPRITE_ATTR
        and #%00000111
        ; go from tiles to pixels (mul 8)
        asl
        asl
        asl

        clc
        adc _PPU_ARGS+3
        sta OAM_UPDATE, x
        inx

        ; store tile
        lda META_SPRITE_TILE
        and #$3F

        ; add offset
        clc
        adc _PPU_ARGS+4
        sta OAM_UPDATE, x
        inx

        ; attr - build from meta sprite attr and tile high bits
        lda META_SPRITE_ATTR
        and #%11000000

        ; store in TEMP
        sta TEMP

        ; load tile to get palette
        lda META_SPRITE_TILE

        ; rotate top two bits to bottom two
        rol
        rol
        rol

        ; mask palette bits
        and #$03

        ; OR with other attributes
        ora TEMP

        ; store attr
        sta OAM_UPDATE, x
        inx

        ; x
        ; get X tile offset from metasprite attributes (X is already shifted)
        lda META_SPRITE_ATTR
        and #%00111000

        adc _PPU_ARGS+2
        sta OAM_UPDATE, x
        inx

        ; check length
        cpy META_SPRITE_LEN
        bcc :-

    ; store updated length
    stx OAM_UPDATE_LEN

    rts
.endproc

; Clear all of the CHR RAM on page PPU_ARGS[0]
ppu_clear_chr_ram:

.proc _ppu_clear_chr_ram_internal

    ;; disable rendering
    ;lda #0
    ;sta PPU_MASK

    ; load starting address
    lda _PPU_ARGS+0
    sta PPU_ADDR
    lda #0
    sta PPU_ADDR

    ; clear all pages
    ldx #16

@loop:
    ; write 0
    sta PPU_DATA

    ; inc, loop until wrapped back to 0
    iny
    bne @loop

    ; repeate until complete
    dex
    bne @loop

    ;; reset mask
    ;lda PPU_MASK_BUFFER
    ;sta PPU_MASK

    rts
.endproc

; Upload a row of tiles into CHR RAM
.proc _ppu_upload_chr_ram_internal
    ;_PPU_ARGS[0] ; src address
    ;_PPU_ARGS[1]
    ;_PPU_ARGS[2] ; dst address
    ;_PPU_ARGS[3]
    ;_PPU_ARGS[4] ; tile count

    ; NOTE: needs to be reversed as offset expects [low, high] in bytes [0, 1]
    ; store address
    lda _PPU_ARGS+0 ;
    sta CHR_UPLOAD_ADDR+1
    lda _PPU_ARGS+1 ;
    sta CHR_UPLOAD_ADDR+0

    ;; disable rendering
    ;lda #0
    ;sta PPU_MASK

    ; reset latch
    bit PPU_STATUS

    ; load starting address
    lda _PPU_ARGS+2
    sta PPU_ADDR
    lda _PPU_ARGS+3
    sta PPU_ADDR

    ; number of tiles to load
    ldx _PPU_ARGS+4

    ldy #0

@loop:
    ; load 16 bytes from address
    .repeat 16
    lda (CHR_UPLOAD_ADDR), y
    sta PPU_DATA
    iny
    .endrepeat

    ; increment page when read 256 bytes
    bne :+
        inc CHR_UPLOAD_ADDR+1
        :

    ; repeat until complete
    dex
    bne @loop

    ;; reset mask
    ;lda PPU_MASK_BUFFER
    ;sta PPU_MASK

    rts
.endproc

;
.proc _ppu_begin_write_chr_ram_internal

    ;; disable rendering
    ;lda #0
    ;sta PPU_MASK

    ; store address
    lda _PPU_ARGS+0
    sta PPU_ADDR
    lda _PPU_ARGS+1
    sta PPU_ADDR

    rts
.endproc

;
.proc _ppu_write_chr_ram_internal

    rts
.endproc

;
.proc _ppu_end_write_chr_ram_internal

    ;; reenable rendering
    ;lda PPU_MASK_BUFFER
    ;sta PPU_MASK

    rts
.endproc

; ppu_upload_palette: uploads palette while PPU is off
.proc _ppu_upload_palette

    ; perform palette update if needed
    lda PALETTE_UPDATE_LEN
    beq @end

    ; reset latch
    bit PPU_STATUS

    ; set PPU palette address
    lda #(>PALETTE_BASE_ADDR)
    sta PPU_ADDR
    lda #(<PALETTE_BASE_ADDR)
    sta PPU_ADDR

    ldx #0
    :
        lda PALETTE_UPDATE, x
        sta PPU_DATA
        inx

        ; fill all palette data
        cpx PALETTE_BYTE_COUNT
        bcc :-

    ; clear palette update
    lda #0
    sta PALETTE_UPDATE_LEN

@end:
    rts
.endproc

; _ppu_upload_nametable: uploads nametable updates while PPU is off
.proc _ppu_upload_nametable

    ; nametable update
    ldx NAMETABLE_UPDATE_POS
    cpx NAMETABLE_UPDATE_LEN
    beq @end

    ;; count nametable updates
    ;lda #0
    ;sta NMI_NAMETABLE_UPDATE_COUNT

    ; reset latch
    bit PPU_STATUS

@update_nametable_loop:
    ; high byte address
    lda NAMETABLE_UPDATE, x
    sta PPU_ADDR
    inx

    ; low byte address
    lda NAMETABLE_UPDATE, x
    sta PPU_ADDR
    inx

    ; tile count (bit7 0 = individualt, 1 = repeat)
    lda NAMETABLE_UPDATE, x
    bpl @update_nametable_individual

@update_nametable_repeat:

    ; load tile count
    inx

    ; mask out bit7 and move to Y
    and #$7F
    tay

    ; tile to repeat
    lda NAMETABLE_UPDATE, x
    inx

    ; write tile to PPU_DATA Y times
    :
        ; add repeat count to nametable count
        ;inc NMI_NAMETABLE_UPDATE_COUNT

        sta PPU_DATA
        dey
        bne :-

    ; jump to end
    jmp @update_nametable_loop_compare

@update_nametable_individual:

    ; load tile count
    inx

    tay
    ; check tile count != 0
    :
        ; add tile count to nametable count
        ;inc NMI_NAMETABLE_UPDATE_COUNT

        ; tile
        lda NAMETABLE_UPDATE, x
        sta PPU_DATA
        inx

        ; decrement count
        dey
        bne :-

@update_nametable_loop_compare:

    ;; if there have been >64 tiles updated, early out of the loop
    ;lda NMI_NAMETABLE_UPDATE_COUNT
    ;cmp #64
    ;bcc @update_nametable_loop_end
    ; bmi @update_nametable_loop_end

    ; loop while X != length
    cpx NAMETABLE_UPDATE_LEN
    bne @update_nametable_loop

@update_nametable_loop_end:

    ; reset nametable update length
    stx NAMETABLE_UPDATE_POS

@end:
    rts
.endproc

;
_ppu_tint_palellete_oam_internal:
.proc ppu_tint_palellete_oam

    ; load tint
    lda _PPU_ARGS+0
    tax

    ; load ptr from remap table
    lda palette_tint_remap_table+0, X
    sta PALETTE_TINT_OAM_PTR+0

    lda palette_tint_remap_table+1, X
    sta PALETTE_TINT_OAM_PTR+1

    ; mark palette as dirty
    inc PALETTE_UPDATE_LEN

    rts
.endproc

;
_ppu_tint_palellete_background_internal:
.proc ppu_tint_palellete_background

    ; load tint
    lda _PPU_ARGS+0
    tax

    ; load ptr from remap table
    lda palette_tint_remap_table+0, X
    sta PALETTE_TINT_BACKGROUND_PTR+0

    lda palette_tint_remap_table+1, X
    sta PALETTE_TINT_BACKGROUND_PTR+1

    ; mark palette as dirty
    inc PALETTE_UPDATE_LEN

    rts
.endproc

;
_ppu_tint_palelletes_internal:
.proc ppu_tint_palelletes

    ; update each palette
    jsr ppu_tint_palellete_oam
    jsr ppu_tint_palellete_background

    rts
.endproc

;
_ppu_tint_reset_internal:
.proc ppu_tint_reset

    lda #0

    ; clear oam ptr
    sta PALETTE_TINT_OAM_PTR+0
    sta PALETTE_TINT_OAM_PTR+1

    ; clear background ptr
    sta PALETTE_TINT_BACKGROUND_PTR+0
    sta PALETTE_TINT_BACKGROUND_PTR+1

    ; mark palette as dirty
    inc PALETTE_UPDATE_LEN

    rts
.endproc

;
; nmi
;

.export nmi

.segment "LOWCODE"

.proc nmi
    push_reg

    ; prevent nmi reentry (0 == locked)
    lda NMI_LOCK
    beq :+
        jmp @nmi_end
    :

    ; lock nmi
    lda #1
    sta NMI_LOCK

    ; increment frame counter
    inc NMI_COUNT

    ; nmi ready == 0, not ready to update PPU
    lda NMI_READY
    bne :+
        jmp @ppu_update_end
    :

    ; nmi ready == 2, turn off rendering
    cmp #2
    bne :+

        lda #0
        sta PPU_MASK
        jmp @nmi_ready
    :

    ; nmi ready == 3, turn on rendering
    cmp #3
    bne :+

        lda PPU_MASK_BUFFER
        sta PPU_MASK
        jmp @nmi_ready
    :

@update_oam:

    ; test dirty bit
    lda DIRTY_UPLOAD_MASK
    cmp #(DIRTY_UPLOAD_MASK_OAM)
    bne @update_palette

    ; clear dirty flag
    and #(DIRTY_UPLOAD_MASK_OAM_CLEAR)
    sta DIRTY_UPLOAD_MASK

    ; DMA upload
    lda #>OAM_UPDATE
    sta PPU_OAM_DMA

@update_palette:
    ; if there are no palette updates, skip to nametable updates
    lda PALETTE_UPDATE_LEN
    bne @update_palette_start

    ; no palette updates, jump to name table updates
    ; NOTE: there's too much code for a beq to branch so a jmp is needed
    jmp @update_nametable

@update_palette_start:
    ; reset palette dirty flag
    lda #0
    sta PALETTE_UPDATE_LEN

    ; reset latch
    bit PPU_STATUS

    ; set PPU palette address
    lda #(>PALETTE_BASE_ADDR)
    sta PPU_ADDR
    lda #(<PALETTE_BASE_ADDR)
    sta PPU_ADDR

@update_palette_background:

    ; if there is no tint ptr, do raw background palette upload
    lda PALETTE_TINT_BACKGROUND_PTR
    beq @update_palette_background_raw

@update_palette_background_tint:
    ; map each background color using the tint ptr
.repeat 4, I
    ; don't map "transparent" color
    lda PALETTE_UPDATE+0+(I*4)
    sta PPU_DATA

    ldy PALETTE_UPDATE+1+(I*4)
    lda (PALETTE_TINT_BACKGROUND_PTR), Y
    sta PPU_DATA

    ldy PALETTE_UPDATE+2+(I*4)
    lda (PALETTE_TINT_BACKGROUND_PTR), Y
    sta PPU_DATA

    ldy PALETTE_UPDATE+3+(I*4)
    lda (PALETTE_TINT_BACKGROUND_PTR), Y
    sta PPU_DATA
.endrepeat

    ; update OAM palette
    jmp @update_palette_oam

@update_palette_background_raw:
    ; copy background palette
    ldx #0
    :
        lda PALETTE_UPDATE+0, X
        sta PPU_DATA

        inx
        cmp #16
        bne :-

@update_palette_oam:

    ; if there is no tint ptr, do raw OAM palette upload
    lda PALETTE_TINT_OAM_PTR
    beq @update_palette_oam_raw

@update_palette_oam_tint:
    ; map each OAM color using tint ptr
.repeat 4, I
    ; don't map "transparent" color
    lda PALETTE_UPDATE+16+0+(I*4)
    sta PPU_DATA

    ldy PALETTE_UPDATE+16+1+(I*4)
    lda (PALETTE_TINT_OAM_PTR), Y
    sta PPU_DATA

    ldy PALETTE_UPDATE+16+2+(I*4)
    lda (PALETTE_TINT_OAM_PTR), Y
    sta PPU_DATA

    ldy PALETTE_UPDATE+16+3+(I*4)
    lda (PALETTE_TINT_OAM_PTR), Y
    sta PPU_DATA
.endrepeat

    ; end of palette updates
    jmp @update_palette_end

@update_palette_oam_raw:
    ; copy OAM palette
    ldx #0
    :
        lda PALETTE_UPDATE+16, X
        sta PPU_DATA

        inx
        cmp #16
        bne :-

@update_palette_end:
    ;; if there has been a palette update, skip nametable updates this frame
    ;jmp @ppu_scroll

@update_nametable:
    ; if there are no nametable updates, skip to scrolling
    ldx NAMETABLE_UPDATE_POS
    cpx NAMETABLE_UPDATE_LEN
    beq @ppu_scroll

    ; count nametable updates
    lda #0
    sta NMI_NAMETABLE_UPDATE_COUNT

@update_nametable_loop:
    ; high byte address
    lda NAMETABLE_UPDATE, x
    sta PPU_ADDR
    inx

    ; low byte address
    lda NAMETABLE_UPDATE, x
    sta PPU_ADDR
    inx

    ; tile count (bit7 0 = individualt, 1 = repeat)
    lda NAMETABLE_UPDATE, x

    ; store processor state because the load dictates where to go next
    php

    ; increment to next byte
    inx

    ; restore processor state from last load
    plp

    ; if there are zero tiles to update, exit loop
    beq @update_nametable_loop_compare

    ; if bit7 is not set, update individual tiles
    bpl @update_nametable_individual

    ; otherwise, fallthrough to repeat tiles

@update_nametable_repeat:

    ; mask out bit7 of tile count and move to Y
    and #$7F
    tay

    ; add repeat count to nametable update count
    clc
    adc NMI_NAMETABLE_UPDATE_COUNT
    sta NMI_NAMETABLE_UPDATE_COUNT

    ; tile to repeat
    lda NAMETABLE_UPDATE, x
    inx

    ; write tile to PPU_DATA Y times
    :
        sta PPU_DATA
        dey
        bne :-

    ; jump to compare
    jmp @update_nametable_loop_compare

@update_nametable_individual:

    ; tile count -> Y
    tay

    ; add tile count to nametable count
    clc
    adc NMI_NAMETABLE_UPDATE_COUNT
    sta NMI_NAMETABLE_UPDATE_COUNT

    ; write individual tiles
    :
        ; load tile to write
        lda NAMETABLE_UPDATE, x
        sta PPU_DATA
        inx

        ; decrement count
        dey
        bne :-

    ; fallthrough to compare

@update_nametable_loop_compare:

    ; if there have been a lot of tiles updated, early out of the loop
    lda NMI_NAMETABLE_UPDATE_COUNT
    cmp NMI_NAMETABLE_UPDATE_COUNT_MAX
    bcc @update_nametable_loop_end
    ; bmi @update_nametable_loop_end
;
    ; loop while X != length
    cpx NAMETABLE_UPDATE_LEN
    bne @update_nametable_loop

@update_nametable_loop_end:

    ; reset nametable update length
    stx NAMETABLE_UPDATE_POS

@ppu_scroll:

    ;; enable
    ;ora #%10001000
    ;lda #0 ; scroll_nmt
    ;and #%00000011 ;
    ;ora #(PPU_CTRL_GENERATE_NMI_ON | PPU_CTRL_SPRITE_PATTERN_TABLE_ADDR_1000 | PPU_CTRL_BASE_NAMETABLE_ADDR_2000 | PPU_CTRL_BACKGROUND_PATTERN_TABLE_ADDR_0000)
    ;sta PPU_CTRL
    ;sta PPU_CTRL_BUFFER
    lda PPU_CTRL_BUFFER
    sta PPU_CTRL

    ;; enable rendering
    ;lda #(PPU_MASK_BACKGROUND_SHOW | PPU_MASK_SPRITE_SHOW | PPU_MASK_BACKGROUND_LEFTMOST_8x8_SHOW | PPU_MASK_SPRITE_LEFTMOST_8x8_SHOW)
    ;sta PPU_MASK
    ;sta PPU_MASK_BUFFER
    lda PPU_MASK_BUFFER
    sta PPU_MASK

@nmi_ready:
    ; set scroll x, y
    lda SCROLL_X
    sta PPU_SCROLL

    lda SCROLL_Y
    sta PPU_SCROLL

    ; NMI ready
    lda #0
    sta NMI_READY

@ppu_update_end:
    ; update APU from NMI
    jsr apu_update_from_nmi

    ; unlock NMI
    lda #0
    sta NMI_LOCK

@nmi_end:
    pop_reg
    rti
.endproc

.segment "RODATA"

; remap table
palette_tint_remap_table:

    .addr palette_tint_remap_0
    .addr palette_tint_remap_1
    .addr palette_tint_remap_2
    .addr palette_tint_remap_3
    .addr palette_tint_remap_4
    .addr palette_tint_remap_5
    .addr palette_tint_remap_6
    .addr palette_tint_remap_7
    .addr palette_tint_remap_8

; black
palette_tint_remap_0:
    .byte $0F, $0F, $0F, $0F, $0F, $0F, $0F, $0F, $0F, $0F, $0F, $0F, $0F, $0F, $0F, $0F

palette_tint_remap_1:
    .byte $0F, $0F, $0F, $0F, $0F, $0F, $0F, $0F, $0F, $0F, $0F, $0F, $0F, $0F, $0F, $0F

palette_tint_remap_2:
    .byte $0F, $0F, $0F, $0F, $0F, $0F, $0F, $0F, $0F, $0F, $0F, $0F, $0F, $0F, $0F, $0F

palette_tint_remap_3:
    .byte $0F, $0F, $0F, $0F, $0F, $0F, $0F, $0F, $0F, $0F, $0F, $0F, $0F, $0F, $0F, $0F

; normal colors
palette_tint_remap_4:
    .byte $00, $01, $02, $03, $04, $05, $06, $07, $08, $09, $0A, $0B, $0C, $0F, $0F, $0F

palette_tint_remap_5:
    .byte $10, $11, $12, $13, $14, $15, $16, $17, $18, $19, $1A, $1B, $1C, $00, $00, $00

palette_tint_remap_6:
    .byte $10, $21, $22, $23, $24, $25, $26, $27, $28, $29, $2A, $2B, $2C, $10, $10, $10

palette_tint_remap_7:
    .byte $30, $31, $32, $33, $34, $35, $36, $37, $38, $39, $3A, $3B, $3C, $20, $20, $20

; white
palette_tint_remap_8:
    .byte $30, $30, $30, $30, $30, $30, $30, $30, $30, $30, $30, $30, $30, $30, $30, $30
    .byte $30, $30, $30, $30, $30, $30, $30, $30, $30, $30, $30, $30, $30, $30, $30, $30
    .byte $30, $30, $30, $30, $30, $30, $30, $30, $30, $30, $30, $30, $30, $30, $30, $30
    .byte $30, $30, $30, $30, $30, $30, $30, $30, $30, $30, $30, $30, $30, $30, $30, $30
