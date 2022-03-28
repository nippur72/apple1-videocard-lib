// LOAD: for basic files in prodos format
// it is like a normal CMD_READ

// PRODOS format:
// "A","1", 510 bytes low memory, basic program

// global cmd

void comando_load_bas() {

   // send command byte
   send_byte_to_MCU(CMD_LOAD);
   if(TIMEOUT) return;

   // send filename
   send_string_to_MCU(filename);
   if(TIMEOUT) return;

   // wait for OK response, MCU sends many WAIT_RESPONSE to avoid TIMEOUT
   while(1) {
      byte response = receive_byte_from_MCU();
      if(TIMEOUT) return;

      if(response == ERR_RESPONSE) {
         // error with file, print message
         print_string_response();
         return;
      }
      if(response == OK_RESPONSE) break;
   }

   // get matching file name
   receive_string_from_MCU(filename);
   woz_puts("FOUND ");
   woz_puts(filename);
   woz_putc('\r');

   byte filetype = 0;
   token_ptr = filename;
   while(1) {
      if(*token_ptr == '#') {
         if(token_ptr[1] == '0' && token_ptr[2] == '6') { filetype = 0x06; break; }
         if(token_ptr[1] == 'F' && token_ptr[2] == '1') { filetype = 0xF1; break; }
      }
      if(*token_ptr == 0) break;
      token_ptr++;
   }

   // calculate start address for 0x06 binary file
   if(filetype == 0x06) {
      token_ptr+=2;
      hex_to_word(token_ptr);
      start_address = tmpword;
   }

   // get file length in tmpword
   receive_word_from_mcu();
   if(TIMEOUT) return;

   if(filetype != 0x06 && filetype != 0xF1) {
      woz_puts("?INVALID FILE NAME TAG #");
      for(word t=0;t!=tmpword;t++) receive_byte_from_MCU(); // empty buffer
      return;
   }

   woz_puts("LOADING\r");

   if(filetype == 0x06) {
      // 0x06 BINARY FILE format

      // get file bytes
      token_ptr = (byte *) start_address;
      for(word t=0;t!=tmpword;t++) {
         byte data = receive_byte_from_MCU();
         if(TIMEOUT) return;
         *token_ptr++ = data;
      }

      // decrease by one for display result
      token_ptr--;

      // print feedback to user
      woz_putc('\r');
      woz_puts(filename);
      woz_puts("\r$");
      woz_print_hexword(start_address);
      woz_puts("-$");
      woz_print_hexword((word)token_ptr);
      woz_puts(" (");
      utoa(tmpword, filename, 10);   // use filename as string buffer
      woz_puts(filename);
      woz_puts(" BYTES)\rOK");

      // executes machine language program at start address
      if(cmd == CMD_RUN) {
         woz_putc('\r');
         tmpword = start_address;
         asm {
            jmp (tmpword)
         }
      }

      return;
   }

   // 0xF1 BASIC FILE TYPE

   // get file bytes
   token_ptr = (byte *) 0;
   for(word t=0;t!=tmpword;t++) {
      byte data = receive_byte_from_MCU();
      if(TIMEOUT) return;

      if((t==0 && data!=0x41) || (t==1 && data!=0x31)) {
         woz_puts("?UNKNOWN FILE FORMAT");
         t=t+1;         
         for(;t!=tmpword;t++) receive_byte_from_MCU(); // empty buffer
         return;
      }
      else if(t<0x004a) {
         // skip zone $00-$49
      }
      else if(t<0x0100) {
         // writes in the zone $4a-$ff (BASIC pointers)
         *token_ptr = data;
      }
      else if(t<0x1ff) {
         // skip zone $100-$1ff (stack)
      }
      else if(t==0x1ff) {
         // basic program chuck follows, move the pointer
         token_ptr = *BASIC_LOMEM;
         token_ptr--; // compensate for the increment in the loop
      }
      else {
         // writes in the BASIC program zone
         *token_ptr = data;
      }
      
      token_ptr++;
   }

   bas_file_info();

   // executes basic program $EFEC = RUN entry point
   if(cmd == CMD_RUN) {
      woz_putc('\r');
      asm {
         jmp $EFEC
      }      
   }
}

void bas_file_info() {
   // print feedback to user
   woz_putc('\r');
   woz_puts(filename);
   woz_putc('\r');   
   bas_info();
   woz_puts("\rOK");
}

void bas_info() {
   woz_puts("(LOMEM=$");
   woz_print_hexword((word) *BASIC_LOMEM);
   woz_puts(" HIMEM=$");
   woz_print_hexword((word) *BASIC_HIMEM);
   woz_putc(')');   
}
