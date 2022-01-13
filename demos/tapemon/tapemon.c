#include <utils.h>
#include <apple1.h>

byte *const HEX1L      = 0x24;             // End address of dump block
byte *const HEX1H      = 0x25;             //
byte *const HEX2L      = 0x26;             // Begin address of dump block
byte *const HEX2H      = 0x27;             //
byte *const LASTSTATE  = 0x29;             // Last input state
byte *const NUMPULSES  = 0x30;             // Number of long pulses to sync at the header
byte *const TAPEIN     = 0xC081;           // Tape input
byte *const DSP        = 0xD012;           // display data port

#define PACKETSIZE 64

const byte *RX_BUFFER = 0x200;
const byte *RX_BUFFER_END = (0x200+PACKETSIZE-1);

void read_packet()
{
   asm {
                // set READ buffer pointers to $0200-$021F (32 characters)
                lda     #<RX_BUFFER_END
                sta     HEX1L
                lda     #>RX_BUFFER_END
                lda     HEX1H

                lda     #<RX_BUFFER
                sta     HEX2L
                lda     #>RX_BUFFER
                lda     HEX2H

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
                sta     ($26,x)       //  save new byte   *** same as "STA (HEX2L,X)" see KickC bug #756 https://gitlab.com/camelot/kickc/-/issues/756
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
incaddr:        lda     HEX2L         //  compare current address with
                cmp     HEX1L         //   end address
                lda     HEX2H
                sbc     HEX1H
                inc     HEX2L         //  and increment current address
                bne     nocarry       //  no carry to msb!
                inc     HEX2H
nocarry:        rts

                // end of read routine "restidx" is the exit point

restidx:        rts

   }
}

byte reference_packet[PACKETSIZE];
byte *vmeter = "0123456789ABCDEF";

void decode_packets() {

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
      else                   woz_putc(vmeter[i>>2]);               

      // exit with "X"
      if(apple1_readkey()=='X') break;
   }
}

void simple_toggle_monitor()
{
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

void main() {
   woz_puts("\r\rTAPE MONITOR\r\r"
      "[1] DECODE PACKETS\r"
      "[2] SIMPLE TOGGLE MONITOR\r\r"
   );

   while(1) {
      byte key = apple1_getkey();
      if(key == '1') decode_packets();
      if(key == '2') simple_toggle_monitor();
   }
}
