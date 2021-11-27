#ifdef APPLE1
   // APPLE1
   const word WOZMON   = 0xFF1F;       // enters monitor
   const word ECHO     = 0xFFEF;       // output ascii character in A (A not destroyed)
   const word PRBYTE   = 0xFFDC;       // print hex byte in A (A destroyed)
   const word VDP_DATA = 0xC000;       // TMS9918 data port (VRAM)
   const word VDP_REG  = 0xC001;       // TMS9918 register port (write) or status (read)
#else
   // VIC20
   const word ECHO     = 0xFFD2;       // chrout routine in kernal rom
   const word VDP_DATA = 0xA000;       // TMS9918 data port (VRAM)
   const word VDP_REG  = 0xA001;       // TMS9918 register port (write) or status (read)
#endif

// typedef unsigned char byte;
// typedef unsigned int  word;

// TMS9918 interface flags
const byte WRITE_TO_REG   = 0b10000000;
const byte WRITE_TO_VRAM  = 0b01000000;
const byte READ_FROM_VRAM = 0b00000000;

#define POKE(a,b) (*((byte *)(a))=(byte)(b))
#define PEEK(a)   (*((byte *)(a)))

// puts a character on the apple1 screen using the WOZMON routine
void woz_putc(byte c) {
   asm {
      lda c
      jsr ECHO
   }
}

// returns to WOZMON prompt
void woz_mon() {
   #ifdef APPLE1
      asm {
         jmp WOZMON
      }
   #endif
}

// sets the VRAM address on the TMS9918
void set_vram_addr(word addr) {
   POKE(VDP_REG,<addr);
   POKE(VDP_REG,(>addr & 0b00111111)|WRITE_TO_VRAM);
}

// sets the VRAM address on the TMS9918
void set_vram_read_addr(word addr) {
   POKE(VDP_REG,<addr);
   POKE(VDP_REG,(>addr & 0b00111111)|READ_FROM_VRAM);
}

// writes a value to a TMS9918 register (0-7)
void write_reg(byte regnum, byte val) {
   // nops are not required
   POKE(VDP_REG, val);
   POKE(VDP_REG, (regnum & 0b00001111)|WRITE_TO_REG);
}

word tms_cursor;

#include "laser500_font.ascii.c"

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

byte SCREEN1_TABLE[8] = {
   0x00, 0xc0, 0x05, 0x80, 0x01, 0x20, 0x00, 0x25
};

// loads the Laser 500 font on the pattern table
void SCREEN1_LOAD_FONT() {

   static byte *source = FONT;
   static word i;

   // start writing into VRAM from space character
   set_vram_addr(SCREEN1_PATTERN_TABLE+(32*8));
   for(i=0;i<768;i++) {
      POKE(VDP_DATA, *source++);
   }

   // reverse font
   source = FONT;
   set_vram_addr(SCREEN1_PATTERN_TABLE+((128+32)*8));
   for(i=0;i<768;i++) {
      POKE(VDP_DATA, ~(*source++));
   }
}

// prints character to TMS (SCREEN 1 MODE)
void SCREEN1_PUTCHAR(byte c) {
   set_vram_addr(tms_cursor++);
   POKE(VDP_DATA, c);
}

// prints 0 terminated string pointed by YA
void SCREEN1_PUTS(byte *s) {
   while(*s) {
      SCREEN1_PUTCHAR(*s++);
   }
}

void SCREEN1_HOME() {
   tms_cursor = SCREEN1_NAME_TABLE;
}

void SCREEN1_LOCATEXY(byte x, byte y) {
   tms_cursor = SCREEN1_NAME_TABLE + ((word)y)*32 + x;
}

void SCREEN1_INIT() {
   for(byte i=0;i<8;i++) {
      write_reg(i, SCREEN1_TABLE[i]);
   }
}

void SCREEN1_FILL() {
   // fills name table with spaces (32)
   set_vram_addr(SCREEN1_NAME_TABLE);
   for(word i=0;i<SCREEN1_SIZE;i++) {
      POKE(VDP_DATA, 32);
      // nops here?
   }

   // fill pattern table with 0
   set_vram_addr(SCREEN1_PATTERN_TABLE);
   for(word i=0;i<256*8;i++) {
      POKE(VDP_DATA, 0);
      // nops here?
   }

   // fill color table with $1F
   set_vram_addr(SCREEN1_COLOR_TABLE);
   for(byte i=0;i<32;i++) {
      POKE(VDP_DATA, 0x1f);
      // nops here?
   }
}

// SCREEN 2 VALUES

// pattern table:      $0000-$17FF   (256*8*3)
// sprite patterns:    $1800-$19FF
// color table:        $2000-$27FF   (256*8*3)
// name table:         $3800-$3AFF   (32*24 = 256*3 = 768)
// sprite attributes:  $3B00-$3BFF
// unused              $3C00-$3FFF
//

const word SCREEN2_PATTERN_TABLE   = 0x0000;
const word SCREEN2_NAME_TABLE      = 0x3800;
const word SCREEN2_COLOR_TABLE     = 0x2000;
const word SCREEN2_SPRITE_PATTERNS = 0x1800;
const word SCREEN2_SPRITE_ATTRS    = 0x3b00;
const word SCREEN2_SIZE            = (32*24);

