#include <stdlib.h>
#include <string.h>

byte **const BASIC_LOMEM = (byte **) 0x004a;   // lomem pointer used by integer BASIC
byte **const BASIC_HIMEM = (byte **) 0x004c;   // himem pointer used by integer BASIC
byte  *const KEYBUF      = (byte  *) 0x0200;   // use the same keyboard buffer as in WOZ monitor

#define KEYBUFSTART (0x200)
#define KEYBUFLEN   (79)

// keyboard buffer 0x200-27F uses only the first 40 bytes, the rest is recycled for free mem

byte *const command  = (byte *) (KEYBUFSTART+KEYBUFLEN       ); // [6]  stores a 5 character command
byte *const filename = (byte *) (KEYBUFSTART+KEYBUFLEN+6     ); // [33] stores a filename or pattern
byte *const hex1     = (byte *) (KEYBUFSTART+KEYBUFLEN+6+33  ); // [5]  stores a hex parameter
byte *const hex2     = (byte *) (KEYBUFSTART+KEYBUFLEN+6+33+5); // [5]  stores a hex parameter

const byte OK_RESPONSE   = 0x00;
const byte WAIT_RESPONSE = 0x01;
const byte ERR_RESPONSE  = 0xFF;

// command constants, which are also byte commands to send to the MCU
const byte CMD_READ  =  0;  
const byte CMD_WRITE =  1;  
const byte CMD_DIR   =  2;  
const byte CMD_TIME  =  3;  
const byte CMD_LOAD  =  4;  
const byte CMD_RUN   =  5;  
const byte CMD_SAVE  =  6;  
const byte CMD_TYPE  =  7;  
const byte CMD_DUMP  =  8;  
const byte CMD_JMP   =  9;  
const byte CMD_BAS   = 10;  
const byte CMD_DEL   = 11;  
const byte CMD_LS    = 12;  
const byte CMD_CD    = 13;  
const byte CMD_MKDIR = 14;  
const byte CMD_RMDIR = 15;  
const byte CMD_RM    = 16;  
const byte CMD_MD    = 17;  
const byte CMD_RD    = 18;  
const byte CMD_PWD   = 19;
const byte CMD_TEST  = 20;
const byte CMD_EXIT  = 21;

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
   "JMP",
   "BAS",
   "DEL",
   "LS",
   "CD",
   "MKDIR",
   "RMDIR",
   "RM",
   "MD",
   "RD",
   "PWD",
   "TEST",
   "EXIT"
};

// parse a string, get the first string delimited by space or end of string
// returns the parsed string in dest
// returns the number of character to advance the pointer
// leading and trailing spaces are ignored
// max is the (maximum) size of dest

