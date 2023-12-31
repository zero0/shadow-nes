
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

_PPU_DATA           =PPU_DATA
.export _PPU_DATA

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

.segment "ZEROPAGE"

    NMI_LOCK:               .res 1 ;
    NMI_COUNT:              .res 1 ;
    NMI_READY:              .res 1 ;
    PPU_CTRL_BUFFER:        .res 1 ;
    PPU_MASK_BUFFER:        .res 1 ;
    NAMETABLE_UPDATE_LEN:   .res 1 ;
    NAMETABLE_UPDATE_POS:   .res 1 ;
    PALETTE_UPDATE_LEN:     .res 1 ;
    OAM_UPDATE_LEN:         .res 1 ;
    SCROLL_X:               .res 1 ;
    SCROLL_Y:               .res 1 ;
    _PPU_ARGS:              .res 8 ;
    TILE_BATCH_INDEX:       .res 1 ;
    META_SPRITE_LEN:        .res 1 ;
    META_SPRITE_ATTR:       .res 1 ;
    META_SPRITE_ADDR:       .res 2 ;
    CHR_UPLOAD_ADDR:        .res 2 ;

.export _PPU_ARGS

.segment "BSS"
    NAMETABLE_UPDATE:       .res 256 ;
    PALETTE_UPDATE:         .res 32 ;

.segment "OAM"
    OAM_UPDATE:             .res 256 ;

;
; ppu functions
;

.export _ppu_frame_index

.export ppu_init
.export ppu_wait_vblank
.export ppu_enable_default

.export ppu_clear_nametable
.export ppu_clear_palette

.export _ppu_set_scroll_internal

.export _ppu_update
.export _ppu_off
.export _ppu_skip

.export _ppu_address_tile
.export _ppu_update_tile_internal

.export _ppu_begin_tile_batch_internal
.export _ppu_push_tile_batch_internal
.export _ppu_end_tile_batch_internal

.export _ppu_update_byte
.export _ppu_clear_nametable_internal
.export _ppu_fill_nametable_attr

.export _ppu_clear_palette
.export _ppu_set_palette_internal
.export _ppu_set_palette_background_internal

.export _ppu_clear_oam
.export _ppu_oam_sprite

.export _ppu_add_meta_sprite_full_internal

.export _ppu_upload_chr_ram_internal
.export _ppu_begin_write_chr_ram_internal
.export _ppu_write_chr_ram_internal
.export _ppu_end_write_chr_ram_internal

.import _knight_sprite_0
.import _knight_sprite_1
.import _knight_sprite_2

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
; ppu_frame_index: returns in A the frame count [0..255]
_ppu_frame_index:
    lda NMI_COUNT
    rts

; ppu_init: initialize PPU ( assumes X == 0)
ppu_init:
    stx SCROLL_X
    stx SCROLL_Y

    stx PPU_SCROLL
    stx PPU_SCROLL

    stx PPU_CTRL    ; disable NMI
    stx PPU_MASK    ; disable rendering

    stx PPU_CTRL_BUFFER
    stx PPU_MASK_BUFFER
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

    lda #(PPU_CTRL_GENERATE_NMI_ON | PPU_CTRL_SPRITE_PATTERN_TABLE_ADDR_1000 | PPU_CTRL_BASE_NAMETABLE_ADDR_2000 | PPU_CTRL_BACKGROUND_PATTERN_TABLE_ADDR_0000)
    sta PPU_CTRL
    sta PPU_CTRL_BUFFER

    lda #(PPU_MASK_BACKGROUND_SHOW | PPU_MASK_SPRITE_SHOW)
    sta PPU_MASK
    sta PPU_MASK_BUFFER
    rts

; ppu_update: waits until next NMI, turns rendering on (if not already), uploads OAM, palette, and nametable update to PPU
.proc _ppu_update

    ; clear remaining OAM data
    lda #$FF
    ldx OAM_UPDATE_LEN
    :
        sta OAM_UPDATE, x
        inx
        bne :-

    ; clear OAM length
    stx OAM_UPDATE_LEN

    ; wait for NMI to be ready
    lda #1
    sta NMI_READY
    :
        lda NMI_READY
        bne :-
    rts
.endproc

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

; ppu_set_scroll: sets the (x,y) scroll position from PPU_ARGS 0 & 1
_ppu_set_scroll_internal:

    lda _PPU_ARGS+0
    sta SCROLL_X
    lda _PPU_ARGS+1
    sta SCROLL_Y

    rts

; ppu_address_tile: use with rendering off, sets memory address to tile at X/Y, ready for a $2007 (PPU_DATA) write
;   Y =  0- 31 nametable $2000
;   Y = 32- 63 nametable $2400
;   Y = 64- 95 nametable $2800
;   Y = 96-127 nametable $2C00
_ppu_address_tile:
    sta TEMP ; y -> temp
    ldy #0
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
_ppu_clear_nametable_internal:

.proc ppu_clear_nametable
    ; reset latch
    bit PPU_STATUS

    ; disable rendering
    lda #0
    sta PPU_MASK

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

    ; reset ppu mask
    lda PPU_MASK_BUFFER
    sta PPU_MASK

    rts
.endproc

; ppu_clear_palette: clear all palettes
_ppu_clear_palette:

ppu_clear_palette:
    lda #0
    ldx PALETTE_BYTE_COUNT
    :
        sta PALETTE_UPDATE, x
        dex
        bne :-

    stx PALETTE_UPDATE_LEN

    rts

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

