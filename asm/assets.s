;
; Macros
;

.macro include_asset file
.include file
.endmacro

;
; Meta-sprites
;

.segment "RODATA"

;include_asset "knight.s"

;
; Text
;

.segment        "ZEROPAGE"
_temp_text_table:   .res  2 ;

.export         _temp_text_table

; Translate macro
.macro TR arg
    .byte   .strlen(arg), arg
    ;.asciiz arg
.endmacro

; Map characters to sprite sheet ordering
;.pushcharmap

.define TEXT_LANGUAGE   "en"
.define VERSION         "0.1.0"
.define COPYRIGHT_YEAR  "2026"

include_asset .concat("shadow-font.", TEXT_LANGUAGE, ".png.font.s")

.feature string_escapes

.if .defined(DEBUG_BUILD)
.define BUILD_TYPE      "d"
.elseif .defined(RELEASE_BUILD)
.define BUILD_TYPE      "r"
.elseif .defined(DISTRO_BUILD)
.define BUILD_TYPE      "s"
.else
.define BUILD_TYPE      "?"
.endif

include_asset .concat("gametext.", TEXT_LANGUAGE, ".s")

;.popcharmap
