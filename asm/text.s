;
; Text
;

.segment "ZEROPAGE"

text_delay_position:    .res 1 ;
text_delay_start_x:     .res 1 ;
text_delay_start_y:     .res 1 ;
text_delay_x:           .res 1 ;
text_delay_y:           .res 1 ;
text_delay_c_ptr:       .res 2 ;
itoa_input:             .res 4 ;

.export _text_delay_position = text_delay_position
.export _text_delay_start_x = text_delay_start_x, _text_delay_start_y = text_delay_start_y
.export _text_delay_x = text_delay_x, _text_delay_y = text_delay_y
.export _text_delay_c_ptr = text_delay_c_ptr
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
