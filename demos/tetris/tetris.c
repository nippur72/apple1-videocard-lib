// TODO: solve division by counter_factor
// TODO: interrupt and wait_interrupt()

//
// TETRIS for Apple1 + TMS9918 video card by P-LAB
// Written by Antonino Porcino, Dec 2021
// nino.porcino@gmail.com
//
// This tetris version has been derived from my 
// previous implementations for the Laser 310 and Laser 500
//

// standard libraries
#include <string.h>        // memset, memcopy  (memcopy no longer necessary)
#include <stdlib.h>        // for sprintf, rand

#define COUNTER_MAX            3000   // the speed counter at level 0
#define COUNTER_FACTOR         8      // speed decrease factor: speed -= speed / factor

// TODO solve division by counter_factor

#include <tms9918.h>

#include "draw.h"
#include "pieces.h"

#define EMPTY_GR_CHAR  32
#define EMPTY_GR_COLOR FG_BG(COLOR_BLACK, COLOR_BLACK)

byte piece_chars[NUMPIECES+1] = {
   0, // L (orange in the original tetris)
   3, // J
   2, // T
   1, // I
   2, // O
   1, // S
   0, // Z
   EMPTY_GR_CHAR // space
};
byte piece_colors[NUMPIECES+1] = {
   FG_BG( COLOR_GREY         , COLOR_WHITE       ),  // L ORANGE
   FG_BG( COLOR_LIGHT_BLUE   , COLOR_DARK_BLUE   ),  // J VIOLET
   FG_BG( COLOR_MAGENTA      , COLOR_WHITE       ),  // T LIGHT_MAGENTA
   FG_BG( COLOR_CYAN         , COLOR_LIGHT_BLUE  ),  // I LIGHT_CYAN
   FG_BG( COLOR_LIGHT_YELLOW , COLOR_DARK_YELLOW ),  // O YELLOW
   FG_BG( COLOR_LIGHT_GREEN  , COLOR_DARK_GREEN  ),  // S LIGHT_GREEN
   FG_BG( COLOR_LIGHT_RED    , COLOR_DARK_RED    ),  // Z LIGHT_RED
   EMPTY_GR_COLOR                                    // empty character
};

void check_crunched_lines();

#define COLLIDES     1
#define NOT_COLLIDES 0

#define PIECE_IS_NEXT 255      /* tells a piece is not on the board but on the "next" display */

#include "sprite.h"

sprite piece_preview;    // the "next" piece 
sprite player;           // the piece moved by the player
sprite new_pos;          // new player position when making a 1-step move

word drop_counter;       // counter used to set the pace 
word drop_counter_max;   // maximum value of the counter 

unsigned long score;          // player's score 
unsigned int level;           // level 
unsigned int total_lines;     // total number of lines 
int lines_remaining;          // lines to complete the level 

// game files
#include "pieces.h"
#include "ckboard.h"
#include "fonts.h"
#include "keyboard_input.h"
#include "grboard.h"
#include "intro.h"

// simulates rand() % 7, since KickC does not support % 7
// extracts a random number % 8 and repeats if it's not below < 7
byte rand_modulo_7() {
   for(;;) {
      byte p = (byte) rand() % 8;
      if(p!=7) {
         return p;
      }
   }
}

// generate a new piece after the last one can no longer move
// piece is taken from the "next" which in turn is generated randomly
// returns "COLLIDES" if a new piece can't be generated (the board is full)

byte generate_new_piece() {
   // move "next" piece onto the board
   player.piece = piece_preview.piece;
   player.angle = piece_preview.angle;
   player.x = 4;
   player.y = 0;

   // get a new "next" piece
   gr_erasepiece(&piece_preview);
   piece_preview.piece = (byte) rand_modulo_7(); // rand() % NUMPIECES;
   piece_preview.angle = (byte) rand() % NUMROT;
   gr_drawpiece(&piece_preview);

   if(collides(&player)) {
      // new piece can't be drawn => game over
      return COLLIDES;
   } else {
      // does not collide, draw it on the board
      ck_drawpiece(&player);
      gr_drawpiece(&player);
      return NOT_COLLIDES;
   }
}

void handle_player_input() {
   byte key = player_input();
   byte allowed = 0;

   if(key == 0) return;

   ck_erasepiece(&player);

   // calculate the new position
   sprite_copy(&new_pos, &player);

   if(key == KEY_LEFT) {
      new_pos.x--;
      if(!collides(&new_pos)) allowed = 1;
   }
   else if(key == KEY_RIGHT) {
      new_pos.x++;
      if(!collides(&new_pos)) allowed = 1;
   }
   else if(key == KEY_ROTATE) {
      new_pos.angle = (new_pos.angle + 1) & 3;
      if(!collides(&new_pos)) allowed = 1;
   }
   else if(key == KEY_DOWN) {
      drop_counter = drop_counter_max;
      return;
   }
   else if(key == KEY_DROP) {
      // animate the falling piece
      while(1) {
         ck_erasepiece(&player);
         sprite_copy(&new_pos, &player);
         new_pos.y++;
         if(collides(&new_pos)) {
            break;
         }         
         // gr_erasepiece(&player);
         // gr_drawpiece(&new_pos);

         // flicker-free version      
         ck_markpiece(&new_pos);           
         gr_erasepiece_unmarked(&player);  
         gr_drawpiece(&new_pos);           

         ck_drawpiece(&new_pos);
         sprite_copy(&player,&new_pos);
      }
      drop_counter=drop_counter_max;  // force an automatic drop
      return;
   }

   if(allowed == 1) {
      // gr_erasepiece(&player);
      // gr_drawpiece(&new_pos);

      // flicker-free version      
      ck_markpiece(&new_pos);           
      gr_erasepiece_unmarked(&player);  
      gr_drawpiece(&new_pos);           

      sprite_copy(&player, &new_pos);
   }
   ck_drawpiece(&player);
}

