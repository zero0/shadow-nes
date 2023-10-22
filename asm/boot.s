; startup code for cc65 and neslib
; based on code by Groepaz/Hitmen <groepaz@gmx.net>, Ullrich von Bassewitz <uz@cc65.org>

;v050517


FT_DPCM_OFF = __DMC_START__     ;set in the linker CFG file via MEMORY/DMC section
                                ;'start' there should be $c000..$ffc0, in 64-byte steps
FT_SFX_STREAMS = 4              ;number of sound effects played at once, 1..4

.define FT_DPCM_ENABLE  0       ;undefine to exclude all DMC code
.define FT_SFX_ENABLE   0       ;undefine to exclude all sound effects code

.export __STARTUP__:absolute=1
.import _main

; Linker generated symbols
.import __RAM_START__       ,__RAM_SIZE__
.import __ROM0_START__      ,__ROM0_SIZE__
.import __STARTUP_LOAD__    ,__STARTUP_RUN__,__STARTUP_SIZE__
.import __CODE_LOAD__       ,__CODE_RUN__   ,__CODE_SIZE__
.import __RODATA_LOAD__     ,__RODATA_RUN__ ,__RODATA_SIZE__
.import __DMC_START__
.import NES_MAPPER          ,NES_PRG_BANKS  ,NES_CHR_BANKS  ,NES_MIRRORING
.include "zeropage.inc"

FT_BASE_ADR         =$0100    ;page in RAM, should be $xx00


PPU_CTRL            =$2000
PPU_MASK            =$2001
PPU_STATUS          =$2002
PPU_OAM_ADDR        =$2003
PPU_OAM_DATA        =$2004
PPU_SCROLL          =$2005
PPU_ADDR            =$2006
PPU_DATA            =$2007
DMC_FREQ            =$4010
PPU_OAM_DMA         =$4014
APU_SOUND           =$4015
CTRL_PORT1          =$4016
APU_CTRL            =$4017

OAM_BUF             =$0200
PAL_BUF             =$01c0

NAMETABLE_A         =$2000
NAMETABLE_B         =$2400
NAMETABLE_C         =$2800
NAMETABLE_D         =$2C00


.segment "ZEROPAGE"

NTSC_MODE:          .res 1
FRAME_CNT1:         .res 1
FRAME_CNT2:         .res 1
VRAM_UPDATE:        .res 1
NAME_UPD_ADR:       .res 2
NAME_UPD_ENABLE:    .res 1
PAL_UPDATE:         .res 1
PAL_BG_PTR:         .res 2
PAL_SPR_PTR:        .res 2
SCROLL_X:           .res 1
SCROLL_Y:           .res 1
SCROLL_X1:          .res 1
SCROLL_Y1:          .res 1
; PAD_STATE:          .res 2        ;one byte per controller
; PAD_STATEP:         .res 2
; PAD_STATET:         .res 2
PPU_CTRL_VAR:       .res 1
PPU_CTRL_VAR1:      .res 1
PPU_MASK_VAR:       .res 1
RAND_SEED:          .res 2
FT_TEMP:            .res 3

TEMP:               .res 11

PAD_BUF             =TEMP+1

PTR                 =TEMP    ;word
LEN                 =TEMP+2    ;word
NEXTSPR             =TEMP+4
SCRX                =TEMP+5
SCRY                =TEMP+6
SRC                 =TEMP+7    ;word
DST                 =TEMP+9    ;word

RLE_LOW             =TEMP
RLE_HIGH            =TEMP+1
RLE_TAG             =TEMP+2
RLE_BYTE            =TEMP+3



.segment "HEADER"

.byte 'N','E','S',$1a
.byte <NES_PRG_BANKS
.byte <NES_CHR_BANKS
.byte <NES_MIRRORING | (<NES_MAPPER << 4)
.byte <NES_MAPPER & $f0
.res 8,0


.segment "RODATA"


.segment "SAMPLES"


.segment "VECTORS"

    .word nmi       ;$fffa vblank nmi
    .word reset     ;$fffc reset
    .word irq       ;$fffe irq / brk


.segment "CHARS"


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

;
; irq
;

.segment "CODE"
irq:
    rti

;
; nmi
;

.segment "ZEROPAGE"

    NMI_LOCK:               .res 1 ; 
    NMI_COUNT:              .res 1 ;
    NMI_READY:              .res 1 ;
    NAMETABLE_UPDATE_LEN:   .res 1 ;


.segment "BSS"
    NAMETABLE_UPDATE:   .res 256 ;
    PALETTE:            .res 32 ;

.segment "OAM"
    OAM:                .res 256 ;

.segment "CODE"
nmi:
    push_reg

    ; prevent nmi reentry
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
        sta NMI_READY
        jmp @ppu_update_end
    :

    ; sprite oam dma
    ldx #0
    stx PPU_OAM_ADDR
    lda #>OAM
    sta PPU_OAM_DMA

    ; palettes
    ; set horizontal nametable inc 32
    lda #%10001000
    sta PPU_CTRL
    lda PPU_STATUS
    
    ; set PPU address to $3f00
    lda #$3f
    sta PPU_ADDR
    stx PPU_ADDR

    :
        lda PALETTE, x
        sta PPU_DATA
        inx
        cpx #32
        bcc :-

    ; nametable update
    ldx #0
    cpx NAMETABLE_UPDATE_LEN
    bcs @ppu_update_end
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

