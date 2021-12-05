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

#define TMS_WRITE_REG(a)    (*VDP_REG=(byte)(a))
#define TMS_WRITE_DATA(a)   (*VDP_DATA=(byte)(a))
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

/*
IRQ on the apple1 goes to 0
- #pragma zp_reserve(0,1,2)
- POKE 0,$4C (JUMP)
- DOKE 1,address routine
*/

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

#define PLOT_MODE_RESET   0
#define PLOT_MODE_SET     1
#define PLOT_MODE_INVERT  2

byte SCREEN2_PLOT_MODE = PLOT_MODE_SET;

void SCREEN2_PLOT(byte x, byte y) {   
   byte pow2_table_reversed[8] = { 128,64,32,16,8,4,2,1 };
   word paddr = SCREEN2_PATTERN_TABLE + (word)(x & 0b11111000) + (word)(y & 0b11111000)*32 + y%8;   
   set_vram_read_addr(paddr);
   byte data = TMS_READ_DATA;
   byte mask = pow2_table_reversed[x%8];
   set_vram_write_addr(paddr);
   switch(SCREEN2_PLOT_MODE) {
      case PLOT_MODE_RESET:
         data &= ~mask;
         break;
      case PLOT_MODE_SET:
         data |= mask;
         break;
      case PLOT_MODE_INVERT:
         data ^= mask;
         break;
   }
   TMS_WRITE_DATA(data);
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

byte amiga_data[612] = {
   107,35,130,35,
   130,35,130,57,
   130,57,185,57,
   185,57,185,35,
   185,35,197,35,
   197,35,208,46,
   208,46,208,118,
   208,118,199,118,
   199,118,199,80,
   199,80,136,80,
   136,80,134,78,
   134,78,125,78,
   125,78,124,80,
   124,80,117,80,
   117,80,117,89,
   117,89,107,96,
   107,96,107,35,
   105,96,105,34,
   105,34,198,34,
   198,34,210,46,
   210,46,210,119,
   210,119,158,119,
   158,119,158,124,
   158,124,154,132,
   154,132,150,137,
   150,137,146,140,
   146,140,140,149,
   140,149,132,153,
   132,153,132,164,
   132,164,82,164,
   82,164,82,133,
   82,133,81,133,
   81,133,81,105,
   81,105,85,99,
   85,99,92,93,
   92,93,93,89,
   93,89,97,82,
   97,82,102,79,
   102,79,105,78,
   105,78,105,78,
   105,78,102,80,
   102,80,98,82,
   98,82,94,89,
   94,89,93,93,
   93,93,86,99,
   86,99,82,105,
   82,105,82,132,
   82,132,83,133,
   83,133,83,163,
   83,163,112,163,
   112,163,112,142,
   112,142,120,142,
   120,142,124,138,
   124,138,124,119,
   124,119,122,116,
   122,116,122,106,
   122,106,137,93,
   137,93,137,81,
   137,81,134,79,
   134,79,131,79,
   131,79,134,82,
   134,82,134,89,
   134,89,133,90,
   133,90,126,90,
   126,90,123,87,
   123,87,114,94,
   114,94,102,100,
   102,100,90,109,
   90,109,90,108,
   90,108,102,99,
   102,99,98,98,
   98,98,95,95,
   95,95,98,97,
   98,97,102,98,
   102,98,105,96,
   122,85,124,78,
   124,78,132,79,
   132,79,133,82,
   133,82,133,89,
   133,89,126,89,
   126,89,122,85,
   123,116,123,106,
   123,106,138,93,
   138,93,138,81,
   138,81,197,81,
   197,81,197,118,
   197,118,124,118,
   124,118,123,114,
   132,35,183,35,
   183,35,183,56,
   183,56,132,56,
   132,56,132,35,
   168,38,179,38,
   179,38,179,52,
   179,52,168,52,
   168,52,168,38,
   170,39,177,39,
   177,39,177,51,
   177,51,170,51,
   170,51,170,39,
   119,81,122,81,
   122,81,122,83,
   122,83,119,87,
   119,87,119,81,
   113,163,113,143,
   113,143,120,143,
   120,143,125,138,
   125,138,125,129,
   125,129,129,131,
   129,131,137,131,
   137,131,137,133,
   137,133,141,138,
   141,138,145,138,
   145,138,145,140,
   145,140,139,149,
   139,149,131,152,
   131,152,131,163,
   131,163,113,163,
   125,119,125,124,
   125,124,130,121,
   130,121,125,119,
   129,122,125,124,
   125,124,125,119,
   125,119,146,119,
   146,119,136,129,
   136,129,135,129,
   135,129,137,127,
   137,127,132,122,
   132,122,129,122,
   150,119,157,119,
   157,119,157,124,
   157,124,153,132,
   153,132,150,136,
   150,136,145,137,
   145,137,142,137,
   142,137,139,133,
   139,133,139,130,
   139,130,150,119,
   140,130,140,133,
   140,133,142,136,
   142,136,144,136,
   144,136,148,135,
   148,135,149,132,
   149,132,145,129,
   145,129,140,130,
   126,128,134,130,
   134,130,136,127,
   136,127,132,123,
   132,123,129,123,
   129,123,126,125,
   126,125,126,128,
   198,35,209,46,
   209,46,209,118
};

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

   vti_line(18, 45,232,187);
   vti_line(18,187,232, 45);

   SCREEN2_PLOT_MODE = PLOT_MODE_RESET;

   vti_line(18+5, 45,232+5,187);
   vti_line(18+5,187,232+5, 45);

   SCREEN2_PLOT_MODE = PLOT_MODE_INVERT;

   vti_line(18+5+5, 45,232+5+5,187);
   vti_line(18+5+5,187,232+5+5, 45);

   SCREEN2_PLOT_MODE = PLOT_MODE_SET;

   //vti_ellipse_rect(7,9,202,167);
}

