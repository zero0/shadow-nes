
;
; gamepad
;

CTRL_PORT1          =$4016

.segment "ZEROPAGE"

    GAMEPAD_STATE:              .res 4  ; four controllers
    GAMEPAD_PREV_STATE:         .res 4  ; four controllers
    GAMEPAD_LAYER:              .res 1  ;

.define GAMEPAD_A      $01
.define GAMEPAD_B      $02
.define GAMEPAD_SELECT $04
.define GAMEPAD_START  $08
.define GAMEPAD_U      $10
.define GAMEPAD_D      $20
.define GAMEPAD_L      $40
.define GAMEPAD_R      $80

.export _gamepad_poll
.export _gamepad_state
.export _gamepad_prev_state
.export _gamepad_clear_states
.export _gamepad_layer
.export _gamepad_push_layer
.export _gamepad_pop_layer
.export _gamepad_reset_layer

.segment "LOWCODE"

; poll game pad at index A
.proc _gamepad_poll

    ; fix gamepad index to [0,3] and move to x
    and #$3
    tax

    ; store previous state
    lda GAMEPAD_STATE, x
    sta GAMEPAD_PREV_STATE, x

    ; clear state (set to 1 so when carry == 1, loop is over)
    lda #$01
    sta GAMEPAD_STATE, x

    ; strobe game pad
    sta CTRL_PORT1
    lda #0
    sta CTRL_PORT1

    ; read 8 bits from controller[x]
    :
        lda CTRL_PORT1

        ; rotate bit 0 to C
        lsr a

        ; rotate C bit back on
        rol GAMEPAD_STATE, x
        bcc :-

    rts
.endproc

; return game pad state at index A into register A
.proc _gamepad_state

    ; fix gamepad index to [0,3] and move to x
    and #$3
    tax

    ; load game pad state
    lda GAMEPAD_STATE, x

    rts
.endproc

; return game pad state at index A into register A
.proc _gamepad_prev_state

    ; fix gamepad index to [0,3] and move to x
    and #$3
    tax

    ; load game pad state
    lda GAMEPAD_PREV_STATE, x

    rts
.endproc

; clear states for pad at index A
.proc _gamepad_clear_states

    ; fix gamepad index to [0,3] and move to x
    and #$3
    tax

    ; clear pad states
    lda #0
    sta GAMEPAD_STATE, x
    sta GAMEPAD_PREV_STATE, x

    rts
.endproc

; returns the current gamepad layer
.proc _gamepad_layer

    lda GAMEPAD_LAYER
    rts
.endproc

; increment the gamepad layer and return the new value
.proc _gamepad_push_layer

    inc GAMEPAD_LAYER
    lda GAMEPAD_LAYER
    rts
.endproc

; decrement the gamepad layer
.proc _gamepad_pop_layer

    dec GAMEPAD_LAYER
    rts
.endproc

; reset gamepad layer
.proc _gamepad_reset_layer

    lda #0
    sta GAMEPAD_LAYER
    rts
.endproc
