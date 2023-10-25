
;
; ppu
;

.importzp TEMP
.importzp sp

PPU_CTRL            =$2000
PPU_MASK            =$2001
PPU_STATUS          =$2002
PPU_OAM_ADDR        =$2003
PPU_OAM_DATA        =$2004
PPU_SCROLL          =$2005
PPU_ADDR            =$2006
PPU_DATA            =$2007
PPU_OAM_DMA         =$4014

NAMETABLE_A         =$2000
NAMETABLE_B         =$2400
NAMETABLE_C         =$2800
NAMETABLE_D         =$2C00

; palette addresses
.define PALETTE_BASE_ADDR               $3f00
.define PALETTE_BYTE_COUNT              32

.define PALETTE_BACKGROUND_BASE_ADDR    PALETTE_BASE_ADDR
.define PALETTE_BACKGROUND_0_ADDR       PALETTE_BACKGROUND_BASE_ADDR + $0
.define PALETTE_BACKGROUND_1_ADDR       PALETTE_BACKGROUND_BASE_ADDR + $4
.define PALETTE_BACKGROUND_2_ADDR       PALETTE_BACKGROUND_BASE_ADDR + $8
.define PALETTE_BACKGROUND_3_ADDR       PALETTE_BACKGROUND_BASE_ADDR + $C
.define PALETTE_BACKGROUND_BYTE_COUNT   16

.define PALETTE_SPRITE_BASE_ADDR        PALETTE_BASE_ADDR + $10
.define PALETTE_SPRITE_0_ADDR           PALETTE_SPRITE_BASE_ADDR + $0
.define PALETTE_SPRITE_1_ADDR           PALETTE_SPRITE_BASE_ADDR + $4
.define PALETTE_SPRITE_2_ADDR           PALETTE_SPRITE_BASE_ADDR + $8
.define PALETTE_SPRITE_3_ADDR           PALETTE_SPRITE_BASE_ADDR + $C
.define PALETTE_SPRITE_BYTE_COUNT       16

.define NAMETABLE_ROWS      30
.define NAMETABLE_COLS      32

.macro popa
    ldy #0
    lda (sp), y
    inc sp
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

.segment "ZEROPAGE"

    NMI_LOCK:               .res 1, 0 ; 
    NMI_COUNT:              .res 1, 0 ;
    NMI_READY:              .res 1, 2 ;
    NAMETABLE_UPDATE_LEN:   .res 1, 0 ;
    PALETTE_UPDATEL_LEN:    .res 1, 0 ;
    SCROLL_X:               .res 1, 0 ;
    SCROLL_Y:               .res 1, 0 ;

.segment "BSS"
    NAMETABLE_UPDATE:       .res 256 ;
    PALETTE_UPDATE:         .res 32 ;

.segment "OAM"
    OAM:                    .res 256 ;

;
; drawing utils
;

.export ppu_init
.export ppu_wait_vblank
.export ppu_enable_default

.export ppu_clear_nametable
.export ppu_clear_palette

.export _ppu_set_scroll

.export _ppu_update
.export _ppu_off
.export _ppu_skip
.export _ppu_address_tile
.export _ppu_update_tile
.export _ppu_update_byte
.export _ppu_clear_nametable
.export _ppu_fill_nametable_attr

.segment "RODATA"

    example_palette:
        .byte $0F,$15,$26,$37 ; bg0 purple/pink
        .byte $0F,$09,$19,$29 ; bg1 green
        .byte $0F,$01,$11,$21 ; bg2 blue
        .byte $0F,$00,$10,$30 ; bg3 greyscale

        .byte $0F,$18,$28,$38 ; sp0 yellow
        .byte $0F,$14,$24,$34 ; sp1 purple
        .byte $0F,$1B,$2B,$3B ; sp2 teal
        .byte $0F,$12,$22,$32 ; sp3 marine

.segment "CODE"

; ppu_init: initialize PPU ( assumes X == 0)
ppu_init:
    stx SCROLL_X
    stx SCROLL_Y

    stx PPU_SCROLL
    stx PPU_SCROLL

    stx PPU_CTRL    ; disable NMI
    stx PPU_MASK    ; disable rendering
    rts

; ppu_wait_vblank: waits for the next vblank
ppu_wait_vblank:
    bit PPU_STATUS
    :
        bit PPU_STATUS
        bpl :- 
    rts

