// reserve locations where the 6502 jumps on IRQ
#pragma zp_reserve(0,1,2)

// reads the status register on the TMS9918 thus acknowledging the interrupt
inline void acknowledge_interrupt() {
   asm { lda VDP_REG };
}

export __address(0) byte IRQ_JUMP_OPCODE;
export __address(1) word IRQ_JUMP_ADDRESS;

export volatile word tick_counts;

export __interrupt(hardware_all) void interrupt_handler() {
   tick_counts++;

   if(tick_counts == 60) {
      tick_counts = 0;
      woz_putc('T');   // print a T every second
   }

   // acknowledge interrupt by reading the status register
   acknowledge_interrupt();
}

void install_interrupt() {
   asm { sei };                                   // disable 6502 interrupts
   IRQ_JUMP_OPCODE  = 0x4C;                       // $4C = JUMP opcode
   IRQ_JUMP_ADDRESS = (word) &interrupt_handler;  // JUMP interrupt_handler
   asm { cli };                                   // re-enable 6502 interrupts
   write_reg(1, 0xc0 | 32);                       // turn on IE bit (interrupt enable) on the TMS9918
}
