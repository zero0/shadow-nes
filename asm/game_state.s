
;
; Game State
;

.segment "ZEROPAGE"

_game_state:        .res 1
_next_game_state:   .res 1

.export _game_state, _next_game_state