ppu_enable_default:
    lda SCROLL_X
    sta PPU_SCROLL

    lda SCROLL_Y
    sta PPU_SCROLL

    lda #(PPU_CTRL_GENERATE_NMI_ON | PPU_CTRL_SPRITE_PATTERN_TABLE_ADDR_0000 | PPU_CTRL_BASE_NAMETABLE_ADDR_2000 | PPU_CTRL_BACKGROUND_PATTERN_TABLE_ADDR_0000)
    sta PPU_CTRL

    lda #(PPU_MASK_BACKGROUND_SHOW | PPU_MASK_SPRITE_SHOW)
    sta PPU_MASK
    rts

; ppu_update: waits until next NMI, turns rendering on (if not already), uploads OAM, palette, and nametable update to PPU
_ppu_update:
    lda #1
    sta NMI_READY
    :
        lda NMI_READY
        bne :-
    rts

; ppu_skip: waits until next NMI, does not update PPU
_ppu_skip:
    lda NMI_COUNT
    :
        cmp NMI_COUNT
        beq :-
    rts

; ppu_off: waits until next NMI, turns rendering off (now safe to write PPU directly via $2007)
_ppu_off:
    lda #2
    sta NMI_READY
    :
        lda NMI_READY
        bne :-
    rts

; ppu_set_scroll: sets the (x,y) scroll position from X and A
_ppu_set_scroll:
    stx SCROLL_X
    sta SCROLL_Y
    rts

; ppu_address_tile: use with rendering off, sets memory address to tile at X/Y, ready for a $2007 (PPU_DATA) write
;   Y =  0- 31 nametable $2000
;   Y = 32- 63 nametable $2400
;   Y = 64- 95 nametable $2800
;   Y = 96-127 nametable $2C00
_ppu_address_tile:
    sta TEMP ; y -> temp
    popa
    tax ; A -> x
    ldy TEMP ; temp -> y

ppu_address_tile:
    ; reset latch
    bit PPU_STATUS

    ; shift Y >> 3
    tya
    lsr
    lsr
    lsr

    ; high bits of ( Y >> 3 ) | $20
    ora #$20
    sta PPU_ADDR

    ; shift Y << 5
    tya
    asl
    asl
    asl
    asl
    asl

    ; ( Y << 5 ) | X
    sta TEMP
    txa
    ora TEMP

    ; low bits of ( Y << 5 ) | X
    sta PPU_ADDR
    rts

; ppu_update_tile: can be used with rendering on, sets the tile at X/Y to tile A next time you call _ppu_update (see ppu_address_tile)
_ppu_update_tile:
    sta TEMP ; A -> temp
    popa ; pull Y off sp
    tay ; A -> y
    popa ; pull X off sp
    tax ; A -> x
    lda TEMP ; TEMP -> A

ppu_update_tile:
    ; temporarily store A on stack
    pha

    ; temporarily store X on stack
    txa
    pha

    ; load length in X
    ldx NAMETABLE_UPDATE_LEN

    ; shift Y >> 3
    tya
    lsr
    lsr
    lsr

    ; high bits of (Y >> 3 ) | $20
    ora #$20 
    sta NAMETABLE_UPDATE, x
    inx
    
    ; shift Y << 5
    tya
    asl
    asl
    asl
    asl
    asl

    ; ( Y << 5 ) | X
    sta TEMP
    ; recover X value
    pla 
    ora TEMP
    sta NAMETABLE_UPDATE, x
    inx

    ; recover A value
    pla
    sta NAMETABLE_UPDATE, x
    inx

    ; store new length
    stx NAMETABLE_UPDATE_LEN
    rts

; ppu_update_byte: like ppu_update_tile, but X/Y makes the high/low bytes of the PPU address to write
;    this may be useful for updating attribute tiles
_ppu_update_byte:
    sta TEMP ; A -> temp
    popa ; pull Y off sp
    tay ; A -> y
    popa ; pull X off sp
    tax ; A -> x
    lda TEMP ; TEMP -> A
    
ppu_update_byte:
    ; temporarily store A on stack
    pha
    
    ; temporarily store Y on stack
    tya
    pha

    ; load length
    ldy NAMETABLE_UPDATE_LEN

    ; store high byte X
    txa
    sta NAMETABLE_UPDATE, y
    iny
    
    ; recover Y value and store low byte Y
    pla 
    sta NAMETABLE_UPDATE, y
    iny
    
    ; recover A value (byte)
    pla
    sta NAMETABLE_UPDATE, y
    iny
    
    ; store new length
    sty NAMETABLE_UPDATE_LEN
    rts

