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

include_asset .concat("text.", TEXT_LANGUAGE, ".s")
