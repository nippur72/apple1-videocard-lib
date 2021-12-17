void demo_end_of_frame() {
   tms_init_regs(SCREEN1_TABLE);
   screen1_prepare();
   screen1_load_font();

   screen1_puts(
      CLS "\n"
      "Waitining for 600 ticks by\n"
      "counting the " REVERSE_ON "END-OF-FRAME" REVERSE_OFF " bit.\n\n"
      "It will take about 10 seconds.\n\n"
   );

   word secs = 0;
   word ticks = 0;
   while(secs<10) {
      tms_wait_end_of_frame();
      ticks++;
      if(ticks==60) {
         secs++;
         ticks = 0;
         screen1_puts("60 ticks!\n");
      }
   }
   screen1_puts("\n\nDONE!");
}

