

void gr4_tile(byte x, byte y, byte ch, byte col) {
   byte *source = &FONTS[(word)ch*8];
   word addr = x*8 + y*256;
   set_vram_write_addr(SCREEN2_PATTERN_TABLE + addr); for(byte i=0;i<8;i++) { TMS_WRITE_DATA_PORT(source[i]); NOP; /*NOP;*/ /*NOP; NOP;*/ /*NOP; NOP; NOP; NOP;*/ }
   set_vram_write_addr(SCREEN2_COLOR_TABLE   + addr); for(byte i=0;i<8;i++) { TMS_WRITE_DATA_PORT(col);       NOP; /*NOP;*/ /*NOP; NOP;*/ /*NOP; NOP; NOP; NOP;*/ }
}

void gr4_prints(byte x, byte y, byte *s, byte color) {
   byte c;
   while(c=*s++) {
      gr4_tile(x++, y, c, color);
   }
}
