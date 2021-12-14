#include <stdlib.h>

#define STARTBOARD_X 11           /* X start position of the board on the screen */
#define STARTBOARD_Y 2            /* Y start position of the board on the screen */

#define BOARD_CHAR_LEFT  6
#define BOARD_CHAR_RIGHT 6

//#define NCOLS 32                  /* number of screen columns */
//#define NROWS 24                  /* number of screen rows, also board height */

#define CRUNCH_CHAR_1  13
#define CRUNCH_COLOR_1 FG_BG(COLOR_BLACK, COLOR_GRAY)

#define CRUNCH_CHAR_2  32
#define CRUNCH_COLOR_2 FG_BG(COLOR_BLACK, COLOR_BLACK)

#define POS_SCORE_X 23
#define POS_SCORE_Y 1

#define POS_LEVEL_X 23
#define POS_LEVEL_Y 7

#define POS_LINES_X 23
#define POS_LINES_Y 13

#define POS_NEXT_X 1
#define POS_NEXT_Y 1

#define NEXT_X (POS_NEXT_X+2)
#define NEXT_Y (POS_NEXT_Y+3)

void bit_fx2(int sound) {
   // throw not implemented
}

void bit_fx3(int sound) {
   // throw not implemented
}

void updateScore();
void drawPlayground();
void gameOver();

void gr_drawpiece(sprite *p);
void gr_erasepiece(sprite *p);

void gr_update_board();
void gr_crunch_line(byte line, byte crunch_char, byte crunch_color);

// grahpic board
// #include <stdio.h>            // for sprintf
// #include <sound.h>

#include "keyboard_input.h"
#include "fonts.h"
#include "pieces.h"
#include "ckboard.h"

extern unsigned long score;

void right_pad_number(unsigned long number) {
   ultoa(number, tmp, DECIMAL); 

   int l = (int) strlen(tmp);
   int offset = 6-l;

   for(int i=l;i>=0;i--)     tmp[i+offset] = tmp[i];   // move to the right
   for(int i=0;i<offset;i++) tmp[i] = 32;              // fill with spaces
}

// update score table
char tmp[32];
void updateScore() {
   byte color = FG_BG(COLOR_WHITE,COLOR_BLACK);
   
   right_pad_number(score);     
   gr4_prints(POS_SCORE_X+1,POS_SCORE_Y+2,tmp,color);

   right_pad_number((unsigned long)total_lines);
   gr4_prints(POS_LINES_X+1,POS_LINES_Y+2,tmp,color);

   right_pad_number((unsigned long)level);
   gr4_prints(POS_LEVEL_X+1,POS_LEVEL_Y+2,tmp,color);
}

#define FRAME_VERT  7
#define FRAME_HORIZ 8

#define FRAME_NW_CORNER 9
#define FRAME_NE_CORNER 10
#define FRAME_SW_CORNER 11
#define FRAME_SE_CORNER 12

void drawFrame(byte x, byte y, byte w, byte h, byte color) {
   byte i;
   for (i=1; i<w-1; i++) {      
      gr4_tile(x+i, y    , FRAME_VERT, color);
      gr4_tile(x+i, y+h-1, FRAME_VERT, color);
   }
   for (i=1; i<h-1; i++) {
      gr4_tile(x    , y+i, FRAME_HORIZ, color);
      gr4_tile(x+w-1, y+i, FRAME_HORIZ, color);
   }

   gr4_tile(x     ,y    , FRAME_NE_CORNER, color);
   gr4_tile(x+w-1 ,y    , FRAME_NW_CORNER, color);
   gr4_tile(x     ,y+h-1, FRAME_SE_CORNER, color);
   gr4_tile(x+w-1 ,y+h-1, FRAME_SW_CORNER, color);
}

void fillFrame(byte x, byte y, byte w, byte h, byte ch, byte color) {
   byte i,j;
   for (i=0; i<w; i++) {
      for (j=0; j<h; j++) {
         gr4_tile(x+i, y+j, ch, color);
      }
   }
}

