; startup code for cc65 and neslib
; based on code by Groepaz/Hitmen <groepaz@gmx.net>, Ullrich von Bassewitz <uz@cc65.org>

;v050517


;FT_DPCM_OFF = __DMC_START__     ;set in the linker CFG file via MEMORY/DMC section
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
.importzp _PPU_ARGS
.import ppu_init, ppu_enable_default, ppu_wait_vblank, ppu_clear_nametable, ppu_clear_palette, ppu_clear_chr_ram, nmi
.import apu_init
.import mapper_reset
.import mapper_init
.import mapper_set_mirroring
.import mapper_set_chr_bank_0
.import mapper_set_chr_bank_1
.import mapper_set_prg_bank
.import mapper_reset_irq
.import mapper_set_scanline_irq
.include "zeropage.inc"
.export reset, irq

.import IRQ_SCANLINE_TABLE
.import IRQ_STATE_TABLE

FT_BASE_ADR         =$0100    ;page in RAM, should be $xx00


DMC_FREQ            =$4010
APU_SOUND           =$4015
APU_CTRL            =$4017

OAM_BUF             =$0200
PAL_BUF             =$01c0


.segment "ZEROPAGE"

_ARGS:              .res 8
TEMP:               .res 2
IRQ_PTR:            .res 2
IRQ_WAIT:           .res 1

.exportzp _ARGS
.exportzp TEMP

;
; Header
;

.include "header.inls"

header_start        Header::NES_2_0
header_mapper       Mapper::MMC5
header_wram_size    8 * KB
header_prg_size     32 * KB
header_chr_size     512 * KB
header_mirror       MirrorMode::FourScreen
header_tv           NTSC, PAL
header_end          "HEADER"

;
;
;

.segment "RODATA"


.segment "SAMPLES"

.pushseg
.segment "VECTORS"

    .addr nmi       ;$fffa vblank nmi
    .addr reset     ;$fffc reset
    .addr irq       ;$fffe irq / brk

.popseg

.segment "CHARS"

;
; irq
;

.segment "LOWCODE"

; set IRQ based on IRQ table index A
.export  _set_irq

_set_irq:
.proc set_irq

    ; store index in X and 2*index in Y
    tax
    asl A
    tay

    ; load scanline at from index A (now X)
    lda IRQ_SCANLINE_TABLE,X

    ; set scanline irq
    jsr mapper_set_scanline_irq

    ; store irq state ptr at index A (now Y)
    lda IRQ_STATE_TABLE+0,Y
    sta IRQ_PTR+0
    lda IRQ_STATE_TABLE+1,Y
    sta IRQ_PTR+1

    rts

.endproc

;
_wait_irq:
.proc wait_irq

    lda #$FF
    sta IRQ_WAIT

    :
        bit IRQ_WAIT
        bmi :-

    rts

.endproc

; jump to IRQ callback (callback must end with rts)
.proc irq_call
    jmp (IRQ_PTR)
.endproc

.proc irq
    ; push registers
    pha
    txa
    pha
    tya
    pha

    ; reset IRQ status based on mapper
    jsr mapper_reset_irq

    ; execute IRQ callback
    jsr irq_call

    ; pop registers
    pla
    tay
    pla
    tax
    pla

    rti
.endproc

;
; start up
;

.define USE_CHR_RAM     0

.segment "STARTUP"

.proc reset

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

    ; init apu
    jsr apu_init

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

    ; clear all CHR RAM
.if USE_CHR_RAM
    lda #$00
    sta _PPU_ARGS+0
    jsr ppu_clear_chr_ram

    lda #$10
    sta _PPU_ARGS+0
    jsr ppu_clear_chr_ram
.endif

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

    ; initialize mapper
    jsr mapper_init

    ; enable interupts
    cli

    ; disable IRQ scanline
    lda #0
    jsr set_irq

    ; run main()
    jmp _main

.endproc
