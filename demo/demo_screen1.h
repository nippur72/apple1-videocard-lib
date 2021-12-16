void screen1_square_sprites() {
   // fills first sprite pattern with 255
   tms_set_vram_write_addr(SCREEN1_SPRITE_PATTERNS);    // start writing in the sprite patterns
   for(byte i=0;i<8;i++) {
      TMS_WRITE_DATA_PORT(255);
   }

   // set sprite coordinates
   tms_set_vram_write_addr(SCREEN1_SPRITE_ATTRS);       // start writing in the sprite attribute
   for(byte i=0;i<32;i++) {
      TMS_WRITE_DATA_PORT((6+i)*8); NOP; NOP; NOP; NOP; // y coordinate
      TMS_WRITE_DATA_PORT((6+i)*8); NOP; NOP; NOP; NOP; // x coordinate
      TMS_WRITE_DATA_PORT(0);       NOP; NOP; NOP; NOP; // name
      TMS_WRITE_DATA_PORT(i);       NOP; NOP; NOP; NOP; // color
   }
}

void prova_screen1() {
   tms_init_regs(SCREEN1_TABLE);
   screen1_prepare();
   screen1_load_font();

   screen1_home();       screen1_puts("*** P-LAB  VIDEO CARD SYSTEM ***");
   screen1_locate(0, 2); screen1_puts("16K VRAM BYTES FREE");
   screen1_locate(0, 4); screen1_puts("READY.");

   screen1_locate(0, 10);
   for(word i=0;i<256;i++) screen1_putc((byte)i);

   screen1_square_sprites();
}

