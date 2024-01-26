
;
; Game State
;

.segment "ZEROPAGE"

_game_state:            .res 1
_next_game_state:       .res 1
_next_game_state_arg:   .res 1
_game_state_internal:   .res 1
_game_state_timer:      .res 1

_game_rt_timer:         .res 4

.export _game_state, _next_game_state, _next_game_state_arg
.export _game_state_internal, _game_state_timer
.export _game_rt_timer
