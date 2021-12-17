byte SCREEN1_TABLE[8] = { 0x00, 0xc0, 0x0e, 0x80, 0x00, 0x76, 0x03, 0x25 };

const word SCREEN1_SIZE = (32*24);

// loads the font on the pattern table
void screen1_load_font() {
   byte *source = FONT;

   // start writing into VRAM from space character (32..127)
   tms_set_vram_write_addr(TMS_PATTERN_TABLE+(32*8));
   for(word i=768;i!=0;i--) {
      TMS_WRITE_DATA_PORT(*source++);
   }

   // reverse font (32..127)
   source = FONT;
   tms_set_vram_write_addr(TMS_PATTERN_TABLE+((128+32)*8));
   for(word i=768;i!=0;i--) {
      TMS_WRITE_DATA_PORT(~(*source++));
   }
}

void screen1_cls() {
   // fills name table with spaces (32)
   tms_set_vram_write_addr(TMS_NAME_TABLE);
   for(word i=SCREEN1_SIZE;i!=0;i--) {
      TMS_WRITE_DATA_PORT(32);
   }
   tms_cursor_x = 0;
   tms_cursor_y = 0;   
}

void screen1_scroll_up() {
   word source = TMS_NAME_TABLE + 1*32;
   word dest   = TMS_NAME_TABLE + 0*32;
   word count  = 768-32;
   for(;count!=0;source++,dest++,count--) {      
      tms_set_vram_read_addr(source);   
      byte c = TMS_READ_DATA_PORT;
      tms_set_vram_write_addr(dest);   
      TMS_WRITE_DATA_PORT(c);
   }

   // fill last line with spaces
   for(word i=0;i<32;i++) {      
      TMS_WRITE_DATA_PORT(32); NOP; NOP; NOP; NOP;
   }
}

void screen1_prepare() {
   // fill name table with spaces (32)
   screen1_cls();

   // clear all the sprites
   tms_clear_sprites();

   // fill pattern table with 0
   tms_set_vram_write_addr(TMS_PATTERN_TABLE);
   for(word i=256*8;i!=0;i--) {
      TMS_WRITE_DATA_PORT(0);
   }

   // fill color table with black on white
   tms_set_vram_write_addr(TMS_COLOR_TABLE);
   for(byte i=32;i!=0;i--) {
      TMS_WRITE_DATA_PORT(FG_BG(COLOR_BLACK,COLOR_WHITE));
   }
}

#ifdef VIC20
#define CHR_BACKSPACE   20
#else
#define CHR_BACKSPACE   8
#endif

#define CHR_HOME        11
#define CHR_CLS         12
#define CHR_REVERSE_ON  15
#define CHR_RETURN      13
#define CHR_REVERSE_OFF 14
#define CHR_SPACE       32
#define CHR_REVSPACE    (32+128)

#define HOME        "\x0b"
#define CLS         "\x0c"
#define REVERSE_OFF "\x0e"
#define REVERSE_ON  "\x0f"

// prints character to TMS (SCREEN 1 MODE)
void screen1_putc(byte c) {
   if(c==CHR_CLS) {
      screen1_cls();
   }
   else if(c==CHR_HOME) {
      tms_cursor_x = 0;
      tms_cursor_y = 0;         
   }
   else if(c==CHR_REVERSE_OFF) {
      // shift out as reverse off
      tms_reverse = 0;
   }
   else if(c==CHR_REVERSE_ON) {
      // shift in as reverse on
      tms_reverse = 1;
   }
   else if(c==CHR_BACKSPACE) {
      // backspace
      if(tms_cursor_x!=0) {
         tms_cursor_x--;
      }
      else {
         if(tms_cursor_y!=0) {
            tms_cursor_y--;
            tms_cursor_x = 31;
         }
      }
   }
   else {
      if(c=='\r'||c=='\n') {
         tms_cursor_x=31;
      }
      else {
         if(tms_reverse) c |= 128;
         word addr = TMS_NAME_TABLE + (word) tms_cursor_y * 32 + tms_cursor_x;
         tms_set_vram_write_addr(addr);
         TMS_WRITE_DATA_PORT(c);      
      }
      if(tms_cursor_x==31) {
         tms_cursor_x=0;
         if(tms_cursor_y==23) screen1_scroll_up();
         else tms_cursor_y++;
      }
      else tms_cursor_x++;
   }
}

// prints a 0 terminated string to TMS (SCREEN 1 MODE)
void screen1_puts(byte *s) {
   byte c;
   while(c=*s++) {
      screen1_putc(c);
   }
}

inline void screen1_locate(byte x, byte y) {
   tms_cursor_x = x;
   tms_cursor_y = y;
}

void screen1_strinput(byte *buffer, byte max_length) {   
   byte pos=0;   
   screen1_putc(CHR_REVSPACE);
   screen1_putc(CHR_BACKSPACE);
   while(1) {
      byte key = apple1_getkey();
      if(key==CHR_RETURN) {
         buffer[pos] = 0;
         screen1_putc(CHR_SPACE);
         screen1_putc(CHR_BACKSPACE);
         return;
      }
      else if(key==CHR_BACKSPACE) {
         if(pos!=0) {
            pos--;
            screen1_putc(CHR_BACKSPACE);
            screen1_putc(CHR_REVSPACE);
            screen1_putc(CHR_SPACE);
            screen1_putc(CHR_BACKSPACE);
            screen1_putc(CHR_BACKSPACE);
         }
      }      
      else if(key>=32 && key<=128) {         
         if(pos < max_length) {
            buffer[pos++] = key;
            screen1_putc(key);
            screen1_putc(CHR_REVSPACE);
            screen1_putc(CHR_BACKSPACE);
         }
      }
   }
}
