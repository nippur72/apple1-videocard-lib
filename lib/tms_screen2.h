byte SCREEN2_TABLE[8] = { 0x02, 0xc0, 0x0e, 0xff, 0x03, 0x76, 0x03, 0x25 };

const word SCREEN2_SIZE = (32*24);

// prepare the screen 2 to be used as a bitmap
void screen2_init_bitmap(byte color) {
   // erase the first sprite pattern
   tms_set_vram_write_addr(TMS_SPRITE_PATTERNS);    // start writing in the sprite patterns
   for(byte i=0;i<8;i++) {
      TMS_WRITE_DATA_PORT(0);      NOP; 
   }

   // set all sprite coordinates to 0
   tms_set_vram_write_addr(TMS_SPRITE_ATTRS);       // start writing in the sprite attribute
   for(byte i=0;i<32;i++) {
      TMS_WRITE_DATA_PORT(0);      NOP;  // y coordinate
      TMS_WRITE_DATA_PORT(0);      NOP;  // x coordinate
      TMS_WRITE_DATA_PORT(0);      NOP;  // name
      TMS_WRITE_DATA_PORT(i);      NOP;  // color
   }

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

void screen2_plot(byte x, byte y) {
   byte pow2_table_reversed[8] = { 128,64,32,16,8,4,2,1 };
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


signed int vti_abs(signed int x) {
    return x < 0 ? -x : x;
}

// http://members.chello.at/~easyfilter/bresenham.html
void screen2_line(byte _x0, byte _y0, byte _x1, byte _y1) {

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
      screen2_plot((byte)x0, (byte)y0);
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
        screen2_plot((byte) x1, (byte) y0); //   I. Quadrant
        screen2_plot((byte) x0, (byte) y0); //  II. Quadrant
        screen2_plot((byte) x0, (byte) y1); // III. Quadrant
        screen2_plot((byte) x1, (byte) y1); //  IV. Quadrant
        e2 = 2*err;
        if (e2 <= dy) { y0++; y1--; err += dy += a; }  // y step
        if (e2 >= dx || 2*err > dy) { x0++; x1--; err += dx += b1; } // x step
    } while (x0 <= x1);

    while (y0-y1 < b) {  // too early stop of flat ellipses a=1
        screen2_plot((byte) x0-1, (byte) (y0)); // -> finish tip of ellipse
        screen2_plot((byte) x1+1, (byte) (y0++));
        screen2_plot((byte) x0-1, (byte) (y1));
        screen2_plot((byte) x1+1, (byte) (y1--));
    }
}
*/

