// TODO make it static once KickC bug is fixed
byte blank = BLANK_OFF;

void flip_blank() {
   blank ^= 1;

   if(blank == BLANK_OFF) woz_puts("NORMAL\r");
   else                   woz_puts("BLANK\r");

   // write "blank" bit
   tms_blank(blank);
}
