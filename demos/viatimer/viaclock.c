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

// const word ONE_TICK = 15996;  // timer constant for 1/60 second calculated as 14318180/14*61/65/60

word ONE_TICK;

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
byte last_sec = 0xFF;

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

      for(byte t=0;t<24;t++) woz_putc('\r');      
      for(byte t=0;t<8;t++) {
         print_digit_scanline(c1+'0',t);
         print_digit_scanline(c2+'0',t);
         print_digit_scanline(   ':',t);
         print_digit_scanline(c3+'0',t);
         print_digit_scanline(c4+'0',t);
      }      
      for(byte t=0;t<4;t++) woz_putc('\r');
      last_sec = 0;
   }   

   if(last_sec < _seconds) {
      //if(last_sec > 20) woz_putc('>');
      /*
           if(last_sec < 10) { woz_puts("...10 "); last_sec = 10; }
      else if(last_sec < 20) { woz_puts("...20 "); last_sec = 20; }
      else if(last_sec < 30) { woz_puts("...30 "); last_sec = 30; }
      else if(last_sec < 40) { woz_puts("...40 "); last_sec = 40; }
      else if(last_sec < 50) { woz_puts("...50 "); last_sec = 50; }
      */

      c1 = divr8u(last_sec, 10, 0); c2 = rem8u;
      woz_putc(' ');
      woz_putc(c1+'0');
      woz_putc(c2+'0');
      woz_putc(' ');
      last_sec++;
   }
}

void bye() {
   disable_timer_interrupt();
   woz_puts("BYE!\r");
   woz_mon();
}

void detect_machine_type() {

   // detect if VIA 6522 is present
   *VIA_DDRB = 0xA5;
   if(*VIA_DDRB != 0xA5) {                    
      woz_puts("\r\rSORRY, THIS PROGRAM NEEDS A VIA 6522 \rMAPPED AT ADDRESS $A000\r\r");
      bye();
   }

   // detect if genuine apple1 or replica-1

   woz_puts("\r\rRUNNING ON ");

   const word *ptr1 = (word *) 0xF000;
   const word *ptr2 = (word *) 0xFF00;

   if(*ptr1 == *ptr2) {
      // genuine Apple1
      ONE_TICK = 15996;  // timer constant for 1/60 second calculated as 14318180/14*61/65/60      
      woz_puts("APPLE-1");
   }
   else {
      // Replica-1
      ONE_TICK = 16667;  // timer constant for 1/60 second calculated as 5000000/5/60
      woz_puts("REPLICA-1");
   }

   woz_puts("\r\r");   
}

byte  *const KEYBUF = (byte  *) 0x0200;   // use the same keyboard buffer as in WOZ monitor

void main() {
   woz_puts("\r\r*** APPLE-1 CLOCK ***\r\r");

   detect_machine_type();

   woz_puts("\rWHAT TIME IS IT ?\r");

   woz_puts("\r(HOURS  ) "); apple1_input_line_prompt(KEYBUF, 2);
   _hours = (byte) atoi(KEYBUF);
   woz_puts("\r(MINUTES) "); apple1_input_line_prompt(KEYBUF, 2);
   _minutes = (byte) atoi(KEYBUF);
   _seconds = 0;

   enable_timer_interrupt();

   woz_putc('\r');

   while(1) {
      byte k = apple1_readkey();
      if(k=='X') break;
      print_clock();
   }

   bye();
}

