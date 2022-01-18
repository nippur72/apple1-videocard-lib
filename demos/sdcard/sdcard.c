#include <utils.h>
#include <apple1.h>

byte *const PORTB = (byte *) 0xA000;  // port B register
byte *const PORTA = (byte *) 0xA001;  // port A register
byte *const DDRB  = (byte *) 0xA002;  // port A data direction register
byte *const DDRA  = (byte *) 0xA003;  // port B data direction register

#define CPU_STROBE(v) (*PORTB = (v))  /* CPU strobe is bit 0 OUTPUT */
#define MCU_STROBE    (*PORTB & 128)  /* MCU strobe is bit 7 INPUT  */

byte TIMEOUT = 0;

void wait_mcu_strobe(byte v) {   
   if(TIMEOUT) return;

   unsigned int time = 0;
   while((v==0 && MCU_STROBE != 0) || (v!=0 && MCU_STROBE == 0)) {
      time++;
      if(time > 5000) {
         TIMEOUT = 1;
         return;
      }  
   }
}

void send_byte_to_MCU(byte data) {   
   *DDRA = 0xFF;        // set port A as output   
   *PORTA = data;       // deposit byte on the data port   
   CPU_STROBE(1);       // set strobe high   
   wait_mcu_strobe(1);  // wait for the MCU to read the data   
   CPU_STROBE(0);       // set strobe low   
   wait_mcu_strobe(0);  // wait for the MCU to set strobe low
}

byte receive_byte_from_MCU() {
   *DDRA = 0;          // set port A as input
   CPU_STROBE(0);      // set listen
   wait_mcu_strobe(1); // wait for the MCU to deposit data
   byte data = *PORTA; // read data
   CPU_STROBE(1);      // set strobe high
   wait_mcu_strobe(0); // wait for the MCU to set strobe low
   CPU_STROBE(0);      // set strobe low
   return data;
}

void VIA_init() {
   *DDRB = 1;       // pin 1 is output (CPU_STROBE), pin7 is input (MCU_STROBE)
   CPU_STROBE(0);   // initial state
}

void messagio_test() {
   
   TIMEOUT = 0;           // resetta il timeout
   send_byte_to_MCU(42);  // manda il comando 42

   if(TIMEOUT) {
      woz_puts("\rTIMEOUT\r");            
      return;
   }

   // legge la stringa di ritorno
   while(1) {
      byte data = receive_byte_from_MCU();
      if(TIMEOUT) {
         woz_puts("\rTIMEOUT\r");
         break;
      }
      if(data == 0) break;  // end of string
      woz_putc(data);
   }
}

void main() {   

   VIA_init();

   woz_puts("\rMCU TEST\r\r");
   woz_puts("[4] SEND 42 TO MCU\r");
   woz_puts("[0] EXIT\r");

   byte data;

   // loop continuo
   while(1) {
      byte key = apple1_getkey();

           if(key == '4') messagio_test();         
      else if(key == '0') {
         woz_puts("BYE\r");
         woz_mon();
      }
   }
}