void get_token(byte *dest, byte max) {
   byte i = 0;
   byte j = 0;
   byte first_char_found = 0;

   while(1) {
      byte c = token_ptr[i];
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
   token_ptr += i+1;
}

// looks "command" into table "DOS_COMMANDS" and
// if found sets the index in "cmd", 0xFF if not found
void find_cmd() {
   for(cmd=0; cmd<sizeof(DOS_COMMANDS); cmd++) {
      byte *ptr = DOS_COMMANDS[cmd];
      for(byte j=0;;j++) {
         if(command[j] != ptr[j]) break;
         else if(command[j] == 0) return;
      }      
   }
   cmd = 0xFF;
}

void hex_to_word(byte *str) {
   hex_to_word_ok = 1;
   tmpword=0;
   byte c;
   byte i;
   for(i=0; c=str[i]; ++i) {
      tmpword = tmpword << 4;
           if(c>='0' && c<='9') tmpword += (c-'0');
      else if(c>='A' && c<='F') tmpword += (c-65)+0x0A;
      else hex_to_word_ok = 0;
   }
   if(i>4 || i==0) hex_to_word_ok = 0;
}

void strcat(char *dest, char *src) {
   while(*dest) dest++;
   while(*src) *dest++ = *src++;
   *dest = 0;
}

void append_hex_digit(char *dest, byte digit) {
   while(*dest) dest++;
   if(digit<10) digit += '0'; 
   else         digit += 'A' - 10;   
   *dest++ = digit;
   *dest = 0;      
}

void append_hex_tmpword(char *dest) {   
   append_hex_digit(dest, *((byte *)&tmpword+1) >> 4);
   append_hex_digit(dest, *((byte *)&tmpword+1) & 0x0F);
   append_hex_digit(dest, *((byte *)&tmpword+0) >> 4);
   append_hex_digit(dest, *((byte *)&tmpword+0) & 0x0F);   
}

#include "cmd_read.h"
#include "cmd_write.h"
#include "cmd_load.h"
#include "cmd_save.h"
#include "cmd_type.h"
#include "cmd_dump.h"
#include "cmd_del.h"
#include "cmd_dir.h"
#include "cmd_mkdir.h"
#include "cmd_rmdir.h"
#include "cmd_chdir.h"
#include "cmd_pwd.h"
#include "cmd_test.h"

void console() {   

   VIA_init();

   woz_puts("\r\r*** SD CARD OS 1.0\r\r");

   cmd = 0;

   // main loop   
   while(1) {

      // clear input buffer
      for(byte i=0; i<KEYBUFLEN; i++) KEYBUF[i] = 0;

      // do not print extra newline for commands that do not have output (CD)
      if(cmd != CMD_CD) woz_putc('\r');                  

      // prints the current path before the prompt
      comando_pwd();        
      
      apple1_input_line_prompt(KEYBUF, KEYBUFLEN);
      if(KEYBUF[0]!=0) woz_putc('\r');

      // decode command
      token_ptr = KEYBUF;
      get_token(command, 5);

      find_cmd();  // put command in cmd
      
      if(cmd == CMD_READ) {
         get_token(filename, 32);  // parse filename
         if(filename[0] == 0) {
            woz_puts("?MISSING FILENAME");
            continue;
         }

         get_token(hex1, 4);       // parse hex start address
         hex_to_word(hex1);         
         start_address = tmpword;

         if(!hex_to_word_ok) {
            woz_puts("?BAD ADDRESS");
            continue;
         }
         comando_read();
      }
      else if(cmd == CMD_WRITE) {
         // parse filename
         get_token(filename, 32);
         if(filename[0] == 0) {
            woz_puts("?MISSING FILENAME");
            continue;
         }

         // parse hex start address
         get_token(hex1, 4);
         hex_to_word(hex1);
         start_address = tmpword;         

         if(!hex_to_word_ok) {
            woz_puts("?BAD ADDRESS");
            continue;
         }

         // parse hex end address
         get_token(hex2, 4);
         hex_to_word(hex2); 
         end_address = tmpword;
         if(!hex_to_word_ok) {
            woz_puts("?BAD ADDRESS");
            continue;
         }
         comando_write();
      }
      else if(cmd == CMD_DIR || cmd == CMD_LS) {
         get_token(filename, 32);  // parse filename
         comando_dir(cmd);
      }
      else if(cmd == CMD_TIME) {
         get_token(hex1, 4);  // parse hex timeout value
         if(strlen(hex1)!=0) {
            hex_to_word(hex1);
            if(!hex_to_word_ok) {
               woz_puts("?BAD ARGUMENT");
               continue;
            }
            TIMEOUT_MAX = tmpword;
         }
         woz_puts("TIMEOUT MAX: $");
         woz_print_hexword(TIMEOUT_MAX);
         woz_puts(" CURR: $");
         woz_print_hexword(TIMEOUT_RANGE);
         TIMEOUT_RANGE = 0;
      }
      else if(cmd == CMD_LOAD || cmd == CMD_RUN) {
         get_token(filename, 32);  // parse filename
         if(filename[0] == 0) {
            woz_puts("?MISSING FILENAME");
            continue;
         }
         comando_load_bas();
      }
      else if(cmd == CMD_SAVE) {
         get_token(filename, 32);  // parse filename
         if(filename[0] == 0) {
            woz_puts("?MISSING FILENAME");
            continue;
         }
         //comando_save();
         
         get_token(hex1, 4);
         if(hex1[0] != 0) {
            // it's SAVE binary file
            hex_to_word(hex1);
            if(!hex_to_word_ok) {
               woz_puts("?BAD ADDRESS");
               continue;
            }
            start_address = tmpword;         

            strcat(filename, "#06");
            append_hex_tmpword(filename);

            get_token(hex2, 4);
            hex_to_word(hex2);
            if(!hex_to_word_ok) {
               woz_puts("?BAD ADDRESS");
               continue;
            }
            end_address = tmpword;
            comando_write();
         }
         else {
            comando_save_bas();
         }         
      }
      else if(cmd == CMD_TYPE) {
         get_token(filename, 32);  // parse filename
         if(filename[0] == 0) {
            woz_puts("?MISSING FILENAME");
            continue;
         }
         comando_type();
      }
      else if(cmd == CMD_DUMP) {
         get_token(filename, 32);  // parse filename
         if(filename[0] == 0) {
            woz_puts("?MISSING FILENAME");
            continue;
         }

         start_address = 0;
         end_address = 0xffff;

         // parse hex start address
         get_token(hex1, 4);
         hex_to_word(hex1);
         
         if(hex_to_word_ok) {
            start_address = tmpword;
            // parse hex end address
            get_token(hex2, 4);
            hex_to_word(hex2);            
            if(hex_to_word_ok) end_address = tmpword;
         }         
         comando_dump();
      }
      else if(cmd == CMD_JMP) {
         get_token(hex1, 4);  // parse hex 
         hex_to_word(hex1);            
         if(!hex_to_word_ok) {
            woz_puts("?BAD ARGUMENT");
            continue;
         }
         asm {
            jmp (tmpword)
         }
      }
      else if(cmd == CMD_BAS) {
         woz_puts("BAS ");
         bas_info();
      }
      else if(cmd == CMD_DEL || cmd == CMD_RM) {
         get_token(filename, 32);  // parse filename
         if(filename[0] == 0) {
            woz_puts("?MISSING FILENAME");
            continue;
         }
         comando_del();
      }
      else if(cmd == CMD_MKDIR || cmd == CMD_MD) {
         get_token(filename, 32);  // parse filename
         if(filename[0] == 0) {
            woz_puts("?MISSING FILENAME");
            continue;
         }
         comando_mkdir();
      }
      else if(cmd == CMD_RMDIR || cmd == CMD_RD) {
         get_token(filename, 32);  // parse filename
         if(filename[0] == 0) {
            woz_puts("?MISSING FILENAME");
            continue;
         }
         comando_rmdir();
      }
      else if(cmd == CMD_CD) {
         get_token(filename, 32);  // parse filename
         comando_cd();         
      }
      else if(cmd == CMD_PWD) {                  
         comando_pwd();         
      }
      else if(cmd == CMD_TEST) {
         comando_test();
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
