;
; Text
;

.export _text_strlen
.export _text_begin_str_at
.export _text_str_at

.importzp _ARGS;
.importzp _temp_text_table;
.import _text_table;

;
;
;

.segment "CODE"

; Get the string length of A, returned in A
_text_strlen:
    ; push A to stack
    pha

    ; transfer A -> X
    tax

    ; transfer address from text_table X to temp_text_table
    lda _text_table, x
    sta _temp_text_table
    lda _text_table+1, x
    sta _temp_text_table+1

    ; pull A from stack
    pla

    ; start from #FF so first inx starts at zero
    ldy #$FF
    ; count string length
    :
        iny
        lda (_temp_text_table), y
        bne :-

    ; transfer X -> for return
    txa
    rts

; Transfer string address to temp location at A
_text_begin_str_at:
    ; transfer A -> X
    tax

    ; store text table address in temp storage
    lda _text_table, x
    sta _temp_text_table
    lda _text_table+1, x
    sta _temp_text_table+1

    rts

; Get the char of the stored string at A
_text_str_at:
    ; transfer A -> Y
    tay

    ; load index Y
    lda (_temp_text_table), y

    rts
