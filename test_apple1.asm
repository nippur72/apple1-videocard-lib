// TODO verificare NOPs
  // Generic ASM 6502
.file [name="test.prg", type="prg", segments="Program"]
.segmentdef Program [segments="Code, Data"]
.segmentdef Code [start=$4000]
.segmentdef Data [startAfter="Code"]
  .const WOZMON = $ff1f
  // enters monitor
  .const ECHO = $ffef
  // print hex byte in A (A destroyed)
  .const KEY_DATA = $d010
  // read key
  .const KEY_CTRL = $d011
  // TMS9918 interface flags
  .const WRITE_TO_REG = $80
  .const WRITE_TO_VRAM = $40
  // sprite patterns:    $0000
  // pattern table:      $0800   (256*8)
  // sprite attributes:  $1000
  // unused:             $1080
  // name table:         $1400   (32*24)
  // unused:             $1800
  // color table:        $2000   (32)
  // unused              $2020-$3FFF
  .const SCREEN1_PATTERN_TABLE = $800
  .const SCREEN1_NAME_TABLE = $1400
  .const SCREEN1_COLOR_TABLE = $2000
  .const SCREEN1_SPRITE_PATTERNS = 0
  .const SCREEN1_SPRITE_ATTRS = $1000
  .const SCREEN1_SIZE = $20*$18
  // pattern table:      $0000-$17FF   (256*8*3)
  // sprite patterns:    $1800-$19FF
  // color table:        $2000-$27FF   (256*8*3)
  // name table:         $3800-$3AFF   (32*24 = 256*3 = 768)
  // sprite attributes:  $3B00-$3BFF
  // unused              $3C00-$3FFF
  //
  .const SCREEN2_PATTERN_TABLE = 0
  .const SCREEN2_NAME_TABLE = $3800
  .const SCREEN2_COLOR_TABLE = $2000
  .const SCREEN2_SPRITE_PATTERNS = $1800
  .const SCREEN2_SPRITE_ATTRS = $3b00
  .const SCREEN2_SIZE = $20*$18
  // control port
  .label VDP_DATA = $c000
  // TMS9918 data port (VRAM)
  .label VDP_REG = $c001
  .label screen1_cursor = $d
