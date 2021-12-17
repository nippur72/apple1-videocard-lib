// image converted with: Convert9918 - A simple image converter for the TMS9918A
// http://harmlesslion.com 

#include "../lib/tms9918.h"

#pragma data_seg(Code)
#include "pic_c.h"
#include "pic_p.h"
#pragma data_seg(Data)

void main() {

#ifdef APPLE1
   apple1_eprom_init();
#endif

   woz_puts("DISPLAYING PICTURE...\r\r");

   // in screen 2
   tms_init_regs(SCREEN2_TABLE);
   screen2_init_bitmap(FG_BG(COLOR_WHITE,COLOR_BLACK));

   // bulk copy pattern table to VRAM
   tms_set_vram_write_addr(TMS_PATTERN_TABLE); 
   for(word t=0; t<sizeof(pic_p); t++) {
      TMS_WRITE_DATA_PORT(pic_p[t]);
   }

   // bulk copy color table to VRAM
   tms_set_vram_write_addr(TMS_COLOR_TABLE); 
   for(word t=0; t<sizeof(pic_c); t++) {
      TMS_WRITE_DATA_PORT(pic_c[t]);
   }

   // bye
   while(apple1_getkey()!=CHR_RETURN);
   woz_puts("BYE!\r");
}
