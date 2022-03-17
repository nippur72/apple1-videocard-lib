// gloabl start_address
// global end_address
// gloabl len

void comando_write() {

   // send command byte
   send_byte_to_MCU(CMD_WRITE);
   if(TIMEOUT) return;

   // send filename
   send_string_to_MCU(filename);
   if(TIMEOUT) return;

   // get first response
   byte response = receive_byte_from_MCU();
   if(TIMEOUT) return;
   if(response == ERR_RESPONSE) {
      // error with file, print message
      print_string_response();
      return;
   }

   // send file size
   //tmpword = (word) end_address - (word) start_address + 1;   // KickC bug: (word) cast is needed
   asm {
      sec
      lda end_address
      sbc start_address
      sta tmpword
      lda end_address+1
      sbc start_address+1
      sta tmpword+1
   }
   tmpword++;

   send_word_to_mcu();
   if(TIMEOUT) return;

   // send actual bytes
   token_ptr = (byte *) start_address;
   for(word t=0;t<tmpword;t++) {
      send_byte_to_MCU(*token_ptr++);
      if(TIMEOUT) return;

      #ifdef LOADING_DOTS
      if(((byte)t) == 0) woz_putc('.');
      #endif
   }

   // get second response
   response = receive_byte_from_MCU();
   if(TIMEOUT) return;
   if(response == ERR_RESPONSE) {
      // error with file, print message
      print_string_response();
      return;
   }

   // print feedback to user
   woz_putc('\r');
   woz_puts(filename);
   woz_puts(":\r");
   woz_print_hexword(start_address);
   woz_putc('.');
   woz_print_hexword(end_address);
   woz_puts(" (");
   utoa(tmpword, filename, 10);   // use filename as string buffer
   woz_puts(filename);
   woz_puts(" BYTES)\rOK");
}
