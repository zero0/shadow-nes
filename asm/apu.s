;
; APU
;

.importzp TEMP
.importzp sp

;
; Config
;

.define APU_CONFIG_MUSIC_CHANNEL_COUNT      1
.define APU_CONFIG_SFX_CHANNEL_COUNT        4

.define APU_CONFIG_USE_MMC5                 1

.if APU_CONFIG_MUSIC_CHANNEL_COUNT < 1
.error "No Music Channels. Need at least 1."
.endif
.if APU_CONFIG_MUSIC_CHANNEL_COUNT > 2
.warning "More than 2 Music Channels. Is this okay?"
.endif

.if APU_CONFIG_SFX_CHANNEL_COUNT < 1
.error "No SFX Channels. Need at least 1."
.endif
.if APU_CONFIG_SFX_CHANNEL_COUNT > 4
.warning "More than 4 SFX Channels. Is this okay?"
.endif

;
;
;

APU_BASE_REG            =$4000

APU_PULSE1              =$4000
APU_PULSE2              =$4004
APU_TRIANGLE            =$4008
APU_NOISE               =$400C
APU_DMC                 =$4010
APU_STATUS              =$4015
APU_FRAME_COUNTER       =$4017

.if APU_CONFIG_USE_MMC5
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
.endif

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
.define APU_PULSE_VOLUME_MASK           $0F

.macro lda_apu_pulse_volume    duty, inf, const, vol
    lda #(((duty & 3) << 6 ) | ((inf & 1) << 5) | ((const & 1) << 4 ) | (vol & #(APU_PULSE_VOLUME_MASK)))
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


.define APU_PULSE_REGISTER_COUNT        4
.define APU_TRIANGLE_REGISTER_COUNT     3
.define APU_NOISE_REGISTER_COUNT        3

.define APU_DMC_REGISTER_COUNT          4

.if APU_CONFIG_USE_MMC5
.define APU_NUM_PULSE_CHANNELS          4
.else
.define APU_NUM_PULSE_CHANNELS          2
.endif

.define APU_NUM_TRIANGLE_CHANNELS       1
.define APU_NUM_NOISE_CHANNELS          1

.if APU_CONFIG_USE_MMC5
.define APU_NUM_DMC_CHANNELS            1
.else
.define APU_NUM_DMC_CHANNELS            2
.endif

.define APU_OUTPUT_BUFFER_LENGTH     (APU_PULSE_REGISTER_COUNT * APU_NUM_PULSE_CHANNELS + APU_TRIANGLE_REGISTER_COUNT * APU_NUM_TRIANGLE_CHANNELS + APU_NOISE_REGISTER_COUNT * APU_NUM_NOISE_CHANNELS + APU_DMC_REGISTER_COUNT * APU_NUM_DMC_CHANNELS)

.define APU_OUTPUT_BUFFER_OFFSET_PULSE1         (0)
.define APU_OUTPUT_BUFFER_OFFSET_PULSE2         (APU_PULSE_REGISTER_COUNT)
.define APU_OUTPUT_BUFFER_OFFSET_TRIANGLE       (APU_PULSE_REGISTER_COUNT + APU_PULSE_REGISTER_COUNT)
.define APU_OUTPUT_BUFFER_OFFSET_NOISE          (APU_PULSE_REGISTER_COUNT + APU_PULSE_REGISTER_COUNT + APU_TRIANGLE_REGISTER_COUNT)
.define APU_OUTPUT_BUFFER_OFFSET_DMC            (APU_PULSE_REGISTER_COUNT + APU_PULSE_REGISTER_COUNT + APU_TRIANGLE_REGISTER_COUNT + APU_NOISE_REGISTER_COUNT)
.define APU_OUTPUT_BUFFER_OFFSET__END           (APU_PULSE_REGISTER_COUNT + APU_PULSE_REGISTER_COUNT + APU_TRIANGLE_REGISTER_COUNT + APU_NOISE_REGISTER_COUNT + APU_OUTPUT_BUFFER_OFFSET_DMC)
.if APU_CONFIG_USE_MMC5
.define APU_OUTPUT_BUFFER_OFFSET_MMC5_PULSE1    (APU_OUTPUT_BUFFER_OFFSET__END) + (0)
.define APU_OUTPUT_BUFFER_OFFSET_MMC5_PULSE2    (APU_OUTPUT_BUFFER_OFFSET__END) + (APU_PULSE_REGISTER_COUNT)
.define APU_OUTPUT_BUFFER_OFFSET_MMC5_DMC       (APU_OUTPUT_BUFFER_OFFSET__END) + (APU_PULSE_REGISTER_COUNT + APU_PULSE_REGISTER_COUNT)
.define APU_OUTPUT_BUFFER_OFFSET_MMC5__END      (APU_OUTPUT_BUFFER_OFFSET__END) + (APU_PULSE_REGISTER_COUNT + APU_PULSE_REGISTER_COUNT + APU_DMC_REGISTER_COUNT)
.endif

