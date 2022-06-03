#define APPLE1_USE_WOZ_MONITOR 1

#define INPUT_LINE_PROMPT_CHAR '?'

#include <string.h>

#include <utils.h>
#include <apple1.h>
#include <stdlib.h>

byte  *const KEYBUF = (byte  *) 0x0200;   // use the same keyboard buffer as in WOZ monitor

byte *start_address;
byte *end_address;

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
   woz_puts("\r\r*** RAM FILL TEST ***\r");

   woz_puts("\rSTART ADDRESS "); apple1_input_line_prompt(KEYBUF, 4);  start_address = (byte *) hex_to_word(KEYBUF);
   woz_puts("\rEND   ADDRESS "); apple1_input_line_prompt(KEYBUF, 4);  end_address   = (byte *) hex_to_word(KEYBUF);

   woz_puts("\r\r");

   while(1) {
      // fill with normal data
      rand_state = 1;
      for(byte *t=start_address;;t++) {
         byte r = (byte) (rand() & 0xFF);
         *t = r;
         if(t==end_address) break;
      }

      // verify normal data
      rand_state = 1;
      for(byte *t=start_address;;t++) {
         byte r = (byte) (rand() & 0xFF);
         if(*t != r) { woz_print_hexword((unsigned int )t); woz_putc(' '); }
         if(t==end_address) break;
      }

      // fill with inverted data
      rand_state = 1;
      for(byte *t=start_address;;t++) {
         byte r = (byte) (rand() & 0xFF);
         *t = ~r;
         if(t==end_address) break;
      }

      // verify inverted data
      rand_state = 1;
      for(byte *t=start_address;;t++) {
         byte r = (byte) (rand() & 0xFF);
         if(*t != ~r) { woz_print_hexword((unsigned int )t); woz_putc(' '); }
         if(t==end_address) break;
      }

      woz_putc('.');
   }
}

