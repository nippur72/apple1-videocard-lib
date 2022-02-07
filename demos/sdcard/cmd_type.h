void comando_type(char *filename) {

   // send command byte
   send_byte_to_MCU(CMD_READ);
   if(TIMEOUT) return;

   // send filename
   send_string_to_MCU(filename);
   if(TIMEOUT) return;

   // response
   byte response = receive_byte_from_MCU();
   if(TIMEOUT) return;

   if(response == ERR_RESPONSE) {
      // error with file, print message
      print_string_response();
      return;
   }

   // get file length
   word len = receive_word_from_mcu();
   if(TIMEOUT) return;

   // get file bytes   
   for(word t=0;t!=len;t++) {
      byte data = receive_byte_from_MCU();
      if(TIMEOUT) return;
      woz_putc(data);
      if(apple1_readkey()) {
         woz_puts("*BRK*\r");
         break;
      }
   }
}
