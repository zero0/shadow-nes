;
; Generated from knight.png
;

; Palette 0
;  0: [255, 255, 255]  #FFFFFF
;  1: [0  , 0  , 0  ]  #000000
;  2: [85 , 85 , 85 ]  #555555
;  3: [170, 170, 170]  #AAAAAA

; Tiles
;     All: 81
;  Unique: 78
;    Size: 1,248b

.export _knight

.segment "RODATA"

_knight:

; Tile 0 24x8
knight_24x8:
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000

.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000011

; Tile 1 32x8
knight_32x8:
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000011
.byte %00000000
.byte %00000000

.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000110
.byte %11111100

; Tile 2 24x16
knight_24x16:
.byte %00001000
.byte %00000001
.byte %00010101
.byte %00010011
.byte %00001011
.byte %00001010
.byte %00001010
.byte %00000001

.byte %00000111
.byte %00001111
.byte %00001011
.byte %00001111
.byte %00010111
.byte %00010111
.byte %00010111
.byte %00011110

; Tile 3 32x16
knight_32x16:
.byte %00000010
.byte %01001001
.byte %00101101
.byte %00101101
.byte %10101100
.byte %10101101
.byte %10011101
.byte %00111101

.byte %11111100
.byte %10110110
.byte %11110010
.byte %11110010
.byte %01110011
.byte %01110010
.byte %01100110
.byte %11000110

; Tile 4 40x16
knight_40x16:
.byte %00000000
.byte %00000000
.byte %00000000
.byte %10000000
.byte %10000000
.byte %10000000
.byte %10000000
.byte %10000000

.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000

; Tile 5 16x24
knight_16x24:
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000110
.byte %00010100
.byte %00111011
.byte %01110001

.byte %00000000
.byte %00000000
.byte %00000001
.byte %00000011
.byte %00001011
.byte %00001111
.byte %00000110
.byte %00001111

; Tile 6 24x24
knight_24x24:
.byte %00010010
.byte %00000100
.byte %11111000
.byte %11111100
.byte %11000000
.byte %01100000
.byte %11011100
.byte %10011110

.byte %00001111
.byte %01111011
.byte %00000111
.byte %00001111
.byte %00110111
.byte %11111111
.byte %11100011
.byte %01100001

; Tile 7 32x24
knight_32x24:
.byte %01101101
.byte %10001100
.byte %00011110
.byte %10000010
.byte %00000001
.byte %00000001
.byte %00100101
.byte %01101100

.byte %10011110
.byte %01110010
.byte %11100000
.byte %01111100
.byte %11111110
.byte %11111110
.byte %11111010
.byte %10111011

; Tile 8 40x24
knight_40x24:
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %10000000
.byte %01000000
.byte %00000000
.byte %00100000

.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %01000000
.byte %10000000
.byte %11100000
.byte %11000000

; Tile 9 16x32
knight_16x32:
.byte %01100010
.byte %01110111
.byte %00111110
.byte %00111111
.byte %00000011
.byte %00000011
.byte %00000010
.byte %00000011

.byte %00011101
.byte %00001000
.byte %01000001
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000001
.byte %00000000

; Tile 10 24x32
knight_24x32:
.byte %01111100
.byte %01011000
.byte %10111111
.byte %11111100
.byte %11011100
.byte %11111111
.byte %01011111
.byte %00001111

.byte %10000011
.byte %10000111
.byte %00000000
.byte %00000011
.byte %00100011
.byte %00000000
.byte %10100000
.byte %11110000

; Tile 11 32x32
knight_32x32:
.byte %01001000
.byte %00010101
.byte %00000101
.byte %00000100
.byte %00100110
.byte %00000000
.byte %00010011
.byte %10000011

.byte %10111111
.byte %11111110
.byte %11111110
.byte %11111111
.byte %11011111
.byte %11111101
.byte %11101111
.byte %01111111

; Tile 12 40x32
knight_40x32:
.byte %01000000
.byte %10000000
.byte %10000000
.byte %01000000
.byte %00000000
.byte %10000000
.byte %00000000
.byte %01000000

.byte %10000000
.byte %01000000
.byte %11000000
.byte %10000000
.byte %11000000
.byte %00000000
.byte %10000000
.byte %10000000

; Tile 13 16x40
knight_16x40:
.byte %00000001
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000001
.byte %00000011
.byte %00000111
.byte %00001101

.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000011

; Tile 14 24x40
knight_24x40:
.byte %01110001
.byte %10010011
.byte %01100110
.byte %11111010
.byte %01011110
.byte %11000011
.byte %11100001
.byte %11101110

