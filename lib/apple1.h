#ifndef APPLE1_H
#define APPLE1_H

#ifdef APPLE1
   // APPLE1   
   const word WOZMON    = 0xFF1F;      // enters monitor
   const word ECHO      = 0xFFEF;      // output ascii character in A (A not destroyed)
   const word PRBYTE    = 0xFFDC;      // print hex byte in A (A destroyed)
   const word KEY_DATA  = 0xd010;      // read key
   const word KEY_CTRL  = 0xd011;      // control port
   const word TERM_DATA = 0xd012;      // write ascii
   const word TERM_CTRL = 0xd013;      // control port
#else
   // VIC20
   const word ECHO      = 0xFFD2;       // chrout routine in kernal rom
   const word GETIN     = 0xFFE4;       // GETIN keyboard read routine
#endif

// prints a hex byte using the WOZMON routine
void woz_print_hex(byte c) {
   #ifdef APPLE1
      asm {
         lda c
         jsr PRBYTE
      };
   #else
      asm {
         lda c
         jsr ECHO
      };
   #endif
}

// puts a character on the apple1 screen using the WOZMON routine
void woz_putc(byte c) {
   asm {
      lda c
      jsr ECHO
   }
}

// puts a 0-terminated string on the apple1 screen
void woz_puts(byte *s) {
   byte c;
   while(c=*s++) woz_putc(c);
}

// returns to WOZMON prompt
void woz_mon() {
   #ifdef APPLE1
      asm {
         jmp WOZMON
      }
   #endif
}

// returns nonzero if a key has been pressed
inline byte apple1_iskeypressed() {
   #ifdef APPLE1
      return PEEK(KEY_CTRL) & 0x80;
   #else
      return 0;
   #endif
}

// blocking keyboard read
// reads a key from the apple-1 keyboard
byte apple1_getkey() {
   #ifdef APPLE1
      asm {
         __wait:
         lda KEY_CTRL
         bpl __wait
      }
      return PEEK(KEY_DATA) & 0x7f;
   #else
      byte key;
      byte const *keyptr = &key;
      kickasm(uses keyptr, uses GETIN) {{
         __wait:
         jsr GETIN
         cmp #0
         beq __wait
         sta keyptr
      }}
      return key;
   #endif
}

// non blocking keyboard read
// reads a key and return 0 if no key is pressed
byte apple1_readkey() {
   #ifdef APPLE1
      if((PEEK(KEY_CTRL) & 0x80)==0) return 0;
      else return PEEK(KEY_DATA) & 0x7f;
   #else
      byte key;
      byte const *keyptr = &key;
      kickasm(uses keyptr, uses GETIN) {{
         jsr GETIN
         cmp #0
         bne __keypress
         lda #0
         __keypress:
         sta keyptr
      }}
      return key;
   #endif
}

#ifdef APPLE1

#include <stdlib.h> // for memcpy

#define LOWRAM_START 0x280
#define LOWRAM_END   0x7FF
#define LOWRAM_SIZE  (LOWRAM_END - LOWRAM_START + 1)
#define DATAINCODE   (0x8000 - LOWRAM_SIZE)

inline void apple1_eprom_init() {
   // copy the initializaton data from ROM to lowram where "Data" segment is allocated
   memcpy((byte *)LOWRAM_START, (byte *)DATAINCODE, LOWRAM_SIZE);
}
#endif

#endif