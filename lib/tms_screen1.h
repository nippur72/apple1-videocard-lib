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

// loads the font on the pattern table
void screen1_load_font() {

   byte *source = FONT;

   // start writing into VRAM from space character (32..127)
   tms_set_vram_write_addr(SCREEN1_PATTERN_TABLE+(32*8));
   for(word i=768;i!=0;i--) {
      TMS_WRITE_DATA_PORT(*source++);
   }

   // reverse font (32..127)
   source = FONT;
   tms_set_vram_write_addr(SCREEN1_PATTERN_TABLE+((128+32)*8));
   for(word i=768;i!=0;i--) {
      TMS_WRITE_DATA_PORT(~(*source++));
   }
}

// holds the cursor location for the putchar routine
word screen1_cursor;

// prints character to TMS (SCREEN 1 MODE)
void screen1_putc(byte c) {
   tms_set_vram_write_addr(screen1_cursor++);
   TMS_WRITE_DATA_PORT(c);
}

// prints a 0 terminated string to TMS (SCREEN 1 MODE)
void screen1_puts(byte *s) {
   byte c;
   while(c=*s++) {
      screen1_putc(c);
   }
}

void screen1_home() {
   screen1_cursor = SCREEN1_NAME_TABLE;
}

void screen1_locate(byte x, byte y) {
   screen1_cursor = SCREEN1_NAME_TABLE + ((word)y)*32 + x;
}

void screen1_prepare() {
   // fills name table with spaces (32)
   tms_set_vram_write_addr(SCREEN1_NAME_TABLE);
   for(word i=SCREEN1_SIZE;i!=0;i--) {
      TMS_WRITE_DATA_PORT(32);
   }

   // fill pattern table with 0
   tms_set_vram_write_addr(SCREEN1_PATTERN_TABLE);
   for(word i=256*8;i!=0;i--) {
      TMS_WRITE_DATA_PORT(0);
   }

   // fill color table with black on white
   tms_set_vram_write_addr(SCREEN1_COLOR_TABLE);
   for(byte i=32;i!=0;i--) {
      TMS_WRITE_DATA_PORT(FG_BG(COLOR_BLACK,COLOR_WHITE));
   }
}