.byte %10001111
.byte %00001111
.byte %00011110
.byte %00000110
.byte %00100001
.byte %01000100
.byte %00100010
.byte %00011000

; Tile 15 32x40
knight_32x40:
.byte %00000001
.byte %10010001
.byte %00001001
.byte %10010011
.byte %00000010
.byte %10000100
.byte %00001101
.byte %00001001

.byte %11111111
.byte %11101111
.byte %01110111
.byte %11101101
.byte %11111101
.byte %01111011
.byte %01110010
.byte %01110110

; Tile 16 40x40
knight_40x40:
.byte %10000000
.byte %10000000
.byte %11100000
.byte %11000000
.byte %11110000
.byte %11101000
.byte %01110000
.byte %01110100

.byte %11000000
.byte %11000000
.byte %11000000
.byte %11100000
.byte %11100000
.byte %11110000
.byte %11111000
.byte %11111000

; Tile 17 16x48
knight_16x48:
.byte %00001011
.byte %00000111
.byte %00001101
.byte %00001101
.byte %00011011
.byte %00010011
.byte %01110011
.byte %00010011

.byte %00000111
.byte %00001111
.byte %00011101
.byte %00011101
.byte %00111011
.byte %00110011
.byte %00110011
.byte %01110011

; Tile 18 24x48
knight_24x48:
.byte %01111100
.byte %00111000
.byte %10111000
.byte %10010000
.byte %10100000
.byte %10100000
.byte %01000000
.byte %01000000

.byte %10000000
.byte %11000000
.byte %11000000
.byte %11100000
.byte %11000000
.byte %11000000
.byte %10000000
.byte %10000000

; Tile 19 32x48
knight_32x48:
.byte %00001000
.byte %00001010
.byte %00001110
.byte %00000110
.byte %00000101
.byte %00000101
.byte %00010001
.byte %00000001

.byte %01110111
.byte %01110101
.byte %01110001
.byte %01111001
.byte %01111010
.byte %01111010
.byte %01100010
.byte %01100000

; Tile 20 40x48
knight_40x48:
.byte %10111010
.byte %10111000
.byte %11111000
.byte %11011001
.byte %01011011
.byte %01101111
.byte %01101111
.byte %00111111

.byte %01111100
.byte %01111100
.byte %00111010
.byte %00111010
.byte %10111100
.byte %10011000
.byte %10010000
.byte %11000000

; Tile 21 8x56
knight_8x56:
.byte %00000000
.byte %00000000
.byte %00000011
.byte %00000111
.byte %00001001
.byte %00001000
.byte %00001111
.byte %00000011

.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000110
.byte %00000111
.byte %00000000
.byte %00000000

; Tile 22 16x56
knight_16x56:
.byte %01011111
.byte %00111100
.byte %00100101
.byte %11100111
.byte %11000010
.byte %11001011
.byte %11111111
.byte %11111110

.byte %00111111
.byte %01001111
.byte %00011010
.byte %00011000
.byte %00111100
.byte %00111100
.byte %00000000
.byte %00000000

; Tile 23 24x56
knight_24x56:
.byte %01000000
.byte %10000000
.byte %10000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000

.byte %10000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000

; Tile 24 32x56
knight_32x56:
.byte %01000001
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000

.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000

; Tile 25 40x56
knight_40x56:
.byte %10111110
.byte %11111111
.byte %01111111
.byte %01111111
.byte %01100111
.byte %01111111
.byte %01111110
.byte %00000000

.byte %01000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00011000
.byte %00000000
.byte %00000000
.byte %00000000

; Tile 26 48x56
knight_48x56:
.byte %00000000
.byte %11000000
.byte %11100000
.byte %11100000
.byte %11100000
.byte %11100000
.byte %00000000
.byte %00000000

.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000

; Tile 27 32x72
knight_32x72:
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000

.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000010
.byte %11010100

; Tile 28 24x80
knight_24x80:
.byte %00000000
.byte %00000000
.byte %00000001
.byte %00010101
.byte %00000111
.byte %00001111
.byte %00001010
.byte %00001010

.byte %00000111
.byte %00001111
.byte %00001111
.byte %00001011
.byte %00011011
.byte %00010011
.byte %00010111
.byte %00010111

; Tile 29 32x80
knight_32x80:
.byte %00000000
.byte %00000010
.byte %00100001
.byte %00101101
.byte %00101101
.byte %10101101
.byte %10001001
.byte %00011101

