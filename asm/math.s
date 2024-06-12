;
; Math.s
;

.pushseg
.segment "ZEROPAGE"

RAND_SEED:      .res 4

.popseg

;
;
;

.export rand_init

.export _rand_seed8 = rand_seed8, _rand_mask8 = rand_mask8, _rand_seed_mask8 = rand_seed_mask8
.export _rand_next8 = rand_next8

.export _rand_seed16 = rand_seed16
.export _rand_mask16 = rand_mask16

.pushseg
.segment "LOWCODE"

.define DEFAULT_RAND_MAGIC_8     $C3
.define DEFAULT_RAND_MAGIC_16    $5ED5

; Initialize random seeds
.proc rand_init

.if .defined(FIXED_RANDOM_SEED8)

.elseif .defined(FIXED_RANDOM_SEED16)

.else
    ; load "random" memory locations to try and get some randomness going
    ; ideally these would be uninitialized memory to get a bit more true randomness
    ;  but on some emulators, all memory is set to 0 so it doesn't matter
    ; this should be called after RAM (including zeropage) is cleared
    lda $DEAD
    sta RAND_SEED+0

    lda $BEEF
    sta RAND_SEED+1

    lda $CAFE
    sta RAND_SEED+2

    lda $BABE
    sta RAND_SEED+3
.endif

    rts

.endproc

; Load random seed from A
.proc rand_seed8

    ; store A
    sta RAND_SEED+0

    rts

.endproc

; Load random mask from A
.proc rand_mask8

    ; store A
    sta RAND_SEED+1

    rts

.endproc


; Load random seed and mask from A, X
.proc rand_seed_mask8

    ; store A
    sta RAND_SEED+0

    ; store magic byte
    stx RAND_SEED+1

    rts

.endproc

; Load random seed from [lo, hi] [A, X]
.proc rand_seed16

    ; store A and X
    sta RAND_SEED+0
    stx RAND_SEED+1

    rts

.endproc

; Load random mask from [lo, hi] [A, X]
.proc rand_mask16

    ; store A and X magic
    sta RAND_SEED+2
    stx RAND_SEED+3

    rts

.endproc

; Get next random 8bit number returning in A
; Code reference: https://codebase64.org/doku.php?id=base:small_fast_8-bit_prng
.proc rand_next8

    ; load seed
    lda RAND_SEED+0

    ; if 0, don't shift
    beq @doEOR

    ; shift
    asl

    ; if 0 or there is no carry, don't EOR
    beq @noEOR
    bcc @noEOR

@doEOR:
    eor RAND_SEED+1

@noEOR:
    ; store seed
    sta RAND_SEED+0

    rts

.endproc

; Get next random 16bit number returned in [A, X]
; Code reference: https://codebase64.org/doku.php?id=base:small_fast_16-bit_prng
.proc rand_next16

    ; load low byte
    lda RAND_SEED+0
    beq @lowZero

    asl RAND_SEED+0
    lda RAND_SEED+1
    rol
    bcc @noEOR

@doEOR:
    eor RAND_SEED+3
    sta RAND_SEED+1

    lda RAND_SEED+0
    eor RAND_SEED+2

    sta RAND_SEED+0

    jmp @end

@lowZero:
    ; load high byte and do
    lda RAND_SEED+1
    ; if 0, do EOR
    beq @doEOR

    ; otherwise, check for $80
    asl

    ; if 0, no EOR needed
    beq @noEOR
    ; otherwise, do OER based on carry flag
    bcs @doEOR

@noEOR:
    sta RAND_SEED+1

@end:
    ; load 16bit value
    lda RAND_SEED+0
    ldx RAND_SEED+1

    rts

.endproc

.popseg
