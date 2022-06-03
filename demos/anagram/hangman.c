#define APPLE1_USE_WOZ_MONITOR 1

#include <string.h>
#include <utils.h>
#include <apple1.h>
#include <stdlib.h>

#include "words.h"                 // vocabulary of words

char *the_word;                    // the complete word to guess
char partial[MAXWORDLEN+1];        // the partial word guessed by the user with "_"
char keybuf[MAXWORDLEN+1];         // keyboard buffer

char len;                          // the length of the word
char hang_state;                   // status progression of the hang

#define MAX_HANG_STATE 6           // 0=initial, 1=head, 2=body, 3=leftarm, 4=left leg, 5=right arm, 6=right leg

// gets a key and randomize seed
byte getkey() {
   byte k = apple1_readkey();
   if(k!=0) return k;

   // randomize random seed
   rand_state++;
   if(rand_state == 0) rand_state = 1;  // avoid 0 state for the LFSR

   return 0;
}

// pick the word to guess randomly from the vocabulary
char *pick_random_word() {     
   unsigned int index;
   do index = rand() & 0x7fff; while(index>=NUMWORDS);   
   return words[index];
}

// 012345678901234567
//0   XXXXXXXXXXX
//1   X  /      !
//2   X /       O
//3   X/       /#\
//4   X        / \
//5   X
//6   X
//7~~~~~~~~~~~~~~~~~~~~~~
//

char L0[20];
char L1[20];
char L2[20];
char L3[20];
char L4[20];
char L5[20];
char L6[20];
char L7[20];

void print_hangman() {

   strcpy(L0, "   ___________");
   strcpy(L1, "   !  /         ");
   strcpy(L2, "   ! /          ");
   strcpy(L3, "   !/           ");
   strcpy(L4, "   !            ");
   strcpy(L5, "   !");
   strcpy(L6, "   !");
   strcpy(L7, "~~~+~~~~~~~~~~~~~~~");

   if(hang_state >= 1) {
      L1[13] = '!';
      L2[13] = 'O';
   }
   if(hang_state >= 2) {
      L3[13] = 'X';
   }
   if(hang_state >= 3) {
      L3[12] = '/';
   }
   if(hang_state >= 4) {
      L4[12] = '/';
   }
   if(hang_state >= 5) {
      L3[14] = '\\';
   }
   if(hang_state >= 6) {
      L4[14] = '\\';
   }

   /*
   woz_puts("\rhang_state:");
   woz_print_hex(hang_state);
   woz_putc('\r');
   */
   woz_puts("\r\r");
   woz_puts(L0); woz_putc('\r');
   woz_puts(L1); woz_putc('\r');
   woz_puts(L2); woz_putc('\r');
   woz_puts(L3); woz_putc('\r');
   woz_puts(L4); woz_putc('\r');
   woz_puts(L5); woz_putc('\r');
   woz_puts(L6); woz_putc('\r');
   woz_puts(L6); woz_putc('\r');
   woz_puts(L7); woz_putc('\r');
   woz_putc('\r');
}

void print_word() {
   woz_puts("\r\rWORD:    ");
   woz_puts(partial);
   woz_puts("\r\r");
}

byte game_loop() {
   byte t;

   // picks a random word
   the_word = pick_random_word();
   len = strlen(the_word);

   // reset partial word
   strcpy(partial, the_word);
   for(t=0;t<len;t++) partial[t] = '_';

   // display the initial hangman
   hang_state = 0;
   print_hangman();

   while(1) {
      print_word();   // display the partial word

      apple1_input_line_prompt(keybuf, len);

      t = strlen(keybuf);

      if(t == 1) {
         // one letter guess
         byte found = 0;
         char c = keybuf[0];

         for(t=0;t<len;t++) {
            char q = the_word[t];
            if(q==c && partial[t] == '_') {
               partial[t] = c;
               found = 1;
            }
         }
         if(found == 0) {
            woz_puts("\r\rWRONG !!!\r\r");
            hang_state++;
            print_hangman();
            if(hang_state == MAX_HANG_STATE) return 0; // game over: man hanged
         }
         else {
            if(strcmp(the_word,partial)==0) {
               print_word();
               return 1;  // game over: word guessed
            }
         }
      }
      else if(t == len) {
         // guess the whole word
         if(strcmp(the_word,keybuf)==0) {
            strcpy(partial, keybuf);
            print_word();
            return 1;  // game over: word guessed
         }
      }
   }

   return 0;   
}

void game() {
   while(1) {
      byte result = game_loop();
      if(result==1) {
         woz_puts("\r\rCORRECT! YOU SAVED THE MAN!!");
         break;
      }
      else if(result==0) {
         woz_puts("\r\rNOOO, HE'S DEAD NOW!\r\r");
         woz_puts("THE WORD WAS: ");
         woz_puts(the_word);
         break;
      }
      else break; // result == 2, ESC pressed
   }
   woz_puts("\r\r");
}

void main() {

   woz_puts("\r\r*** HANGMAN ***");
   woz_puts("\r\rSAVE THE MAN BY GUESSING THE WORD\r\r");

   while(!getkey());  // randomize while waiting for a key

   while(1) {
      game();
      woz_puts("PLAY AGAIN? ");
      byte k;
      do k = apple1_getkey(); while(k!='N' && k!='Y');
      if(k=='N') break;
   }
   woz_puts("\r\rGAME WRITTEN BY ANTONINO PORCINO\r\rBYE!\r\r");
   woz_mon();
}

