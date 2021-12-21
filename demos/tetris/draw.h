// color escape codes that can be used with "print_string"
#define BG_TRANSPARENT  "\x1c\x00"
#define BG_BLACK        "\x1c\x01"
#define BG_MEDIUM_GREEN "\x1c\x02"
#define BG_LIGHT_GREEN  "\x1c\x03"
#define BG_DARK_BLUE    "\x1c\x04"
#define BG_LIGHT_BLUE   "\x1c\x05"
#define BG_DARK_RED     "\x1c\x06"
#define BG_CYAN         "\x1c\x07"
#define BG_MEDIUM_RED   "\x1c\x08"
#define BG_LIGHT_RED    "\x1c\x09"
#define BG_DARK_YELLOW  "\x1c\x0A"
#define BG_LIGHT_YELLOW "\x1c\x0B"
#define BG_DARK_GREEN   "\x1c\x0C"
#define BG_MAGENTA      "\x1c\x0D"
#define BG_GREY         "\x1c\x0E"
#define BG_WHITE        "\x1c\x0F"

#define FG_TRANSPARENT  "\x1b\x00"
#define FG_BLACK        "\x1b\x10"
#define FG_MEDIUM_GREEN "\x1b\x20"
#define FG_LIGHT_GREEN  "\x1b\x30"
#define FG_DARK_BLUE    "\x1b\x40"
#define FG_LIGHT_BLUE   "\x1b\x50"
#define FG_DARK_RED     "\x1b\x60"
#define FG_CYAN         "\x1b\x70"
#define FG_MEDIUM_RED   "\x1b\x80"
#define FG_LIGHT_RED    "\x1b\x90"
#define FG_DARK_YELLOW  "\x1b\xA0"
#define FG_LIGHT_YELLOW "\x1b\xB0"
#define FG_DARK_GREEN   "\x1b\xC0"
#define FG_MAGENTA      "\x1b\xD0"
#define FG_GREY         "\x1b\xE0"
#define FG_WHITE        "\x1b\xF0"


void draw_tile(byte x, byte y, byte ch, byte col) {
   byte *source = &FONTS[(word)ch*8];
   word addr = x*8 + y*256;
   tms_set_vram_write_addr(TMS_PATTERN_TABLE + addr); for(byte i=0;i<8;i++) { TMS_WRITE_DATA_PORT(source[i]); NOP; /*NOP;*/ /*NOP; NOP;*/ /*NOP; NOP; NOP; NOP;*/ }
   tms_set_vram_write_addr(TMS_COLOR_TABLE   + addr); for(byte i=0;i<8;i++) { TMS_WRITE_DATA_PORT(col);       NOP; /*NOP;*/ /*NOP; NOP;*/ /*NOP; NOP; NOP; NOP;*/ }
}

void draw_string(byte x, byte y, byte *s, byte color) {
   byte c;
   while(c=*s++) {
      draw_tile(x++, y, c, color);
   }
}

// print string with ESC + color support
void print_string(byte x, byte y, byte *s, byte color) {
   byte c;
   while(c=*s++) {
           if(c==0x1b) color = *s++ | (color & 0x0F);
      else if(c==0x1c) color = *s++ | (color & 0xF0);
      else {
         draw_tile(x++, y, c, color);
      }      
   }
}

/*
byte screen2_cursor_pos_x;
byte screen2_cursor_pos_y;
byte screen2_cursor_color;

void color(byte color) {
   screen2_cursor_color = color;
}

void locate(byte x, byte y) {
   screen2_cursor_pos_x = x;
   screen2_cursor_pos_y = y;
}

void print_string(byte *s) {
   byte c;   
   while(c=*s++) {
      draw_tile(screen2_cursor_pos_x++, screen2_cursor_pos_y, c, screen2_cursor_color);
      if(screen2_cursor_pos_x>=32) {
         screen2_cursor_pos_x = 0;
         screen2_cursor_pos_y++;         
      }
   }
}
*/