#include <utils.h>
#include <apple1.h>

__address(0x24)   word ENDADDR;    // End address of dump block
__address(0x26)   word STARTADDR;  // Begin address of dump block
__address(0x29)   byte LASTSTATE;  // Last input state
__address(0x30)   byte NUMPULSES;  // Number of long pulses to sync at the header
__address(0xC081) byte TAPEIN;     // Tape input
__address(0xD012) byte DSP;        // display data port

/*
#pragma zp_reserve(0x24)
#pragma zp_reserve(0x25)
#pragma zp_reserve(0x26)
#pragma zp_reserve(0x27)
#pragma zp_reserve(0x29)
#pragma zp_reserve(0x30)

__export byte *const ENDADDR    = (byte *) 0x24;             // End address of dump block
__export byte *const STARTADDR  = (byte *) 0x26;             // Begin address of dump block
__export byte *const LASTSTATE  = (byte *) 0x29;             // Last input state
__export byte *const NUMPULSES  = (byte *) 0x30;             // Number of long pulses to sync at the header
__export byte *const TAPEIN     = (byte *) 0xC081;           // Tape input
__export byte *const DSP        = (byte *) 0xD012;           // display data port
*/

byte PACKETSIZE;

byte reference_packet[256];
byte *vmeter = "0123456789ABCDEF";

//byte *const RX_BUFFER = (byte *) 0x060c; //0x4200;
const byte RX_BUFFER[256];   // TODO ??????????????????????????????????????????

void read_packet()
{
   word RX_BUFFER_END = (word) RX_BUFFER + (word) PACKETSIZE - 1;

   ENDADDR    = (word) RX_BUFFER_END;
   STARTADDR  = (word) RX_BUFFER;

   /*
   *((word *)ENDADDR)   = (word) RX_BUFFER_END;
   *((word *)STARTADDR) = (word) RX_BUFFER;
   */

   /*
   woz_putc('\r');
   woz_print_hex(*(STARTADDR+1)); woz_print_hex(*(STARTADDR));
   woz_putc('.');
   woz_print_hex(*(ENDADDR+1)); woz_print_hex(*(ENDADDR));
   */

   asm {
                // synchronizes with the short header

syncstart:      lda     #24           // 24 cycles (3 bytes of $ff)
                sta     NUMPULSES     // count 24 cycles pulses
                jsr     fullcycle     // skip the first full cycle (when looping)
nextsync:       ldy     #58           // full cycle duration
                jsr     fullcycle     // read a full cycle
                bcc     syncstart     // if short cycle found (c=0), redo from start
                dec     NUMPULSES     // else long cycle found, decrease count
                bne     nextsync      // if not 24 cycles, get next cycle

                // else read bit start and 32 bytes of data normally
                // the following routine was copied directly from the ACI ROM

notstart:       ldy     #31           // try to detect the much shorter start bit
                jsr     cmplevel      //
                bcs     notstart      // start bit not detected yet!
                jsr     cmplevel      // wait for 2nd phase of start bit
                ldy     #58           //  set threshold value in middle
rdbyte:         ldx     #8            //  receiver 8 bits
rdbit:          pha
                jsr     fullcycle     //  detect a full cycle
                pla
                rol                   //  roll new bit into result
                ldy     #57           //  set threshold value in middle
                dex                   //  decrement bit counter
                bne     rdbit         //  read next bit!
                sta     (STARTADDR,x) //  save new byte
                jsr     incaddr       //  increment address
                ldy     #53           //  compensate threshold with workload
                bcc     rdbyte        //  do next byte if not done yet!
                bcs     restidx       //  always taken! restore parse index
fullcycle:      jsr     cmplevel      //  wait for two level changes
cmplevel:       dey                   //  decrement time counter
                lda     TAPEIN        //  get tape in data
                cmp     LASTSTATE     //  same as before?
                beq     cmplevel      //  yes!
                sta     LASTSTATE     //  save new data
                cpy     #128          //  compare threshold
                rts
                // [...]
incaddr:        lda     <STARTADDR    //  compare current address with
                cmp     <ENDADDR      //    end address
                lda     >STARTADDR
                sbc     >ENDADDR      //  carry set if STARTADDR = ENDADDR
                inc     <STARTADDR    //  increment current address
                bne     no_inc_hi     //
                inc     >STARTADDR
no_inc_hi:      rts

                // end of read routine "restidx" is the exit point

restidx:        rts

   }
}

