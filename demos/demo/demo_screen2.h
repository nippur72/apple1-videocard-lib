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
}

