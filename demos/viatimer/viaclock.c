#define APPLE1_USE_WOZ_MONITOR 1

#define INPUT_LINE_PROMPT_CHAR '?'

#include <string.h>

#include <utils.h>
#include <apple1.h>
#include <via.h>
#include <interrupt.h>
#include <stdlib.h>
#include <division.h>

#include "..\..\lib\c64font.h"

const word ONE_TICK = 16666;  // timer constant for 1/60 second

void enable_timer_interrupt() {

   // install the interrupt handler
   install_interrupt((word) &time_interrupt_handler);

   *VIA_IER  = 0b11000000;       // enable T1 interrupts
   *VIA_ACR  = 0b01000000;       // T1 continous, PB7 disabled
   *VIA_T1CL = BYTE0(ONE_TICK);  // programs the counter to 1/60
   *VIA_T1CH = BYTE1(ONE_TICK);  // and starts counting
}

void disable_timer_interrupt() {
   *VIA_IER  = 0b01000000;   // disable T1 interrupts
   *VIA_ACR  = 0;            // stop T1 (makes it one-shot)
}

byte last_min = 0xFF;

void print_digit_scanline(byte c, byte y) {
   word index = ((word) c-32) * 8 + (word) y;
   byte *ptr = FONT + index;

   byte mask = 128;
   for(byte t=0;t<8;t++) {
      if(*ptr & mask) woz_putc('@');
      else            woz_putc(' ');
      mask = mask >> 1;
   }
}

void print_clock() {
   byte c1,c2,c3,c4;

   if(last_min != _minutes) {
      last_min = _minutes;

      c1 = divr8u(_hours, 10, 0); c2 = rem8u;
      c3 = divr8u(_minutes, 10, 0); c4 = rem8u;

      woz_puts("\r\r\r\r\r\r\r\r");
      for(byte t=0;t<8;t++) {
         print_digit_scanline(c1+'0',t);
         print_digit_scanline(c2+'0',t);
         print_digit_scanline(   ':',t);
         print_digit_scanline(c3+'0',t);
         print_digit_scanline(c4+'0',t);
      }
      woz_puts("\r\r\r\r\r\r\r\r");
   }
}

void bye() {
   disable_timer_interrupt();
   woz_puts("BYE!\r");
   woz_mon();
}

byte  *const KEYBUF = (byte  *) 0x0200;   // use the same keyboard buffer as in WOZ monitor

void main() {
   enable_timer_interrupt();

   woz_puts("\r\r*** APPLE1 CLOCK ***\r\r");

   woz_puts("\r\rREQUIRES A VIA 6522 AT $A000\r\r");

   woz_puts("\rWHAT TIME IS IT (HOURS  ) "); apple1_input_line_prompt(KEYBUF, 2);
   _hours = (byte) atoi(KEYBUF);
   woz_puts("\rWHAT TIME IS IT (MINUTES) "); apple1_input_line_prompt(KEYBUF, 2);
   _minutes = (byte) atoi(KEYBUF);

   woz_putc('\r');

   while(1) {
      byte k = apple1_readkey();
      if(k=='X') break;
      print_clock();
   }

   bye();
}

