#include <utils.h>
#include <apple1.h>

#pragma start_address(0x3000)

byte *const HEX1L      = 0x24;             // End address of dump block
byte *const HEX1H      = 0x25;             //
byte *const HEX2L      = 0x26;             // Begin address of dump block
byte *const HEX2H      = 0x27;             //
byte *const LASTSTATE  = 0x29;             // Last input state
byte *const NUMPULSES  = 0x30;             // Number of long pulses to sync at the header
byte *const TAPEIN     = 0xC081;           // Tape input
byte *const DSP        = 0xD012;           // display data port

void load_from_tape(word start_address, word end_address)
{
   *((word *)HEX1L) = end_address;
   *((word *)HEX2L) = start_address;

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

// converts the hexadecimal string argument to 16 bit word
word xtow(byte *str) {
   word res=0;

   byte c;
   for(byte i=0; c=str[i]; ++i) {
      res = res << 4;
      if(c<'A') res += (c-'0');
      else      res += (c-'A');
   }
   return res;
}

byte keybuf[32];

void main() {
   woz_puts("\r\rTAPE ALTERNATE LOAD\r");

   word start_address;
   word end_address;

   woz_puts("\rSTART ADDRESS: "); apple1_input_line(keybuf, 32); start_address = xtow(keybuf);
   woz_puts("\rEND   ADDRESS: "); apple1_input_line(keybuf, 32); end_address   = xtow(keybuf);

   end_address += 1; // include checksum byte

   while(1) {
      woz_puts("\r\rPRESS PLAY ON TAPE\r\r");

      load_from_tape(start_address, end_address);

      // calculate checksum
      byte chk = 0xFF;
      for(byte *t=start_address; t<end_address; t++) {
         chk ^= *t;
      }

      if(chk == 0) {
         woz_puts("OK\r");
         break;
      }

      woz_puts("?LOAD ERROR\r\r");
      woz_puts("RETRY ? (Y/N) ");
      if(apple1_getkey()!='Y') break;
   }

   woz_mon();
}