.segment Code
main: {
    lda #<0
    sta.z screen1_cursor
    sta.z screen1_cursor+1
    ldx #'1'
  __b1:
    cpx #'1'
    beq __b2
    cpx #'2'
    beq __b3
    cpx #$d
    beq __b4
    stx.z woz_putc.c
    jsr woz_putc
  __b5:
    jsr woz_getkey
    tax
    jmp __b1
  __b4:
    lda #$2a
    sta.z woz_putc.c
    jsr woz_putc
    jsr woz_mon
    rts
  __b3:
    jsr prova_screen2
    jmp __b5
  __b2:
    jsr prova_screen1
    jmp __b5
}
// puts a character on the apple1 screen using the WOZMON routine
// woz_putc(byte zp($11) c)
woz_putc: {
    .label c = $11
    lda c
    jsr ECHO
    rts
}
// reads a key from the apple-1 keyboard
woz_getkey: {
  __b1:
    lda KEY_CTRL
    cmp #0
    bne __b1
    lda #$7f
    and KEY_DATA
    rts
}
// returns to WOZMON prompt
woz_mon: {
    jmp WOZMON
}
prova_screen2: {
    .label i = 2
    .label i1 = 3
    lda #<SCREEN2_TABLE
    sta.z SCREEN_INIT.table
    lda #>SCREEN2_TABLE
    sta.z SCREEN_INIT.table+1
    jsr SCREEN_INIT
    jsr SCREEN2_FILL
    jsr screen2_square_sprites
    lda #$1f
    sta.z SCREEN2_PUTS.col
    lda #0
    sta.z SCREEN2_PUTS.y
    sta.z SCREEN2_PUTS.x
    lda #<s
    sta.z SCREEN2_PUTS.s
    lda #>s
    sta.z SCREEN2_PUTS.s+1
    jsr SCREEN2_PUTS
    lda #$1f
    sta.z SCREEN2_PUTS.col
    lda #2
    sta.z SCREEN2_PUTS.y
    lda #0
    sta.z SCREEN2_PUTS.x
    lda #<s1
    sta.z SCREEN2_PUTS.s
    lda #>s1
    sta.z SCREEN2_PUTS.s+1
    jsr SCREEN2_PUTS
    lda #$1f
    sta.z SCREEN2_PUTS.col
    lda #4
    sta.z SCREEN2_PUTS.y
    lda #0
    sta.z SCREEN2_PUTS.x
    lda #<s2
    sta.z SCREEN2_PUTS.s
    lda #>s2
    sta.z SCREEN2_PUTS.s+1
    jsr SCREEN2_PUTS
    lda #0
    sta.z i
  __b1:
    lda.z i
    cmp #$10
    bcc __b2
    lda #0
    sta.z i1
  __b3:
    lda.z i1
    cmp #$c0
    bcc __b4
    rts
  __b4:
    lda.z i1
    lsr
    tax
    lda.z i1
    sta.z SCREEN2_PSET.x
    jsr SCREEN2_PSET
    lda.z i1
    sta.z SCREEN2_PSET.x
    ldx.z i1
    jsr SCREEN2_PSET
    lda.z i1
    lsr
    sta.z SCREEN2_PSET.x
    ldx.z i1
    jsr SCREEN2_PSET
    inc.z i1
    jmp __b3
  __b2:
    lax.z i
    axs #-[6]
    stx.z SCREEN2_PUTS.y
    lda #$f
    sec
    sbc.z i
    asl
    asl
    asl
    asl
    clc
    adc.z i
    sta.z SCREEN2_PUTS.col
    lda #5
    sta.z SCREEN2_PUTS.x
    lda #<s3
    sta.z SCREEN2_PUTS.s
    lda #>s3
    sta.z SCREEN2_PUTS.s+1
    jsr SCREEN2_PUTS
    inc.z i
    jmp __b1
  .segment Data
    s3: .text "     SCREEN 2     "
    .byte 0
}
.segment Code
prova_screen1: {
    .label i = 4
    lda #<SCREEN1_TABLE
    sta.z SCREEN_INIT.table
    lda #>SCREEN1_TABLE
    sta.z SCREEN_INIT.table+1
    jsr SCREEN_INIT
    jsr SCREEN1_FILL
    jsr SCREEN1_LOAD_FONT
    lda #<SCREEN1_NAME_TABLE
    sta.z screen1_cursor
    lda #>SCREEN1_NAME_TABLE
    sta.z screen1_cursor+1
    lda #<s
    sta.z SCREEN1_PUTS.s
    lda #>s
    sta.z SCREEN1_PUTS.s+1
    jsr SCREEN1_PUTS
    lda #2
    jsr SCREEN1_LOCATEXY
    lda #<s1
    sta.z SCREEN1_PUTS.s
    lda #>s1
    sta.z SCREEN1_PUTS.s+1
    jsr SCREEN1_PUTS
    lda #4
    jsr SCREEN1_LOCATEXY
    lda #<s2
    sta.z SCREEN1_PUTS.s
    lda #>s2
    sta.z SCREEN1_PUTS.s+1
    jsr SCREEN1_PUTS
    lda #$a
    jsr SCREEN1_LOCATEXY
    lda #<0
    sta.z i
    sta.z i+1
  __b1:
    lda.z i+1
    cmp #>$100
    bcc __b2
    bne !+
    lda.z i
    cmp #<$100
    bcc __b2
  !:
    jsr screen1_square_sprites
    rts
  __b2:
    lda.z i
    tay
    jsr SCREEN1_PUTCHAR
    inc.z i
    bne !+
    inc.z i+1
  !:
    jmp __b1
}
// SCREEN_INIT(byte* zp($14) table)
SCREEN_INIT: {
    .label table = $14
    ldy #0
  __b1:
    cpy #8
    bcc __b2
    rts
  __b2:
    tya
    tax
    lda (table),y
    jsr write_reg
    iny
    jmp __b1
}
SCREEN2_FILL: {
    .label i = 4
    .label i1 = $14
    .label i2 = 9
  // fills name table x3 with increasing numbers
    lda #<SCREEN2_NAME_TABLE
    sta.z set_vram_write_addr.addr
    lda #>SCREEN2_NAME_TABLE
    sta.z set_vram_write_addr.addr+1
    jsr set_vram_write_addr
    lda #<0
    sta.z i
    sta.z i+1
  __b1:
    lda.z i+1
    cmp #>SCREEN2_SIZE
    bcc __b2
    bne !+
    lda.z i
    cmp #<SCREEN2_SIZE
    bcc __b2
  !:
  // fill pattern table with 0 (clear screen)
    lda #<SCREEN2_PATTERN_TABLE
    sta.z set_vram_write_addr.addr
    lda #>SCREEN2_PATTERN_TABLE
    sta.z set_vram_write_addr.addr+1
    jsr set_vram_write_addr
    lda #<$300*8
    sta.z i1
    lda #>$300*8
    sta.z i1+1
  __b4:
    lda.z i1
    ora.z i1+1
    bne __b5
  // fill color table with $1F
    lda #<SCREEN2_COLOR_TABLE
    sta.z set_vram_write_addr.addr
    lda #>SCREEN2_COLOR_TABLE
    sta.z set_vram_write_addr.addr+1
    jsr set_vram_write_addr
    lda #<$300*8
    sta.z i2
    lda #>$300*8
    sta.z i2+1
  __b7:
    lda.z i2
    ora.z i2+1
    bne __b8
    rts
  __b8:
    lda #$1f
    sta VDP_DATA
    lda.z i2
    bne !+
    dec.z i2+1
  !:
    dec.z i2
    jmp __b7
  __b5:
    lda #0
    sta VDP_DATA
    lda.z i1
    bne !+
    dec.z i1+1
  !:
    dec.z i1
    jmp __b4
  __b2:
    lda #$ff
    and.z i
    sta VDP_DATA
    inc.z i
    bne !+
    inc.z i+1
  !:
    jmp __b1
}
screen2_square_sprites: {
  // fills first sprite pattern with 255
    lda #<SCREEN2_SPRITE_PATTERNS
    sta.z set_vram_write_addr.addr
    lda #>SCREEN2_SPRITE_PATTERNS
    sta.z set_vram_write_addr.addr+1
    jsr set_vram_write_addr
    ldx #0
  // start writing in the sprite patterns
  __b1:
    cpx #8
    bcc __b2
  // set sprite coordinates
    lda #<SCREEN2_SPRITE_ATTRS
    sta.z set_vram_write_addr.addr
    lda #>SCREEN2_SPRITE_ATTRS
    sta.z set_vram_write_addr.addr+1
    jsr set_vram_write_addr
    ldx #0
  // start writing in the sprite attribute
  __b4:
    cpx #$20
    bcc __b5
    rts
  __b5:
    lda #0
    sta VDP_DATA
    sta VDP_DATA
    sta VDP_DATA
    stx VDP_DATA
    inx
    jmp __b4
  __b2:
    lda #0
    sta VDP_DATA
    inx
    jmp __b1
}
// SCREEN2_PUTS(byte zp(8) x, byte zp(6) y, byte zp(7) col, byte* zp(9) s)
SCREEN2_PUTS: {
    .label s = 9
    .label x = 8
    .label y = 6
    .label col = 7
  __b1:
    ldy #0
    lda (s),y
    inc.z s
    bne !+
    inc.z s+1
  !:
    cmp #0
    bne __b2
    rts
  __b2:
    tay
    lda.z x
    sta.z SCREEN2_PUTC.x
    lda.z y
    sta.z SCREEN2_PUTC.y
    ldx.z col
    jsr SCREEN2_PUTC
    inc.z x
    jmp __b1
}
// SCREEN2_PSET(byte zp($12) x, byte register(X) y)
SCREEN2_PSET: {
    .label __1 = $f
    .label __3 = $14
    .label __4 = $f
    .label __12 = $14
    .label paddr = $f
    .label data = $13
    .label x = $12
    lda #$f8
    and.z x
    sta.z __1
    lda #0
    sta.z __1+1
    txa
    and #$f8
    sta.z __12
    lda #0
    sta.z __12+1
    asl.z __3
    rol.z __3+1
    asl.z __3
    rol.z __3+1
    asl.z __3
    rol.z __3+1
    asl.z __3
    rol.z __3+1
    asl.z __3
    rol.z __3+1
    lda.z __4
    clc
    adc.z __3
    sta.z __4
    lda.z __4+1
    adc.z __3+1
    sta.z __4+1
    txa
    and #8-1
    clc
    adc.z paddr
    sta.z paddr
    bcc !+
    inc.z paddr+1
  !:
    lda.z paddr
    sta.z set_vram_read_addr.addr
    lda.z paddr+1
    sta.z set_vram_read_addr.addr+1
    jsr set_vram_read_addr
    lda VDP_DATA
    sta.z data
    jsr set_vram_write_addr
    lda #8-1
    and.z x
    tay
    lda pow2_table,y
    ora.z data
    sta VDP_DATA
    rts
  .segment Data
    pow2_table: .byte $80, $40, $20, $10, 8, 4, 2, 1
}
.segment Code
SCREEN1_FILL: {
    .label i = $14
    .label i1 = 9
  // fills name table with spaces (32)
    lda #<SCREEN1_NAME_TABLE
    sta.z set_vram_write_addr.addr
    lda #>SCREEN1_NAME_TABLE
    sta.z set_vram_write_addr.addr+1
    jsr set_vram_write_addr
    lda #<SCREEN1_SIZE
    sta.z i
    lda #>SCREEN1_SIZE
    sta.z i+1
  __b1:
    lda.z i
    ora.z i+1
    bne __b2
  // fill pattern table with 0
    lda #<SCREEN1_PATTERN_TABLE
    sta.z set_vram_write_addr.addr
    lda #>SCREEN1_PATTERN_TABLE
    sta.z set_vram_write_addr.addr+1
    jsr set_vram_write_addr
    lda #<$100*8
    sta.z i1
    lda #>$100*8
    sta.z i1+1
  __b4:
    lda.z i1
    ora.z i1+1
    bne __b5
  // fill color table with $1F
    lda #<SCREEN1_COLOR_TABLE
    sta.z set_vram_write_addr.addr
    lda #>SCREEN1_COLOR_TABLE
    sta.z set_vram_write_addr.addr+1
    jsr set_vram_write_addr
    ldx #$20
  __b7:
    cpx #0
    bne __b8
    rts
  __b8:
    lda #$1f
    sta VDP_DATA
    dex
    jmp __b7
  __b5:
    lda #0
    sta VDP_DATA
    lda.z i1
    bne !+
    dec.z i1+1
  !:
    dec.z i1
    jmp __b4
  __b2:
    lda #$20
    sta VDP_DATA
    lda.z i
    bne !+
    dec.z i+1
  !:
    dec.z i
    jmp __b1
}
// loads the Laser 500 font on the pattern table
SCREEN1_LOAD_FONT: {
    // reverse font (32..127)
    .label source = 9
    .label i = $14
    // reverse font (32..127)
    .label source_1 = $d
    .label i_1 = $b
  // start writing into VRAM from space character (32..127)
    lda #<SCREEN1_PATTERN_TABLE+$20*8
    sta.z set_vram_write_addr.addr
    lda #>SCREEN1_PATTERN_TABLE+$20*8
    sta.z set_vram_write_addr.addr+1
    jsr set_vram_write_addr
    lda #<FONT
    sta.z source
    lda #>FONT
    sta.z source+1
    lda #<$300
    sta.z i
    lda #>$300
    sta.z i+1
  __b1:
    lda.z i
    ora.z i+1
    bne __b2
    lda #<SCREEN1_PATTERN_TABLE+($80+$20)*8
    sta.z set_vram_write_addr.addr
    lda #>SCREEN1_PATTERN_TABLE+($80+$20)*8
    sta.z set_vram_write_addr.addr+1
    jsr set_vram_write_addr
    lda #<FONT
    sta.z source_1
    lda #>FONT
    sta.z source_1+1
    lda #<$300
    sta.z i_1
    lda #>$300
    sta.z i_1+1
  __b4:
    lda.z i_1
    ora.z i_1+1
    bne __b5
    rts
  __b5:
    ldy #0
    lda (source_1),y
    eor #$ff
    sta VDP_DATA
    inc.z source_1
    bne !+
    inc.z source_1+1
  !:
    lda.z i_1
    bne !+
    dec.z i_1+1
  !:
    dec.z i_1
    jmp __b4
  __b2:
    ldy #0
    lda (source),y
    sta VDP_DATA
    inc.z source
    bne !+
    inc.z source+1
  !:
    lda.z i
    bne !+
    dec.z i+1
  !:
    dec.z i
    jmp __b1
}
// prints 0 terminated string pointed by YA
// SCREEN1_PUTS(byte* zp($b) s)
SCREEN1_PUTS: {
    .label s = $b
  __b1:
    ldy #0
    lda (s),y
    inc.z s
    bne !+
    inc.z s+1
  !:
    cmp #0
    bne __b2
    rts
  __b2:
    tay
    jsr SCREEN1_PUTCHAR
    jmp __b1
}
// SCREEN1_LOCATEXY(byte register(A) y)
SCREEN1_LOCATEXY: {
    .label __0 = $d
    .label __3 = $d
    sta.z __3
    lda #0
    sta.z __3+1
    asl.z __0
    rol.z __0+1
    asl.z __0
    rol.z __0+1
    asl.z __0
    rol.z __0+1
    asl.z __0
    rol.z __0+1
    asl.z __0
    rol.z __0+1
    clc
    lda.z screen1_cursor
    adc #<SCREEN1_NAME_TABLE
    sta.z screen1_cursor
    lda.z screen1_cursor+1
    adc #>SCREEN1_NAME_TABLE
    sta.z screen1_cursor+1
    rts
}
screen1_square_sprites: {
  // fills first sprite pattern with 255
    lda #<SCREEN1_SPRITE_PATTERNS
    sta.z set_vram_write_addr.addr
    lda #>SCREEN1_SPRITE_PATTERNS
    sta.z set_vram_write_addr.addr+1
    jsr set_vram_write_addr
    ldx #0
  // start writing in the sprite patterns
  __b1:
    cpx #8
    bcc __b2
  // set sprite coordinates
    lda #<SCREEN1_SPRITE_ATTRS
    sta.z set_vram_write_addr.addr
    lda #>SCREEN1_SPRITE_ATTRS
    sta.z set_vram_write_addr.addr+1
    jsr set_vram_write_addr
    ldy #0
  // start writing in the sprite attribute
  __b4:
    cpy #$20
    bcc __b5
    rts
  __b5:
    tya
    clc
    adc #6
    asl
    asl
    asl
    sta VDP_DATA
    tya
    clc
    adc #6
    asl
    asl
    asl
    sta VDP_DATA
    lda #0
    sta VDP_DATA
    sty VDP_DATA
    iny
    jmp __b4
  __b2:
    lda #$ff
    sta VDP_DATA
    inx
    jmp __b1
}
// prints character to TMS (SCREEN 1 MODE)
// SCREEN1_PUTCHAR(byte register(Y) c)
SCREEN1_PUTCHAR: {
    lda.z screen1_cursor
    sta.z set_vram_write_addr.addr
    lda.z screen1_cursor+1
    sta.z set_vram_write_addr.addr+1
    jsr set_vram_write_addr
    inc.z screen1_cursor
    bne !+
    inc.z screen1_cursor+1
  !:
    sty VDP_DATA
    rts
}
// writes a value to a TMS9918 register (0-7)
// write_reg(byte register(X) regnum, byte register(A) val)
write_reg: {
    sta VDP_REG
    txa
    and #$f
    ora #WRITE_TO_REG
    sta VDP_REG
    rts
}
// sets the VRAM write address on the TMS9918
// set_vram_write_addr(word zp($f) addr)
set_vram_write_addr: {
    .label addr = $f
    lda.z addr
    sta VDP_REG
    lda.z addr+1
    and #$3f
    ora #WRITE_TO_VRAM
    sta VDP_REG
    rts
}
// SCREEN2_PUTC(byte register(Y) ch, byte zp($12) x, byte zp($13) y, byte register(X) col)
SCREEN2_PUTC: {
    .label __1 = $14
    .label __4 = $16
    .label __12 = $14
    .label source = $14
    .label addr = $16
    .label x = $12
    .label y = $13
    tya
    sec
    sbc #$20
    sta.z __12
    lda #0
    sta.z __12+1
    asl.z __1
    rol.z __1+1
    asl.z __1
    rol.z __1+1
    asl.z __1
    rol.z __1+1
    clc
    lda.z source
    adc #<FONT
    sta.z source
    lda.z source+1
    adc #>FONT
    sta.z source+1
    lda.z x
    asl
    asl
    asl
    tay
    lda.z y
    sta.z __4+1
    lda #0
    sta.z __4
    tya
    clc
    adc.z addr
    sta.z addr
    bcc !+
    inc.z addr+1
  !:
    lda.z addr
    sta.z set_vram_write_addr.addr
    lda.z addr+1
    sta.z set_vram_write_addr.addr+1
    jsr set_vram_write_addr
    ldy #0
  __b1:
    cpy #8
    bcc __b2
    clc
    lda.z addr
    adc #<SCREEN2_COLOR_TABLE
    sta.z set_vram_write_addr.addr
    lda.z addr+1
    adc #>SCREEN2_COLOR_TABLE
    sta.z set_vram_write_addr.addr+1
    jsr set_vram_write_addr
    ldy #0
  __b4:
    cpy #8
    bcc __b5
    rts
  __b5:
    stx VDP_DATA
    iny
    jmp __b4
  __b2:
    lda (source),y
    sta VDP_DATA
    iny
    jmp __b1
}
// sets the VRAM read address on the TMS9918
// set_vram_read_addr(word zp($16) addr)
set_vram_read_addr: {
    .label addr = $16
    lda.z addr
    sta VDP_REG
    lda.z addr+1
    and #$3f
    sta VDP_REG
    rts
}
.segment Data
  FONT: .byte 0, 0, 0, 0, 0, 0, 0, 0, 8, 8, 8, 8, 8, 0, 8, 0, $14, $14, $14, 0, 0, 0, 0, 0, $14, $14, $3e, $14, $3e, $14, $14, 0, 8, $1e, $28, $1c, $a, $3c, 8, 0, $30, $32, 4, 8, $10, $26, 6, 0, $10, $28, $28, $10, $2a, $24, $1a, 0, 8, 8, 8, 0, 0, 0, 0, 0, 8, $10, $20, $20, $20, $10, 8, 0, 8, 4, 2, 2, 2, 4, 8, 0, 8, $2a, $1c, 8, $1c, $2a, 8, 0, 0, 8, 8, $3e, 8, 8, 0, 0, 0, 0, 0, 0, 8, 8, $10, 0, 0, 0, 0, $3e, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 2, 4, 8, $10, $20, 0, 0, $1c, $22, $26, $2a, $32, $22, $1c, 0, 8, $18, 8, 8, 8, 8, $1c, 0, $1c, $22, 2, $c, $10, $20, $3e, 0, $3e, 2, 4, $c, 2, $22, $1c, 0, 4, $c, $14, $24, $3e, 4, 4, 0, $3e, $20, $3c, 2, 2, $22, $1c, 0, $e, $10, $20, $3c, $22, $22, $1c, 0, $3e, 2, 4, 8, $10, $20, $20, 0, $1c, $22, $22, $1c, $22, $22, $1c, 0, $1c, $22, $22, $1e, 2, 4, $38, 0, 0, 0, 8, 0, 8, 0, 0, 0, 0, 0, 8, 0, 8, 8, $10, 0, 4, 8, $10, $20, $10, 8, 4, 0, 0, 0, $3e, 0, $3e, 0, 0, 0, $10, 8, 4, 2, 4, 8, $10, 0, $1c, $22, 4, 8, 8, 0, 8, 0, $1c, $22, $2a, $2e, $2c, $20, $1c, 0, 8, $14, $22, $22, $3e, $22, $22, 0, $3c, $22, $22, $3c, $22, $22, $3c, 0, $1c, $22, $20, $20, $20, $22, $1c, 0, $38, $24, $22, $22, $22, $24, $38, 0, $3e, $20, $20, $38, $20, $20, $3e, 0, $3e, $20, $20, $38, $20, $20, $20, 0, $1e, $20, $20, $20, $26, $22, $1e, 0, $22, $22, $22, $3e, $22, $22, $22, 0, $1c, 8, 8, 8, 8, 8, $1c, 0, 2, 2, 2, 2, 2, $22, $1c, 0, $22, $24, $28, $30, $28, $24, $22, 0, $20, $20, $20, $20, $20, $20, $3e, 0, $22, $36, $2a, $2a, $22, $22, $22, 0, $22, $22, $32, $2a, $26, $22, $22, 0, $1c, $22, $22, $22, $22, $22, $1c, 0, $3c, $22, $22, $3c, $20, $20, $20, 0, $1c, $22, $22, $22, $2a, $24, $1a, 0, $3c, $22, $22, $3c, $28, $24, $22, 0, $1c, $22, $20, $1c, 2, $22, $1c, 0, $3e, 8, 8, 8, 8, 8, 8, 0, $22, $22, $22, $22, $22, $22, $1c, 0, $22, $22, $22, $22, $22, $14, 8, 0, $22, $22, $22, $2a, $2a, $36, $22, 0, $22, $22, $14, 8, $14, $22, $22, 0, $22, $22, $14, 8, 8, 8, 8, 0, $3e, 2, 4, 8, $10, $20, $3e, 0, $3e, $30, $30, $30, $30, $30, $3e, 0, 0, $20, $10, 8, 4, 2, 0, 0, $3e, 6, 6, 6, 6, 6, $3e, 0, 8, $14, $22, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, $3e, $10, 8, 4, 0, 0, 0, 0, 0, 0, 0, $1c, 2, $1e, $22, $1e, 0, $20, $20, $3c, $22, $22, $22, $3c, 0, 0, 0, $1e, $20, $20, $20, $1e, 0, 2, 2, $1e, $22, $22, $22, $1e, 0, 0, 0, $1c, $22, $3e, $20, $1e, 0, $c, $12, $10, $3c, $10, $10, $10, 0, 0, 0, $1c, $22, $22, $1e, 2, $1c, $20, $20, $3c, $22, $22, $22, $22, 0, 8, 0, $18, 8, 8, 8, $1c, 0, 2, 0, 6, 2, 2, 2, $12, $c, $20, $20, $22, $24, $38, $24, $22, 0, $18, 8, 8, 8, 8, 8, $1c, 0, 0, 0, $34, $2a, $2a, $2a, $22, 0, 0, 0, $3c, $22, $22, $22, $22, 0, 0, 0, $1c, $22, $22, $22, $1c, 0, 0, 0, $3c, $22, $22, $3c, $20, $20, 0, 0, $1e, $22, $22, $1e, 2, 2, 0, 0, $2e, $30, $20, $20, $20, 0, 0, 0, $1e, $20, $1c, 2, $3c, 0, $10, $10, $3c, $10, $10, $12, $c, 0, 0, 0, $22, $22, $22, $26, $1a, 0, 0, 0, $22, $22, $22, $14, 8, 0, 0, 0, $22, $22, $2a, $2a, $14, 0, 0, 0, $22, $14, 8, $14, $22, 0, 0, 0, $22, $24, $14, $18, 8, $30, 0, 0, $3e, 4, 8, $10, $3e, 0, 6, 8, 8, $30, 8, 8, 6, 0, 8, 8, 8, 8, 8, 8, 8, 8, $30, 8, 8, 6, 8, 8, $30, 0, $1a, $2c, 0, 0, 0, 0, 0, 0
  .fill 8, 0
  SCREEN1_TABLE: .byte 0, $c0, 5, $80, 1, $20, 0, $25
  SCREEN2_TABLE: .byte 2, $c0, $e, $ff, 3, $76, 3, $25
  s: .text "*** P-LAB  VIDEO CARD SYSTEM ***"
  .byte 0
  s1: .text "16K VRAM BYTES FREE"
  .byte 0
  s2: .text "READY."
  .byte 0
