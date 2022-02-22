#define APPLE1_USE_WOZ_MONITOR 1

//#pragma start_address(0x8000)
//#pragma start_address(0x0280)

#include <string.h>

#include <utils.h>
#include <apple1.h>
#include <via.h>
#include <interrupt.h>

const word ONE_TICK = 16666;  // timer constant for 1/60 second

byte last_sec = 0xFF;

void print_clock() {   
   if(last_sec != _seconds) {
      last_sec = _seconds;
      woz_print_hex(_hours); woz_putc('.');
      woz_print_hex(_minutes); woz_putc('.');
      woz_print_hex(_seconds); woz_putc('\r');
   }      
}

// same routine as interrupt handler, but does not use RTI
void time_count() {  
   // update the watch
   if(++_ticks == 60) {
      _ticks = 0;
      if(++_seconds == 60) {
         _seconds = 0;
         if(++_minutes == 60) {
            _minutes = 0;
            _hours++;
         }
      }
   }
   _irq_trigger = 1;         // signals that an interrupt has been triggered   

   #ifdef VIA6522
   VIA_ACK_INTERRUPT;
   #endif
}

void test_timer_poll() {

   woz_puts("*** TIMER IN POLL MODE ***\r\rPRESS X TO EXIT\r\r");

   *VIA_IER  = 0;                // disable T1 interrupts (poll mode)
   *VIA_ACR  = 0b01000000;       // timer 1 generates continuous interrupts
   *VIA_T1CL = BYTE0(ONE_TICK);  // programs the counter to 1/60
   *VIA_T1CH = BYTE1(ONE_TICK);  // and starts counting

   while(1)
   {
      // wait until IFR bit 6 is set by the counter reaching zero
      while(!(*VIA_IFR & VIA_IFR_MASK_T1));

      // do the time counting as if it was a normal iterrupt
      time_count();
      print_clock();
      if(apple1_readkey()=='X') break;
   }   

   *VIA_ACR  = 0; // stop T1 (makes it one-shot)
}


void test_timer_interrupt() {

   woz_puts("*** TIMER IN INTERRUPT MODE ***\r\rPRESS X TO EXIT\r\r");

   // install the interrupt handler
   install_interrupt((word) &time_interrupt_handler);

   *VIA_IER  = 0b11000000;       // enable T1 interrupts
   *VIA_ACR  = 0b01000000;       // T1 continous, PB7 disabled
   *VIA_T1CL = BYTE0(ONE_TICK);  // programs the counter to 1/60
   *VIA_T1CH = BYTE1(ONE_TICK);  // and starts counting
   
   while(1)
   {
      print_clock();
      if(apple1_readkey()=='X') break;
   }

   *VIA_IER  = 0b01000000;   // disable T1 interrupts   
   *VIA_ACR  = 0;            // stop T1 (makes it one-shot)
}

void main() {
   woz_puts("\rTESTING THE VIA 6522\r");

   while(1) {
      woz_puts("\r"
         "P - POLL MODE\r"
         "I - INTERRUPT MODE\r"
         "X - EXIT\r\r"
      );

      byte k = apple1_getkey();

           if(k=='P') test_timer_poll();
      else if(k=='I') test_timer_interrupt();
      else if(k=='X') break;
   }

   woz_puts("BYE!\r");
   woz_mon();
}

