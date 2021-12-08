// TODO make it static once KickC bug is fixed
byte blank = 0;

void flip_blank() {
   blank ^= 1;

   if(blank) woz_puts("NORMAL\r");
   else      woz_puts("BLANK\r");

   // write "blank" bit
   tms_blank(blank);
}
