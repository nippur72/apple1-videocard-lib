#include <font8x8.h>

void demo_screen2() {
   tms_init_regs(SCREEN2_TABLE);

   byte text_color = FG_BG(COLOR_BLACK,COLOR_WHITE);

   screen2_init_bitmap(text_color);
   screen2_puts("*** P-LAB  VIDEO CARD SYSTEM ***", 0, 0, text_color);
   screen2_puts("16K VRAM BYTES FREE"             , 0, 2, text_color);
   screen2_puts("READY."                          , 0, 4, text_color);

   // display all colors in the palette
   for(byte i=0;i<16;i++) {
      screen2_puts("     SCREEN 2     ",5,(byte)(6+i),(byte)(((15-i)<<4)+i));
   }

   screen2_line(18, 45,232,187);
   screen2_line(18,187,232, 45);

   screen2_plot_mode = PLOT_MODE_RESET;

   screen2_line(18+5, 45,232+5,187);
   screen2_line(18+5,187,232+5, 45);

   screen2_plot_mode = PLOT_MODE_INVERT;

   screen2_line(18+5+5, 45,232+5+5,187);
   screen2_line(18+5+5,187,232+5+5, 45);

   screen2_plot_mode = PLOT_MODE_SET;

   // define sprites using bitmap fonts
   tms_copy_to_vram(&FONT[64*8], 32*8, TMS_SPRITE_PATTERNS);

   // set 16x16 sprites
   tms_set_sprite_double_size(1);

   // set double pixel sprites
   tms_set_sprite_magnification(1);

   tms_sprite spr;
   for(byte t=0;t<32;t++) {
      spr.x = 10 + t*32;
      spr.y =  5 + t*32;
      spr.name = t;
      spr.color = t+1;
      tms_set_sprite(t, &spr);
   }
}
