// TODO make it static once KickC bug is fixed
byte last_seconds = 0;

void demo_interrupt() {

   // resets the watch to 00:00:00.0
   _ticks   = 0;
   _seconds = 0;
   _minutes = 0;
   _hours   = 0;

   // installs the interrupt handler routine
   install_interrupt();

   // enables interrupts on the TMS9918
   tms_set_interrupt_bit(INTERRUPT_ENABLED);

   woz_puts("INTERRUPT INSTALLED\r");
   woz_puts("0 TURNS OFF\r");
   woz_puts("1 TURNS ON\r");
   woz_puts("E EXIT TO MAIN MENU\r");

   for(;;) {
      if(apple1_iskeypressed()) {
         byte k = apple1_getkey();
              if(k=='1') { tms_set_interrupt_bit(INTERRUPT_ENABLED);  woz_puts("INT ENABLED\r"); }
         else if(k=='0') { tms_set_interrupt_bit(INTERRUPT_DISABLED); woz_puts("INT DISABLED\r"); }
         else if(k=='E') break;
      }

      if(last_seconds != _seconds) {
         woz_print_hex(_hours);   woz_putc(':');
         woz_print_hex(_minutes); woz_putc(':');
         woz_print_hex(_seconds); woz_putc('.');
         woz_print_hex(_ticks);   woz_putc('\r');
         last_seconds = _seconds;
      }
   }

   // disables interrupts on the TMS9918
   tms_set_interrupt_bit(INTERRUPT_DISABLED);

   woz_puts("INTERRUPT STOPPED\r");
}
