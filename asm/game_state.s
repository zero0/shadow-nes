
;
; Game State
;

.segment "ZEROPAGE"

_game_state:                .res 1
_next_game_state:           .res 1
_next_game_state_arg:       .res 1
_next_game_state_direction: .res 1
_game_state_internal:       .res 1
_game_state_timer:          .res 1

_game_rt_timer:             .res 4

.export _game_state, _next_game_state, _next_game_state_arg, _next_game_state_direction
.export _game_state_internal, _game_state_timer
.export _game_rt_timer

;
; IRQs
;

.segment "RODATA"

.export IRQ_SCANLINE_TABLE
.export IRQ_STATE_TABLE

IRQ_SCANLINE_TABLE:
    .byte 0

IRQ_STATE_TABLE:
    .addr IrqExt

.segment "LOWCODE"

;
.proc IrqExt

    rts

.endproc
