;
; Text
;

.export _text_strlen_str
.export _text_begin_str_at
.export _text_strlen
.export _text_str_at

.importzp _ARGS;
.importzp _temp_text_table;
.import _text_table;

;
;
;

.segment "CODE"

; Get the string length of A, returned in A
.proc _text_strlen_str

    ; transfer A -> X
    tax

    ; transfer address from text_table X to temp_text_table
    lda _text_table+0, x
    sta _temp_text_table+0
    lda _text_table+1, x
    sta _temp_text_table+1

    ; load first byte of text as it's the length
    ldy #0
    lda (_temp_text_table), y

    rts
.endproc

; Transfer string address to temp location at A
.proc _text_begin_str_at

    ; transfer A -> X
    tax

    ; store text table address in temp storage
    lda _text_table+0, x
    sta _temp_text_table+0
    lda _text_table+1, x
    sta _temp_text_table+1

    rts
.endproc

; Get the length of the cached string
.proc _text_strlen

    ; load index Y
    ldy #0
    lda (_temp_text_table), y

    rts
.endproc

; Get the char of the stored string at A
.proc _text_str_at

    ; transfer A -> Y
    tay

    ; inc y since strings are 1-based
    iny

    ; load index Y
    lda (_temp_text_table), y

    rts
.endproc
