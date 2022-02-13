void comando_cd() {
   // send command byte
   send_byte_to_MCU(CMD_CD);
   if(TIMEOUT) return;

   // send filename
   send_string_to_MCU(filename);
   if(TIMEOUT) return;

   print_string_response();
   return;
}
