#ifndef SCREEN2_H
#define SCREEN2_H

#include <fastmultiply.h>
byte tms_global_mulf_initialized = 0;

byte SCREEN2_TABLE[8] = { 0x02, 0xc0, 0x0e, 0xff, 0x03, 0x76, 0x03, 0x25 };

const word SCREEN2_SIZE = (32*24);

// prepare the screen 2 to be used as a bitmap
void screen2_init_bitmap(byte color) {
   // clear all the sprites
   tms_set_total_sprites(0);

   // fill pattern table with 0 (clear screen)
   tms_set_vram_write_addr(TMS_PATTERN_TABLE);
   for(word i=768*8;i!=0;i--) {
      TMS_WRITE_DATA_PORT(0);
      NOP;
   }

   // fill color table with black on white
   tms_set_vram_write_addr(TMS_COLOR_TABLE);
   for(word i=768*8;i!=0;i--) {
      TMS_WRITE_DATA_PORT(color);  
      NOP;
   }

   // fills name table x3 with increasing numbers
   tms_set_vram_write_addr(TMS_NAME_TABLE);
   for(word i=0;i<SCREEN2_SIZE;i++) {
      TMS_WRITE_DATA_PORT(i & 0xFF); 
      NOP;
   }
}

void screen2_putc(byte ch, byte x, byte y, byte col) {
   byte *source = &FONT[(word)(ch-32)*8];
   word addr = x*8 + y*256;
   tms_set_vram_write_addr(TMS_PATTERN_TABLE + addr); for(byte i=0;i<8;i++) TMS_WRITE_DATA_PORT(source[i]);
   tms_set_vram_write_addr(TMS_COLOR_TABLE   + addr); for(byte i=0;i<8;i++) TMS_WRITE_DATA_PORT(col);
}

void screen2_puts(char *s, byte x, byte y, byte col) {
   byte c;
   while(c=*s++) {
      screen2_putc(c, x++, y, col);
   }
}

#define PLOT_MODE_RESET   0
#define PLOT_MODE_SET     1
#define PLOT_MODE_INVERT  2

byte screen2_plot_mode = PLOT_MODE_SET;

byte pow2_table_reversed[8] = { 128,64,32,16,8,4,2,1 };

void screen2_plot(byte x, byte y) {
   word paddr = TMS_PATTERN_TABLE + (word)(x & 0b11111000) + (word)(y & 0b11111000)*32 + y%8;
   tms_set_vram_read_addr(paddr);
   byte data = TMS_READ_DATA_PORT;
   byte mask = pow2_table_reversed[x%8];
   tms_set_vram_write_addr(paddr);
   switch(screen2_plot_mode) {
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
   TMS_WRITE_DATA_PORT(data);
}

byte screen2_point(byte x, byte y) {
   word paddr = TMS_PATTERN_TABLE + (word)(x & 0b11111000) + (word)(y & 0b11111000)*32 + y%8;
   tms_set_vram_read_addr(paddr);
   byte data = TMS_READ_DATA_PORT;
   byte mask = pow2_table_reversed[x%8];
   return (data & mask) != 0 ? 1 : 0;
}

signed int math_abs(signed int x) {
    return x < 0 ? -x : x;
}

// http://members.chello.at/~easyfilter/bresenham.html
void screen2_line(byte _x0, byte _y0, byte _x1, byte _y1) {

   signed int x0 = (signed int) _x0;
   signed int x1 = (signed int) _x1;
   signed int y0 = (signed int) _y0;
   signed int y1 = (signed int) _y1;

   signed int dx =  math_abs(x1-x0);
   signed int dy = -math_abs(y1-y0);
   signed int err = dx+dy; /* error value e_xy */

   bool ix = x0<x1;
   bool iy = y0<y1;
   signed int e2;

   for(;;){  /* loop */
      screen2_plot((byte)x0, (byte)y0);
      if (x0==x1 && y0==y1) break;
      e2 = err<<1;//2*err;
      if (e2 >= dy) { err += dy; if(ix) ++x0; else --x0; } /* e_xy+e_x > 0 */
      if (e2 <= dx) { err += dx; if(iy) ++y0; else --y0; } /* e_xy+e_y < 0 */
   }
}

#define mul(a,b) ((signed long)mulf16s((signed int)(a),(signed int)(b)))

// http://members.chello.at/~easyfilter/bresenham.html
void screen2_ellipse_rect(byte _x0, byte _y0, byte _x1, byte _y1)
{
/*
   if (tms_global_mulf_initialized == 0) {
      mulf_init();
      tms_global_mulf_initialized = 1;
   }
   
    //unsigned int x0,y0,x1,y1;
    signed int x0 = (signed int) _x0;
    signed int y0 = (signed int) _y0;
    signed int x1 = (signed int) _x1;
    signed int y1 = (signed int) _y1;

    signed int a = math_abs(x1-x0), b = math_abs(y1-y0);
    signed int b1 = b&1; // values of diameter
    long dx = mulf16s(1-a, (signed int) mulf16s(b,b))*4;
    long dy = mulf16s(b1+1,(signed int) mulf16s(a,a))*4; // error increment
    long err = dx+dy+(signed int) mulf16s((signed int) b1, (signed int) mulf16s(a,a));
    long e2; // error of 1.step

    if (x0 > x1) { x0 = x1; x1 += a; } // if called with swapped points
    if (y0 > y1) y0 = y1; // .. exchange them
    y0 += (b+1)/2;
    y1 = y0 - b1;   // starting pixel
    a = ((signed int) mulf16s(a,a))*8; b1 = ((signed int) mulf16s(b,b))*8;

    do {
      screen2_plot((byte) x1, (byte) y0); //   I. Quadrant
      screen2_plot((byte) x0, (byte) y0); //  II. Quadrant
      screen2_plot((byte) x0, (byte) y1); // III. Quadrant
      screen2_plot((byte) x1, (byte) y1); //  IV. Quadrant
      e2 = err*2;
      if ((signed int) e2 <= (signed int) dy) { y0++; y1--; err += dy += a; }  // y step
      if ((signed int) e2 >= (signed int) dx || (signed int) e2 > (signed int) dy) { x0++; x1--; err += dx += b1; } // x step
    } while (x0 <= x1);

    while (y0-y1 < b) {  // too early stop of flat ellipses a=1
        screen2_plot((byte) x0-1, (byte) (y0)); // -> finish tip of ellipse
        screen2_plot((byte) x1+1, (byte) (y0++));
        screen2_plot((byte) x0-1, (byte) (y1));
        screen2_plot((byte) x1+1, (byte) (y1--));
    }
    
}
*/
   }

// http://members.chello.at/~easyfilter/bresenham.html
void screen2_circle(byte _xm, byte _ym, byte _r) {

   signed int xm = (signed int) _xm;
   signed int ym = (signed int) _ym;
   signed int r = (signed int) _r;

   int x = -r, y = 0, err = 2-2*r; /* II. Quadrant */ 
   do {
      screen2_plot((byte) (xm-x), (byte) (ym+y)); /*   I. Quadrant */
      screen2_plot((byte) (xm-y), (byte) (ym-x)); /*  II. Quadrant */
      screen2_plot((byte) (xm+x), (byte) (ym-y)); /* III. Quadrant */
      screen2_plot((byte) (xm+y), (byte) (ym+x)); /*  IV. Quadrant */
      r = err;
      if (r <= y) err += ++y*2+1;           /* e_xy+e_y < 0 */
      if (r > x || err > y) err += ++x*2+1; /* e_xy+e_x > 0 or no 2nd y-step */
   } while (x < 0);
}


#endif
