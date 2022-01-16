#include <utils.h>
#include <apple1.h>

byte *const LASTSTATE  = 0x29;             // Last input state
byte *const TAPEIN     = 0xC081;           // Tape input
byte *const DSP        = 0xD012;           // display data port

void main()
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
