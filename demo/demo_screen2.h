void prova_screen2() {
   tms_init_regs(SCREEN2_TABLE);
   screen2_init_bitmap(FG_BG(COLOR_WHITE,COLOR_BLACK));   

   screen2_puts(0,0,FG_BG(COLOR_BLACK,COLOR_WHITE),"*** P-LAB  VIDEO CARD SYSTEM ***");
   screen2_puts(0,2,FG_BG(COLOR_BLACK,COLOR_WHITE),"16K VRAM BYTES FREE");
   screen2_puts(0,4,FG_BG(COLOR_BLACK,COLOR_WHITE),"READY.");

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
