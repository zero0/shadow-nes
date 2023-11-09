;
; Sprites
;

.macro x_sprite x0, x1, x2, x3, x4, x5, x6, x7
.byte x0 | x1 | x2 | x3 | x4 | x5 | x6 | x7
.endmacro

.macro x_sprite0 x0, x1, x2, x3, x4, x5, x6, x7
x_sprite ( ( x0 & $1 ) << 7 ), ( ( x1 & $1 ) << 6 ), ( ( x2 & $1 ) << 5 ), ( ( x3 & $1 ) << 4 ), ( ( x4 & $1 ) << 3 ), ( ( x5 & $1 ) << 2 ), ( ( x6 & $1 ) << 1, ( ( x7 & $1 ) << 0 ) )
.endmacro

.macro x_sprite1 x0, x1, x2, x3, x4, x5, x6, x7
x_sprite ( ( x0 & $2 ) << 6 ), ( ( x1 & $2 ) << 5 ), ( ( x2 & $2 ) << 4 ), ( ( x3 & $2 ) << 3 ), ( ( x4 & $2 ) << 2 ), ( ( x5 & $2 ) << 1 ), ( ( x6 & $2 ) << 0, ( ( x7 & $2 ) >> 1 ) )
.endmacro

.define _ 0
.define l 3
.define m 2
.define d 1

.include "../assets/nesfont.s"

.include "../assets/player.s"

.include "../assets/env.s"
