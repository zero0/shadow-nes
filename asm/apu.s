;
; APU
;

.importzp TEMP
.importzp sp

APU_BASE_REG            =$4000

APU_PULSE1              =$4000
APU_PULSE2              =$4004
APU_TRIANGLE            =$4008
APU_NOISE               =$400C
APU_DMC                 =$4010
APU_STATUS              =$4015
APU_FRAME_COUNTER       =$4017

APU_MMC5_PULSE1         =$5000
APU_MMC5_PULSE2         =$5004

APU_MMC5_PULSE1_VOLUME  =APU_MMC5_PULSE1 + 0
APU_MMC5_PULSE1_SWEEP   =APU_MMC5_PULSE1 + 1
APU_MMC5_PULSE1_LO      =APU_MMC5_PULSE1 + 2
APU_MMC5_PULSE1_HI      =APU_MMC5_PULSE1 + 3

APU_MMC5_PULSE2_VOLUME  =APU_MMC5_PULSE2 + 0
APU_MMC5_PULSE2_SWEEP   =APU_MMC5_PULSE2 + 1
APU_MMC5_PULSE2_LO      =APU_MMC5_PULSE2 + 2
APU_MMC5_PULSE2_HI      =APU_MMC5_PULSE2 + 3

APU_MMC5_PCM_MODE       =$5010
APU_MMC5_SND_CHN        =$5015
APU_MMC5_EXRAM_MODE     =$5104

APU_PULSE1_VOLUME       =APU_PULSE1 + 0
APU_PULSE1_SWEEP        =APU_PULSE1 + 1
APU_PULSE1_TIMER_LO     =APU_PULSE1 + 2
APU_PULSE1_TIMER_HI     =APU_PULSE1 + 3

APU_PULSE2_VOLUME       =APU_PULSE2 + 0
APU_PULSE2_SWEEP        =APU_PULSE2 + 1
APU_PULSE2_TIMER_LO     =APU_PULSE2 + 2
APU_PULSE2_TIMER_HI     =APU_PULSE2 + 3

APU_TRIANGLE_LINEAR     =APU_TRIANGLE + 0
APU_TRIANGLE_TIMER_LO   =APU_TRIANGLE + 2
APU_TRIANGLE_TIMER_HI   =APU_TRIANGLE + 3

APU_NOISE_VOLUME        =APU_NOISE + 0
APU_NOISE_NOISE         =APU_NOISE + 2
APU_NOISE_LENGTH        =APU_NOISE + 3

APU_DMC_TIMER           =APU_DMC + 0
APU_DMC_LOAD_COUNTER    =APU_DMC + 1
APU_DMC_SAMPLE_ADDR     =APU_DMC + 2
APU_DMC_SAMPLE_LENGTH   =APU_DMC + 3

.define APU_STATUS_ENABLE_DMC       %00010000
.define APU_STATUS_ENABLE_NOISE     %00001000
.define APU_STATUS_ENABLE_TRIANGLE  %00000100
.define APU_STATUS_ENABLE_PULSE2    %00000010
.define APU_STATUS_ENABLE_PULSE1    %00000001

; $4017
.define APU_FRAME_COUNTER_4_STEP_SEQ                %10000000
.define APU_FRAME_COUNTER_5_STEP_SEQ                %00000000
.define APU_FRAME_COUNTER_CLEAR_FRAME_INTERUPT      %01000000

; Pulse Wave
; $4000/4
.define APU_PULSE_DUTY_CYCLE_12_5       %00000000
.define APU_PULSE_DUTY_CYCLE_25_0       %01000000
.define APU_PULSE_DUTY_CYCLE_50_0       %10000000
.define APU_PULSE_DUTY_CYCLE_25_N       %11000000

.define APU_PULSE_PLAY_INFINITE         %00100000
.define APU_PULSE_PLAY_ONE_SHOT         %00000000

.define APU_PULSE_VOLUME_CONSTANT       %00010000
.define APU_PULSE_VOLUME_ENVOLOPE       %00000000

.macro lda_apu_pulse_volume    duty, inf, const, vol
    lda #(((duty & 3) << 6 ) | ((inf & 1) << 5) | ((const & 1) << 4 ) | (vol & $0F))