; _ppu_clear_oam: clear oam buffer
_ppu_clear_oam:

    ; clear OAM buffer
    ldx #0
    lda #$FF
    :
        sta OAM_UPDATE, x
        inx
        ; inx
        ; inx
        ; inx
        bne :-

    ; reset length
    lda #0
    sta OAM_UPDATE_LEN

    rts

; ppu_oam_sprite: add a sprite from _OAM_ARGS to OAM_UPDATE
_ppu_oam_sprite:

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

;
_ppu_add_meta_sprite_full_internal:

    ; load OAM buffer length
    ldx OAM_UPDATE_LEN

    ; store metasprite address
    lda #(<_knight_sprite_0)
    sta META_SPRITE_ADDR+0
    lda #(>_knight_sprite_0)
    sta META_SPRITE_ADDR+1

    ; load metasprite length
    ldy #2
    lda (META_SPRITE_ADDR), y
    sta META_SPRITE_LEN
    iny

    :
        ; load metasprite attributes
        lda (META_SPRITE_ADDR), y
        sta META_SPRITE_ATTR
        iny

        ; store OAM updates
        ; y
        ; get Y tile offset from metasprite attributes
        and #%00000111
        ; go from tiles to pixels (mul 8)
        asl
        asl
        asl

        adc _PPU_ARGS+0
        sta OAM_UPDATE, x
        inx

        ; tile
        lda (META_SPRITE_ADDR), y
        sta OAM_UPDATE, x
        inx
        iny

        ; attr
        lda _PPU_ARGS+2
        sta OAM_UPDATE, x
        inx

        ; x
        ; get X tile offset from metasprite attributes (X is already shifted)
        lda META_SPRITE_ATTR
        and #%00111000

        adc _PPU_ARGS+3
        sta OAM_UPDATE, x
        inx

        ; check length
        cpy META_SPRITE_LEN
        bcc :-

    ; store updated length
    stx OAM_UPDATE_LEN

    rts

; Upload a row of tiles into CHR RAM
_ppu_upload_chr_ram_internal:

.proc ppu_upload_chr_ram
    ;_PPU_ARGS[0] ; src address
    ;_PPU_ARGS[1]
;
    ;_PPU_ARGS[2] ; row count
;
    ;_PPU_ARGS[3] ; dst address (row, 0x00, 0x01, 0x02, ...)
    ;_PPU_ARGS[0] ; dst address (col, 0x00, 0x10, ...)

    ; store address
    lda _PPU_ARGS+0 ;
    sta CHR_UPLOAD_ADDR+0
    lda _PPU_ARGS+1 ;
    sta CHR_UPLOAD_ADDR+1

    lda #0

    ; disable rendering
    sta PPU_MASK

    ; load starting address
    ldy _PPU_ARGS+3
    sty PPU_ADDR
    ldy #0
    sty PPU_ADDR

    ; number of 256-byte pages to load
    ldx _PPU_ARGS+2

@loop:
    ; load byte from address
    lda (CHR_UPLOAD_ADDR), y
    sta PPU_DATA

    ; inc, loop until wrapped back to 0
    iny
    bne @loop

    ; increment to next page
    inc CHR_UPLOAD_ADDR+1
    dex

    ; repeat until complete
    bne @loop

    ; reset mask
    lda PPU_MASK_BUFFER
    sta PPU_MASK

    rts
.endproc

;
.proc _ppu_begin_write_chr_ram_internal

    ; disable rendering
    lda #0
    sta PPU_MASK

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

    ; reenable rendering
    lda PPU_MASK_BUFFER
    sta PPU_MASK

    rts
.endproc


;
; nmi
;

.export nmi

.segment "CODE"

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

@update_oam:
    ; DMA upload
    lda #>OAM_UPDATE
    sta PPU_OAM_DMA

@update_palette:
    ; reset latch
    bit PPU_STATUS

    ; palette update
    ldx PALETTE_UPDATE_LEN
    beq @update_nametable

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

    lda #0
    sta PALETTE_UPDATE_LEN

@update_nametable:
    ; nametable update
    ldx NAMETABLE_UPDATE_POS
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

        ; tile count
        ldy NAMETABLE_UPDATE, x
        inx

        ; check tile count != 0
        cpy #0
        :
            beq :+

            ; tile
            lda NAMETABLE_UPDATE, x
            sta PPU_DATA
            inx

            ; decrement count
            dey
            jmp :-

        :
        ; loop while X != length
        cpx NAMETABLE_UPDATE_LEN
        bcc :---

    ; reset nametable update length
    stx NAMETABLE_UPDATE_POS

@ppu_scroll:

    ; set scroll x, y
    lda SCROLL_X
    sta PPU_SCROLL

    lda SCROLL_Y
    sta PPU_SCROLL

    ; enable
    ;ora #%10001000
    lda #0 ; scroll_nmt
    and #%00000011 ;
    ora #(PPU_CTRL_GENERATE_NMI_ON | PPU_CTRL_SPRITE_PATTERN_TABLE_ADDR_1000 | PPU_CTRL_BASE_NAMETABLE_ADDR_2000 | PPU_CTRL_BACKGROUND_PATTERN_TABLE_ADDR_0000)
    sta PPU_CTRL
    sta PPU_CTRL_BUFFER

    ; enable rendering
    lda #(PPU_MASK_BACKGROUND_SHOW | PPU_MASK_SPRITE_SHOW | PPU_MASK_BACKGROUND_LEFTMOST_8x8_SHOW | PPU_MASK_SPRITE_LEFTMOST_8x8_SHOW)
    sta PPU_MASK
    sta PPU_MASK_BUFFER

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
.endproc
