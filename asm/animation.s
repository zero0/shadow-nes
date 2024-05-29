;
; Animation
;
.segment "ZEROPAGE"

    LERP_TEMP:     .res 2;

.segment "CODE"

.macro make_funcs tmax, log2
.repeat tmax, t

.proc .ident( .concat( "_lerp_uint8_", .string(t), "_", .string(tmax) ) )
    sta LERP_TEMP+0
    stx LERP_TEMP+1

    ; (tmax - t ) * a
    lda #0

    clc
    .repeat (tmax - t)
    adc LERP_TEMP+0
    .endrepeat

    sta LERP_TEMP+0

    ; (t) * b
    lda #0

    clc
    .repeat t
    adc LERP_TEMP+1
    .endrepeat

    ; a + b
    adc LERP_TEMP+0

    rts
.endproc

.export .ident( .concat( "_lerp_uint8_", .string(t), "_", .string(tmax) ) )

;.proc concat( "_lerp_uint8_", t, "_", tmax )
;
;    rts
;.endproc

.endrepeat
.endmacro

;make_funcs 8, 3
