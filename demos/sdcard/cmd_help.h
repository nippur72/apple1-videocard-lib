void comando_help() {

   if(filename[0] == 0) {
      // HELP (no arguments)
      strcpy(filename, "COMMANDS");
   } else {
      // HELP <command>      
      // check if it's a valid command for a customized error      
      strcpy(command, filename);
      find_cmd();
      if(cmd == 0xFF) {
         woz_puts("?UNKNOWN COMMAND \"");
         woz_puts(filename);
         woz_putc('"');
         return;
      }
   }

   // builds the name of the help file   
   strcpy((char *)KEYBUFSTART, "/HELP/");
   strcat((char *)KEYBUFSTART, filename);
   strcat((char *)KEYBUFSTART, ".TXT");

   strcpy(filename, (char *)KEYBUFSTART);

   comando_type();
}
