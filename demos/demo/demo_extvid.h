// TODO make it static once KickC bug is fixed
byte external_input = 0;

void flip_external_input() {
   external_input ^= 1;

   if(external_input) woz_puts("EXT INPUT ON\r");
   else               woz_puts("EXT INPUT OFF\r");

   // fill color table with transparent color so that external input can be seen
   tms_set_vram_write_addr(TMS_COLOR_TABLE);
   for(byte i=32;i!=0;i--) {
      TMS_WRITE_DATA_PORT(FG_BG(COLOR_DARK_YELLOW, COLOR_TRANSPARENT));
   }

   tms_set_external_video(external_input);  // write "external video input" bit
   tms_set_color(0);                        // transparent color
}

