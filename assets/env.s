;
; Environment Sprites
;

    .export         brick_top_endcap
    .export         brick_top_middle
    .export         stone_block_wall

.segment        "CHARS"

brick_top_endcap:

x_sprite0 d,d,d,d,d,d,d,d
x_sprite0 d,l,l,l,l,m,l,l
x_sprite0 d,m,l,l,m,m,l,l
x_sprite0 d,m,m,m,m,m,l,l
x_sprite0 d,l,l,m,l,l,m,m
x_sprite0 d,d,d,d,l,l,m,l
x_sprite0 _,_,_,d,d,d,d,l
x_sprite0 _,_,_,_,_,_,d,d

x_sprite1 d,d,d,d,d,d,d,d
x_sprite1 d,l,l,l,l,m,l,l
x_sprite1 d,m,l,l,m,m,l,l
x_sprite1 d,m,m,m,m,m,l,l
x_sprite1 d,l,l,m,l,l,m,m
x_sprite1 d,d,d,d,l,l,m,l
x_sprite1 _,_,_,d,d,d,d,l
x_sprite1 _,_,_,_,_,_,d,d

brick_top_middle:

x_sprite0 d,d,d,d,d,d,d,d
x_sprite0 l,l,l,l,l,m,l,l
x_sprite0 m,m,l,l,m,m,l,l
x_sprite0 l,m,m,m,m,m,l,l
x_sprite0 l,l,l,m,l,l,m,m
x_sprite0 d,d,d,d,l,l,m,l
x_sprite0 _,_,_,d,d,d,d,l
x_sprite0 _,_,_,_,_,_,d,d

x_sprite1 d,d,d,d,d,d,d,d
x_sprite1 l,l,l,l,l,m,l,l
x_sprite1 m,m,l,l,m,m,l,l
x_sprite1 l,m,m,m,m,m,l,l
x_sprite1 l,l,l,m,l,l,m,m
x_sprite1 d,d,d,d,l,l,m,l
x_sprite1 _,_,_,d,d,d,d,l
x_sprite1 _,_,_,_,_,_,d,d

stone_block_wall:

x_sprite0 l,l,l,m,l,l,l,l
x_sprite0 l,l,l,m,l,l,l,l
x_sprite0 l,l,l,m,l,l,l,l
x_sprite0 m,m,m,m,m,m,m,m
x_sprite0 l,l,l,l,l,l,l,m
x_sprite0 l,l,l,l,l,l,l,m
x_sprite0 l,l,l,l,l,l,l,m
x_sprite0 m,m,m,m,m,m,m,m

x_sprite1 l,l,l,m,l,l,l,l
x_sprite1 l,l,l,m,l,l,l,l
x_sprite1 l,l,l,m,l,l,l,l
x_sprite1 m,m,m,m,m,m,m,m
x_sprite1 l,l,l,l,l,l,l,m
x_sprite1 l,l,l,l,l,l,l,m
x_sprite1 l,l,l,l,l,l,l,m
x_sprite1 m,m,m,m,m,m,m,m
