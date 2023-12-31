
;
; gamepad
;

CTRL_PORT1          =$4016

.segment "ZEROPAGE"

    GAMEPAD_STATE:              .res 4  ; four controllers
    GAMEPAD_PREV_STATE:         .res 4  ; four controllers

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

.segment "CODE"

; poll game pad at index A
_gamepad_poll:

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

; return game pad state at index A into register A
_gamepad_state:

    ; fix gamepad index to [0,3] and move to x
    and #$3
    tax

    ; load game pad state
    lda GAMEPAD_STATE, x

    rts

; return game pad state at index A into register A
_gamepad_prev_state:

    ; fix gamepad index to [0,3] and move to x
    and #$3
    tax

    ; load game pad state
    lda GAMEPAD_PREV_STATE, x

    rts
