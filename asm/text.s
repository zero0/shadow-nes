;
; Text
;

.export _text_delay_start = text_delay_start
.export _text_delay_advance = text_delay_advance
.export _text_delay_display_full = text_delay_display_full

;
;
;

.segment "ZEROPAGE"

text_delay_position:    .res 1 ;
itoa_input:             .res 4 ;

.export _text_delay_position = text_delay_position
.export _itoa_input = itoa_input

.export _itoa_uint8_impl = itoa_uint8_impl
.export _itoa_uint16_impl = itoa_uint16_impl
.export _itoa_uint32_impl = itoa_uint32_impl

;
;
;

.segment "BSS"

bcd:   .res 5

.export _bcd = bcd

;
;
;

.segment "CODE"

.proc text_delay_start

    lda #1
    sta text_delay_position

    rts

.endproc

.proc text_delay_advance

    lda text_delay_position
    cmp #$FF
    beq :+
        inc text_delay_position
    :

    rts

.endproc

.proc text_delay_display_full

    lda #$FF
    sta text_delay_position

    rts

.endproc

.macro double_dabble_base bitcount, max_num_count

    ; clear bcd
    lda #0
    .repeat 5, I ;((max_num_count+1)/2), I
    sta bcd+I
    .endrepeat

    ; loop for each bit count
    ldy #(bitcount)
@loop:
        ; compare and add block
        .repeat max_num_count, I

            ; load shift
            lda bcd+(I / 2)

            ; cache A in X
            tax

            ; mask and compare
.if (I & 1) = 0
            and #$0F
            cmp #$05
.else
            and #$F0
            cmp #$50
.endif

            ; if less, skip add
            bcc :+

                ; transfer back X -> A
                txa

                ; clear carry
                clc

                ; add value based on shift
.if (I & 1) = 0
                adc #$03
.else
                adc #$30
.endif
                ; store updated value
                sta bcd+(I / 2)
            :

        .endrepeat

        ; clear carry
        clc

        ; rotate the values up in reverse order
        .repeat (bitcount/8), I

            rol itoa_input+(((bitcount/8)-1)-I)

        .endrepeat

        ; roll carry from ones -> tens -> etc.
        .repeat (max_num_count-1), I

            rol bcd+I

        .endrepeat

        ; decrement counter
        dey

        ; use branch for smaller bit counts, otherwise use jmp
.if bitcount < 32
        bne @loop
.else
        beq :+
            jmp @loop
        :
.endif

.endmacro

;
.proc itoa_uint8_impl

    double_dabble_base 8, 3

    rts

.endproc

;
.proc itoa_uint16_impl

    double_dabble_base 16, 5

    rts

.endproc

;
.proc itoa_uint32_impl

    double_dabble_base 32, 10

    rts

.endproc
