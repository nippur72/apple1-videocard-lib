#ifdef APPLE1
   #define WOZMON   0xFF1F       // enters monitor
   #define ECHO     0xFFEF       // output ascii character in A (A not destroyed)
   #define PRBYTE   0xFFDC       // print hex byte in A (A destroyed)
   #define VDP_DATA 0xC000       // TMS9918 data port (VRAM)
   #define VDP_REG  0xC001       // TMS9918 register port (write) or status (read)
#endif

#ifdef VIC20
   #define ECHO     0xFFD2       // chrout routine in kernal rom
   #define VDP_DATA 0xA000       // TMS9918 data port (VRAM)
   #define VDP_REG  0xA001       // TMS9918 register port (write) or status (read)

   #include "cbm_ascii_charmap.h"  // allow VIC-20 to work with pure ASCII

#endif

// TMS9918 interface flags
#define WRITE_TO_REG   0b10000000
#define WRITE_TO_VRAM  0b01000000
#define READ_FROM_VRAM 0b00000000

// utils
typedef unsigned char byte;
typedef unsigned int  word;

#define POKE(a,b) (*((byte *)(a))=(byte)(b))
#define PEEK(a)   (*((byte *)(a)))

// puts a character on the apple1 screen using the WOZMON routine
void fastcall woz_putc(byte c) {
   asm("jsr %w", ECHO);
}

// returns to WOZMON prompt
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

// sets the VRAM address on the TMS9918
void fastcall set_vram_read_addr(word addr) {
   asm("sta %w", VDP_REG);         // write address low byte
   // nops here ?
   asm("txa");                     // X = addres high byte
   asm("and #%b", 0b00111111);     // mask address high byte
   asm("ora #%b", READ_FROM_VRAM); // set "write to vram" flag bits "01" upper bits ("00" for read)
   asm("sta %w", VDP_REG);         // write flags and address high byte
   // nops here ?
}

// writes a value to a TMS9918 register (0-7)
void write_reg(byte regnum, byte val) {
   // nops are not required
   POKE(VDP_REG, val);
   POKE(VDP_REG, (regnum & 0b00001111)|WRITE_TO_REG);
}

static word tms_cursor;

#include "laser500_font.ascii.c"
#include "screen1.c"
#include "screen2.c"

void screen1_square_sprites() {
   static byte i;

   // fills first sprite pattern with 255
   set_vram_addr(SCREEN1_SPRITE_PATTERNS);    // start writing in the sprite patterns
   for(i=0;i<8;i++) {
      POKE(VDP_DATA, 255);
   }

   // set sprite coordinates
   set_vram_addr(SCREEN1_SPRITE_ATTRS);       // start writing in the sprite attribute
   for(i=0;i<32;i++) {
      POKE(VDP_DATA,(6+i)*8);   // y coordinate
      POKE(VDP_DATA,(6+i)*8);   // x coordinate
      POKE(VDP_DATA,0);         // name
      POKE(VDP_DATA,i);         // color
   }
}

void screen2_square_sprites() {
   static byte i;

   // fills first sprite pattern with 255
   set_vram_addr(SCREEN2_SPRITE_PATTERNS);    // start writing in the sprite patterns
   for(i=0;i<8;i++) {
      POKE(VDP_DATA, 0);
   }

   // set sprite coordinates
   set_vram_addr(SCREEN2_SPRITE_ATTRS);       // start writing in the sprite attribute
   for(i=0;i<32;i++) {
      POKE(VDP_DATA,0);   // y coordinate
      POKE(VDP_DATA,0);   // x coordinate
      POKE(VDP_DATA,0);         // name
      POKE(VDP_DATA,i);         // color
   }
}

void main() {
   word i;

   if(0) {
      SCREEN1_INIT();
      SCREEN1_FILL();
      SCREEN1_LOAD_FONT();

      SCREEN1_HOME();         SCREEN1_PUTS("*** P-LAB  VIDEO CARD SYSTEM ***");
      SCREEN1_LOCATEXY(0, 2); SCREEN1_PUTS("16K VRAM BYTES FREE");
      SCREEN1_LOCATEXY(0, 4); SCREEN1_PUTS("READY.");

      SCREEN1_LOCATEXY(0, 10);
      for(i=0;i<256;i++) SCREEN1_PUTCHAR(i);

      screen1_square_sprites();
   }

   if(1) {
      SCREEN2_INIT();
      SCREEN2_FILL();
      screen2_square_sprites();
      //SCREEN2_PUTC(65,1,1,0x1F);

      SCREEN2_PUTS(0,0,0x1F,"*** P-LAB  VIDEO CARD SYSTEM ***");
      SCREEN2_PUTS(0,2,0x1F,"16K VRAM BYTES FREE");
      SCREEN2_PUTS(0,4,0x1F,"READY.");

      for(i=0;i<16;i++) {
         SCREEN2_PUTS(5,6+i,((15-i)<<4)+i,"     COLOR     ");
      }

      for(i=0;i<192;i++) {
         SCREEN2_PSET(i,i/2);
         SCREEN2_PSET(i,i);
         SCREEN2_PSET(i/2,i);
      }
   }

   woz_putc(42);
   woz_mon();
}

