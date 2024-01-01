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
.import NES_MAPPER          ,NES_PRG_BANKS  ,NES_CHR_BANKS  ,NES_MIRRORING, NES_BATTERY
.importzp _PPU_ARGS
.import ppu_init, ppu_enable_default, ppu_wait_vblank, ppu_clear_nametable, ppu_clear_palette, ppu_upload_chr_ram, nmi
.import shadow_font
.import knight_sprite_0
.import knight_sprite_1
.include "zeropage.inc"

FT_BASE_ADR         =$0100    ;page in RAM, should be $xx00


DMC_FREQ            =$4010
APU_SOUND           =$4015
APU_CTRL            =$4017

OAM_BUF             =$0200
PAL_BUF             =$01c0


.segment "ZEROPAGE"

_ARGS:              .res 8
TEMP:               .res 2

.export _ARGS
.export TEMP

.segment "HEADER"

.byte 'N','E','S',$1A
.byte <NES_PRG_BANKS
.byte <NES_CHR_BANKS
.byte <NES_MIRRORING | (<NES_BATTERY << 2) | ( <NES_MAPPER << 4 )
.byte (<NES_MAPPER & $F0)
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
    ldx #$FF
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
    lda #$20
    sta _PPU_ARGS+0
    lda #$00
    sta _PPU_ARGS+1
    lda #$FF
    sta _PPU_ARGS+2
    lda #0
    sta _PPU_ARGS+3
    jsr ppu_clear_nametable

    lda #$24
    sta _PPU_ARGS+0
    lda #$00
    sta _PPU_ARGS+1
    lda #0
    sta _PPU_ARGS+2
    lda #0
    sta _PPU_ARGS+3
    jsr ppu_clear_nametable

    lda #$28
    sta _PPU_ARGS+0
    lda #$00
    sta _PPU_ARGS+1
    lda #0
    sta _PPU_ARGS+2
    lda #0
    sta _PPU_ARGS+3
    jsr ppu_clear_nametable

    lda #$2C
    sta _PPU_ARGS+0
    lda #$00
    sta _PPU_ARGS+1
    lda #0
    sta _PPU_ARGS+2
    lda #0
    sta _PPU_ARGS+3
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
    jmp _main
