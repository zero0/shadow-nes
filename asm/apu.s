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

APU_NOISE_VOLUM         =APU_NOISE + 0
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
    lda #( ((enable & 1) << 7) | ((negate & 1) << 3) | (((period & 7) << APU_PULSE_SWEEP_PERIOD_SHIFT) & APU_NOISE_PERIOD_MASK) | (shift & APU_PULSE_SWEEP_SHIFT_MASK) )
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
    APU_SFX_SPEED:              .res 1 ;

;
;
;

.define APU_OUTPUT_BUFFER_LENGTH    14
.segment "BSS"

    APU_OUTPUT_BUFFER:         .res APU_OUTPUT_BUFFER_LENGTH ;

APU_OUT_PULSE1_TIMER        =APU_OUTPUT_BUFFER + 0
APU_OUT_PULSE1_LENGTH       =APU_OUTPUT_BUFFER + 1
APU_OUT_PULSE1_ENVELOPE     =APU_OUTPUT_BUFFER + 2
APU_OUT_PULSE1_SWEEP        =APU_OUTPUT_BUFFER + 3
APU_OUT_PULSE2_TIMER        =APU_OUTPUT_BUFFER + 4
APU_OUT_PULSE2_LENGTH       =APU_OUTPUT_BUFFER + 5
APU_OUT_PULSE2_ENVELOPE     =APU_OUTPUT_BUFFER + 6
APU_OUT_PULSE2_SWEEP        =APU_OUTPUT_BUFFER + 7
APU_OUT_TRIANGLE_TIMER      =APU_OUTPUT_BUFFER + 8
APU_OUT_TRIANGLE_LENGTH     =APU_OUTPUT_BUFFER + 9
APU_OUT_TRIANGLE_LINEAR     =APU_OUTPUT_BUFFER + 10
APU_OUT_NOISE_TIMER         =APU_OUTPUT_BUFFER + 11
APU_OUT_NOISE_LENGTH        =APU_OUTPUT_BUFFER + 12
APU_OUT_NOISE_ENVELOPE      =APU_OUTPUT_BUFFER + 13


;
;
;

.export apu_init
.export apu_update
.export apu_enable_all, apu_disable_all
.export apu_enable_dmc, apu_disable_dmc
.export _apu_play_music = apu_play_music
.export _apu_play_sfx = apu_play_sfx

.segment "LOWCODE"

;
.proc apu_init

    ; enable all channels (-DMC)
    lda #(APU_STATUS_ENABLE_PULSE1 | APU_STATUS_ENABLE_PULSE2 | APU_STATUS_ENABLE_TRIANGLE | APU_STATUS_ENABLE_NOISE )
    sta APU_STATUS_BUFF
    sta APU_STATUS

    ; set step sequence
    lda #(APU_FRAME_COUNTER_4_STEP_SEQ)
    sta APU_FRAME_COUNTER

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

; Enable pulse 1, 2, triangle, noies
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

;
.proc apu_update

    ;; copy output buffer into apu registers
    ;.repeat APU_OUTPUT_BUFFER_LENGTH, I
    ;lda APU_OUTPUT_BUFFER+I
    ;sta APU_BASE_REG+I
    ;.endrepeat

    rts

.endproc

; Play music at index A
.proc apu_play_music

.endproc

; Play SFX at index A
.proc apu_play_sfx

    ; NOTE: temp sound to see that it's working
    lda_apu_pulse_volume 0, 1, 1, 15
    sta APU_PULSE1_VOLUME

    lda_apu_pulse_sweep 1, 0, 2, 3
    sta APU_PULSE1_SWEEP

    ldax_apu_pulse_timer 46000, 10
    sta APU_PULSE1_TIMER_LO
    stx APU_PULSE1_TIMER_HI

    rts

.endproc
