// TODO verificare NOPs

#ifdef APPLE1
   // APPLE1
   #pragma start_address(0x4000)
   const word WOZMON    = 0xFF1F;      // enters monitor
   const word ECHO      = 0xFFEF;      // output ascii character in A (A not destroyed)
   const word PRBYTE    = 0xFFDC;      // print hex byte in A (A destroyed)
   const word KEY_DATA  = 0xd010;      // read key
   const word KEY_CTRL  = 0xd011;      // control port
   const word TERM_DATA = 0xd012;      // write ascii
   const word TERM_CTRL = 0xd013;      // control port

   const byte *VDP_DATA = 0xC000;       // TMS9918 data port (VRAM)
   const byte *VDP_REG  = 0xC001;       // TMS9918 register port (write) or status (read)
#else
   // VIC20
   const word ECHO      = 0xFFD2;       // chrout routine in kernal rom
   const word GETIN     = 0xFFE4;       // GETIN keyboard read routine
   const byte *VDP_DATA = 0xA000;       // TMS9918 data port (VRAM)
   const byte *VDP_REG  = 0xA001;       // TMS9918 register port (write) or status (read)
#endif

// typedef unsigned char byte;
// typedef unsigned int  word;

// TMS9918 interface flags
const byte WRITE_TO_REG   = 0b10000000;
const byte WRITE_TO_VRAM  = 0b01000000;
const byte READ_FROM_VRAM = 0b00000000;

#define POKE(a,b) (*((byte *)(a))=(byte)(b))
#define PEEK(a)   (*((byte *)(a)))

#define NOP asm { nop }

#define TMS_WRITE_REG(a)    (*VDP_REG=(byte)(a));
#define TMS_WRITE_DATA(a)   (*VDP_DATA=(byte)(a));
#define TMS_READ_REG        (*VDP_REG);
#define TMS_READ_DATA       (*VDP_DATA);

// status register
#define FRAME_BIT(a)        ((a) & 0b10000000)
#define FIVESPR_BIT(a)      ((a) & 0b01000000)
#define COLLISION_BIT(a)    ((a) & 0b00100000)
#define SPRITE_NUM(a)       ((a) & 0b00011111)

// sets the VRAM write address on the TMS9918
void set_vram_write_addr(word addr) {
   TMS_WRITE_REG(<addr);
   TMS_WRITE_REG((>addr & 0b00111111)|WRITE_TO_VRAM);
}

// sets the VRAM read address on the TMS9918
void set_vram_read_addr(word addr) {
   TMS_WRITE_REG(<addr);
   TMS_WRITE_REG((>addr & 0b00111111)|READ_FROM_VRAM);
}

// writes a value to a TMS9918 register (0-7)
void write_reg(byte regnum, byte val) {
   TMS_WRITE_REG(val);
   TMS_WRITE_REG((regnum & 0b00001111)|WRITE_TO_REG);
}

inline void set_color(byte col) {
   write_reg(7, col);
}

byte SCREEN1_TABLE[8] = {
   0x00, 0xc0, 0x05, 0x80, 0x01, 0x20, 0x00, 0x25
};

byte SCREEN2_TABLE[8] = {
   0x02, 0xc0, 0x0e, 0xff, 0x03, 0x76, 0x03, 0x25
};

word screen1_cursor;

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

// loads the Laser 500 font on the pattern table
void SCREEN1_LOAD_FONT() {

   static byte *source = FONT;
   static word i;

   // start writing into VRAM from space character (32..127)
   set_vram_write_addr(SCREEN1_PATTERN_TABLE+(32*8));
   for(i=768;i!=0;i--) {
      TMS_WRITE_DATA(*source++);
   }

   // reverse font (32..127)
   source = FONT;
   set_vram_write_addr(SCREEN1_PATTERN_TABLE+((128+32)*8));
   for(i=768;i!=0;i--) {
      TMS_WRITE_DATA(~(*source++));
   }
}

