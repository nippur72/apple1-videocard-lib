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
const byte COLOR_GRAY         = 0xE;
const byte COLOR_WHITE        = 0xF;

// macro for combining foreground and background into a single byte value
#define COLOR_BYTE(f,b)     (((f)<<4)|(b))

// status register bits (read only)
#define FRAME_BIT(a)        ((a) & 0b10000000)
#define FIVESPR_BIT(a)      ((a) & 0b01000000)
#define COLLISION_BIT(a)    ((a) & 0b00100000)
#define SPRITE_NUM(a)       ((a) & 0b00011111)

// read/write to TMS9918 macros
#define TMS_WRITE_CTRL_PORT(a)    (*VDP_REG=(byte)(a))
#define TMS_WRITE_DATA_PORT(a)    (*VDP_DATA=(byte)(a))
#define TMS_READ_CTRL_PORT        (*VDP_REG);
#define TMS_READ_DATA_PORT        (*VDP_DATA);

// sets the VRAM address on the TMS9918 for a write operation
void set_vram_write_addr(word addr) {
   TMS_WRITE_CTRL_PORT(LOBYTE(addr));
   TMS_WRITE_CTRL_PORT((HIBYTE(addr) & HIADDRESS_MASK)|WRITE_TO_VRAM);
}

// sets the VRAM address on the TMS9918 for a read operation
void set_vram_read_addr(word addr) {
   TMS_WRITE_CTRL_PORT(LOBYTE(addr));
   TMS_WRITE_CTRL_PORT((HIBYTE(addr) & HIADDRESS_MASK)|READ_FROM_VRAM);
}

// buffer containing the last register values, because TMS registers are write only
byte TMS_REGS_LATCH[8];

// writes a value to a TMS9918 register (0-7)
void TMS_WRITE_REG(byte regnum, byte val) {
   TMS_WRITE_CTRL_PORT(val);
   TMS_WRITE_CTRL_PORT((regnum & REGNUM_MASK)|WRITE_TO_REG);
   TMS_REGS_LATCH[regnum] = val;  // save value to buffer
}

// sets border color and background for mode 0
inline void set_color(byte col) {
   TMS_WRITE_REG(7, col);
}

// initialize all registers from a table
void TMS_INIT(byte *table) {
   for(byte i=0;i<8;i++) {
      TMS_WRITE_REG(i, table[i]);
   }
}

const byte INTERRUPT_ENABLED  = 1;
const byte INTERRUPT_DISABLED = 0;

// sets the interrupt enable bit on register 1
void tms_set_interrupt_bit(byte val) {
   byte regvalue = TMS_REGS_LATCH[1] & (~REG1_IE_MASK);
   if(val) regvalue |= REG1_IE_MASK;
   TMS_WRITE_REG(1, regvalue);
}

const byte BLANK_ON  = 0;
const byte BLANK_OFF = 1;

// sets the blank bit on register 1
void tms_blank(byte val) {
   byte regvalue = TMS_REGS_LATCH[1] & (~REG1_BLANK_MASK);
   if(val) regvalue |= REG1_BLANK_MASK;
   TMS_WRITE_REG(1, regvalue);
}

// sets the external video input bit on register 0
void tms_external_video(byte val) {
   byte regvalue = TMS_REGS_LATCH[0] & (~REG0_EXTVID_MASK);
   if(val) regvalue |= REG0_EXTVID_MASK;
   TMS_WRITE_REG(0, regvalue);
}
