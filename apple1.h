#ifdef APPLE1
   // APPLE1
   #pragma start_address(0x4000)
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
void woz_put_hex(byte c) {
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
inline byte keypressed() {
   #ifdef APPLE1
      return PEEK(KEY_CTRL) & 0x80;
   #else
      return 0;
   #endif
}

// reads a key from the apple-1 keyboard
byte woz_getkey() {
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
