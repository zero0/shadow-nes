;
; Timers
;

.export _request_timer = request_timer
.export _release_timer = release_timer
.export _tick_timers = tick_timers
.export _init_timers = init_timers
.export _g_timers = g_timers

.segment "ZEROPAGE"

g_timers:               .res 8


.segment "BSS"

g_timer_count:          .res 1;
g_timer_free_count:     .res 1;
g_timer_free_list:      .res 8;

;
;
;

.segment "CODE"

.proc init_timers

    lda #0

    ; clear counts
    sta g_timer_count
    sta g_timer_free_count

    ; clear free lists
.repeat 8, I
    sta g_timer_free_list+I
.endrepeat

    ; clear timers
.repeat 8, I
    sta g_timers+I
.endrepeat

    rts

.endproc

; tick all timers
.proc tick_timers

.repeat 8, I
    ldx g_timers+I
    beq :+
        dex
        stx g_timers+I
        :
.endrepeat

    rts

.endproc

; request a new timer with initial ticks A
.proc request_timer

    ; push A
    pha

    ; load free count
    ldx g_timer_free_count
    beq @no_free

    ; dec
    dex

    ; load timer handle
    lda g_timer_free_list, x

    ; transfer A -> X
    tax
    jmp @store_ticks

@no_free:
    ; load timer count -> X
    ldx g_timer_count

@store_ticks:
    ; increment timer
    inc g_timer_count

    ; reload "ticks"
    pla

    ; store ticks
    sta g_timers, x

    ; transfer X (handle) -> A
    txa

    ; return
    rts

.endproc

; release the timer handle A
.proc release_timer

    ; transfer A (handle) -> X
    tax

    ; clear timer
    lda #0
    sta g_timers, x

    ; transfer X (handle) -> A
    txa

    ; store handle into free list
    ldx g_timer_free_count
    sta g_timer_free_list, x

    ; increment free count
    inc g_timer_free_count

    ; decrement timer count
    dec g_timer_count

    rts

.endproc
