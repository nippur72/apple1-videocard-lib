void comando_save(char *filename) {

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
   word len = ((word) *BASIC_HIMEM) - ((word)*BASIC_LOMEM) + 512;
   send_word_to_mcu(len);
   if(TIMEOUT) return;

   // send actual bytes

   // "A1" header 
   send_byte_to_MCU('A');
   send_byte_to_MCU('1');
   if(TIMEOUT) return;
   
   // lowmem + stack chuck
   for(byte *ptr=(byte *)2; ptr<=(byte *)0x1ff; ptr++) {
      send_byte_to_MCU(*ptr);
      if(TIMEOUT) return;
   }

   // basic data
   for(word ptr=*BASIC_LOMEM; ptr<*BASIC_HIMEM; ptr++) {
      send_byte_to_MCU(*((byte *)ptr));
      if(TIMEOUT) return;
      if(((byte)ptr) == 0) woz_putc('.');
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
   woz_puts(": LOMEM=");
   woz_print_hexword(*BASIC_LOMEM);
   woz_puts(" HIMEM=");
   woz_print_hexword(*BASIC_HIMEM);
   woz_puts("\rOK");
}