byte SCREEN2_TABLE[8] = {
   0x02, 0xc0, 0x0e, 0xff, 0x03, 0x76, 0x03, 0x25
};

void SCREEN2_INIT() {
   for(byte i=0;i<8;i++) {
      write_reg(i, SCREEN2_TABLE[i]);
   }
}

void SCREEN2_FILL() {
   // fills name table x3 with increasing numbers
   set_vram_addr(SCREEN2_NAME_TABLE);
   for(word i=0;i<SCREEN2_SIZE;i++) {
      POKE(VDP_DATA, i & 0xFF);
   }

   // fill pattern table with 0 (clear screen)
   set_vram_addr(SCREEN2_PATTERN_TABLE);
   for(word i=0;i<768*8;i++) {
      POKE(VDP_DATA, 0);
   }

   // fill color table with $1F
   set_vram_addr(SCREEN2_COLOR_TABLE);
   for(word i=0;i<768*8;i++) {
      POKE(VDP_DATA, 0x1f);
   }
}

void SCREEN2_PUTC(byte ch, byte x, byte y, byte col) {
   byte *source = &FONT[(word)(ch-32)*8];
   word paddr = SCREEN2_PATTERN_TABLE + x*8 + y*256;
   word caddr = SCREEN2_COLOR_TABLE   + x*8 + y*256;

   set_vram_addr(paddr); for(byte i=0;i<8;i++) POKE(VDP_DATA, source[i]);
   set_vram_addr(caddr); for(byte i=0;i<8;i++) POKE(VDP_DATA, col);
}

void SCREEN2_PUTS(byte x, byte y, byte col, char *s) {
   while(*s) {
      SCREEN2_PUTC(*s++, x++, y, col);
   }
}

void SCREEN2_PSET(byte x, byte y) {
   word paddr = SCREEN2_PATTERN_TABLE + (word)(x & 0b11111000) + (word)(y & 0b11111000)*32 + y%8;
   byte pattern = 128 >> (x%8);
   byte data;

   set_vram_read_addr(paddr);
   data = PEEK(VDP_DATA);
   set_vram_addr(paddr);
   POKE(VDP_DATA,data | pattern);
}

void screen1_square_sprites() {
   // fills first sprite pattern with 255
   set_vram_addr(SCREEN1_SPRITE_PATTERNS);    // start writing in the sprite patterns
   for(byte i=0;i<8;i++) {
      POKE(VDP_DATA, 255);
   }

   // set sprite coordinates
   set_vram_addr(SCREEN1_SPRITE_ATTRS);       // start writing in the sprite attribute
   for(byte i=0;i<32;i++) {
      POKE(VDP_DATA,(6+i)*8);   // y coordinate
      POKE(VDP_DATA,(6+i)*8);   // x coordinate
      POKE(VDP_DATA,0);         // name
      POKE(VDP_DATA,i);         // color
   }
}

void screen2_square_sprites() {
   // fills first sprite pattern with 255
   set_vram_addr(SCREEN2_SPRITE_PATTERNS);    // start writing in the sprite patterns
   for(byte i=0;i<8;i++) {
      POKE(VDP_DATA, 0);
   }

   // set sprite coordinates
   set_vram_addr(SCREEN2_SPRITE_ATTRS);       // start writing in the sprite attribute
   for(byte i=0;i<32;i++) {
      POKE(VDP_DATA,0);         // y coordinate
      POKE(VDP_DATA,0);         // x coordinate
      POKE(VDP_DATA,0);         // name
      POKE(VDP_DATA,i);         // color
   }
}

void main() {
   if(1) {
      SCREEN1_INIT();
      SCREEN1_FILL();
      SCREEN1_LOAD_FONT();

      SCREEN1_HOME();         SCREEN1_PUTS("*** P-LAB  VIDEO CARD SYSTEM ***");
      SCREEN1_LOCATEXY(0, 2); SCREEN1_PUTS("16K VRAM BYTES FREE");
      SCREEN1_LOCATEXY(0, 4); SCREEN1_PUTS("READY.");

      SCREEN1_LOCATEXY(0, 10);
      for(word i=0;i<256;i++) SCREEN1_PUTCHAR((byte)i);

      screen1_square_sprites();
   }

   if(0) {
      SCREEN2_INIT();
      SCREEN2_FILL();
      screen2_square_sprites();
      //SCREEN2_PUTC(65,1,1,0x1F);

      SCREEN2_PUTS(0,0,0x1F,"*** P-LAB  VIDEO CARD SYSTEM ***");
      SCREEN2_PUTS(0,2,0x1F,"16K VRAM BYTES FREE");
      SCREEN2_PUTS(0,4,0x1F,"READY.");

      for(byte i=0;i<16;i++) {
         SCREEN2_PUTS(5,(byte)(6+i),(byte)(((15-i)<<4)+i),"     COLOR     ");
      }

      for(byte i=0;i<192;i++) {
         SCREEN2_PSET((byte)i  ,(byte)i/2);
         SCREEN2_PSET((byte)i  ,(byte)i);
         SCREEN2_PSET((byte)i/2,(byte)i);
      }
   }

   woz_putc(42);
   woz_mon();
}

