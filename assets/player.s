;
; Player sprites
;

    .export         player_skull
    .export         player_walk_body_0
    .export         player_walk_body_1
    .export         player_walk_body_2
    .export         player_generic_head

.segment        "CHARS"

player_skull:

x_sprite0 _,_,_,d,d,d,d,d
x_sprite0 _,_,d,m,m,l,l,l
x_sprite0 _,d,m,m,m,m,l,l
x_sprite0 _,d,m,m,m,m,m,m
x_sprite0 _,d,m,m,m,m,m,m
x_sprite0 _,d,m,m,m,m,d,d
x_sprite0 _,d,m,m,m,m,d,d
x_sprite0 _,d,d,m,m,m,m,l

x_sprite1 _,_,_,d,d,d,d,d
x_sprite1 _,_,d,m,m,l,l,l
x_sprite1 _,d,m,m,m,m,l,l
x_sprite1 _,d,m,m,m,m,m,m
x_sprite1 _,d,m,m,m,m,m,m
x_sprite1 _,d,m,m,m,m,d,d
x_sprite1 _,d,m,m,m,m,d,d
x_sprite1 _,d,d,m,m,m,m,l



x_sprite0 d,d,d,d,_,_,_,_
x_sprite0 l,l,l,l,d,_,_,_
x_sprite0 l,l,l,l,l,d,_,_
x_sprite0 l,l,l,l,l,d,_,_
x_sprite0 l,l,l,l,l,d,_,_
x_sprite0 d,l,l,d,d,d,_,_
x_sprite0 d,l,l,d,d,d,_,_
x_sprite0 l,l,d,l,l,d,_,_

x_sprite1 d,d,d,d,_,_,_,_
x_sprite1 l,l,l,l,d,_,_,_
x_sprite1 l,l,l,l,l,d,_,_
x_sprite1 l,l,l,l,l,d,_,_
x_sprite1 l,l,l,l,l,d,_,_
x_sprite1 d,l,l,d,d,d,_,_
x_sprite1 d,l,l,d,d,d,_,_
x_sprite1 l,l,d,l,l,d,_,_



x_sprite0 _,_,_,d,d,d,m,l
x_sprite0 _,_,_,_,_,d,m,l
x_sprite0 _,_,_,_,_,_,d,d
x_sprite0 _,_,_,_,_,_,_,_
x_sprite0 _,_,_,_,_,_,_,_
x_sprite0 _,_,_,_,_,_,_,_
x_sprite0 _,_,_,_,_,_,_,_
x_sprite0 _,_,_,_,_,_,_,_

x_sprite1 _,_,_,d,d,d,m,l
x_sprite1 _,_,_,_,_,d,m,l
x_sprite1 _,_,_,_,_,_,d,d
x_sprite1 _,_,_,_,_,_,_,_
x_sprite1 _,_,_,_,_,_,_,_
x_sprite1 _,_,_,_,_,_,_,_
x_sprite1 _,_,_,_,_,_,_,_
x_sprite1 _,_,_,_,_,_,_,_



x_sprite0 l,l,l,l,l,d,_,_
x_sprite0 d,l,l,d,l,d,_,_
x_sprite0 _,d,d,_,d,d,_,_
x_sprite0 _,_,_,_,_,_,_,_
x_sprite0 _,_,_,_,_,_,_,_
x_sprite0 _,_,_,_,_,_,_,_
x_sprite0 _,_,_,_,_,_,_,_
x_sprite0 _,_,_,_,_,_,_,_

x_sprite1 l,l,l,l,l,d,_,_
x_sprite1 d,l,l,d,l,d,_,_
x_sprite1 _,d,d,_,d,d,_,_
x_sprite1 _,_,_,_,_,_,_,_
x_sprite1 _,_,_,_,_,_,_,_
x_sprite1 _,_,_,_,_,_,_,_
x_sprite1 _,_,_,_,_,_,_,_
x_sprite1 _,_,_,_,_,_,_,_

player_walk_body_0:

x_sprite0 _,d,m,l,l,l,l,l
x_sprite0 _,d,l,l,m,l,l,l
x_sprite0 d,m,m,m,d,m,l,l
x_sprite0 d,m,m,d,d,m,l,l
x_sprite0 _,d,d,d,m,m,l,l
x_sprite0 _,_,_,d,m,l,l,d
x_sprite0 _,_,_,d,m,l,d,_
x_sprite0 _,_,_,d,d,d,_,_

x_sprite1 _,d,m,l,l,l,l,l
x_sprite1 _,d,l,l,m,l,l,l
x_sprite1 d,m,m,m,d,m,l,l
x_sprite1 d,m,m,d,d,m,l,l
x_sprite1 _,d,d,d,m,m,l,l
x_sprite1 _,_,_,d,m,l,l,d
x_sprite1 _,_,_,d,m,l,d,_
x_sprite1 _,_,_,d,d,d,_,_



x_sprite0 l,l,l,l,l,d,_,_
x_sprite0 l,l,l,l,l,d,_,_
x_sprite0 l,l,l,l,m,l,d,_
x_sprite0 l,m,l,m,m,l,d,_
x_sprite0 m,l,l,l,d,d,_,_
x_sprite0 d,l,l,m,d,_,_,_
x_sprite0 _,d,m,m,d,_,_,_
x_sprite0 _,d,d,d,_,_,_,_

x_sprite1 l,l,l,l,l,d,_,_
x_sprite1 l,l,l,l,l,d,_,_
x_sprite1 l,l,l,l,m,l,d,_
x_sprite1 l,m,l,m,m,l,d,_
x_sprite1 m,l,l,l,d,d,_,_
x_sprite1 d,l,l,m,d,_,_,_
x_sprite1 _,d,m,m,d,_,_,_
x_sprite1 _,d,d,d,_,_,_,_