;
; Zero Page
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

    APU_NEXT_SFX_CHANNEL:       .res 1 ;

    _APU_TEMP:                  .res 4 ;
    _APU_ARGS:                  .res 4 ;

_APU_TEMP_PTR = _APU_TEMP
_APU_TEMP_VAR0 = _APU_TEMP+2
_APU_TEMP_VAR1 = _APU_TEMP+3

;
; BSS
;

.segment "BSS"

    APU_OUTPUT_BUFFER:          .res APU_OUTPUT_BUFFER_LENGTH ;
    APU_SFX_OUTPUT_BUFFER:      .res APU_CONFIG_SFX_CHANNEL_COUNT * APU_OUTPUT_BUFFER_LENGTH ;

    APU_SFX_TIMERS:             .res APU_CONFIG_SFX_CHANNEL_COUNT ;
    APU_SFX_OFFSET:             .res APU_CONFIG_SFX_CHANNEL_COUNT ;

    APU_CONFIG:                 .res 1

    ; Currently playing music ptr per channel
    APU_MUSIC_CHANNEL_PTR_H:    .res APU_CONFIG_MUSIC_CHANNEL_COUNT ;
    APU_MUSIC_CHANNEL_PTR_L:    .res APU_CONFIG_MUSIC_CHANNEL_COUNT ;

    ; Currently playing sfx ptr per channel
    APU_SFX_CHANNEL_PTR_H:      .res APU_CONFIG_SFX_CHANNEL_COUNT ;
    APU_SFX_CHANNEL_PTR_L:      .res APU_CONFIG_SFX_CHANNEL_COUNT ;

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

    ; store sfx pointer
    lda _APU_ARGS+0,Y
    sta APU_SFX_TABLE_PTR+0
    lda _APU_ARGS+1,Y
    sta APU_SFX_TABLE_PTR+1

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

;
; C API
;

; Update APU mixer
.proc apu_update_mixer

    ; process music channels
.repeat APU_CONFIG_MUSIC_CHANNEL_COUNT, I
    ldx #(I)
    jsr _apu_process_music_channel
.endrepeat

    ; process sfx channels
.repeat APU_CONFIG_SFX_CHANNEL_COUNT, I
    ldx #(I)
    jsr _apu_process_sfx_channel
.endrepeat

    rts

.endproc

; Play music at index A (at channel X (0 for now))
.proc apu_play_music

    ; convert index to offset Y
    asl
    tay

    ; music channel
    ldx #0

    ; clear music
    jsr _apu_clear_music_channel

    ; load music ptr at Y into music channel X
    lda (APU_MUSIC_TABLE_PTR), Y
    sta APU_MUSIC_CHANNEL_PTR_H, X

    iny

    lda (APU_MUSIC_TABLE_PTR), Y
    sta APU_MUSIC_CHANNEL_PTR_L, X

    rts

.endproc

;
.proc apu_play_sfx

; if there are more than 1 sfx channels, calculate which channel to use next
.if APU_CONFIG_SFX_CHANNEL_COUNT > 1
    ; transfer A -> Y
    tay

    ; load channel
    lda APU_NEXT_SFX_CHANNEL

    ; increment next channel
    inc APU_NEXT_SFX_CHANNEL

; for pow2 number of channels, mask out the channels
.if (APU_CONFIG_SFX_CHANNEL_COUNT & (APU_CONFIG_SFX_CHANNEL_COUNT - 1)) = 0
    ; mask channel count
    and #(APU_CONFIG_SFX_CHANNEL_COUNT - 1)
; otherwise,
.else
    ; compare to the number of channels
    cmp #(APU_CONFIG_SFX_CHANNEL_COUNT)

    ; if the value is less than the number of channels, skip
    bcs :+
        ; reset when the value is greater than the number of channels
        lda #0
        sta APU_NEXT_SFX_CHANNEL
        :
.endif

    ; transfer channel A -> X
    tax

    ; transfer Y -> A restoring A sfx index
    tya

; otherwise, use channel 0
.else
    ldx #0
.endif

    ; jmp to play sfx with channle (it will return)
    jmp apu_play_sfx_channel

.endproc