@ppu_update_end:
    ; clear NMI
    ldx #0
    stx NMI_READY

    ; TODO: play sound/music

    ; unlock NMI
    lda #0
    sta NMI_LOCK

@nmi_end:
    pop_reg
    rti

;
; drawing utils
;

.export _ppu_update

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

; ppu_update: waits until next NMI, turns rendering on (if not already), uploads OAM, palette, and nametable update to PPU
_ppu_update:
    lda #1
    sta NMI_READY
    :
        lda NMI_READY
        bne :-
    rts

; ppu_skip: waits until next NMI, does not update PPU
ppu_skip:
    lda NMI_COUNT
    :
        cmp NMI_COUNT
        beq :-
    rts

; ppu_off: waits until next NMI, turns rendering off (now safe to write PPU directly via $2007)
ppu_off:
    lda #2
    sta NMI_READY
    :
        lda NMI_READY
        bne :-
    rts

; ppu_address_tile: use with rendering off, sets memory address to tile at X/Y, ready for a $2007 (PPU_DATA) write
;   Y =  0- 31 nametable $2000
;   Y = 32- 63 nametable $2400
;   Y = 64- 95 nametable $2800
;   Y = 96-127 nametable $2C00
ppu_address_tile:
    ; reset latch
    lda PPU_STATUS

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

; clears nametable data and resets 
clear_nametable:
    ; reset latch
    lda PPU_STATUS

    ; store $2000 (NAMETABLE_A) to start
    lda #$20
    sta PPU_ADDR
    lda #$00
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
; gamepad
;

.segment "ZEROPAGE"

    GAMEPAD_STATE:              .res 4  ; four controllers
    GAMEPAD_PREV_STATE:         .res 4  ; four controllers

GAMEPAD_A      = $01
GAMEPAD_B      = $02
GAMEPAD_SELECT = $04
GAMEPAD_START  = $08
GAMEPAD_U      = $10
GAMEPAD_D      = $20
GAMEPAD_L      = $40
GAMEPAD_R      = $80

.export _gamepad_poll
.export _gamepad_state
.export _gamepad_prev_state

.segment "CODE"

; poll game pad at index A
_gamepad_poll:

    ; fix gamepad index to [0,3] and move to x
    and #$3
    tax

    ; strobe game pad
    lda #1
    sta CTRL_PORT1
    lda #0
    sta CTRL_PORT1

    ; store previous state
    ldy GAMEPAD_STATE, x
    sty GAMEPAD_PREV_STATE, x

    ; clear state
    sta GAMEPAD_STATE, x

    ; read 8 bytes from controller[x]
    ldy #8
    :
        lda CTRL_PORT1, x
        ror a
        ror GAMEPAD_STATE, x
        dey
        bne :-

    rts

; get game pad state at index A
_gamepad_state:

    ; fix gamepad index to [0,3] and move to x
    and #$3
    tax

    ; load game pad state
    lda GAMEPAD_STATE, x

    rts

; get game pad state at index A
_gamepad_prev_state:

    ; fix gamepad index to [0,3] and move to x
    and #$3
    tax

    ; load game pad state
    lda GAMEPAD_PREV_STATE, x

    rts

;
;
;

.segment "STARTUP"

.segment "CODE"
reset:
    sei             ; mask interupts
    cld             ; disable decimal mode

    lda #$40
    sta APU_CTRL    ; disable APU IRQ

    ; initialize stack
    ldx #$ff
    txs
    inx     ; x == 0

    ; 
    stx PPU_CTRL    ; disable NMI
    stx PPU_MASK    ; disable rendering
    stx APU_SOUND   ; disable APU sound
    stx DMC_FREQ    ; disable DMC IRQ

    ; wait for first vblank
    bit PPU_STATUS
    :
        bit PPU_STATUS
        bpl :-

    ; clear pallete (store $3f00)
    lda #$3f
    sta PPU_ADDR
    stx PPU_ADDR 
    lda #$0f
    ldy #$20
    :
        sta PPU_DATA
        dey
        bne :-

    ; clear nametable (store $2000)
    txa
    ldy #$20
    sty PPU_ADDR
    sta PPU_ADDR
    ldy #$10
    :
        sta PPU_DATA
        inx             ; x goes from 0 -> 255 -> 0
        bne :-
        dey
        bne :-

    ; clear ram
    txa
    :
        sta $0000, x
        sta $0100, x
        sta $0200, x
        sta $0300, x
        sta $0400, x
        sta $0500, x
        sta $0600, x
        sta $0700, x
        inx
        bne :-

    ; wait for second vblank
    :
        bit PPU_STATUS
        bpl :-
    
    ; enable NMI
    lda #%10001000
    sta PPU_CTRL

    ; enable interupts
    cli

    ; run main
    jsr _main