// prints character to TMS (SCREEN 1 MODE)
void SCREEN1_PUTCHAR(byte c) {
   set_vram_write_addr(screen1_cursor++);
   TMS_WRITE_DATA(c);
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
      TMS_WRITE_DATA(32);
   }

   // fill pattern table with 0
   set_vram_write_addr(SCREEN1_PATTERN_TABLE);
   for(word i=256*8;i!=0;i--) {
      TMS_WRITE_DATA(0);
   }

   // fill color table with $1F
   set_vram_write_addr(SCREEN1_COLOR_TABLE);
   for(byte i=32;i!=0;i--) {
      TMS_WRITE_DATA(0x1f);
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


void SCREEN_INIT(byte *table) {
   for(byte i=0;i<8;i++) {
      write_reg(i, table[i]);
   }
}

void SCREEN2_FILL() {
   // fills name table x3 with increasing numbers
   set_vram_write_addr(SCREEN2_NAME_TABLE);
   for(word i=0;i<SCREEN2_SIZE;i++) {
      TMS_WRITE_DATA(i & 0xFF);
   }

   // fill pattern table with 0 (clear screen)
   set_vram_write_addr(SCREEN2_PATTERN_TABLE);
   for(word i=768*8;i!=0;i--) {
      TMS_WRITE_DATA(0);
   }

   // fill color table with $1F
   set_vram_write_addr(SCREEN2_COLOR_TABLE);
   for(word i=768*8;i!=0;i--) {
      TMS_WRITE_DATA(0x1f);
   }
}

void SCREEN2_PUTC(byte ch, byte x, byte y, byte col) {
   byte *source = &FONT[(word)(ch-32)*8];
   word addr = x*8 + y*256;
   set_vram_write_addr(SCREEN2_PATTERN_TABLE + addr); for(byte i=0;i<8;i++) TMS_WRITE_DATA(source[i]);
   set_vram_write_addr(SCREEN2_COLOR_TABLE   + addr); for(byte i=0;i<8;i++) TMS_WRITE_DATA(col);
}

void SCREEN2_PUTS(byte x, byte y, byte col, char *s) {
   byte c;
   while(c=*s++) {
      SCREEN2_PUTC(c, x++, y, col);
   }
}

void SCREEN2_PSET(byte x, byte y) {
   static byte pow2_table[8] = { 128,64,32,16,8,4,2,1 };
   word paddr = SCREEN2_PATTERN_TABLE + (word)(x & 0b11111000) + (word)(y & 0b11111000)*32 + y%8;
   set_vram_read_addr(paddr);
   byte data = TMS_READ_DATA;
   set_vram_write_addr(paddr);
   TMS_WRITE_DATA(data|pow2_table[x%8]);
}

void screen1_square_sprites() {
   // fills first sprite pattern with 255
   set_vram_write_addr(SCREEN1_SPRITE_PATTERNS);    // start writing in the sprite patterns
   for(byte i=0;i<8;i++) {
      TMS_WRITE_DATA(255);
   }

   // set sprite coordinates
   set_vram_write_addr(SCREEN1_SPRITE_ATTRS);       // start writing in the sprite attribute
   for(byte i=0;i<32;i++) {
      TMS_WRITE_DATA((6+i)*8); NOP; NOP; NOP; NOP; // y coordinate
      TMS_WRITE_DATA((6+i)*8); NOP; NOP; NOP; NOP; // x coordinate
      TMS_WRITE_DATA(0);       NOP; NOP; NOP; NOP; // name
      TMS_WRITE_DATA(i);       NOP; NOP; NOP; NOP; // color
   }
}

void screen2_square_sprites() {
   // fills first sprite pattern with 255
   set_vram_write_addr(SCREEN2_SPRITE_PATTERNS);    // start writing in the sprite patterns
   for(byte i=0;i<8;i++) {
      TMS_WRITE_DATA(0);
   }

   // set sprite coordinates
   set_vram_write_addr(SCREEN2_SPRITE_ATTRS);       // start writing in the sprite attribute
   for(byte i=0;i<32;i++) {
      TMS_WRITE_DATA(0);      NOP; NOP; NOP; NOP; // y coordinate
      TMS_WRITE_DATA(0);      NOP; NOP; NOP; NOP; // x coordinate
      TMS_WRITE_DATA(0);      NOP; NOP; NOP; NOP; // name
      TMS_WRITE_DATA(i);      NOP; NOP; NOP; NOP; // color
   }
}

void prova_screen1() {
   SCREEN_INIT(SCREEN1_TABLE);
   SCREEN1_FILL();
   SCREEN1_LOAD_FONT();

   SCREEN1_HOME();         SCREEN1_PUTS("*** P-LAB  VIDEO CARD SYSTEM ***");
   SCREEN1_LOCATEXY(0, 2); SCREEN1_PUTS("16K VRAM BYTES FREE");
   SCREEN1_LOCATEXY(0, 4); SCREEN1_PUTS("READY.");

   SCREEN1_LOCATEXY(0, 10);
   for(word i=0;i<256;i++) SCREEN1_PUTCHAR((byte)i);

   screen1_square_sprites();
}

void prova_screen2() {
   SCREEN_INIT(SCREEN2_TABLE);
   SCREEN2_FILL();
   screen2_square_sprites();

   SCREEN2_PUTS(0,0,0x1F,"*** P-LAB  VIDEO CARD SYSTEM ***");
   SCREEN2_PUTS(0,2,0x1F,"16K VRAM BYTES FREE");
   SCREEN2_PUTS(0,4,0x1F,"READY.");

   for(byte i=0;i<16;i++) {
      SCREEN2_PUTS(5,(byte)(6+i),(byte)(((15-i)<<4)+i),"     SCREEN 2     ");
   }

   for(byte i=0;i<192;i++) {
      SCREEN2_PSET((byte)i  ,(byte)i/2);
      SCREEN2_PSET((byte)i  ,(byte)i);
      SCREEN2_PSET((byte)i/2,(byte)i);
   }
}

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

// reads a key from the apple-1 keyboard
byte woz_getkey() {
   #ifdef APPLE1
      asm {
         __wait:
         lda KEY_CTRL
         bpl __wait
      }
      return PEEK(KEY_DATA) & 0x7f;
   #else
      byte key;
      byte const *keyptr = &key;
      kickasm(uses keyptr, uses GETIN) {{
         __wait:
         jsr GETIN
         cmp #0
         beq __wait
         sta keyptr
      }}
      return key;
   #endif
}

void main() {
   byte key = '1';
   for(;;) {
           if(key == '1')  prova_screen1();
      else if(key == '2')  prova_screen2();
      else if(key == '0')  break;
      else woz_putc(key);

      key = woz_getkey();
   }
   woz_putc(42);
   woz_mon();
}

