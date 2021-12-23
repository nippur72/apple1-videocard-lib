#include <font8x8.h>

#include "cbm_balloon.h"

void move_balloon(byte x, byte y, byte color) {
   tms_sprite spr0,spr1;
   spr0.x = x;
   spr0.y = y;
   spr0.name = 0;
   spr0.color = color;
   spr1.x = spr0.x;
   spr1.y = spr0.y + 16*1;
   spr1.name = spr0.name + 4;
   spr1.color = spr0.color;
   tms_set_sprite(0, &spr0);
   tms_set_sprite(1, &spr1);   
}

void demo_balloon() {
   tms_init_regs(SCREEN2_TABLE);   
   tms_set_color(COLOR_LIGHT_BLUE);

   byte text_color = FG_BG(COLOR_GREY,COLOR_DARK_BLUE);

   screen2_init_bitmap(text_color);
              // 12345678901234567890123456789012
   screen2_puts("*** COMMODORE-APPLE BASIC V2 ***", 0, 0, text_color);
   screen2_puts("38911 BASIC BYTES FREE"          , 0, 2, text_color);
   screen2_puts("READY."                          , 0, 4, text_color);
   screen2_puts(" "                               , 0, 5, FG_BG(COLOR_DARK_BLUE,COLOR_GREY));

   // ballon demo

   // define sprites using bitmap fonts
   tms_copy_to_vram(cbm_balloon, 4*8*2, TMS_SPRITE_PATTERNS);
   tms_set_sprite_double_size(1);   // set 16x16 sprites   
   tms_set_sprite_magnification(0); // set double pixel sprites

   int x = 200;
   int y = 80;
   int dx = 1;
   int dy = 1;
   int delay = 0;   
   byte sprcolor = COLOR_LIGHT_YELLOW;

   for(;;) {
      for(delay=0; delay<800; delay++) {
         delay = delay+1;
         delay = delay-1;
      }

      if(apple1_readkey()==0x0d) break;
      
      if(x>=228 || x<=0) { dx = -dx; sprcolor++; }
      if(y>=148 || y<=0) { dy = -dy; sprcolor++; }

      x += dx;
      y += dy;
      move_balloon((byte)x,(byte)y, COLOR_WHITE);     
   }
}

