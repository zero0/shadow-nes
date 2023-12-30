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

.segment "RODATA"

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

include_asset "shadow-font.font.s"

include_asset .concat("gametext.", TEXT_LANGUAGE, ".s")

.popcharmap
