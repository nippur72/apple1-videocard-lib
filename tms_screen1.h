byte SCREEN1_TABLE[8] = {
   0x00, 0xc0, 0x05, 0x80, 0x01, 0x20, 0x00, 0x25
};

// SCREEN 1 VALUES

// sprite patterns:    $0000
// pattern table:      $0800   (256*8)
// sprite attributes:  $1000
// unused:             $1080
// name table:         $1400   (32*24)
// unused:             $1800
// color table:        $2000   (32)
// unused              $2020-$3FFF

const word SCREEN1_PATTERN_TABLE   = 0x0800;
const word SCREEN1_NAME_TABLE      = 0x1400;
const word SCREEN1_COLOR_TABLE     = 0x2000;
const word SCREEN1_SPRITE_PATTERNS = 0x0000;
const word SCREEN1_SPRITE_ATTRS    = 0x1000;
const word SCREEN1_SIZE            = (32*24);

// loads the a font on the pattern table
void SCREEN1_LOAD_FONT() {

   byte *source = FONT;

   // start writing into VRAM from space character (32..127)
   set_vram_write_addr(SCREEN1_PATTERN_TABLE+(32*8));
   for(word i=768;i!=0;i--) {
      TMS_WRITE_DATA_PORT(*source++);
   }

   // reverse font (32..127)
   source = FONT;
   set_vram_write_addr(SCREEN1_PATTERN_TABLE+((128+32)*8));
   for(word i=768;i!=0;i--) {
      TMS_WRITE_DATA_PORT(~(*source++));
   }
}

// hold the cursor location for the putchar routine
word screen1_cursor;

// prints character to TMS (SCREEN 1 MODE)
void SCREEN1_PUTCHAR(byte c) {
   set_vram_write_addr(screen1_cursor++);
   TMS_WRITE_DATA_PORT(c);
}

// prints 0 terminated string pointed by YA
void SCREEN1_PUTS(byte *s) {
   byte c;
   while(c=*s++) {
      SCREEN1_PUTCHAR(c);
   }
}

void SCREEN1_HOME() {
   screen1_cursor = SCREEN1_NAME_TABLE;
}

void SCREEN1_LOCATEXY(byte x, byte y) {
   screen1_cursor = SCREEN1_NAME_TABLE + ((word)y)*32 + x;
}

void SCREEN1_FILL() {
   // fills name table with spaces (32)
   set_vram_write_addr(SCREEN1_NAME_TABLE);
   for(word i=SCREEN1_SIZE;i!=0;i--) {
      TMS_WRITE_DATA_PORT(32);
   }

   // fill pattern table with 0
   set_vram_write_addr(SCREEN1_PATTERN_TABLE);
   for(word i=256*8;i!=0;i--) {
      TMS_WRITE_DATA_PORT(0);
   }

   // fill color table with black on white
   set_vram_write_addr(SCREEN1_COLOR_TABLE);
   for(byte i=32;i!=0;i--) {
      TMS_WRITE_DATA_PORT(COLOR_BYTE(COLOR_BLACK,COLOR_WHITE));
   }
}

void screen1_square_sprites() {
   // fills first sprite pattern with 255
   set_vram_write_addr(SCREEN1_SPRITE_PATTERNS);    // start writing in the sprite patterns
   for(byte i=0;i<8;i++) {
      TMS_WRITE_DATA_PORT(255);
   }

   // set sprite coordinates
   set_vram_write_addr(SCREEN1_SPRITE_ATTRS);       // start writing in the sprite attribute
   for(byte i=0;i<32;i++) {
      TMS_WRITE_DATA_PORT((6+i)*8); NOP; NOP; NOP; NOP; // y coordinate
      TMS_WRITE_DATA_PORT((6+i)*8); NOP; NOP; NOP; NOP; // x coordinate
      TMS_WRITE_DATA_PORT(0);       NOP; NOP; NOP; NOP; // name
      TMS_WRITE_DATA_PORT(i);       NOP; NOP; NOP; NOP; // color
   }
}

void prova_screen1() {
   TMS_INIT(SCREEN1_TABLE);
   SCREEN1_FILL();
   SCREEN1_LOAD_FONT();

   SCREEN1_HOME();         SCREEN1_PUTS("*** P-LAB  VIDEO CARD SYSTEM ***");
   SCREEN1_LOCATEXY(0, 2); SCREEN1_PUTS("16K VRAM BYTES FREE");
   SCREEN1_LOCATEXY(0, 4); SCREEN1_PUTS("READY.");

   SCREEN1_LOCATEXY(0, 10);
   for(word i=0;i<256;i++) SCREEN1_PUTCHAR((byte)i);

   screen1_square_sprites();
}
