// reserve locations 0,1,2 used by the APPLE1 IRQ jump vector
// these will contain a three byte instruction "JUMP <interrupt_handler>"
// in order to make the interrupt routine not reside in zero page
#pragma zp_reserve(0,1,2)

// acknowledge the interrupt by reading the status register on the TMS9918 (see manual section 2.3.1)
inline void acknowledge_interrupt() {
   asm { lda VDP_REG };
}

export __address(0) byte IRQ_JUMP_OPCODE;    // location 0 contains the opcode for "JMP"
export __address(1) word IRQ_JUMP_ADDRESS;   // location 1 and 2 contain the jump address

// storage for a simple watch timer
export volatile byte _ticks;
export volatile byte _seconds;
export volatile byte _minutes;
export volatile byte _hours;

// interrupt routine called every 1/60th by the CPU after TMS9918 sets the /INT pin
export __interrupt(hardware_all) void interrupt_handler() {
   // update the watch
   if(++_ticks == 60) {
      _ticks = 0;
      if(++_seconds == 60) {
         _seconds = 0;
         if(++_minutes == 60) {
            _minutes = 0;
            _hours++;
         }
      }
   }
   acknowledge_interrupt();  // acknowledge interrupt by reading the status register
}

// safely installs the interrupt routine
void install_interrupt() {
   asm { sei };                                   // disable 6502 interrupts
   IRQ_JUMP_OPCODE  = 0x4C;                       // $4C = JUMP opcode
   IRQ_JUMP_ADDRESS = (word) &interrupt_handler;  // JUMP interrupt_handler
   asm { cli };                                   // re-enable 6502 interrupts
}
