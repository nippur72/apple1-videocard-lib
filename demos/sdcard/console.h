#include <stdlib.h>
#include <stdlib.h>

word *const BASIC_LOMEM = (word *) 0x004a;   // lomem pointer used by integer BASIC
word *const BASIC_HIMEM = (word *) 0x004c;   // himem pointer used by integer BASIC
byte *const KEYBUF      = (byte *) 0x0200;   // use the same keyboard buffer as in WOZ monitor

#define KEYBUFSTART (0x200)
#define KEYBUFLEN   (40)

// keyboard buffer 0x200-27F uses only the first 40 bytes, the rest is recycled for free mem

byte *const command  = (byte *) (KEYBUFSTART+KEYBUFLEN       ); // [6]  stores a 5 character command
byte *const filename = (byte *) (KEYBUFSTART+KEYBUFLEN+6     ); // [33] stores a filename or pattern
byte *const hex1     = (byte *) (KEYBUFSTART+KEYBUFLEN+6+33  ); // [5]  stores a hex parameter
byte *const hex2     = (byte *) (KEYBUFSTART+KEYBUFLEN+6+33+5); // [5]  stores a hex parameter

const byte ERR_RESPONSE = 0xFF;

// command constants, which are also byte commands to send to the MCU
const byte CMD_READ  = 0;
const byte CMD_WRITE = 1;
const byte CMD_DIR   = 2;
const byte CMD_TIME  = 3;
const byte CMD_LOAD  = 4;
const byte CMD_RUN   = 5;
const byte CMD_SAVE  = 6;
const byte CMD_TYPE  = 7;
const byte CMD_DUMP  = 8;
const byte CMD_EXIT  = 9;

// the list of recognized commands
byte *DOS_COMMANDS[] = {
   "READ",
   "WRITE",
   "DIR",   
   "TIME",
   "LOAD",
   "RUN",
   "SAVE",
   "TYPE",
   "DUMP",
   "EXIT"
};

// parse a string, get the first string delimited by space or end of string
// returns the parsed string in dest
// returns the number of character to advance the pointer
// leading and trailing spaces are ignored
// max is the (maximum) size of dest
byte get_token(byte *source, byte *dest, byte max) {
   byte i = 0;
   byte j = 0;
   byte first_char_found = 0;

   while(1) {
      byte c = source[i];
      if(c == 0) {
         break;
      }
      else if(c == 32) {
         if(first_char_found) {
            break;
         }
      }
      else {
         first_char_found = 1;
         dest[j++] = c;
      }
      if(j<max) i++;
      else break;
   }
   dest[j] = 0;
   return i+1;
}

// returns the command code or 0xff if not recognized
byte find_command() {   
   for(byte cmd=0; cmd<sizeof(DOS_COMMANDS); cmd++) {
      if(strcmp(command, DOS_COMMANDS[cmd]) == 0) return cmd;      
   }
   return 0xFF;
}

// converts the hexadecimal string argument to 16 bit word
byte hex_to_word_ok;
word hex_to_word(byte *str) {
   hex_to_word_ok = 1;
   word res=0;
   byte c;
   byte i;
   for(i=0; c=str[i]; ++i) {
      res = res << 4;
           if(c>='0' && c<='9') res += (c-'0');
      else if(c>='A' && c<='F') res += (c-65)+0x0A;
      else hex_to_word_ok = 0;
   }
   if(i>4 || i==0) hex_to_word_ok = 0;
   return res;
}

#include "cmd_read.h"
#include "cmd_write.h"
#include "cmd_load.h"
#include "cmd_save.h"
#include "cmd_type.h"
#include "cmd_dump.h"
#include "cmd_dir.h"