.byte %11111100
.byte %11111100
.byte %11111110
.byte %11110010
.byte %11110010
.byte %01110010
.byte %01110110
.byte %11100110

; Tile 30 40x80
knight_40x80:
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %10000000
.byte %10000000
.byte %10000000
.byte %10000000

.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000

; Tile 31 16x88
knight_16x88:
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000001
.byte %00000100
.byte %00010001
.byte %00110010

.byte %00000000
.byte %00000000
.byte %00000001
.byte %00000001
.byte %00000110
.byte %00011011
.byte %00001010
.byte %01001111

; Tile 32 24x88
knight_24x88:
.byte %00010001
.byte %00110000
.byte %01101100
.byte %11110000
.byte %11000110
.byte %01101000
.byte %01101101
.byte %11011110

.byte %00001110
.byte %01001111
.byte %10010011
.byte %00001111
.byte %00110111
.byte %11111111
.byte %01110011
.byte %11100001

; Tile 33 32x88
knight_32x88:
.byte %00110101
.byte %11011001
.byte %10001110
.byte %00001110
.byte %00001011
.byte %00100001
.byte %01000001
.byte %00010010

.byte %11001110
.byte %00111110
.byte %01110000
.byte %11110000
.byte %11110100
.byte %11111110
.byte %10111110
.byte %11111101

; Tile 34 40x88
knight_40x88:
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %10000000
.byte %00000000
.byte %00000000
.byte %00100000

.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %11000000
.byte %11000000
.byte %11000000

; Tile 35 16x96
knight_16x96:
.byte %01100001
.byte %01100011
.byte %00110110
.byte %01111111
.byte %00001011
.byte %00000011
.byte %00000011
.byte %00000010

.byte %00011111
.byte %00011100
.byte %01001001
.byte %00000000
.byte %00010000
.byte %00000000
.byte %00000000
.byte %00000001

; Tile 36 24x96
knight_24x96:
.byte %10111100
.byte %01011100
.byte %01011000
.byte %11111111
.byte %11111101
.byte %10011110
.byte %11111111
.byte %00001111

.byte %01000011
.byte %10100011
.byte %10000111
.byte %00000000
.byte %00000011
.byte %01100001
.byte %00000000
.byte %11110000

; Tile 37 32x96
knight_32x96:
.byte %00010000
.byte %00100000
.byte %00001000
.byte %00000000
.byte %10010000
.byte %00001100
.byte %10001000
.byte %11000010

.byte %11111111
.byte %11011111
.byte %11111111
.byte %11111111
.byte %01101111
.byte %11110111
.byte %01110111
.byte %00111111

; Tile 38 40x96
knight_40x96:
.byte %00100000
.byte %10000000
.byte %00000000
.byte %01000000
.byte %00000000
.byte %01000000
.byte %00100000
.byte %00010000

.byte %11000000
.byte %01000000
.byte %11000000
.byte %10000000
.byte %11000000
.byte %10100000
.byte %11010000
.byte %11101000

; Tile 39 16x104
knight_16x104:
.byte %00000001
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000011
.byte %00000111
.byte %00001101

.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000010

; Tile 40 24x104
knight_24x104:
.byte %10011111
.byte %00010001
.byte %00100011
.byte %11100110
.byte %11111110
.byte %11101111
.byte %11100011
.byte %11100110

.byte %01100000
.byte %10001111
.byte %00011111
.byte %00011110
.byte %10000010
.byte %01110000
.byte %01100000
.byte %00100100

; Tile 41 32x104
knight_32x104:
.byte %11000100
.byte %10000000
.byte %10001000
.byte %11001010
.byte %11001010
.byte %11001010
.byte %11001100
.byte %00001001

.byte %00111011
.byte %11111111
.byte %11110111
.byte %11110101
.byte %11110101
.byte %00110101
.byte %00110111
.byte %01110110

; Tile 42 40x104
knight_40x104:
.byte %00001000
.byte %00000100
.byte %00000010
.byte %10000110
.byte %10000001
.byte %11000011
.byte %11000000
.byte %11000000

.byte %11110100
.byte %11111010
.byte %11111101
.byte %11111101
.byte %11111110
.byte %11111110
.byte %11111111
.byte %11111111

; Tile 43 48x104
knight_48x104:
.byte %00000000
.byte %00000000
.byte %00000000
.byte %10000000
.byte %00000000
.byte %11000000
.byte %10000000
.byte %11000000

.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %10000000
.byte %00000000
.byte %01000000
.byte %00000000