// the main game loop, exits when GAME OVER
// if the speed counter reaches its max then the piece is automatically pushed down 1 position
// else lets the player move the piece with keyboard/joystick commands
void gameLoop() {
   while(1) {
      if(drop_counter++==drop_counter_max) {
         // automatic drop down
         drop_counter = 0;

         // erase from the check board in order to make the move
         ck_erasepiece(&player);

         // calculate the new position (1 square down)
         sprite_copy(&new_pos, &player);
         new_pos.y++;

         if(collides(&new_pos)) {
            // collides, redraw it again on the check board
            ck_drawpiece(&player);
            // check if lines to be crunched
            check_crunched_lines();
            // generate a new piece if possible, otherwise exit to game over
            if(generate_new_piece()==COLLIDES) return;
         }
         else {
            // automatic drop does not collide, simply draw it
            // gr_erasepiece(&player);   // erase and draw are as close as possible
            // gr_drawpiece(&new_pos);

            // flicker-free version      
            ck_markpiece(&new_pos);           
            gr_erasepiece_unmarked(&player);  
            gr_drawpiece(&new_pos);           

            sprite_copy(&player, &new_pos);   
            ck_drawpiece(&player);          
         }
      }
      else {
         handle_player_input();
      }
   }
}

unsigned int scores[5] = {0, 40, 100, 300, 1200};   // variable score on number of lines crunched

byte lines_cruched[BROWS];                 // stores which lines have been crunched 

// checks if player has made complete lines and "crunches" them
void check_crunched_lines() {
   byte num_lines_crunched = 0;

   // mark completed lines
   for(byte line=(BROWS-1);line>0;line--) {
      byte filled = is_line_filled(line);
      lines_cruched[line] = filled; 
      if(filled) {
         ck_erase_line(line);
         gr_crunch_line(line, CRUNCH_CHAR_1, CRUNCH_COLOR_1);
         num_lines_crunched++;
      }
   }

   if(num_lines_crunched == 0) return;

   // wait 5 frames so the effect is visible
   for(byte t=1; t<5; t++) {
      wait_interrupt();
   }

   // assign score

   // does multiplication by repeat sums, as KickC does not support multiplication
   unsigned int s=0;
   for(byte t=0;t<(level+1);t++) s += scores[num_lines_crunched];
   score += (unsigned long) s;

   // score += scores[num_lines_crunched] * (level+1);
   lines_remaining -= (int) num_lines_crunched;
   total_lines += num_lines_crunched;

   // advance level
   if(lines_remaining <= 0) {
      level = level + 1;
      lines_remaining += 10;
      drop_counter_max -= drop_counter_max/COUNTER_FACTOR;      
      // TODO effect when advancing level?
   }

   // update score
   updateScore();

   // // marks the lines crunched with another character
   for(byte line=0; line<BROWS; line++) {
       if(lines_cruched[line]) {
         gr_crunch_line(line, CRUNCH_CHAR_2, CRUNCH_COLOR_2);
       }
   }

   // wait 5 frames so the effect is visible
   for(byte t=1; t<5; t++) {
      wait_interrupt();
   }

   // compact the heap of tetrominos, removing empty lines
   for(byte line=0; line<BROWS; line++) {
      if(lines_cruched[line]) {
         ck_scroll_down(line);
      }
   }

   // update the screen
   gr_update_board();

   // sound effect
   // ERASED: bit_fx3(0);
}

// initializes a new game
void initGame() {
   level = 0;
   score = 0;
   total_lines = 0;
   lines_remaining = 10;
   drop_counter_max = COUNTER_MAX;
   ck_init();

   drawPlayground();
   updateScore();   

   piece_preview.x = PIECE_IS_NEXT;              
   piece_preview.y = PIECE_IS_NEXT;  // piece is on "next" display

   // generate pieces twice: one for "next" and one for player   
   generate_new_piece();
   ck_erasepiece(&player);
   gr_erasepiece(&player);
   generate_new_piece();
}

void main() {

   // install the start-of-frame interrupt detection
   //install_interrupt();

#ifdef APPLE1_JUKEBOX
   apple1_eprom_init();
#endif

   // initialize the screen
   tms_init_regs(SCREEN2_TABLE);
   screen2_init_bitmap(FG_BG(COLOR_BLACK,COLOR_BLACK));   

   while(1) {
      introScreen();
      initGame();
      gameLoop();
      gameOver();
   }
}
