;
;
;

.define NES_MAPPER_NROM     0   ; Mapper 000
.define NES_MAPPER_MMC1     1   ; Mapper 001
.define NES_MAPPER_UxROM    2   ; Mapper 002
.define NES_MAPPER_CNROM    3   ; Mapper 003
.define NES_MAPPER_MMC3     4   ; Mapper 004
.define NES_MAPPER_MMC5     5   ; Mapper 005

.ifndef NES_MAPPER
.define NES_MAPPER          NES_MAPPER_MMC5
.endif

.segment "ZEROPAGE"

_PRG_BANK_STACK_LEVEL:  .res 1
_PRG_BANK_STACK:        .res 8

.export _PRG_BANK_STACK_LEVEL
.export _PRG_BANK_STACK

;
; Mapper Implementation
;

.segment "LOWCODE"

.if NES_MAPPER = NES_MAPPER_NROM
    .include "mapper.nrom.inls"
.elseif NES_MAPPER = NES_MAPPER_MMC1
    .include "mapper.mmc1.inls"
.elseif NES_MAPPER = NES_MAPPER_MMC5
    .include "mapper.mmc5.inls"
.else
    .error "No Mapper Defined"
.endif

;
; Mapper API
;

.export mapper_reset, _mapper_reset
.export mapper_init
.export mapper_set_mirroring, _mapper_set_mirroring
.export mapper_set_chr_bank_0, _mapper_set_chr_bank_0
.export mapper_set_chr_bank_1, _mapper_set_chr_bank_1
.export mapper_set_prg_bank, _mapper_set_prg_bank
.export mapper_reset_irq, _mapper_reset_irq
.export mapper_set_scanline_irq, _mapper_set_scanline_irq

;
_mapper_reset:
.proc mapper_reset
    mapper_reset_impl
    rts
.endproc

;
.proc mapper_init
    mapper_init_impl
    rts
.endproc

;
_mapper_set_mirroring:
.proc mapper_set_mirroring
    mapper_set_mirroring_impl
    rts
.endproc

;
_mapper_set_chr_bank_0:
.proc mapper_set_chr_bank_0
    mapper_set_chr_bank_0_impl
    rts
.endproc

;
_mapper_set_chr_bank_1:
.proc mapper_set_chr_bank_1
    mapper_set_chr_bank_1_impl
    rts
.endproc

;
_mapper_set_prg_bank:
.proc mapper_set_prg_bank
    mapper_set_prg_bank_impl
    rts
.endproc

;
_mapper_reset_irq:
.proc mapper_reset_irq
    mapper_reset_irq_impl
    rts
.endproc

;
_mapper_set_scanline_irq:
.proc mapper_set_scanline_irq
    mapper_set_scanline_irq_impl
    rts
.endproc