.endmacro

; $4001/5
.define APU_PULSE_SWEEP_ENABLED         %10000000
.define APU_PULSE_SWEEP_NEGATE          %00001000
.define APU_PULSE_SWEEP_PERIOD_SHIFT    4
.define APU_PULSE_SWEEP_PERIOD_MASK     %01110000
.define APU_PULSE_SWEEP_SHIFT_MASK      %00000111

.macro lda_apu_pulse_sweep  enable, negate, period, shift
    lda #( ((enable & 1) << 7) | ((negate & 1) << 3) | (((period & 7) << APU_PULSE_SWEEP_PERIOD_SHIFT) & APU_PULSE_SWEEP_PERIOD_MASK) | (shift & APU_PULSE_SWEEP_SHIFT_MASK) )
.endmacro

; $4002/6
.define APU_PULSE_PERIOD_LOW_MASK       %11111111

; $4003/7
.define APU_PULSE_PERIOD_HIGH_MASK      %00000111
.define APU_PULSE_LENGTH_LOAD_SHIFT     3
.define APU_PULSE_LENGTH_LOAD_MASK      %11111000

.macro ldax_apu_pulse_timer     period, length
    lda #( .lobyte(period) )
    ldx #( (.hibyte(period) & APU_PULSE_PERIOD_HIGH_MASK) | ((length << APU_PULSE_LENGTH_LOAD_SHIFT) & APU_PULSE_LENGTH_LOAD_MASK ) )
.endmacro


; Triangle Wave
; $4008
.define APU_TRIANGLE_LENGTH_COUNTER_HALT        %10000000

.define APU_TRIANGLE_LINEAR_COUNTER_LOAD_SHIFT  0
.define APU_TRIANGLE_LINEAR_COUNTER_LOAD_MASK   %01111111

.define APU_TRIANGLE_LENGTH_COUNTER_LOAD_SHIFT  3
.define APU_TRIANGLE_LENGTH_COUNTER_LOAD_MASK   %11111000


; $400A
.define APU_TRIANGLE_PERIOD_LOW_MASK            %11111111

; $400B
.define APU_TRIANGLE_PERIOD_HIGH_MASK           %00000111

.define APU_TRIANGLE_LENGTH_LOAD_SHIFT          3
.define APU_TRIANGLE_LENGTH_LOAD_MASK           %11111000

.macro ldax_apu_triangle_pulse_timer    period, length
    lda #( .lobyte(period) )
    ldx #( .hibyte(period) & APU_TRIANGLE_PERIOD_HIGH_MASK ) | ((length << APU_TRIANGLE_LENGTH_LOAD_SHIFT) & APU_TRIANGLE_LENGTH_LOAD_MASK ) )
.endmacro

; Noise
; $400C
.define APU_NOISE_PLAY_INFINITE                 %00100000
.define APU_NOISE_PLAY_ONE_SHOT                 %00000000

.define APU_NOISE_VOLUME_CONSTANT               %00010000
.define APU_NOISE_VOLUME_ENVOLOPE               %00000000

.define APU_NOISE_VOLUME_OR_ENVOLOPE_MASK       %00001111

; $400E
.define APU_NOISE_MODE_FLAG                     %10000000
.define APU_NOISE_PERIOD_MASK                   %00001111

; $400F
.define APU_NOISE_LENGTH_COUNTER_LOAD_SHIFT     3
.define APU_NOISE_LENGTH_COUNTER_LOAD_MASK      %11111000

; DMC
; $4010
.define APU_DMC_ENABLE_IRQ          %10000000
.define APU_DMC_DISABLE_IRQ         %00000000

.define APU_DMC_PLAY_INFINITE       %01000000
.define APU_DMC_PLAY_ONE_SHOT       %00000000

.define APU_DMC_FREQUENCY_MASK      %00001111

; $4011
.define APU_DMC_LOAD_COUNTER_MASK   %01111111

; $4012
.define APU_DMC_SAMPLE_ADDR_MASK    %11111111

; $4013
.define APU_DMC_SAMPLE_LENGTH_MASK  %11111111

;
;
;

