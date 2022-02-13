void comando_dir(byte cmd) {   
   send_byte_to_MCU(cmd);  
   if(TIMEOUT) return;

   // send filename
   send_string_to_MCU(filename);
   if(TIMEOUT) return;

   print_string_response();   
}
