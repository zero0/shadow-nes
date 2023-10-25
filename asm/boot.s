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
.import ppu_init, ppu_enable_default, ppu_wait_vblank, ppu_clear_nametable, ppu_clear_palette, nmi
.include "zeropage.inc"

FT_BASE_ADR         =$0100    ;page in RAM, should be $xx00


DMC_FREQ            =$4010
APU_SOUND           =$4015
APU_CTRL            =$4017

OAM_BUF             =$0200
PAL_BUF             =$01c0


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
; SCROLL_X:           .res 1
; SCROLL_Y:           .res 1
; SCROLL_X1:          .res 1
; SCROLL_Y1:          .res 1
; PAD_STATE:          .res 2        ;one byte per controller
; PAD_STATEP:         .res 2
; PAD_STATET:         .res 2
PPU_CTRL_VAR:       .res 1
PPU_CTRL_VAR1:      .res 1
PPU_MASK_VAR:       .res 1
RAND_SEED:          .res 2
FT_TEMP:            .res 3
_ARGS:              .res 8
TEMP:               .res 2

.export _ARGS
.export TEMP

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

;
; irq
;

.segment "CODE"
irq:
    rti

;
; start up
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

    stx APU_SOUND   ; disable APU sound
    stx DMC_FREQ    ; disable DMC IRQ

    ; init ppu
    jsr ppu_init

    ; wait for first vblank
    jsr ppu_wait_vblank

    ; clear pallete (store $3f00)
    jsr ppu_clear_palette

    ;lda #$3f
    ;sta PPU_ADDR
    ;stx PPU_ADDR 
    ;ldy #0
    ;:
    ;    lda example_palette, y
    ;    sta PALETTE_UPDATE, y
    ;    iny
    ;    cpy #32
    ;    bne :-
;
    ;sty PALETTE_UPDATEL_LEN

    ; clear all nametables
    ldx #$20
    lda #$00
    ldy #65
    jsr ppu_clear_nametable

    ldx #$24
    lda #$00
    ldy #66
    jsr ppu_clear_nametable

    ldx #$28
    lda #$00
    ldy #67
    jsr ppu_clear_nametable

    ldx #$2C
    lda #$00
    ldy #68
    jsr ppu_clear_nametable

    ; clear ram
    lda #0
    ldx #0
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
    jsr ppu_wait_vblank

    ; enable NMI
    jsr ppu_enable_default
    
    ; enable interupts
    cli

    ; run main()
    jsr _main
