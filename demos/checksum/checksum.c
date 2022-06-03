//#pragma start_address(0x8000)

#define APPLE1_USE_WOZ_MONITOR 1

#define INPUT_LINE_PROMPT_CHAR '?'

#include <string.h>

#include <utils.h>
#include <apple1.h>
#include <stdlib.h>

byte  *const KEYBUF = (byte  *) 0x0200;   // use the same keyboard buffer as in WOZ monitor

unsigned int start_address;
unsigned int end_address;
unsigned int check_sum;

unsigned int hex_to_word(byte *str) {
   unsigned int tmpword=0;
   byte hex_to_word_ok = 1;
   byte c;
   byte i;
   for(i=0; c=str[i]; ++i) {
      tmpword = tmpword << 4;
           if(c>='0' && c<='9') tmpword += (c-'0');
      else if(c>='A' && c<='F') tmpword += (c-65)+0x0A;
      else hex_to_word_ok = 0;
   }
   if(i>4 || i==0) hex_to_word_ok = 0;
   return tmpword;
}

void main() {
   woz_puts("\r\r*** CHECKSUM ***\r");

   while(1) {
      woz_puts("\rSTART ADDRESS "); apple1_input_line_prompt(KEYBUF, 4);  if(KEYBUF[0]!=0) start_address = hex_to_word(KEYBUF);
      woz_puts("\rEND   ADDRESS "); apple1_input_line_prompt(KEYBUF, 4);  if(KEYBUF[0]!=0) end_address   = hex_to_word(KEYBUF);

      check_sum = 0;

      for(unsigned int t=start_address;;t++) {
         byte b = *((byte *)t);
         check_sum += (unsigned int) b;
         if(t==end_address) break;
      }

      woz_puts("\r\r");
      woz_print_hexword(start_address);
      woz_putc('-');
      woz_print_hexword(end_address);
      woz_puts(" => ");
      woz_print_hexword(check_sum);
      woz_puts(" CHECKSUM\r\r");
   }
}

