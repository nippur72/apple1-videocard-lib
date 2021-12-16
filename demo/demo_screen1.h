#include <string.h>

void screen1_square_sprites() {
   // fills first sprite pattern with 255
   tms_set_vram_write_addr(TMS_SPRITE_PATTERNS);    // start writing in the sprite patterns
   for(byte i=0;i<8;i++) {
      TMS_WRITE_DATA_PORT(255);
   }

   // set sprite coordinates
   tms_set_vram_write_addr(TMS_SPRITE_ATTRS);       // start writing in the sprite attribute
   for(byte i=0;i<32;i++) {
      TMS_WRITE_DATA_PORT((6+i)*8); NOP; NOP; NOP; NOP; // y coordinate
      TMS_WRITE_DATA_PORT((6+i)*8); NOP; NOP; NOP; NOP; // x coordinate
      TMS_WRITE_DATA_PORT(0);       NOP; NOP; NOP; NOP; // name
      TMS_WRITE_DATA_PORT(i);       NOP; NOP; NOP; NOP; // color
   }
}

byte buffer[32];
void demo_screen1() {
   tms_init_regs(SCREEN1_TABLE);
   screen1_prepare();
   screen1_load_font();

   screen1_putc(CHR_CLS);
   screen1_puts(
      "*** P-LAB  VIDEO CARD SYSTEM ***\n"
      "16K VRAM BYTES FREE\n\n"
      "READY.\n\n\n"
   );

   screen1_puts("what about " REVERSE_ON " REVERSE text " REVERSE_OFF " ?\n\n\n\n");
   
   for(word i=32;i<128;i++) screen1_putc((byte)i);
   screen1_puts("\n\n" REVERSE_ON);
   for(word i=32;i<128;i++) screen1_putc((byte)i);

   screen1_square_sprites();

   while(1) {
      screen1_puts(REVERSE_OFF "\n\nWRITE HERE: >");
      screen1_strinput(buffer,16);
      if(strlen(buffer)==0) break;
      screen1_puts("\n\n\nyou wrote: '");
      screen1_puts(buffer);
      screen1_puts("'");      
   }
}

