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

.export _text_delay_position = text_delay_position

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