; Tile 44 16x112
knight_16x112:
.byte %00011001
.byte %00000111
.byte %00001101
.byte %00001011
.byte %00011011
.byte %00010011
.byte %00110011
.byte %00010011

.byte %00000111
.byte %00001111
.byte %00011101
.byte %00111011
.byte %00111011
.byte %00110011
.byte %01110011
.byte %01110011

; Tile 45 24x112
knight_24x112:
.byte %11111110
.byte %01111000
.byte %10111000
.byte %10110000
.byte %10100000
.byte %10000000
.byte %01000000
.byte %01000000

.byte %00000000
.byte %10000000
.byte %11000000
.byte %11000000
.byte %11000000
.byte %11000000
.byte %10000000
.byte %10000000

; Tile 46 32x112
knight_32x112:
.byte %00000101
.byte %00000111
.byte %00001001
.byte %10000001
.byte %10000001
.byte %00000001
.byte %00000001
.byte %00000001

.byte %01111110
.byte %01111000
.byte %01110000
.byte %01100000
.byte %01000000
.byte %00000000
.byte %00000000
.byte %00000000

; Tile 47 40x112
knight_40x112:
.byte %11100000
.byte %00100001
.byte %00100110
.byte %10110110
.byte %11110111
.byte %10001111
.byte %11011111
.byte %11111111

.byte %11111111
.byte %10111110
.byte %10111000
.byte %00111000
.byte %01111000
.byte %01110000
.byte %00100000
.byte %00000000

; Tile 48 48x112
knight_48x112:
.byte %00000000
.byte %00100000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000

.byte %00100000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000

; Tile 49 8x120
knight_8x120:
.byte %00000000
.byte %00000000
.byte %00000011
.byte %00000111
.byte %00001000
.byte %00001000
.byte %00001111
.byte %00000001

.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000111
.byte %00000111
.byte %00000000
.byte %00000000

; Tile 50 16x120
knight_16x120:
.byte %01010111
.byte %00110100
.byte %00100101
.byte %11100111
.byte %11100010
.byte %11001011
.byte %11111111
.byte %11111110

.byte %00110111
.byte %01001111
.byte %00011010
.byte %00011000
.byte %00011100
.byte %00111100
.byte %00000000
.byte %00000000

; Tile 51 40x120
knight_40x120:
.byte %11111110
.byte %11110011
.byte %01111111
.byte %01101111
.byte %01101111
.byte %01111111
.byte %01111111
.byte %00000000

.byte %00000000
.byte %00001100
.byte %00000000
.byte %00010000
.byte %00010000
.byte %00000000
.byte %00000000
.byte %00000000

; Tile 52 32x136
knight_32x136:
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000

.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000010

; Tile 53 24x144
knight_24x144:
.byte %00000000
.byte %00000000
.byte %00000001
.byte %00000001
.byte %00010100
.byte %00001010
.byte %00001110
.byte %00001010

.byte %00000001
.byte %00000111
.byte %00001111
.byte %00001111
.byte %00001011
.byte %00010111
.byte %00010011
.byte %00010111

; Tile 54 32x144
knight_32x144:
.byte %00000000
.byte %00000000
.byte %01001000
.byte %00100101
.byte %00101101
.byte %10101101
.byte %10101101
.byte %10011001

.byte %11111100
.byte %11111100
.byte %10110110
.byte %11111010
.byte %11110010
.byte %01110010
.byte %01110010
.byte %01100110

; Tile 55 40x144
knight_40x144:
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %10000000
.byte %10000000
.byte %10000000

.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000

; Tile 56 16x152
knight_16x152:
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000001
.byte %00000001
.byte %00000111
.byte %00000001
.byte %00011100

.byte %00000000
.byte %00000000
.byte %00000001
.byte %00000000
.byte %00000000
.byte %00000010
.byte %00011111
.byte %00000111

; Tile 57 24x152
knight_24x152:
.byte %00001010
.byte %00000000
.byte %11011100
.byte %11111100
.byte %11111110
.byte %11110101
.byte %11101000
.byte %11101100

.byte %00010111
.byte %00011111
.byte %00100011
.byte %00000011
.byte %00011101
.byte %01110111
.byte %11111111
.byte %11110011

; Tile 58 32x152
knight_32x152:
.byte %00011101
.byte %01101101
.byte %11010000
.byte %00001110
.byte %00001110
.byte %00001001
.byte %00000001
.byte %00010001

