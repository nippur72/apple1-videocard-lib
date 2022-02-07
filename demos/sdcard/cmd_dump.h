void comando_dump(char *filename, word start, word end) {

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

   // 1234567890123456789012345678901234567890
   // 0000: 00 00 00 00 00 00 00 00 

   // get file bytes   
   byte row = 0;
   for(word t=0;t!=len;t++) {
      byte data = receive_byte_from_MCU();
      if(TIMEOUT) return;

      if(!(t>=start && t<=end)) continue;

      if(row == 0) {
         woz_putc('\r');
         woz_print_hexword(t);
         woz_puts(": ");
      }

      woz_print_hex(data);
      woz_putc(' ');

      row++;      
      row &= 7;
      
      if(apple1_readkey()) {
         woz_puts("*BRK*\r");
         break;
      }
   }
}
