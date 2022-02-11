void comando_dir(byte cmd) {   
   send_byte_to_MCU(cmd);  
   if(TIMEOUT) return;
   print_string_response();   
}
