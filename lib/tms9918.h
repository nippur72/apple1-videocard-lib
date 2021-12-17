// TODO a bitmapped text writing routine (double size ecc)
// TODO console like text output in screen 2
// TODO more fonts (C64 and PET/VIC20)
// TODO how to switch fonts?
// TODO do fonts need to start from 32 or 0?
// TODO sprite routines
// TODO test the interrupt routines
// TODO finalize hexdump.js and update README.md
// TODO allow redefinition of I/O ports
// TODO wait_end_of_frame() 

#pragma encoding(ascii)    // encode strings in plain ascii

#include "utils.h"

#ifdef APPLE1
   const byte *VDP_DATA = 0xCC00;       // TMS9918 data port (VRAM)
   const byte *VDP_REG  = 0xCC01;       // TMS9918 register port (write) or status (read)
#else
   const byte *VDP_DATA = 0xA000;       // TMS9918 data port (VRAM)
   const byte *VDP_REG  = 0xA001;       // TMS9918 register port (write) or status (read)
#endif

// control port bits
const byte WRITE_TO_VRAM  = 0b01000000;   // write to VRAM command
const byte READ_FROM_VRAM = 0b00000000;   // read from VRAM command
const byte HIADDRESS_MASK = 0b00111111;   // bit mask for the high byte of the address

const byte WRITE_TO_REG   = 0b10000000;   // write to register command
const byte REGNUM_MASK    = 0b00000111;   // bit mask for register number (0-7)

// register 0 masks
const byte REG0_M3_MASK     = 0b00000010;
const byte REG0_EXTVID_MASK = 0b00000001;

// register 1 masks
const byte REG1_16K_MASK    = 0b10000000;
const byte REG1_BLANK_MASK  = 0b01000000;
const byte REG1_IE_MASK     = 0b00100000;
const byte REG1_M1M2_MASK   = 0b00011000;
const byte REG1_UNUSED_MASK = 0b00000100;
const byte REG1_SIZE_MASK   = 0b00000010;
const byte REG1_MAG_MASK    = 0b00000001;

// TMS9918 color palette
const byte COLOR_TRANSPARENT  = 0x0;
const byte COLOR_BLACK        = 0x1;
const byte COLOR_MEDIUM_GREEN = 0x2;
const byte COLOR_LIGHT_GREEN  = 0x3;
const byte COLOR_DARK_BLUE    = 0x4;
const byte COLOR_LIGHT_BLUE   = 0x5;
const byte COLOR_DARK_RED     = 0x6;
const byte COLOR_CYAN         = 0x7;
const byte COLOR_MEDIUM_RED   = 0x8;
const byte COLOR_LIGHT_RED    = 0x9;
const byte COLOR_DARK_YELLOW  = 0xA;
const byte COLOR_LIGHT_YELLOW = 0xB;
const byte COLOR_DARK_GREEN   = 0xC;
const byte COLOR_MAGENTA      = 0xD;
const byte COLOR_GREY         = 0xE;
const byte COLOR_WHITE        = 0xF;

// The library has a fixed VRAM memory layout valid for SCREEN 1 and 2 
// 
// pattern table:      $0000-$17FF   (256*8*3)
// sprite patterns:    $1800-$19FF
// color table:        $2000-$27FF   (256*8*3)
// name table:         $3800-$3AFF   (32*24 = 256*3 = 768)
// sprite attributes:  $3B00-$3BFF
// unused              $3C00-$3FFF
//

const word TMS_NAME_TABLE      = 0x3800; 
const word TMS_COLOR_TABLE     = 0x2000; 
const word TMS_PATTERN_TABLE   = 0x0000; 
const word TMS_SPRITE_ATTRS    = 0x3b00; 
const word TMS_SPRITE_PATTERNS = 0x1800; 

// macro for combining foreground and background into a single byte value
#define FG_BG(f,b)          (((f)<<4)|(b))

// status register bits (read only)
#define FRAME_BIT(a)        ((a) & 0b10000000)
#define FIVESPR_BIT(a)      ((a) & 0b01000000)
#define COLLISION_BIT(a)    ((a) & 0b00100000)
#define SPRITE_NUM(a)       ((a) & 0b00011111)

// read/write to TMS9918 macros
#define TMS_WRITE_CTRL_PORT(a)    (*VDP_REG=(byte)(a))
#define TMS_WRITE_DATA_PORT(a)    (*VDP_DATA=(byte)(a))
#define TMS_READ_CTRL_PORT        (*VDP_REG)
#define TMS_READ_DATA_PORT        (*VDP_DATA)

// buffer containing the last register values, because TMS registers are write only
byte tms_regs_latch[8];

// cursor coordinates for the console functions
byte tms_cursor_x;
byte tms_cursor_y;
byte tms_reverse;

// sets the VRAM address on the TMS9918 for a write operation
void tms_set_vram_write_addr(word addr) {
   TMS_WRITE_CTRL_PORT(LOBYTE(addr));
   TMS_WRITE_CTRL_PORT((HIBYTE(addr) & HIADDRESS_MASK)|WRITE_TO_VRAM);
}

// sets the VRAM address on the TMS9918 for a read operation
void tms_set_vram_read_addr(word addr) {
   TMS_WRITE_CTRL_PORT(LOBYTE(addr));
   TMS_WRITE_CTRL_PORT((HIBYTE(addr) & HIADDRESS_MASK)|READ_FROM_VRAM);
}

// writes a value to a TMS9918 register (0-7)
void tms_write_reg(byte regnum, byte val) {
   TMS_WRITE_CTRL_PORT(val);
   TMS_WRITE_CTRL_PORT((regnum & REGNUM_MASK)|WRITE_TO_REG);
   tms_regs_latch[regnum] = val;  // save value to buffer
}

// sets border color and background for mode 0
inline void tms_set_color(byte col) {
   tms_write_reg(7, col);
}

// initialize all registers from a table
void tms_init_regs(byte *table) {
   for(byte i=0;i<8;i++) {
      tms_write_reg(i, table[i]);
   }
}

const byte INTERRUPT_ENABLED  = 1;
const byte INTERRUPT_DISABLED = 0;

// sets the interrupt enable bit on register 1
void tms_set_interrupt_bit(byte val) {
   byte regvalue = tms_regs_latch[1] & (~REG1_IE_MASK);
   if(val) regvalue |= REG1_IE_MASK;
   tms_write_reg(1, regvalue);
}

const byte BLANK_ON  = 0;
const byte BLANK_OFF = 1;

// sets the blank bit on register 1
void tms_set_blank(byte val) {
   byte regvalue = tms_regs_latch[1] & (~REG1_BLANK_MASK);
   if(val) regvalue |= REG1_BLANK_MASK;
   tms_write_reg(1, regvalue);
}

// sets the external video input bit on register 0
void tms_set_external_video(byte val) {
   byte regvalue = tms_regs_latch[0] & (~REG0_EXTVID_MASK);
   if(val) regvalue |= REG0_EXTVID_MASK;
   tms_write_reg(0, regvalue);
}

void tms_wait_end_of_frame() {
   while(!FRAME_BIT(TMS_READ_CTRL_PORT));
}

#include "apple1.h"
#include "font8x8.h"
#include "sprites.h"
#include "screen1.h"
#include "screen2.h"
#include "interrupt.h"
