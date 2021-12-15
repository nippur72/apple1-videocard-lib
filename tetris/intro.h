#include <stdlib.h>      // for rand()

#include "keyboard_input.h"

#pragma data_seg(Code)

const byte *logo =
 // 12345678901234567890123456789012
   "TTTTT EEEE XXXXX RRRR   I   SSS "
   "  T   E      X   R   R  I  S   S"
   "  T   E      X   R   R  I  S    "
   "  T   EEE    X   RRRR   I   SSS "
   "  T   E      X   R R    I      S"
   "  T   E      X   R  R   I  S   S"
   "  T   EEEE   X   R   R  I   SSS ";

#pragma data_seg(Data)

void drawLogo() {
   byte *s = logo;
   for(byte r=0;r<7;r++) {
      for(byte c=0;c<32;c++) {
         byte tile = 0;
         switch(*s++) {
            case 'T': tile = 1; break;
            case 'E': tile = 2; break;
            case 'X': tile = 3; break;
            case 'R': tile = 4; break;
            case 'I': tile = 5; break;
            case 'S': tile = 6; break;
         }
         if(tile) {
            byte ch  = piece_chars[tile];
            byte col = piece_colors[tile];
            draw_tile(c,r+1,ch,col);
         }
      }
   }
}

// introduction screen
void introScreen() {
   tms_set_color(COLOR_BLACK);

   // simulate cls (TODO improve speed)
   fillFrame(0, 0, 32, 24, 32, FG_BG(COLOR_BLACK, COLOR_BLACK));

   drawLogo();

   print_string(8, 10, " FOR THE APPLE-1", FG_BG(COLOR_WHITE, COLOR_BLACK));
   print_string(8, 14, "   WRITTEN BY   ", FG_BG(COLOR_WHITE, COLOR_BLACK));
   print_string(8, 16, "ANTONINO PORCINO", FG_BG(COLOR_WHITE, COLOR_BLACK));

   print_string(7,21,"USE " BG_DARK_BLUE"I"BG_BLACK" "BG_DARK_BLUE"J"BG_BLACK" "BG_DARK_BLUE"K"BG_BLACK" "BG_DARK_BLUE"L"BG_BLACK" "BG_DARK_BLUE"SPACE"BG_BLACK , FG_BG(COLOR_DARK_YELLOW ,COLOR_BLACK));
   print_string(5,23,"PRESS " BG_DARK_BLUE "RETURN" BG_BLACK " TO START" , FG_BG(COLOR_DARK_YELLOW ,COLOR_BLACK));

   // wait for key released
   while(test_key(KEY_RETURN));

   // wait for key press 
   while(!test_key(KEY_RETURN)) {
      // TODO music      
      rand();  // extract random numbers, making rand() more "random"
   }
}

