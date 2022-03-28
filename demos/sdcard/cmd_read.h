// READ:
// CPU sends CMD_READ + filename as 0 terminated string
// MCU sends $00 + 2 bytes file length (MSB first) + file data bytes  (if OK)
// MCU sends $FF + string error description (if error)
//

// global start_address
// global len
// global token_ptr

void comando_read() {

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

   // get file length in tmpword
   receive_word_from_mcu();
   if(TIMEOUT) return;

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
}
