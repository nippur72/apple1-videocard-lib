#ifdef APPLE1
   #define WOZMON 0xFF1F         // enters monitor
   #define ECHO   0xFFEF         // output ascii character in A (A not destroyed)
   #define PRBYTE 0xFFDC         // print hex byte in A (A destroyed)
   #define VDP_DATA 0xC000       // TMS9918 data port (VRAM)
   #define VDP_REG  0xC001       // TMS9918 register port (write) or status (read)
#endif

#ifdef VIC20
   #define ECHO     0xFFD2       // chrout routine in kernal rom
   #define VDP_DATA 0xA000       // TMS9918 data port (VRAM)
   #define VDP_REG  0xA001       // TMS9918 register port (write) or status (read)
#endif

#define WRITE_TO_REG   0b10000000
#define WRITE_TO_VRAM  0b01000000
#define READ_FROM_VRAM 0b00000000

typedef unsigned char byte;
typedef unsigned int  word;

#define POKE(a,b) (*((byte *)(a))=(byte)(b))

// puts a character on the apple1 screen using the WOZMON routine
void fastcall woz_putc(byte c) {
   asm("jsr %w", ECHO);
}

// puts a character on the apple1 screen using the WOZMON routine
void woz_mon() {
   #ifdef APPLE1
      asm("jmp %w", WOZMON);
   #endif
}

// sets the VRAM address on the TMS9918
void fastcall set_vram_addr(word addr) {
   asm("sta %w", VDP_REG);         // write address low byte
   // nops here ?
   asm("txa");                     // X = addres high byte
   asm("and #%b", 0b00111111);     // mask address high byte
   asm("ora #%b", WRITE_TO_VRAM);  // set "write to vram" flag bits "01" upper bits ("00" for read)
   asm("sta %w", VDP_REG);         // write flags and address high byte
   // nops here ?
}

// writes a value to a TMS9918 register (0-7)
void write_reg(byte num, byte val) {
   POKE(VDP_REG, val);
   // nops here?
   POKE(VDP_REG, (num & 0b00001111)|WRITE_TO_REG);
   // nops here?
}

// SCREEN 1 VALUES

// sprite patterns:    $0000
// pattern table:      $0800   (256*8)
// sprite attributes:  $1000
// unused:             $1080
// name table:         $1400   (32*24)
// unused:             $1800
// color table:        $2000   (32)
// unused              $2020-$3FFF

#define PATTERN_TABLE   0x0800
#define NAME_TABLE      0x1400
#define COLOR_TABLE     0x2000
#define SPRITE_PATTERNS 0x0000
#define SPRITE_ATTRS    0x1000

#define SCREEN1_SIZE    (32*24)

byte SCREEN1_TABLE[8] = {
   0x00, 0xc0, 0x05, 0x80, 0x01, 0x20, 0x00, 0x25
};

#include "laser500_font.ascii.c"

// loads the Laser 500 font on the pattern table
void load_font() {

   static byte *source = FONT;
   static word i;

   // start writing into VRAM from "space" character
   set_vram_addr(PATTERN_TABLE+(32*8));
   for(i=0;i<768;i++) {
      POKE(VDP_DATA, *source);
      source++;
   }

   source = FONT;
   set_vram_addr(PATTERN_TABLE+((128+32)*8));
   for(i=0;i<768;i++) {
      POKE(VDP_DATA, ~(*source));
      source++;
   }

   // TODO: inverted characters ?
}

static word tms_cursor;

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
      c = *s & 127;     // TODO: explain why???
      if(c==0) return;
      SCREEN1_PUTCHAR(c);
      s++;
   }
}

void SCREEN1_HOME() {
   tms_cursor = NAME_TABLE;
}

void SCREEN1_LOCATEXY(byte x, byte y) {
   tms_cursor = NAME_TABLE + ((word)y)*32 + x;
}

void INIT_SCREEN1() {
   static byte i;
   for(i=0;i<8;i++) {
      write_reg(i, SCREEN1_TABLE[i]);
   }
}

void FILL_SCREEN1() {
   static word i;

   // fills name table with spaces (32)
   set_vram_addr(NAME_TABLE);
   for(i=0;i<SCREEN1_SIZE;i++) {
      POKE(VDP_DATA, 32);
      // nops here?
   }

   // fill pattern table with 0
   set_vram_addr(PATTERN_TABLE);
   for(i=0;i<256*8;i++) {
      POKE(VDP_DATA, 0);
      // nops here?
   }

   // fill color table with $1F
   set_vram_addr(COLOR_TABLE);
   for(i=0;i<32;i++) {
      POKE(VDP_DATA, 0x1f);
      // nops here?
   }
}

void square_sprites() {
   static byte i;

   // fills first sprite pattern with 255
   set_vram_addr(SPRITE_PATTERNS);    // start writing in the sprite patterns
   for(i=0;i<8;i++) {
      POKE(VDP_DATA, 255);
   }

   // set sprite coordinates
   set_vram_addr(SPRITE_ATTRS);       // start writing in the sprite attribute
   for(i=0;i<32;i++) {
      POKE(VDP_DATA,(6+i)*8);   // y coordinate
      POKE(VDP_DATA,(6+i)*8);   // x coordinate
      POKE(VDP_DATA,0);         // name
      POKE(VDP_DATA,i);         // color
   }
}

void main() {
   word i;

   INIT_SCREEN1();
   FILL_SCREEN1();
   load_font();

   SCREEN1_HOME();         SCREEN1_PUTS("*** P-LAB  VIDEO CARD SYSTEM ***");
   SCREEN1_LOCATEXY(0, 2); SCREEN1_PUTS("16K VRAM BYTES FREE");
   SCREEN1_LOCATEXY(0, 4); SCREEN1_PUTS("READY.");

   SCREEN1_LOCATEXY(0, 10);
   for(i=0;i<256;i++) SCREEN1_PUTCHAR(i);

   square_sprites();

   woz_putc(42);
   woz_mon();
}

