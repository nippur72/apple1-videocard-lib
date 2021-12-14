#ifndef CKBOARD_H
#define CKBOARD_H

// #include <string.h>                // for memcpy()

// #include "sprite.h"
// #include "pieces.h"

#define BCOLS 10                   // number of board columns
#define BROWS 20                   // number of board rows
#define EMPTY NUMPIECES            // the empty character is the character after all tetrominoes

// TODO KickC doesn't allow double arrays[][]

//byte board[32 /*BROWS*/ ][16 /*BCOLS*/];          // the board
byte board[16*BROWS];

#define BOARD_INDEX(x,y) (((int)(y))*16+((int)(x)))
#define WRITE_BOARD(x,y,c) board[BOARD_INDEX(x,y)]=(c)
#define READ_BOARD(x,y)    board[BOARD_INDEX(x,y)]

// prototypes
void ck_init();
void ck_drawpiece(sprite *pl);
void ck_erasepiece(sprite *pl);
int collides(sprite *pl);
byte is_line_filled(byte line);
void ck_erase_line(byte line);
void ck_scroll_down(byte endline);

// fills the check board with EMPTY
void ck_init() {
   for(byte r=0;r<BROWS;r++) {
      ck_erase_line(r);
   }
}

// draw a piece on the check board
void ck_drawpiece(sprite *pl) {
   tile_offset *data = get_piece_offsets(pl->piece, pl->angle);
   for(byte t=0; t<4; t++) {
      int x = pl->x;  byte x1 = data->offset_x; x+= (int) x1;
      int y = pl->y;  byte y1 = data->offset_y; y+= (int) y1;
      /*
      int x = pl->x + d->offset_x;
      int y = pl->y + d->offset_y;
      */
      WRITE_BOARD(x,y,pl->piece);
      data++;
   }
}

// erase a piece from the check board
void ck_erasepiece(sprite *pl) {
   tile_offset *data = get_piece_offsets(pl->piece, pl->angle);   
   for(byte t=0; t<4; t++) {
      int x = pl->x;  byte x1 = data->offset_x; x+= (int) x1;
      int y = pl->y;  byte y1 = data->offset_y; y+= (int) y1;
      /*
      int x = pl->x + (int) data->offset_x;
      int y = pl->y + (int) data->offset_y;
      */
      WRITE_BOARD(x,y,EMPTY);
      data++;
   }   
}

// returns 1 if the piece collides with something
int collides(sprite *pl) {
   tile_offset *data = get_piece_offsets(pl->piece, pl->angle);
   for(byte t=0; t<4; t++) {
      int x = pl->x;  byte x1 = data->offset_x; x+= (int) x1;
      int y = pl->y;  byte y1 = data->offset_y; y+= (int) y1;

      //int x = pl->x + (int) data->offset_x;
      //int y = pl->y + (int) data->offset_y;
      if(x<0) return 1;                  // does it collide with left border?
      if(x>=BCOLS) return 1;             // does it collide with right border?
      if(y>=BROWS) return 1;             // does it collide with bottom? 
      if(READ_BOARD(x,y) != EMPTY) return 1; // does it collide with something?
      data++;
   }
   return 0;
}

// returns 1 if the line is all filled
byte is_line_filled(byte line) {
   for(byte t=0;t<BCOLS;t++) {
      if(READ_BOARD(t,line)==EMPTY) return 0;
   }
   return 1;
}

// fills the specified line with an empty character 
void ck_erase_line(byte line) {
   for(byte t=0; t<BCOLS; t++) {
      WRITE_BOARD(t,line,EMPTY);
   }
}

// scroll down the board by 1 position from top to specified line
void ck_scroll_down(byte endline) {
   for(byte line=endline;line>0;line--) {
      for(byte x=0;x<BCOLS;x++) {
         WRITE_BOARD(x,line,READ_BOARD(x,line-1));
      }
   }
   // clears the top line
   ck_erase_line(0);
}

#endif
