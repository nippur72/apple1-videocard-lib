void prova_screen2() {
   tms_init_regs(SCREEN2_TABLE);

   byte text_color = FG_BG(COLOR_BLACK,COLOR_WHITE);

   screen2_init_bitmap(text_color);
   screen2_puts(0, 0, text_color, "*** P-LAB  VIDEO CARD SYSTEM ***");
   screen2_puts(0, 2, text_color, "16K VRAM BYTES FREE");
   screen2_puts(0, 4, text_color, "READY.");

   // display all colors in the palette
   for(byte i=0;i<16;i++) {
      screen2_puts(5,(byte)(6+i),(byte)(((15-i)<<4)+i),"     SCREEN 2     ");
   }

   vti_line(18, 45,232,187);
   vti_line(18,187,232, 45);

   SCREEN2_PLOT_MODE = PLOT_MODE_RESET;

   vti_line(18+5, 45,232+5,187);
   vti_line(18+5,187,232+5, 45);

   SCREEN2_PLOT_MODE = PLOT_MODE_INVERT;

   vti_line(18+5+5, 45,232+5+5,187);
   vti_line(18+5+5,187,232+5+5, 45);

   SCREEN2_PLOT_MODE = PLOT_MODE_SET;

   //vti_ellipse_rect(7,9,202,167);
}
