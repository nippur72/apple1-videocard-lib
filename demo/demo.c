// TODO make screen tables parametric (fixed values calculated by macros)

#include "../lib/utils.h"
#include "../lib/apple1.h"
#include "../lib/tms9918.h"
#include "../lib/font8x8.h"
#include "../lib/tms_screen1.h"
#include "../lib/tms_screen2.h"
#include "../lib/interrupt.h"

#include "demo_screen1.h"
#include "demo_screen2.h"
#include "demo_amiga_hand.h"
#include "demo_interrupt.h"
#include "demo_extvid.h"
#include "demo_blank.h"

void help() {
   woz_puts(
      "\rTMS9918 DEMOS\r"
      "=============\r"
      "1 SCREEN1\r"
      "2 SCREEN2\r"
      "A AMIGA HAND\r"
      "I INTERRUPT\r"
      "E FLIP EXT VIDEO\r"
      "B BLANK ON/OFF\r"
      "H HELP\r"
      "0 EXITS\r\r"
   );
}

void main() {
   byte key = 'H';
   for(;;) {
           if(key == '1')  prova_screen1();
      else if(key == '2')  prova_screen2();
      else if(key == 'A')  demo_amiga_hand();
      else if(key == 'I')  demo_interrupt();
      else if(key == 'E')  flip_external_input();
      else if(key == 'B')  flip_blank();
      else if(key == 'H')  help();
      else if(key == '0')  break;
      else woz_putc(key);

      key = apple1_getkey();
   }
   woz_puts("BYE\r");
   woz_mon();
}
