;
; Assets
;

.macro BlankCHR count
    .repeat count, I
    .byte   %00000000
    .byte   %00000000
    .byte   %00000000
    .byte   %00000000
    .byte   %00000000
    .byte   %00000000
    .byte   %00000000
    .byte   %00000000

    .byte   %00000000
    .byte   %00000000
    .byte   %00000000
    .byte   %00000000
    .byte   %00000000
    .byte   %00000000
    .byte   %00000000
    .byte   %00000000
    .endrep
.endmacro

.segment "CHARS"

BlankCHR 1

;
;blank:
;
;.byte   %00000000
;.byte   %00000000
;.byte   %00000000
;.byte   %00000000
;.byte   %00000000
;.byte   %00000000
;.byte   %00000000
;.byte   %00000000
;
;.byte   %00000000
;.byte   %00000000
;.byte   %00000000
;.byte   %00000000
;.byte   %00000000
;.byte   %00000000
;.byte   %00000000
;.byte   %00000000
;

.define TEXT_LANGUAGE   "en"
.define VERSION         "0.1.0"
.define COPYRIGHT_YEAR  "2024"

.macro include_asset file
.include .concat("../assets/", file);
.endmacro

;.include "../assets/nesfont.s"

;.include "../assets/player.s"

;.include "../assets/env.s"

include_asset "shadow-font.s"

BlankCHR 177

include_asset "knight.s"

;
; Text
;

.segment        "ZEROPAGE"
_temp_text_table:   .res  2 ;

.export         _temp_text_table

; Translate macro
.macro TR arg
    ;.byte   .strlen(arg), arg
    .asciiz arg
.endmacro

; Map characters to sprite sheet ordering
.pushcharmap

.if 1
.feature string_escapes +

.charmap '0', 1
.charmap '1', 2
.charmap '2', 3
.charmap '3', 4
.charmap '4', 5
.charmap '5', 6
.charmap '6', 7
.charmap '7', 8
.charmap '8', 9
.charmap '9', 10

.charmap 'a', 11
.charmap 'b', 12
.charmap 'c', 13
.charmap 'd', 14
.charmap 'e', 15
.charmap 'f', 16
.charmap 'g', 17
.charmap 'h', 18
.charmap 'i', 19
.charmap 'j', 20
.charmap 'k', 21
.charmap 'l', 22
.charmap 'm', 23
.charmap 'n', 24
.charmap 'o', 25
.charmap 'p', 26
.charmap 'q', 27
.charmap 'r', 28
.charmap 's', 29
.charmap 't', 30
.charmap 'u', 31
.charmap 'v', 32
.charmap 'w', 33
.charmap 'x', 34
.charmap 'y', 35
.charmap 'z', 36

.charmap 'A', 37
.charmap 'B', 38
.charmap 'C', 39
.charmap 'D', 40
.charmap 'E', 41
.charmap 'F', 42
.charmap 'G', 43
.charmap 'H', 44
.charmap 'I', 45
.charmap 'J', 46
.charmap 'K', 47
.charmap 'L', 48
.charmap 'M', 49
.charmap 'N', 50
.charmap 'O', 51
.charmap 'P', 52
.charmap 'Q', 53
.charmap 'R', 54
.charmap 'S', 55
.charmap 'T', 56
.charmap 'U', 57
.charmap 'V', 58
.charmap 'W', 59
.charmap 'X', 60
.charmap 'Y', 61
.charmap 'Z', 62

.charmap '@', 63 ; something
.charmap '.', 64
.charmap '!', 65
.charmap '<', 66 ; double-quote left "
.charmap '>', 67 ; double-quote right "
.charmap '?', 68
.charmap ',', 69
.charmap ':', 70
.charmap  59, 71 ; semicolon
.charmap '(', 72
.charmap ')', 73
.charmap '[', 74
.charmap ']', 75
.charmap '/', 76
.charmap  39, 77 ; single-quote '

; extra
.charmap ' ', 78 ; make spaces a character that can be read and handled
.charmap  10, 79 ; handle new lines
.charmap  13, 80 ; handle new carage return (new screen)
.charmap   9, 81 ; handl tab
.endif

include_asset .concat("gametext.", TEXT_LANGUAGE, ".s")

.popcharmap
