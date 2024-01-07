
;
; Game State
;

.segment "ZEROPAGE"

_game_state:        .res 1
_next_game_state:   .res 1

_game_rt_timer:     .res 4

.export _game_state, _next_game_state
.export _game_rt_timer