void prova_screen3() {
   SCREEN_INIT(SCREEN2_TABLE);
   SCREEN2_FILL();
   screen2_square_sprites();

   SCREEN2_PUTS(0,0,0x1F,"*** P-LAB  VIDEO CARD SYSTEM ***");
   SCREEN2_PUTS(0,2,0x1F,"16K VRAM BYTES FREE");
   SCREEN2_PUTS(0,4,0x1F,"READY.");

   for(word p=0;p<612;p+=4) {
      vti_line(amiga_data[p],amiga_data[p+1],amiga_data[p+2],amiga_data[p+3]);
   }
   SCREEN2_PUTS(18,12,0x4F,"APPLE1");   
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

signed int vti_abs(signed int x) {
    return x < 0 ? -x : x;
}

// http://members.chello.at/~easyfilter/bresenham.html
void vti_line(byte _x0, byte _y0, byte _x1, byte _y1) {

   signed int x0 = (signed int) _x0;
   signed int x1 = (signed int) _x1;   
   signed int y0 = (signed int) _y0;
   signed int y1 = (signed int) _y1;

   signed int dx =  vti_abs(x1-x0);
   signed int dy = -vti_abs(y1-y0);
   signed int err = dx+dy; /* error value e_xy */

   bool ix = x0<x1;
   bool iy = y0<y1;
   signed int e2;

   for(;;){  /* loop */
      SCREEN2_PLOT((byte)x0, (byte)y0);
      if (x0==x1 && y0==y1) break;
      e2 = err<<1;//2*err;
      if (e2 >= dy) { err += dy; if(ix) ++x0; else --x0; } /* e_xy+e_x > 0 */
      if (e2 <= dx) { err += dx; if(iy) ++y0; else --y0; } /* e_xy+e_y < 0 */
   }
}

/*
// http://members.chello.at/~easyfilter/bresenham.html
void vti_ellipse_rect(byte _x0, byte _y0, byte _x1, byte _y1)
{
    //unsigned int x0,y0,x1,y1;
    signed int x0 = (signed int) _x0;
    signed int y0 = (signed int) _y0;
    signed int x1 = (signed int) _x1;
    signed int y1 = (signed int) _y1;

    signed int a = vti_abs(x1-x0), b = vti_abs(y1-y0);
    signed int b1 = b&1; // values of diameter 
    signed int dx = 4*(1-a)*b*b, dy = 4*(b1+1)*a*a; // error increment 
    signed int err = dx+dy+b1*a*a, e2; // error of 1.step

    if (x0 > x1) { x0 = x1; x1 += a; } // if called with swapped points 
    if (y0 > y1) y0 = y1; // .. exchange them 
    y0 += (b+1)/2; y1 = y0-b1;   // starting pixel 
    a *= 8*a; b1 = 8*b*b;

    do {
        SCREEN2_PLOT((byte) x1, (byte) y0); //   I. Quadrant 
        SCREEN2_PLOT((byte) x0, (byte) y0); //  II. Quadrant 
        SCREEN2_PLOT((byte) x0, (byte) y1); // III. Quadrant 
        SCREEN2_PLOT((byte) x1, (byte) y1); //  IV. Quadrant 
        e2 = 2*err;
        if (e2 <= dy) { y0++; y1--; err += dy += a; }  // y step 
        if (e2 >= dx || 2*err > dy) { x0++; x1--; err += dx += b1; } // x step 
    } while (x0 <= x1);
   
    while (y0-y1 < b) {  // too early stop of flat ellipses a=1 
        SCREEN2_PLOT((byte) x0-1, (byte) (y0)); // -> finish tip of ellipse 
        SCREEN2_PLOT((byte) x1+1, (byte) (y0++));
        SCREEN2_PLOT((byte) x0-1, (byte) (y1));
        SCREEN2_PLOT((byte) x1+1, (byte) (y1--));
    }
}
*/

#include "interrupt.h"

void prova_interrupt() {
   // la seguente linea è un workaround temporaneo a causa di un bug di KickC
   // fa uso della variabile tick_counts in modo che non venga ottimizzata
   //*((word *)0xFFFE) = tick_counts;

   install_interrupt();
}

void main() {

   //word pippo = mul8(3,2);

   byte key = '1';
   for(;;) {
           if(key == '1')  prova_screen1();
      else if(key == '2')  prova_screen2();
      else if(key == '3')  prova_screen3();
      else if(key == '4')  prova_interrupt();
      else if(key == '0')  break;
      else woz_putc(key);

      key = woz_getkey();
   }
   woz_putc(42);
   woz_mon();
}

