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
            gr4_tile(c,r+3,ch,col);
         }
      }
   }
}

// introduction screen
void introScreen() {
   TMS_INIT(SCREEN2_TABLE);
   SCREEN2_FILL();
   screen2_square_sprites();
   set_color(COLOR_BLACK);

   // simulate cls (TODO improve speed)
   fillFrame(0, 0, 32, 24, 32, FG_BG(COLOR_BLACK, COLOR_BLACK));

   drawLogo();

   gr4_prints(3,13,"(C) 2021 ANTONINO PORCINO"   , FG_BG(COLOR_LIGHT_YELLOW,COLOR_BLACK));
   gr4_prints(2,18,"USE ARROWS+SPACE OR JOYSTICK", FG_BG(COLOR_WHITE ,COLOR_BLACK));
   gr4_prints(5,20,"PRESS RETURN TO START"       , FG_BG(COLOR_WHITE ,COLOR_BLACK));

   // wait for key released
   while(test_key(KEY_RETURN));

   // wait for key press and do the coloured animation   
   while(!test_key(KEY_RETURN)) {
      // TODO music      
      rand();  // extract random numbers, making rand() more "random"
   }
}
