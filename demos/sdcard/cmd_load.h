// LOAD: for basic files in prodos format
// it is like a normal CMD_READ

// PRODOS format:
// "A","1", 510 bytes low memory, basic program

void comando_load(char *filename, byte cmd) {

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
   byte *dest = (byte *) 0;
   for(word t=0;t!=len;t++) {
      byte data = receive_byte_from_MCU();
      if(TIMEOUT) return;

      if((t==0 && data!=0x41) || (t==1 && data!=0x31)) {
         woz_puts("?NOT CFFA1/PRODOS FORMAT");
         return;
      }
      else if(t<0x004a) {
         // skip zone $00-$49
      }
      else if(t<0x0100) {
         // writes in the zone $4a-$ff (BASIC pointers)
         *dest = data;
      }
      else if(t<0x1ff) {
         // skip zone $100-$1ff (stack)
      }
      else if(t==0x1ff) {
         // basic program chuck follows, move the pointer
         dest = (byte *) ((*BASIC_LOMEM) -1);  // compensate for the increment in the loop         
      }
      else {
         // writes in the BASIC program zone
         *dest = data;
      }

      dest++;
      if(((byte)t) == 0) woz_putc('.');
   }

   // print feedback to user
   woz_putc('\r');
   woz_puts(filename);
   woz_puts(": LOMEM=");
   woz_print_hexword(*BASIC_LOMEM);
   woz_puts(" HIMEM=");
   woz_print_hexword(*BASIC_HIMEM);
   woz_puts("\rOK");

   // executes basic program $EFEC = RUN entry point
   if(cmd == CMD_RUN) {
      woz_putc('\r');
      asm {
         jmp $EFEC
      }      
   }
}