player_walk_body_1:

x_sprite0 d,m,l,l,l,l,l,l
x_sprite0 d,l,l,m,l,l,l,l
x_sprite0 _,d,m,m,d,m,l,l
x_sprite0 _,d,m,m,d,m,m,m
x_sprite0 _,_,d,d,m,m,l,m
x_sprite0 _,_,d,m,m,l,d,d
x_sprite0 _,_,d,m,m,d,_,_
x_sprite0 _,_,d,d,d,_,_,_

x_sprite1 d,m,l,l,l,l,l,l
x_sprite1 d,l,l,m,l,l,l,l
x_sprite1 _,d,m,m,d,m,l,l
x_sprite1 _,d,m,m,d,m,m,m
x_sprite1 _,_,d,d,m,m,l,m
x_sprite1 _,_,d,m,m,l,d,d
x_sprite1 _,_,d,m,m,d,_,_
x_sprite1 _,_,d,d,d,_,_,_



x_sprite0 l,l,l,l,l,d,_,_
x_sprite0 l,l,l,l,l,d,_,_
x_sprite0 l,l,l,l,m,d,_,_
x_sprite0 l,m,l,m,m,d,_,_
x_sprite0 m,l,l,l,d,d,_,_
x_sprite0 d,d,l,l,l,d,_,_
x_sprite0 _,_,d,m,m,d,_,_
x_sprite0 _,_,_,d,d,d,_,_

x_sprite1 l,l,l,l,l,d,_,_
x_sprite1 l,l,l,l,l,d,_,_
x_sprite1 l,l,l,l,m,d,_,_
x_sprite1 l,m,l,m,m,d,_,_
x_sprite1 m,l,l,l,d,d,_,_
x_sprite1 d,d,l,l,l,d,_,_
x_sprite1 _,_,d,m,m,d,_,_
x_sprite1 _,_,_,d,d,d,_,_


player_walk_body_2:

x_sprite0 _,d,m,l,l,l,l,l
x_sprite0 d,m,l,l,m,l,l,l
x_sprite0 d,l,l,d,m,m,m,m
x_sprite0 m,l,m,d,m,m,m,m
x_sprite0 d,d,d,m,m,m,l,l
x_sprite0 _,_,d,m,l,l,m,d
x_sprite0 _,_,_,d,m,l,d,_
x_sprite0 _,_,_,_,d,d,d,_

x_sprite1 _,d,m,l,l,l,l,l
x_sprite1 d,m,l,l,m,l,l,l
x_sprite1 d,l,l,d,m,m,m,m
x_sprite1 m,l,m,d,m,m,m,m
x_sprite1 d,d,d,m,m,m,l,l
x_sprite1 _,_,d,m,l,l,m,d
x_sprite1 _,_,_,d,m,l,d,_
x_sprite1 _,_,_,_,d,d,d,_



x_sprite0 l,l,l,l,l,d,_,_
x_sprite0 l,l,l,l,l,m,d,_
x_sprite0 l,l,l,l,m,d,m,d
x_sprite0 l,m,l,m,m,d,m,d
x_sprite0 m,l,l,l,d,d,d,_
x_sprite0 d,m,m,l,d,_,_,_
x_sprite0 d,l,l,d,_,_,_,_
x_sprite0 d,d,d,_,_,_,_,_

x_sprite1 l,l,l,l,l,d,_,_
x_sprite1 l,l,l,l,l,m,d,_
x_sprite1 l,l,l,l,m,d,m,d
x_sprite1 l,m,l,m,m,d,m,d
x_sprite1 m,l,l,l,d,d,d,_
x_sprite1 d,m,m,l,d,_,_,_
x_sprite1 d,l,l,d,_,_,_,_
x_sprite1 d,d,d,_,_,_,_,_


player_generic_head:

x_sprite0 _,_,d,d,d,d,d,d
x_sprite0 _,d,m,m,m,m,m,m
x_sprite0 _,d,m,m,m,m,m,m
x_sprite0 _,d,m,m,m,m,m,m
x_sprite0 _,d,m,m,m,m,m,m
x_sprite0 _,d,m,m,m,m,m,m
x_sprite0 _,d,m,m,m,m,m,m
x_sprite0 _,_,d,m,m,l,l,l


x_sprite1 _,_,d,d,d,d,d,d
x_sprite1 _,d,m,m,m,m,m,m
x_sprite1 _,d,m,m,m,m,m,m
x_sprite1 _,d,m,m,m,m,m,m
x_sprite1 _,d,m,m,m,m,m,m
x_sprite1 _,d,m,m,m,m,m,m
x_sprite1 _,d,m,m,m,m,m,m
x_sprite1 _,_,d,m,m,l,l,l



x_sprite0 d,d,d,d,_,_,_,_
x_sprite0 m,m,m,m,d,_,_,_
x_sprite0 m,m,m,m,d,_,_,_
x_sprite0 m,m,m,m,d,_,_,_
x_sprite0 m,m,m,m,d,_,_,_
x_sprite0 m,m,m,m,d,_,_,_
x_sprite0 m,m,m,m,d,_,_,_
x_sprite0 l,l,l,m,d,_,_,_

x_sprite1 d,d,d,d,_,_,_,_
x_sprite1 m,m,m,m,d,_,_,_
x_sprite1 m,m,m,m,d,_,_,_
x_sprite1 m,m,m,m,d,_,_,_
x_sprite1 m,m,m,m,d,_,_,_
x_sprite1 m,m,m,m,d,_,_,_
x_sprite1 m,m,m,m,d,_,_,_
x_sprite1 l,l,l,m,d,_,_,_