.segment "ZEROPAGE"

    APU_STATUS_BUFF:            .res 1 ;

    APU_MUSIC_SPEED:            .res 1 ;
    APU_MUSIC_IDX_CUR:          .res 1 ;
    APU_MUSIC_IDX_NXT:          .res 1 ;

    APU_SFX_SPEED:              .res 1 ;
    APU_SFX_IDX_0:              .res 1 ;
    APU_SFX_IDX_1:              .res 1 ;

    APU_MUSIC_TABLE_PTR:        .res 2 ;
    APU_SFX_TABLE_PTR:          .res 2 ;

    _APU_TEMP:                  .res 2 ;
    _APU_ARGS:                  .res 4 ;

;
;
;

.enum

.endenum

.define APU_MUSIC_CHANNEL_COUNT     1
.define APU_SFX_CHANNEL_COUNT       4
.define APU_CHANNEL_COUNT           APU_SFX_CHANNEL_COUNT + APU_MUSIC_CHANNEL_COUNT

.define APU_OUTPUT_BUFFER_LENGTH    14
.define APU_OUTPUT_TIMER_LEGNTH     4

.segment "BSS"

    APU_OUTPUT_BUFFER:          .res APU_OUTPUT_BUFFER_LENGTH ;
    APU_OUTPUT_TIMERS:          .res APU_OUTPUT_TIMER_LEGNTH ;
    APU_CONFIG:                 .res 1

.define APU_CONFIG_PLAYBACK_MASK    #%00000001

.define APU_CONFIG_PLAYBACK_NTSC    #%00000000
.define APU_CONFIG_PLAYBACK_PAL     #%00000001

APU_OUT_PULSE1_VOLUME       =APU_OUTPUT_BUFFER + 0
APU_OUT_PULSE1_SWEEP        =APU_OUTPUT_BUFFER + 1
APU_OUT_PULSE1_TIMER_LO     =APU_OUTPUT_BUFFER + 2
APU_OUT_PULSE1_TIMER_HI     =APU_OUTPUT_BUFFER + 3

APU_OUT_PULSE2_VOLUME       =APU_OUTPUT_BUFFER + 4
APU_OUT_PULSE2_SWEEP        =APU_OUTPUT_BUFFER + 5
APU_OUT_PULSE2_TIMER_LO     =APU_OUTPUT_BUFFER + 6
APU_OUT_PULSE2_TIMER_HI     =APU_OUTPUT_BUFFER + 7

APU_OUT_TRIANGLE_LINEAR     =APU_OUTPUT_BUFFER + 8
APU_OUT_TRIANGLE_TIMER_LO   =APU_OUTPUT_BUFFER + 9
APU_OUT_TRIANGLE_TIMER_HI   =APU_OUTPUT_BUFFER + 10

APU_OUT_NOISE_VOLUME        =APU_OUTPUT_BUFFER + 11
APU_OUT_NOISE_NOISE         =APU_OUTPUT_BUFFER + 12
APU_OUT_NOISE_LENGTH        =APU_OUTPUT_BUFFER + 13

APU_OUT_PULSE1              =APU_OUT_PULSE1_VOLUME
APU_OUT_PULSE2              =APU_OUT_PULSE2_VOLUME
APU_OUT_TRIANGLE            =APU_OUT_TRIANGLE_LINEAR
APU_OUT_NOISE               =APU_OUT_NOISE_VOLUME

APU_OUTPUT_TIMER_PULSE1     =APU_OUTPUT_TIMERS + 0
APU_OUTPUT_TIMER_PULSE2     =APU_OUTPUT_TIMERS + 1
APU_OUTPUT_TIMER_TRIANGLE   =APU_OUTPUT_TIMERS + 2
APU_OUTPUT_TIMER_NOISE      =APU_OUTPUT_TIMERS + 3

;
;
;

.export apu_init
.export apu_update_from_nmi
.export apu_update_mixer
.export apu_enable_all, apu_disable_all
.export apu_enable_dmc, apu_disable_dmc

.export _apu_update_mixer = apu_update_mixer
.export _apu_play_music = apu_play_music
.export _apu_play_sfx = apu_play_sfx