.byte %11100110
.byte %10011110
.byte %00111110
.byte %11110000
.byte %11110000
.byte %11110110
.byte %11111110
.byte %11111110

; Tile 59 40x152
knight_40x152:
.byte %10000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %10000000
.byte %01000000
.byte %00000000

.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %10000000
.byte %11000000

; Tile 60 16x160
knight_16x160:
.byte %00110010
.byte %01100001
.byte %01100011
.byte %01111110
.byte %00111111
.byte %00000011
.byte %00000011
.byte %00000011

.byte %01001111
.byte %00011111
.byte %00011100
.byte %00000001
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000

; Tile 61 24x160
knight_24x160:
.byte %01011110
.byte %00011101
.byte %01111001
.byte %10011001
.byte %11111111
.byte %11011100
.byte %11111110
.byte %11111111

.byte %11100001
.byte %11100010
.byte %10000110
.byte %00000110
.byte %00000000
.byte %00100011
.byte %00000001
.byte %00000000

; Tile 62 32x160
knight_32x160:
.byte %00010101
.byte %00101000
.byte %00001001
.byte %00001101
.byte %00010101
.byte %10010100
.byte %10000000
.byte %10001000

.byte %11111010
.byte %11111111
.byte %11111110
.byte %11111110
.byte %11111110
.byte %01111111
.byte %01111111
.byte %01111111

; Tile 63 40x160
knight_40x160:
.byte %00100000
.byte %01100000
.byte %11000000
.byte %10000000
.byte %01000000
.byte %10000000
.byte %11000000
.byte %01000000

.byte %11000000
.byte %10000000
.byte %00000000
.byte %01000000
.byte %10000000
.byte %01000000
.byte %00000000
.byte %10000000

; Tile 64 16x168
knight_16x168:
.byte %00000011
.byte %00000011
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000011
.byte %00000110
.byte %00001101

.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000001
.byte %00000010

; Tile 65 24x168
knight_24x168:
.byte %00001111
.byte %11111000
.byte %00110001
.byte %11100111
.byte %01110110
.byte %00011111
.byte %11111111
.byte %10000111

.byte %11110000
.byte %00000111
.byte %10001111
.byte %00011111
.byte %10001110
.byte %11100001
.byte %01111000
.byte %01000100

; Tile 66 32x168
knight_32x168:
.byte %10000100
.byte %00010110
.byte %11000110
.byte %11001001
.byte %11001001
.byte %11000000
.byte %11000100
.byte %11100000

.byte %01111111
.byte %11111111
.byte %10111111
.byte %10111111
.byte %10111111
.byte %10111111
.byte %00111111
.byte %00011111

; Tile 67 40x168
knight_40x168:
.byte %00000000
.byte %00010000
.byte %00001000
.byte %00000000
.byte %10000000
.byte %11000000
.byte %01100000
.byte %10100000

.byte %11100000
.byte %11100000
.byte %11110000
.byte %11111000
.byte %11111110
.byte %11111110
.byte %11111111
.byte %01111111

; Tile 68 48x168
knight_48x168:
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %10000000

.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %10000000
.byte %11000000

; Tile 69 16x176
knight_16x176:
.byte %00011010
.byte %00000111
.byte %00001111
.byte %00101111
.byte %00011011
.byte %00010011
.byte %00110011
.byte %00010011

.byte %00000111
.byte %00001111
.byte %00011111
.byte %00111111
.byte %00111011
.byte %00110011
.byte %01110011
.byte %01110011

; Tile 70 24x176
knight_24x176:
.byte %11111010
.byte %11111100
.byte %11111000
.byte %10110000
.byte %10100000
.byte %01000000
.byte %01000000
.byte %01000000

.byte %00011100
.byte %00000000
.byte %10000000
.byte %11000000
.byte %11000000
.byte %10000000
.byte %10000000
.byte %10000000

; Tile 71 32x176
knight_32x176:
.byte %01100000
.byte %01100000
.byte %00100001
.byte %00110001
.byte %00110001
.byte %00100001
.byte %00000001
.byte %00000000

.byte %00011111
.byte %00011111
.byte %00011110
.byte %00001100
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000

; Tile 72 40x176
knight_40x176:
.byte %10110000
.byte %01011000
.byte %01101000
.byte %10101100
.byte %11110110
.byte %11011111
.byte %11101111
.byte %11111111

.byte %01111111
.byte %10111111
.byte %10011111
.byte %01011111
.byte %00001100
.byte %00100000
.byte %00010000
.byte %00000000

