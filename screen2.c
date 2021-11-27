// SCREEN 2 VALUES

// pattern table:      $0000-$17FF   (256*8*3)
// sprite patterns:    $1800-$19FF
// color table:        $2000-$27FF   (256*8*3)
// name table:         $3800-$3AFF   (32*24 = 256*3 = 768)
// sprite attributes:  $3B00-$3BFF
// unused              $3C00-$3FFF
//

#define SCREEN2_PATTERN_TABLE   0x0000
#define SCREEN2_NAME_TABLE      0x3800
#define SCREEN2_COLOR_TABLE     0x2000
#define SCREEN2_SPRITE_PATTERNS 0x1800
#define SCREEN2_SPRITE_ATTRS    0x3b00
#define SCREEN2_SIZE            (32*24)

byte SCREEN2_TABLE[8] = {
   0x02, 0xc0, 0x0e, 0xff, 0x03, 0x76, 0x03, 0x25
};

void SCREEN2_INIT() {
   static byte i;
   for(i=0;i<8;i++) {
      write_reg(i, SCREEN2_TABLE[i]);
   }
}

void SCREEN2_FILL() {
   static word i;

   // fills name table x3 with increasing numbers
   set_vram_addr(SCREEN2_NAME_TABLE);
   for(i=0;i<SCREEN2_SIZE;i++) {
      POKE(VDP_DATA, i & 0xFF);
   }

   // fill pattern table with 0 (clear screen)
   set_vram_addr(SCREEN2_PATTERN_TABLE);
   for(i=0;i<768*8;i++) {
      POKE(VDP_DATA, 0);
   }

   // fill color table with $1F
   set_vram_addr(SCREEN2_COLOR_TABLE);
   for(i=0;i<768*8;i++) {
      POKE(VDP_DATA, 0x1f);
   }
}

void SCREEN2_PUTC(byte ch, byte x, byte y, byte col) {
   byte *source = &FONT[(ch-32)*8];
   byte i;
   word paddr = SCREEN2_PATTERN_TABLE + x*8 + y*256;
   word caddr = SCREEN2_COLOR_TABLE   + x*8 + y*256;

   set_vram_addr(paddr); for(i=0;i<8;i++) POKE(VDP_DATA, source[i]);
   set_vram_addr(caddr); for(i=0;i<8;i++) POKE(VDP_DATA, col);
}

void SCREEN2_PUTS(byte x, byte y, byte col, char *s) {
   while(*s) {
      SCREEN2_PUTC(*s++, x++, y, col);
   }
}

void SCREEN2_PSET(byte x, byte y) {
   word paddr = SCREEN2_PATTERN_TABLE + (x & 0b11111000) + (y & 0b11111000)*32 + y%8;
   byte pattern = 128 >> (x%8);
   byte data;

   set_vram_read_addr(paddr);
   data = PEEK(VDP_DATA);
   set_vram_addr(paddr);
   POKE(VDP_DATA,data | pattern);
}

// int vti_abs(int x) {
//     return x < 0 ? -x : x;
// }
//
// // http://members.chello.at/~easyfilter/bresenham.html
// void vti_ellipse_rect(int x0, int y0, int x1, int y1)
// {
//     long a,b,b1,dx,dy,e2,err;
//
//     a = vti_abs(x1-x0);
//     b = vti_abs(y1-y0);
//     b1 = b&1; /* values of diameter */
//     dx = 4*(1-a)*b*b;
//     dy = 4*(b1+1)*a*a; /* error increment */
//     err = dx+dy+b1*a*a; /* error of 1.step */
//
//     if (x0 > x1) { x0 = x1; x1 += a; } /* if called with swapped points */
//     if (y0 > y1) y0 = y1; /* .. exchange them */
//     y0 += (b+1)/2; y1 = y0-b1;   /* starting pixel */
//     a *= 8*a; b1 = 8*b*b;
//
//     do {
//         SCREEN2_PSET(x1, y0); /*   I. Quadrant */
//         SCREEN2_PSET(x0, y0); /*  II. Quadrant */
//         SCREEN2_PSET(x0, y1); /* III. Quadrant */
//         SCREEN2_PSET(x1, y1); /*  IV. Quadrant */
//         e2 = 2*err;
//         if (e2 <= dy) { y0++; y1--; err += dy += a; }  /* y step */
//         if (e2 >= dx || 2*err > dy) { x0++; x1--; err += dx += b1; } /* x step */
//     } while (x0 <= x1);
//
//     while (y0-y1 < b) {  /* too early stop of flat ellipses a=1 */
//         SCREEN2_PSET(x0-1, y0); /* -> finish tip of ellipse */
//         SCREEN2_PSET(x1+1, y0++);
//         SCREEN2_PSET(x0-1, y1);
//         SCREEN2_PSET(x1+1, y1--);
//     }
// }