// draws the board
void drawPlayground() {
   set_color(COLOR_DARK_BLUE);
   byte frame_color = FG_BG(COLOR_GRAY,COLOR_BLACK);
   byte text_color  = FG_BG(COLOR_LIGHT_YELLOW,COLOR_BLACK);

   // draw tetris board

   byte bg = FG_BG(COLOR_BLACK, COLOR_DARK_BLUE);

   // tetris frame
   fillFrame(0,0, 10,24,  16, bg);

   fillFrame(STARTBOARD_X,STARTBOARD_Y,BCOLS,BROWS,32,FG_BG(COLOR_BLACK, COLOR_BLACK));
   drawFrame(STARTBOARD_X-1,STARTBOARD_Y-1,BCOLS+2,BROWS+2, frame_color);

   fillFrame(22,0,10,24,   16, bg);
   fillFrame(0, 0,32, 1,   16, bg);
   fillFrame(0,23,32, 1,   16, bg);

   drawFrame(POS_SCORE_X, POS_SCORE_Y, 8, 4, frame_color);
   drawFrame(POS_LEVEL_X, POS_LEVEL_Y, 8, 4, frame_color);
   drawFrame(POS_LINES_X, POS_LINES_Y, 8, 4, frame_color);

   drawFrame(POS_NEXT_X , POS_NEXT_Y , 8, 8, frame_color);
   fillFrame(POS_NEXT_X+1 , POS_NEXT_Y+1 , 6, 6, 32, FG_BG(COLOR_BLACK, COLOR_BLACK));

   gr4_prints(POS_SCORE_X+1, POS_SCORE_Y+1, "SCORE ", text_color);
   gr4_prints(POS_LEVEL_X+1, POS_LEVEL_Y+1, "LEVEL ", text_color);

   gr4_prints(POS_LINES_X+1, POS_LINES_Y+1,"LINES ", text_color);
   gr4_prints(POS_NEXT_X +1, POS_NEXT_Y +1,"NEXT" , text_color);
}

// displays "game over" and waits for return key
void gameOver() {
   byte color = FG_BG(COLOR_LIGHT_YELLOW, COLOR_BLACK);
   byte frame_color = FG_BG(COLOR_GRAY,COLOR_BLACK);

   byte y = (STARTBOARD_Y+BROWS)/2;

   drawFrame(STARTBOARD_X-2, y-1,14,3, frame_color);

   gr4_prints(STARTBOARD_X-1,y-0," GAME  OVER ", FG_BG(COLOR_LIGHT_YELLOW,COLOR_BLACK));

   // sound effect
   bit_fx2(7);

   // since it's game over, there's no next piece
   gr_erasepiece(&piece_preview);

   byte flip = 0;
   while(1) {
      flip++;
      if(test_key(KEY_RETURN)) break;
           if(flip <  60 ) color = FG_BG(COLOR_LIGHT_YELLOW ,COLOR_BLACK);
      else if(flip < 120 ) color = FG_BG(COLOR_DARK_BLUE    ,COLOR_BLACK);
      else flip = 0;
      gr4_prints(STARTBOARD_X,y-0,"GAME  OVER", color);
   }

   while(test_key(KEY_RETURN));
}

// erase piece from the screen
void gr_erasepiece(sprite *p) {
   tile_offset *data = get_piece_offsets(p->piece, p->angle);
   int px = p->x;
   int py = p->y;

   // are we erasing the "next" piece ?
   if(py==PIECE_IS_NEXT) {
      px = NEXT_X;
      py = NEXT_Y;
   }
   else {
      px += STARTBOARD_X;
      py += STARTBOARD_Y;
   }
   
   for(byte t=0; t<4; t++) {
      int x = px + data->offset_x;
      int y = py + data->offset_y;
      data++;
      gr4_tile((byte)x,(byte)y,EMPTY_GR_CHAR,EMPTY_GR_COLOR);
   }
}

// draw a piece on the screen
void gr_drawpiece(sprite *p) {
   tile_offset *data = get_piece_offsets(p->piece, p->angle);
   int px = p->x;
   int py = p->y;

   // are we drawing the "next" piece ?
   if(py==PIECE_IS_NEXT) {
      px = NEXT_X;
      py = NEXT_Y;
   }
   else {
      px += STARTBOARD_X;
      py += STARTBOARD_Y;
   }

   byte piece = p->piece;
   for(byte t=0; t<4; t++) {
      int x = px;  byte x1 = data->offset_x; x+= (int) x1;
      int y = py;  byte y1 = data->offset_y; y+= (int) y1;
      data++;
      /*
      int x = px + data[t].offset_x;
      int y = py + data[t].offset_y;
      */
      byte ch  = piece_chars[piece];  //piece_chars[p->piece];
      byte col = piece_colors[piece]; //piece_colors[p->piece];
      gr4_tile((byte)x,(byte)y,ch,col);
   }
}

// fills the specified line with an empty character
void gr_crunch_line(byte line, byte crunch_char, byte crunch_color) {   
   for(byte i=0; i<BCOLS; i++) {
      gr4_tile(STARTBOARD_X+i, STARTBOARD_Y+line, crunch_char, crunch_color);
   }   
}

void gr_update_board() {
   byte tile,ch,col;
   for(byte line=0;line<BROWS;line++) {
      for(byte column=0;column<BCOLS;column++) {
         tile = READ_BOARD(column,line);
         ch = piece_chars[tile];
         col = piece_colors[tile];         
         gr4_tile(STARTBOARD_X+column, STARTBOARD_Y+line, ch, col);
      }
   }
}