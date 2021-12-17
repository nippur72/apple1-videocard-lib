// clears all the sprites
void tms_clear_sprites() {
   // fills first sprite pattern with 0
   tms_set_vram_write_addr(TMS_SPRITE_PATTERNS);  
   for(byte i=0;i<8;i++) {
      TMS_WRITE_DATA_PORT(0);
   }

   // set sprite coordinates to (0,0) and set pattern name 0
   tms_set_vram_write_addr(TMS_SPRITE_ATTRS);       
   for(byte i=0;i<32;i++) {
      TMS_WRITE_DATA_PORT((6+i)*8); NOP; NOP; NOP; NOP; // y coordinate
      TMS_WRITE_DATA_PORT((6+i)*8); NOP; NOP; NOP; NOP; // x coordinate
      TMS_WRITE_DATA_PORT(0);       NOP; NOP; NOP; NOP; // name
      TMS_WRITE_DATA_PORT(i);       NOP; NOP; NOP; NOP; // color
   }
}
