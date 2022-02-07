// READ:
// CPU sends CMD_READ + filename as 0 terminated string
// MCU sends $00 + 2 bytes file length (MSB first) + file data bytes  (if OK)
// MCU sends $FF + string error description (if error)
//
void comando_read(char *filename, word start) {

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
   byte *dest = (byte *) start;
   for(word t=0;t!=len;t++) {
      byte data = receive_byte_from_MCU();
      if(TIMEOUT) return;
      *dest++ = data;
      if(((byte)t) == 0) woz_putc('.');
   }

   // print feedback to user
   woz_putc('\r');
   woz_puts(filename);
   woz_puts(": ");
   woz_print_hexword(start);
   woz_putc('.');
   woz_print_hexword(start+len-1);
   woz_puts(" (");
   utoa(len, filename, 10);   // use filename as string buffer
   woz_puts(filename);
   woz_puts(" BYTES)\rOK");
}
