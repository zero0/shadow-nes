
;
; gamepad
;

CTRL_PORT1          =$4016
CTRL_PORT2          =$4017

.define NUM_GAMEPADS    2

.if NUM_GAMEPADS < 1
.error "Too few game pads defined. At least 1 is required."
.endif
.if NUM_GAMEPADS > 4
.warning "More than 4 game pads. Is this okay?"
.endif


.segment "ZEROPAGE"

    GAMEPAD_STATE:              .res NUM_GAMEPADS  ; four controllers
    GAMEPAD_PREV_STATE:         .res NUM_GAMEPADS  ; four controllers
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

; poll all game pads
.proc _gamepad_poll

.repeat NUM_GAMEPADS, I

    ; store previous state
    lda GAMEPAD_STATE+I
    sta GAMEPAD_PREV_STATE+I

    ; clear state (set to 1 so when carry == 1, loop is over)
    lda #$01
    sta GAMEPAD_STATE+I

    ; strobe game pad (using A = 1)
    sta CTRL_PORT1

    ; reset to enter serial mode
    lda #0
    sta CTRL_PORT1

    ; read 8 bits from controller I
    :
        lda CTRL_PORT1+I

        ; rotate bit 0 to C
        lsr a

        ; rotate C bit back on
        rol GAMEPAD_STATE+I
        bcc :-

.endrepeat

    rts

.endproc

; return game pad state at index A into register A
.proc _gamepad_state

.if NUM_GAMEPADS > 1

    ; fix gamepad index to [0,3] and move to x
.if (NUM_GAMEPADS & (NUM_GAMEPADS - 1)) = 0

    ; mask out for pow2 number of controllers
    and #(NUM_GAMEPADS - 1)

.else
    ; compare index to max controllers
    cmp #(NUM_GAMEPADS)

    ; if the index is less than the number of game pads, skip
    bcs :+
        ; clamp to end
        sta #(NUM_GAMEPADS - 1)
        :
.endif

    ; transfer index A -> offset X
    tax

    ; load game pad state
    lda GAMEPAD_STATE, x

.else

    ; load game pad state
    lda GAMEPAD_STATE

.endif

    rts
.endproc

; return game pad state at index A into register A
.proc _gamepad_prev_state


.if NUM_GAMEPADS > 1

    ; fix gamepad index to num supported game pads
.if (NUM_GAMEPADS & (NUM_GAMEPADS - 1)) = 0

    ; mask out for pow2 number of controllers
    and #(NUM_GAMEPADS - 1)

.else
    ; compare index to max controllers
    cmp #(NUM_GAMEPADS)

    ; if the index is less than the number of game pads, skip
    bcs :+
        ; clamp to end
        sta #(NUM_GAMEPADS - 1)
        :
.endif

    ; transfer index A -> offset X
    tax

    ; load game pad state
    lda GAMEPAD_PREV_STATE, x

.else

    ; load game pad state
    lda GAMEPAD_PREV_STATE

.endif

    rts

.endproc

; clear states for pad at index A
.proc _gamepad_clear_states

.if NUM_GAMEPADS > 1

    ; fix gamepad index to num supported game pads
.if (NUM_GAMEPADS & (NUM_GAMEPADS - 1)) = 0

    ; mask out for pow2 number of controllers
    and #(NUM_GAMEPADS - 1)

.else
    ; compare index to max controllers
    cmp #(NUM_GAMEPADS)

    ; if the index is less than the number of game pads, skip
    bcs :+
        ; clamp to end
        sta #(NUM_GAMEPADS - 1)
        :
.endif

    ; transfer index A -> offset X
    tax

    ; clear pad states
    lda #0
    sta GAMEPAD_STATE, x
    sta GAMEPAD_PREV_STATE, x

.else

    ; clear pad states
    lda #0
    sta GAMEPAD_STATE
    sta GAMEPAD_PREV_STATE

.endif

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
