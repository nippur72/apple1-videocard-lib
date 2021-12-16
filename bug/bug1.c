#pragma zp_reserve(0,1,2)

volatile word tick_counts;

__interrupt(hardware_all) void interrupt_handler() {
   tick_counts++;
}

void install_interrupt() {
   *((word *)0x0001) = (word) &interrupt_handler;
}

void main() {
   // make some use of "tick_counts"
   *((word *)0x0001) = tick_counts;

   install_interrupt();
}

/*
const byte *VDP_DATA = 0xA000;       // TMS9918 data port (VRAM)
#define TMS_READ_DATA       (*VDP_DATA);

void main() {
   byte x = TMS_READ_DATA();
   *((byte *)0xFFFF) = x;
}
*/