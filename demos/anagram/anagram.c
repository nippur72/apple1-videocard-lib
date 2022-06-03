#include <string.h>
#include <utils.h>
#include <apple1.h>
#include <stdlib.h>

#include "words.h"                 // vocabulary of words

char *the_word;                    // the word to guess
char scrambled[MAXWORDLEN+1];      // the word with scrambled letters
char buf[16];                      // string buffer containing the number of the score

unsigned int score;                          // keeps score (accumulated length of guessed words)
__address(0x03) unsigned long int time = 0;  // keeps track of time
__address(0x04) unsigned int time_high;      // high word of the time counter
unsigned int timeout; 
byte len;

#define LETTERSCORE (150)

// gets a key, increment 32 bit timer and randomize seed
byte getkey() {
   byte k = apple1_readkey();
   if(k!=0) return k;

   // increment 32 bit timer
   asm {
      inc time
      bne inc_end
      inc time+1
      bne inc_end
      inc time+2
      bne inc_end
      inc time+3
      inc_end:
   }

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

// pick a random letter within the word
byte pick_random_letter(byte l) {     
   byte index;
   do index = rand() & 31; while(index>=l);   
   return index;
}

// scramble the word to guess
void scramble_word() {
   strcpy(scrambled, the_word);

   len = strlen(scrambled);
   
   // permute each letter of the word with a random position
   for(byte x=0;x<len;x++) {      
      byte y = pick_random_letter(len);   // gets a random position

      // do the letter swapping
      byte c = scrambled[x];
      scrambled[x] = scrambled[y];
      scrambled[y] = c;
   }
}

byte game_loop() {

   the_word = pick_random_word();     // picks a random word 
   scramble_word();                   // and scrambles it

   // calculate timeout
   timeout = 0;
   for(byte t=0;t<len;t++) {
      timeout += LETTERSCORE;
   }

   time = 0;    // reset time   
   byte x = 0;  // position of letter guessed correctly

   // display the scrambled word
   woz_puts("\r\r>>>   ");
   woz_puts(scrambled);
   woz_puts("   <<< \r\r");

   // loop that asks for a letter and counts time
   while(1) {
      byte k = getkey();
      if(k) {
         if(k==27) return 2; // ESC quits the game

         // if letter is correct, display and advance
         if(k==the_word[x]) {                                 
            woz_putc(k);
            x = x + 1;               
            if(the_word[x] == 0) return 1; // whole word guessed correctly, game completed            
         }
         else {
            // wrong letter, time penalty
            time_high += (LETTERSCORE/2);
         }
      }
      else {
         // check if time elapsed
         if(time_high > timeout) break;         
      }         
   }   

   return 0;   
}

void game() {
   //woz_puts("\r\rGUESS THE WORDS!");

   score = 0;

   while(1) {
      byte result = game_loop();
      if(result==1) {
         score += strlen(the_word);
         utoa(score, buf, 10);
         woz_puts("\r\rCORRECT! SCORE: ");
         woz_puts(buf);
      }
      else if(result==0) {
         woz_puts("\r\rOUT OF TIME!!! - YOU LOOSE\r\r");
         woz_puts("THE WORD WAS: ");
         woz_puts(the_word);
         break;
      }
      else break; // result == 2, ESC pressed
   }

   utoa(score, buf, 10);
   woz_puts("\r\rFINAL SCORE: ");
   woz_puts(buf);
   woz_puts("\r\r");
}

void main() {

   woz_puts("\r\r*** THE ANAGRAM GAME ***");
   woz_puts("\r\rGUESS THE WORD BEFORE TIME RUNS OUT");
   woz_puts("\r\rTIME PENALTY FOR WRONG LETTERS");
   woz_puts("\r\rPRESS ANY KEY TO START ");
   
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

