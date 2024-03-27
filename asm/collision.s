
;
; Collision
;

.segment "ZEROPAGE"

_test_collision_lh:  .res 4
_test_collision_rh:  .res 4

.export _test_collision_lh
.export _test_collision_rh

;
;
;

.segment "CODE"

.define AABB_LEFT    0
.define AABB_TOP     1
.define AABB_RIGHT   2
.define AABB_BOTTOM  3

.export _perform_collision_test

; TODO: use `adc` to accumulate carry bit instead?
; returns in A the collision test of _test_collision_lh vs _test_collision_lh
.proc _perform_collision_test

    ; count number of passed tests
    ldx #0

    ; lh.left > rh.right ? x++
    lda _test_collision_lh + AABB_LEFT
    cmp _test_collision_rh + AABB_RIGHT
    bcc :+
        inx
        :

    ; lh.right < rh.left ? x++
    lda _test_collision_lh + AABB_RIGHT
    cmp _test_collision_rh + AABB_LEFT
    bcs :+
        inx
        :

    ; lh.bottom < rh.top ? x++
    lda _test_collision_lh + AABB_BOTTOM
    cmp _test_collision_rh + AABB_TOP
    bcs :+
        inx
        :

    ; lh.top > rh.bottom ? x++
    lda _test_collision_lh + AABB_TOP
    cmp _test_collision_rh + AABB_BOTTOM
    bcc :+
        inx
        :

    ; if all tests failed, it's a hit; otherwise it's a miss
    cpx #0
    beq @hit

@miss:
    lda #0
    rts

@hit:
    lda #1
    rts

.endproc