; Play SFX at index A to channel X
.proc apu_play_sfx_channel

    ; convert index to offset Y
    asl
    tay

    ; clear channel at X
    jsr _apu_clear_sfx_channel

    ; load table ptr from Y and store in channel at X
    lda (APU_SFX_TABLE_PTR), Y
    sta APU_SFX_CHANNEL_PTR_L, X

    iny

    lda (APU_SFX_TABLE_PTR), Y
    sta APU_SFX_CHANNEL_PTR_H, X


.if 0
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
.endif

    rts

.endproc


;
; Internal
;

; Clear the music channel at X
.proc _apu_clear_music_channel

    lda #0
    sta APU_MUSIC_CHANNEL_PTR_H, X
    sta APU_MUSIC_CHANNEL_PTR_L, X

    rts

.endproc

; Clear the SFX channel at X
.proc _apu_clear_sfx_channel

    lda #0
    sta APU_SFX_CHANNEL_PTR_H, X
    sta APU_SFX_CHANNEL_PTR_L, X
    sta APU_SFX_TIMERS, X
    sta APU_SFX_OFFSET, X

    rts

.endproc

; Process the music channel in X
.proc _apu_process_music_channel

    rts

.endproc

; Process the sfx channel in X
.proc _apu_process_sfx_channel

    ; load channel timer
    lda APU_SFX_TIMERS, X

    ; if the timer is done, check for more data
    beq @timer_done

    ; decrement timer
    dec APU_SFX_TIMERS, X

    ; if there is still time left, just update the buffer
    bne @update_buffer

@timer_done:
    ; load high ptr value
    lda APU_SFX_CHANNEL_PTR_H, X

    ; if there is a ptr, continue
    bne :+
        ; otherwise, return when high ptr is 0
        rts
        :

    ; store ptr in zeropage tmp
    sta _APU_TEMP_PTR+0
    lda APU_SFX_CHANNEL_PTR_L, X
    sta _APU_TEMP_PTR+1

    ; load offset for channel into Y
    ldy APU_SFX_OFFSET, X

@read_byte:
    ; read byte from ptr stream
    lda (_APU_TEMP_PTR), Y

    ; increment Y to next byte
    iny

    ; if zero is read, it's an end of stream
    beq @eos

    ; if 7bit set, it's a register write
    bmi @write_register

    ; otherwise, it's a frame skip, store in timer for channel X
    sta APU_SFX_TIMERS, X

    ; end of data, go to updating the buffer
    jmp @update_buffer

@write_register:

    ; store channel into temp
    stx _APU_TEMP_VAR0

    ; mask out the high bit of value to get register offset
    and #$7F

    ; transfer A register offset -> X
    tax

    ; read byte from ptr stream
    lda (_APU_TEMP_PTR), Y

    ; increment Y to next byte
    iny

    ; write byte to sfx buffer at register offset X
    sta APU_SFX_OUTPUT_BUFFER, X

    ; restore channel
    ldx _APU_TEMP_VAR0

    ; read the next byte
    jmp @read_byte

@eos
    ; end of stream, mark ptr as invalid (A is already 0)
    sta APU_SFX_CHANNEL_PTR_H, X
    sta APU_SFX_CHANNEL_PTR_L, X

@update_buffer:

    ; store offset Y for channel
    sty APU_SFX_OFFSET, X

    ; load existing output buffer pulse 1 volume
    lda APU_OUTPUT_BUFFER + APU_OUTPUT_BUFFER_OFFSET_PULSE1
    and #(APU_PULSE_VOLUME_MASK)
    sta _APU_TEMP_VAR0

    lda APU_SFX_OUTPUT_BUFFER + APU_OUTPUT_BUFFER_OFFSET_PULSE1



    lda APU_SFX_OUTPUT_BUFFER + APU_OUTPUT_BUFFER_OFFSET_PULSE2, X
    lda APU_SFX_OUTPUT_BUFFER + APU_OUTPUT_BUFFER_OFFSET_TRIANGLE, X
    lda APU_SFX_OUTPUT_BUFFER + APU_OUTPUT_BUFFER_OFFSET_NOISE, X
    lda APU_SFX_OUTPUT_BUFFER + APU_OUTPUT_BUFFER_OFFSET_DMC, X


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
; 0000 0010 - use dmc (next byte is index to play)
; 0000 0001 - skip N frames (next byte)
; 0000 0000 - end of stream

; pulse 4 bytes
; ddic vvvv = duty inf const volume
; eppp nsss = enable period neg shift
; pppp pppp llll lPPP = period length

; triangle 3 bytes

; noise 3 bytes
