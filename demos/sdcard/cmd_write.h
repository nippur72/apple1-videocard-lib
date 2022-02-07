void comando_write(char *filename, word start, word end) {

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
   word len = end-start + 1;
   send_word_to_mcu(len);
   if(TIMEOUT) return;

   // send actual bytes
   byte *ptr = (byte *) start;
   for(word t=0;t<len;t++) {
      send_byte_to_MCU(*ptr++);
      if(TIMEOUT) return;
      if(((byte)t) == 0) woz_putc('.');
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
   woz_puts(": ");
   woz_print_hexword(start);
   woz_putc('.');
   woz_print_hexword(end);
   woz_puts(" (");
   utoa(len, filename, 10);   // use filename as string buffer
   woz_puts(filename);
   woz_puts(" BYTES)\rOK");
}