void console() {   

   VIA_init();

   //          1234567890123456789012345678901234567890
   woz_puts("\rREAD,WRITE,LOAD,RUN,SAVE,TYPE,DUMP,DIR\r"
              "TIME,EXIT\r");

   woz_puts("\rSD CARD DOS 1.0\r");

   // main loop   
   while(1) {

      // clear input buffer
      for(byte i=0; i<KEYBUFLEN; i++) KEYBUF[i] = 0;

      // input from keyboard
      woz_puts("\r]");
      apple1_input_line(KEYBUF, KEYBUFLEN);
      woz_putc('\r');

      // decode command
      byte *ptr = KEYBUF;
      ptr += get_token(ptr, command, 5);
      byte cmd = find_command();
      
      if(cmd == CMD_READ) {
         ptr += get_token(ptr, filename, 32);  // parse filename
         if(filename[0] == 0) {
            woz_puts("?MISSING FILENAME");
            continue;
         }

         ptr += get_token(ptr, hex1, 4);       // parse hex start address
         word start = hex_to_word(hex1);
         if(!hex_to_word_ok) {
            woz_puts("?BAD ADDRESS");
            continue;
         }
         comando_read(filename, start);
      }
      else if(cmd == CMD_WRITE) {
         // parse filename
         ptr += get_token(ptr, filename, 32);
         if(filename[0] == 0) {
            woz_puts("?MISSING FILENAME");
            continue;
         }

         // parse hex start address
         ptr += get_token(ptr, hex1, 4);
         word start = hex_to_word(hex1);
         if(!hex_to_word_ok) {
            woz_puts("?BAD ADDRESS");
            continue;
         }

         // parse hex end address
         ptr += get_token(ptr, hex2, 4);
         word end = hex_to_word(hex2);
         if(!hex_to_word_ok) {
            woz_puts("?BAD ADDRESS");
            continue;
         }

         comando_write(filename, start, end);
      }
      else if(cmd == CMD_DIR)  {
         comando_dir();
      }
      else if(cmd == CMD_TIME) {
         ptr += get_token(ptr, hex1, 4);  // parse hex timeout value
         if(strlen(hex1)!=0) {
            TIMEOUT_MAX = hex_to_word(hex1);
            if(!hex_to_word_ok) {
               woz_puts("?BAD ARGUMENT");
               continue;
            }
         }
         woz_puts("TIMEOUT_MAX: ");
         woz_print_hexword(TIMEOUT_MAX);
      }
      else if(cmd == CMD_LOAD || cmd == CMD_RUN) {
         ptr += get_token(ptr, filename, 32);  // parse filename
         if(filename[0] == 0) {
            woz_puts("?MISSING FILENAME");
            continue;
         }
         comando_load(filename, cmd);
      }
      else if(cmd == CMD_SAVE) {
         ptr += get_token(ptr, filename, 32);  // parse filename
         if(filename[0] == 0) {
            woz_puts("?MISSING FILENAME");
            continue;
         }
         comando_save(filename);
      }
      else if(cmd == CMD_TYPE) {
         ptr += get_token(ptr, filename, 32);  // parse filename
         if(filename[0] == 0) {
            woz_puts("?MISSING FILENAME");
            continue;
         }
         comando_type(filename);
      }
      else if(cmd == CMD_DUMP) {
         ptr += get_token(ptr, filename, 32);  // parse filename
         if(filename[0] == 0) {
            woz_puts("?MISSING FILENAME");
            continue;
         }

         // parse hex start address
         ptr += get_token(ptr, hex1, 4);
         word start = hex_to_word(hex1);
         if(!hex_to_word_ok) {
            woz_puts("?BAD ADDRESS");
            continue;
         }

         // parse hex end address
         ptr += get_token(ptr, hex2, 4);
         word end = hex_to_word(hex2);
         if(!hex_to_word_ok) {
            woz_puts("?BAD ADDRESS");
            continue;
         }
         if(end<start) end = 0xFFFF;
         comando_dump(filename, start, end);
      }
      else if(cmd == CMD_EXIT) {
         woz_puts("BYE\r");
         woz_mon();
      }
      else {
         if(strlen(command)!=0) {
            woz_puts(command);
            woz_puts("??");
         }        
      }

      if(TIMEOUT) {
         woz_puts("?I/O ERROR");
         TIMEOUT = 0;
      }
   }
}
