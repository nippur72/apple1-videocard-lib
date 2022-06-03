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

// 0 = bad ram
// 1 = good ram
// 2 = rom
byte test_location(byte *ptr) {
   byte oldval = *ptr;
   *ptr = ~oldval; if(*ptr == oldval) return 2;  // ROM found

                  if(*ptr != ~oldval) return 0;

   *ptr =      0; if(*ptr !=   0 ) return 0;
   *ptr =    255; if(*ptr != 255 ) return 0;

   *ptr = (1<<0); if(*ptr != (1<<0) ) return 0;
   *ptr = (1<<1); if(*ptr != (1<<1) ) return 0;
   *ptr = (1<<2); if(*ptr != (1<<2) ) return 0;
   *ptr = (1<<3); if(*ptr != (1<<3) ) return 0;
   *ptr = (1<<4); if(*ptr != (1<<4) ) return 0;
   *ptr = (1<<5); if(*ptr != (1<<5) ) return 0;
   *ptr = (1<<6); if(*ptr != (1<<6) ) return 0;
   *ptr = (1<<7); if(*ptr != (1<<7) ) return 0;

   *ptr = (~(1<<0)); if(*ptr != (~(1<<0)) ) return 0;
   *ptr = (~(1<<1)); if(*ptr != (~(1<<1)) ) return 0;
   *ptr = (~(1<<2)); if(*ptr != (~(1<<2)) ) return 0;
   *ptr = (~(1<<3)); if(*ptr != (~(1<<3)) ) return 0;
   *ptr = (~(1<<4)); if(*ptr != (~(1<<4)) ) return 0;
   *ptr = (~(1<<5)); if(*ptr != (~(1<<5)) ) return 0;
   *ptr = (~(1<<6)); if(*ptr != (~(1<<6)) ) return 0;
   *ptr = (~(1<<7)); if(*ptr != (~(1<<7)) ) return 0;

   *ptr = oldval; if(*ptr != oldval) return 0;

   return 1; // good ram
}

void main() {
   woz_puts("\r\r*** RAM CHECK ***\r");

   while(1) {
      byte curr_state = 3;

      woz_puts("\rSTART ADDRESS "); apple1_input_line_prompt(KEYBUF, 4);  start_address = (byte *) hex_to_word(KEYBUF);
      woz_puts("\rEND   ADDRESS "); apple1_input_line_prompt(KEYBUF, 4);  end_address   = (byte *) hex_to_word(KEYBUF);

      woz_puts("\r\r");

      for(byte *t=start_address;;t++) {
         byte new_state = test_location(t);
         if(new_state != curr_state) {
            curr_state = new_state;
            woz_putc('\r');
            woz_print_hexword((unsigned int )t);
                 if(curr_state == 0) { woz_puts(" BAD RAM"); curr_state = 3; }
            else if(curr_state == 1) woz_puts(" RAM");
            else if(curr_state == 2) woz_puts(" ROM");
         }

         //if(!) {
         //   woz_print_hexword((unsigned int )t);
         //   woz_putc(' ');
         //}

         if(t==end_address) break;
         //if((t & 0xFF)==0) woz_putc('.');
      }

      woz_puts("\r\rDONE\r\r");
   }
}

