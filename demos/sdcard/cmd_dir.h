void comando_dir() {   
   send_byte_to_MCU(CMD_DIR);  
   if(TIMEOUT) return;
   print_string_response();   
}