.segment "LOWCODE"

; init APU, config in A
.proc apu_init

    ; store config
    sta APU_CONFIG

    ; enable all channels (-DMC)
    lda #(APU_STATUS_ENABLE_PULSE1 | APU_STATUS_ENABLE_PULSE2 | APU_STATUS_ENABLE_TRIANGLE | APU_STATUS_ENABLE_NOISE )
    sta APU_STATUS_BUFF
    sta APU_STATUS

    ; set step sequence
    lda #(APU_FRAME_COUNTER_4_STEP_SEQ)
    sta APU_FRAME_COUNTER

    ; clear output timers
    lda #0
    .repeat APU_OUTPUT_TIMER_LEGNTH, I
    sta APU_OUTPUT_TIMERS+I
    .endrepeat

    rts

.endproc

; set music table ptr from ARG 0,1
.proc apu_init_music

    ; 0 NTSC 1 PAL
    lda APU_CONFIG
    and APU_CONFIG_PLAYBACK_MASK

    ; shift playback to offset
    lsr
    tay

    ; store music ptr
    lda _APU_ARGS+0,Y
    sta APU_MUSIC_TABLE_PTR+0
    lda _APU_ARGS+1,Y
    sta APU_MUSIC_TABLE_PTR+1

    rts

.endproc

; set sfx table ptr from ARG 0,1
.proc apu_init_sfx

    ; 0 NTSC 1 PAL
    lda APU_CONFIG
    and APU_CONFIG_PLAYBACK_MASK

    ; shift playback to offset
    lsr
    tay

    ; TODO: store sfx pointer


    rts

.endproc

; Disable pulse 1, 2, triangle, noise
.proc apu_disable_all

    lda APU_STATUS_BUFF
    and #$F0
    sta APU_STATUS_BUFF
    sta APU_STATUS

    rts

.endproc

; Disable DMC
.proc apu_disable_dmc

    lda APU_STATUS_BUFF
    and #$0F
    sta APU_STATUS_BUFF
    sta APU_STATUS

.endproc

; Enable pulse 1, 2, triangle, noise
.proc apu_enable_all

    lda APU_STATUS_BUFF
    ora #(APU_STATUS_ENABLE_PULSE1 | APU_STATUS_ENABLE_PULSE2 | APU_STATUS_ENABLE_TRIANGLE | APU_STATUS_ENABLE_NOISE)
    sta APU_STATUS_BUFF
    sta APU_STATUS

    rts

.endproc

; Enable DMC
.proc apu_enable_dmc

    lda APU_STATUS_BUFF
    ora #(APU_STATUS_ENABLE_DMC)
    sta APU_STATUS_BUFF
    sta APU_STATUS

    rts

.endproc

; Update APU during NMI which just copies buffered data into APU registers
.proc apu_update_from_nmi

@update_pulse1:

    ; load pulse1 timer
    ldx APU_OUTPUT_TIMER_PULSE1

    ; if timer == 0, skip
    beq @update_pulse2

    ; copy buffer to registers
    .repeat 4, I
    lda APU_OUT_PULSE1+I
    sta APU_PULSE1+I
    .endrepeat

    ; decrement timer
    dex
    stx APU_OUTPUT_TIMER_PULSE1

    ; if timer == 0, clear buffer
    ;bne :+
    ;    lda #0
    ;    .repeat 4, I
    ;    lda APU_OUT_PULSE1+I
    ;    .endrepeat
    ;    :
;
@update_pulse2:

    ; load pulse2 timer
    ldx APU_OUTPUT_TIMER_PULSE2

    ; if timer == 0, skip
    beq @update_triangle

    ; copy buffer to registers
    .repeat 4, I
    lda APU_OUT_PULSE2+I
    sta APU_PULSE2+I
    .endrepeat

    ; decrement timer
    dex
    stx APU_OUTPUT_TIMER_PULSE2

    ; if timer == 0, clear buffer
    ;bne :+
    ;    lda #0
    ;    .repeat 4, I
    ;    lda APU_OUT_PULSE2+I
    ;    .endrepeat
    ;    :