; clears specific nametable at $X/A to value Y
_ppu_clear_nametable:

ppu_clear_nametable:
    ; reset latch
    bit PPU_STATUS

    ; store
    stx PPU_ADDR
    sta PPU_ADDR

    tya

    ldy #(NAMETABLE_ROWS) ; 30 rows
    :
        ldx #(NAMETABLE_COLS) ; 32 columns
        :
            sta PPU_DATA
            adc #1
            dex
            bne :-
        dey
        bne :--

    lda #0

    ; empty attribute table
    ldx #64 ; 64 bytes
    :
        sta PPU_DATA
        clc
        adc #%01010101
        dex
        bne :-

    rts

; ppu_clear_palette: clear all palettes
_ppu_clear_palette:

ppu_clear_palette:
    ; reset latch
    bit PPU_STATUS

    ; load palette address
    ldx #(>PALETTE_BASE_ADDR)
    lda #(<PALETTE_BASE_ADDR)

    ; store
    stx PPU_ADDR
    sta PPU_ADDR

    ldy #0
    :
        lda example_palette, y ; temp
        sta PPU_DATA
        iny
        cpy #32
        bne :-
    
    rts

; fill nametable at $ARGS+0/ARGS+1 with table ARGS+2 and attr ARGS+3
_ppu_fill_nametable_attr:

ppu_fill_nametable_attr:
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

;
; nmi
;

.export nmi

.segment "CODE"

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

nmi:
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

        ;lda #0
        ;sta PPU_MASK
        jmp @nmi_ready
    :

    ;; sprite oam dma
    ;ldx #0
    ;stx PPU_OAM_ADDR
    ;lda #>OAM
    ;sta PPU_OAM_DMA

@update_palette:
    ; palette update
    ldx #0
    cpx PALETTE_UPDATEL_LEN
    beq @update_nametable

    ; set PPU palette address
    ldx #(>PALETTE_BASE_ADDR)
    lda #(<PALETTE_BASE_ADDR)

    stx PPU_ADDR
    sta PPU_ADDR
    :
        lda PALETTE_UPDATE, x
        sta PPU_DATA
        inx

        ; loop while X !+ PALETTE_UPDATEL_LEN
        cpx PALETTE_UPDATEL_LEN
        bcc :-

    lda #0
    sta PALETTE_UPDATEL_LEN

@update_nametable:
    ; nametable update
    ldx #0
    cpx NAMETABLE_UPDATE_LEN
    beq @ppu_scroll
    :
        ; high byte address
        lda NAMETABLE_UPDATE, x
        sta PPU_ADDR
        inx
        
        ; low byte address
        lda NAMETABLE_UPDATE, x
        sta PPU_ADDR
        inx
        
        ; tile 
        lda NAMETABLE_UPDATE, x
        sta PPU_DATA
        inx
        
        ; loop while X != length
        cpx NAMETABLE_UPDATE_LEN
        bcc :-

    ; reset nametable update length
    lda #0
    sta NAMETABLE_UPDATE_LEN

@ppu_scroll:

    lda SCROLL_X ; NMI_COUNT
    sta PPU_SCROLL

    lda SCROLL_Y ; NMI_COUNT
    sta PPU_SCROLL

    ; enable 
    ;ora #%10001000
    lda #0 ; scroll_nmt
    and #%00000011 ;
    ora #(PPU_CTRL_GENERATE_NMI_ON | PPU_CTRL_SPRITE_PATTERN_TABLE_ADDR_0000 | PPU_CTRL_BASE_NAMETABLE_ADDR_2000)
    sta PPU_CTRL

    ; enable rendering
    lda #(PPU_MASK_BACKGROUND_SHOW | PPU_MASK_SPRITE_SHOW | PPU_MASK_BACKGROUND_LEFTMOST_8x8_SHOW | PPU_MASK_SPRITE_LEFTMOST_8x8_SHOW)
    sta PPU_MASK

@nmi_ready:
    ; NMI ready
    ldx #0
    stx NMI_READY

@ppu_update_end:
    ; TODO: play sound/music

    ; unlock NMI
    lda #0
    sta NMI_LOCK

@nmi_end:
    pop_reg
    rti