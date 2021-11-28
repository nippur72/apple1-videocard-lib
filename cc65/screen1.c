// SCREEN 1 VALUES

// sprite patterns:    $0000
// pattern table:      $0800   (256*8)
// sprite attributes:  $1000
// unused:             $1080
// name table:         $1400   (32*24)
// unused:             $1800
// color table:        $2000   (32)
// unused              $2020-$3FFF

#define SCREEN1_PATTERN_TABLE   0x0800
#define SCREEN1_NAME_TABLE      0x1400
#define SCREEN1_COLOR_TABLE     0x2000
#define SCREEN1_SPRITE_PATTERNS 0x0000
#define SCREEN1_SPRITE_ATTRS    0x1000
#define SCREEN1_SIZE            (32*24)

byte SCREEN1_TABLE[8] = {
   0x00, 0xc0, 0x05, 0x80, 0x01, 0x20, 0x00, 0x25
};

// loads the Laser 500 font on the pattern table
void SCREEN1_LOAD_FONT() {

   static byte *source = FONT;
   static word i;

   // start writing into VRAM from "space" character
   set_vram_addr(SCREEN1_PATTERN_TABLE+(32*8));
   for(i=0;i<768;i++) {
      POKE(VDP_DATA, *source);
      source++;
   }

   source = FONT;
   set_vram_addr(SCREEN1_PATTERN_TABLE+((128+32)*8));
   for(i=0;i<768;i++) {
      POKE(VDP_DATA, ~(*source));
      source++;
   }

   // TODO: inverted characters ?
}

// prints character to TMS (SCREEN 1 MODE)
void fastcall SCREEN1_PUTCHAR(byte c) {
   set_vram_addr(tms_cursor);
   POKE(VDP_DATA, c);
   tms_cursor++;
}

// prints 0 terminated string pointed by YA
void SCREEN1_PUTS(char *s) {
   byte c;

   while(1) {
      c = *s;
      // c = *s & 127;     // TODO: explain why???
      if(c==0) return;
      SCREEN1_PUTCHAR(c);
      s++;
   }
}

void SCREEN1_HOME() {
   tms_cursor = SCREEN1_NAME_TABLE;
}

void SCREEN1_LOCATEXY(byte x, byte y) {
   tms_cursor = SCREEN1_NAME_TABLE + ((word)y)*32 + x;
}

void SCREEN1_INIT() {
   static byte i;
   for(i=0;i<8;i++) {
      write_reg(i, SCREEN1_TABLE[i]);
   }
}

void SCREEN1_FILL() {
   static word i;

   // fills name table with spaces (32)
   set_vram_addr(SCREEN1_NAME_TABLE);
   for(i=0;i<SCREEN1_SIZE;i++) {
      POKE(VDP_DATA, 32);
      // nops here?
   }

   // fill pattern table with 0
   set_vram_addr(SCREEN1_PATTERN_TABLE);
   for(i=0;i<256*8;i++) {
      POKE(VDP_DATA, 0);
      // nops here?
   }

   // fill color table with $1F
   set_vram_addr(SCREEN1_COLOR_TABLE);
   for(i=0;i<32;i++) {
      POKE(VDP_DATA, 0x1f);
      // nops here?
   }
}