@update_triangle:

    ; load triangle timer
    ldx APU_OUTPUT_TIMER_TRIANGLE

    ; if timer == 0, skip
    beq @update_noise

    ; copy buffer to registers
    .repeat 3, I
    lda APU_OUT_TRIANGLE+I
    sta APU_TRIANGLE+I
    .endrepeat

    ; decrement timer
    dex
    stx APU_OUTPUT_TIMER_TRIANGLE

    ; if timer == 0, clear buffer
    ;bne :+
    ;    lda #0
    ;    .repeat 3, I
    ;    lda APU_OUT_TRIANGLE+I
    ;    .endrepeat
    ;    :

@update_noise:

    ; load noise timer
    ldx APU_OUTPUT_TIMER_NOISE

    ; if timer == 0, skip
    beq @update_end

    ; copy buffer to registers
    .repeat 3, I
    lda APU_OUT_NOISE+I
    sta APU_NOISE+I
    .endrepeat

    ; decrement timer
    dex
    stx APU_OUTPUT_TIMER_NOISE

    ; if timer == 0, clear buffer
    ;bne :+
    ;    lda #0
    ;    .repeat 3, I
    ;    lda APU_OUT_TRIANGLE+I
    ;    .endrepeat
    ;    :

@update_end:
    rts

.endproc

; Update APU mixer
.proc apu_update_mixer



    rts

.endproc

; Play music at index A
.proc apu_play_music

    ; convert index to offset
    asl

    rts

.endproc

; Play SFX at index A
.proc apu_play_sfx

    ; NOTE: temp sound to see that it's working
    lda_apu_pulse_volume 0, 1, 1, 15
    ;sta APU_PULSE1_VOLUME
    sta APU_OUT_PULSE1_VOLUME

    lda_apu_pulse_sweep 1, 0, 2, 3
    ;sta APU_PULSE1_SWEEP
    sta APU_OUT_PULSE1_SWEEP

    ldax_apu_pulse_timer 20000, 5
    ;sta APU_PULSE1_TIMER_LO
    ;stx APU_PULSE1_TIMER_HI
    sta APU_OUT_PULSE1_TIMER_LO
    stx APU_OUT_PULSE1_TIMER_HI

    lda #5
    sta APU_OUTPUT_TIMERS+0

    rts

.endproc

.define APU_SFX_STREAM_USE_PULSE1   #%10000000

.proc _apu_proc_sfx_channel

    ldy #0

    ;lda (ptr), y
    iny

    sta _APU_TEMP

    and APU_SFX_STREAM_USE_PULSE1
    bne @read_pulse1

@read_pulse1:

@read_pulse2:

@read_triangle:

@read_noise:

@read_mmc5_pulse1:

@read_mmc5_pulse2:

@end:

    rts

.endproc

; -= 1. channel updates =-
; sfx engine and music engine write to channels during their decoding of streams

; -= 2. mixer =-
; take all channels and find highest volume for pulse 1, pulse 2, triangle, noise, mmc5 pulse 1, mmc5 pulse 2
; use channel's values to determine
; write required updates to buffer for nmi processing

; -= 3. nmi upload =-
; update required apu registers with updates from buffer only when needed

; -= sfx format =-
; only logs changes so it reads only when things should change
; read 1 byte, determine how many bytes to read based on used flags (up to 4 + 4 + 3 + 3 + 4 + 4 = 22 bytes if all flags set)
; data comes in pre-formatted so can do 1-1 copy from RODATA to APU registers
; no need for tempo since it's a sfx and will probably be a one shot
; need to handle PAL vs NTSC if no tempo (assume NTSC for now)
;
; 1000 0000 - use pulse 1
; 0100 0000 - use pulse 2
; 0010 0000 - use triangle
; 0001 0000 - use noise
; 0000 1000 - use mmc5 pulse 1
; 0000 0100 - use mmc5 pulse 2
; 0000 0010 - skip N frames (next byte)
; 0000 0001 - skip frame
; 0000 0000 - end of stream

; pulse 4 bytes
; ddic vvvv = duty inf const volume
; eppp nsss = enable period neg shift
; pppp pppp llll lPPP = period length

; triangle 3 bytes

; noise 3 bytes