; Tile 73 48x176
knight_48x176:
.byte %01100000
.byte %01010000
.byte %00110000
.byte %00110000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000

.byte %10000000
.byte %10100000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000

; Tile 74 8x184
knight_8x184:
.byte %00000000
.byte %00000000
.byte %00000011
.byte %00000101
.byte %00001000
.byte %00001010
.byte %00001111
.byte %00000000

.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000010
.byte %00000111
.byte %00000101
.byte %00000000
.byte %00000000

; Tile 75 16x184
knight_16x184:
.byte %01010111
.byte %00100000
.byte %10100101
.byte %11100111
.byte %11100010
.byte %11001011
.byte %11111111
.byte %11111110

.byte %00110111
.byte %01011111
.byte %00011010
.byte %00011000
.byte %00011100
.byte %00111100
.byte %00000000
.byte %00000000

; Tile 76 24x184
knight_24x184:
.byte %00000000
.byte %10000000
.byte %10000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000

.byte %10000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000

; Tile 77 40x184
knight_40x184:
.byte %11111110
.byte %11111111
.byte %01111111
.byte %01111111
.byte %01101111
.byte %01111111
.byte %01111111
.byte %00000000

.byte %00000000
.byte %00000000
.byte %00000000
.byte %00000000
.byte %00010000
.byte %00000000
.byte %00000000
.byte %00000000

;
; Meta-Sprites
;  Dimenstions: 64x64
;        Count: 10
;         Size: 186b
;       Format: .byte [tile_base_address hi] [tile_base_address lo] [tile count * 2] {attr:%HVXXXYYY tile-index:%PPIIIIII} ...
;             : V - flip vertical; H - flip horizontal
;             : XXX - tile offset X from 0,0 of meta-sprite; YYY - tile offset Y from 0,0 of meta-sprite
;             : PP - tile palette to use for the meta-sprite; IIIIII - tile index from [base address]

.segment "RODATA"

; Meta-Sprite Tile Count: 27   Meta-Sprite Min Tile Index: 0
_knight_sprite_0:
.export _knight_sprite_0
.addr knight_24x8
.byte 57 ; byte count (including 2 byte address and 1 byte size)
.byte $19, 0
.byte $21, 1
.byte $1A, 2
.byte $22, 3
.byte $2A, 4
.byte $13, 5
.byte $1B, 6
.byte $23, 7
.byte $2B, 8
.byte $14, 9
.byte $1C, 10
.byte $24, 11
.byte $2C, 12
.byte $15, 13
.byte $1D, 14
.byte $25, 15
.byte $2D, 16
.byte $16, 17
.byte $1E, 18
.byte $26, 19
.byte $2E, 20
.byte $0F, 21
.byte $17, 22
.byte $1F, 23
.byte $27, 24
.byte $2F, 25
.byte $37, 26

; Meta-Sprite Tile Count: 25   Meta-Sprite Min Tile Index: 27
_knight_sprite_1:
.export _knight_sprite_1
.addr knight_32x72
.byte 53 ; byte count (including 2 byte address and 1 byte size)
.byte $21, 0
.byte $1A, 1
.byte $22, 2
.byte $2A, 3
.byte $13, 4
.byte $1B, 5
.byte $23, 6
.byte $2B, 7
.byte $14, 8
.byte $1C, 9
.byte $24, 10
.byte $2C, 11
.byte $15, 12
.byte $1D, 13
.byte $25, 14
.byte $2D, 15
.byte $35, 16
.byte $16, 17
.byte $1E, 18
.byte $26, 19
.byte $2E, 20
.byte $36, 21
.byte $0F, 22
.byte $17, 23
.byte $2F, 25

; Meta-Sprite Tile Count: 26   Meta-Sprite Min Tile Index: 54
_knight_sprite_2:
.export _knight_sprite_2
.addr knight_32x136
.byte 55 ; byte count (including 2 byte address and 1 byte size)
.byte $21, 0
.byte $1A, 1
.byte $22, 2
.byte $2A, 3
.byte $13, 4
.byte $1B, 5
.byte $23, 6
.byte $2B, 7
.byte $14, 8
.byte $1C, 9
.byte $24, 10
.byte $2C, 11
.byte $15, 12
.byte $1D, 13
.byte $25, 14
.byte $2D, 15
.byte $35, 16
.byte $16, 17
.byte $1E, 18
.byte $26, 19
.byte $2E, 20
.byte $36, 21
.byte $0F, 22
.byte $17, 23
.byte $1F, 24
.byte $2F, 25