void decode_packets() {

   woz_puts("\rPACKET DECODER MONITOR\r");

   // fill the reference packet with the known values
   for(byte t=0;t<PACKETSIZE;t++) reference_packet[t] = t;

   // tape monitor loop
   for(;;) {
      read_packet();  // attempt reading 1 packet

      // compare received packet with reference
      byte i;
      for(i=0;i<PACKETSIZE;i++) {
         if(RX_BUFFER[i] != reference_packet[i]) break;
      }

      // display result
           if(i==0)          woz_putc('.');
      else if(i==PACKETSIZE) woz_putc('*');
      else {
         if(PACKETSIZE ==  32) woz_putc(vmeter[i>>1]);
         if(PACKETSIZE ==  64) woz_putc(vmeter[i>>2]);
         if(PACKETSIZE == 128) woz_putc(vmeter[i>>3]);
         if(PACKETSIZE == 255) woz_putc(vmeter[i>>4]);
      }

      /*
      if(apple1_readkey()=='D') {
         woz_puts("\r\r");
         for(i=0;i<PACKETSIZE;i++) {
            woz_print_hex(RX_BUFFER[i]); woz_putc(' ');
         }
      }
      */

      // exit with "X"
      if(apple1_readkey()=='X') break;
   }
}

void simple_toggle_monitor()
{
   woz_puts("\rTAPE BIT TOGGLE MONITOR\r");

   asm {
simple_monitor:   lda TAPEIN               // read tape input
                  cmp LASTSTATE            // compare to previous state
                  beq no_toggle            // if same just skip
                  sta LASTSTATE            // else save new state
                  ldx #35                  // set "toggle detected" flag in X, 35 is also the char to print
no_toggle:        bit DSP                  // check if display is ready to accept a character
                  bmi simple_monitor       // if not, just keep reading tape           
                  stx DSP                  // else display the "toggle detected" flag character
                  ldx #45                  // resets the "toggle detected" flag to the "-" sign, sets also Z=0 flag                  
                  bne simple_monitor       // cheap jump because Z is also 0
   }
}

// store the lengths for each phase
byte phase0[256];
byte phase1[256];

void count_256_phase_lengths() {
   asm {
                ldx     #0            //  count pulses

countpulse:     ldy     #00           //  phase length
lphase0:        iny                   //  decrement phase length
                lda     TAPEIN        //  get tape in data
                cmp     LASTSTATE     //  same as before?
                beq     lphase0       //  yes, keep counting
                sta     LASTSTATE     //  save new state
                tya     
                sta     phase0,x      //  save phase0 length

                ldy     #00           //  phase length
lphase1:        iny                   //  decrement phase length
                lda     TAPEIN        //  get tape in data
                cmp     LASTSTATE     //  same as before?
                beq     lphase1       //  yes, keep counting
                sta     LASTSTATE     //  save new state
                tya     
                sta     phase1,x      //  save phase0 length

                inx
                bne     countpulse    //  if not 256, repeat

   }
}

void duty_cycle_monitor() {
   woz_puts("\rDUTY CYCLE MONITOR\r");
   while(1) {      
      count_256_phase_lengths();

      // don't consider the first phase, because it's measured during transition
      phase0[0] = phase0[1];
      phase1[0] = phase1[1];

      word sum_0 = 0;
      word sum_1 = 0;
      byte i=0;
      do {
         sum_0 += phase0[i];
         sum_1 += phase1[i];
         i++;
      } while(i!=255);

      // make average
      sum_0 = sum_0 / 256;
      sum_1 = sum_1 / 256;

      // display result in 5 columns
      woz_print_hex((byte)sum_0);
      woz_putc('-');
      woz_print_hex((byte)sum_1);
      woz_puts("   ");

      byte k = apple1_readkey();
      if(k=='X') break;
   }
}

void main() {

   while(1) {
      woz_puts("\r\rTAPE MONITOR\r\r"
         "1,2,3,4 => 32,64,128,255 BYTES PACKETS\r"
         "T SIMPLE TOGGLE MONITOR\r"
         "D DUTY CYLE MONITOR\r\r"
         "X EXIT\r\r"
      );

      byte key = apple1_getkey();
      if(key == '1') { PACKETSIZE =  32; decode_packets(); }
      if(key == '2') { PACKETSIZE =  64; decode_packets(); }
      if(key == '3') { PACKETSIZE = 128; decode_packets(); }
      if(key == '4') { PACKETSIZE = 255; decode_packets(); }
      if(key == 'T') simple_toggle_monitor();
      if(key == 'D') duty_cycle_monitor();
      if(key == 'X') woz_mon();
   }
}
