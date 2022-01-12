#include <utils.h>
#include <apple1.h>

const byte *HEX1L      = 0x24;             // End address of dump block
const byte *HEX1H      = 0x25;             //
const byte *HEX2L      = 0x26;             // Begin address of dump block
const byte *HEX2H      = 0x27;             //
const byte *LASTSTATE  = 0x29;             // Last input state
const byte *NUMPULSES  = 0x30;             // Number of long pulses to sync at the header
const byte *TAPEIN     = 0xC081;           // Tape input

#define FRAMELEN 64

const byte *RX_BUFFER = 0x200;
const byte *RX_BUFFER_END = (0x200+FRAMELEN-1);

void read_frame()
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

byte data_block[FRAMELEN];
byte *vmeter = "0123456789ABCDEF";

void main() {

   // fill the data block with the known values
   for(byte t=0;t<FRAMELEN;t++) data_block[t] = t;

   woz_puts("\r\rTAPE MONITOR\r\r");

   // tape monitor loop
   for(;;) {
      read_frame();  // attempt reading 1 data block

      // compare received data block with known data
      byte i;
      for(i=0;i<FRAMELEN;i++) {
         if(RX_BUFFER[i] != data_block[i]) break;
      }

      // display result
           if(i==0)        woz_putc('.');
      else if(i==FRAMELEN) woz_putc('*');
      else {
         #if FRAMELEN == 32
         woz_putc(vmeter[i>>1]);
         #endif
         #if FRAMELEN == 64
         woz_putc(vmeter[i>>2]);
         #endif
      }

      // exit with "X"
      if(apple1_readkey()=='X') break;
   }
}

