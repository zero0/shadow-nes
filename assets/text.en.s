;
; Game Text
;

.macro TR arg
    ;.byte   .strlen(arg), arg
    .asciiz arg
.endmacro

;
;
;

.segment        "ZEROPAGE"
_temp_text_table:   .res  2 ;

.export         _temp_text_table

;
;
;

.segment        "RODATA"
_text_table:
.byte           <tr_title_title, >tr_title_title
.byte           <tr_title_new_game, >tr_title_new_game
.byte           <tr_title_continue, >tr_title_continue
.byte           <tr_title_version, >tr_title_version
.byte           <tr_title_copyright, >tr_title_copyright

.export         _text_table

;
;
;

.segment        "RODATA"

; Map characters to sprite sheet ordering
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

_text_root:

tr_title_title:     TR "<Shadowborn>"
tr_title_new_game:  TR "New Game"
tr_title_continue:  TR "Continue"
tr_title_version:   TR .concat("Version ", VERSION)
tr_title_copyright: TR .concat("(C) ", COPYRIGHT_YEAR)
