# apple1-videocard-lib

Library and demos for the "Apple-1 Graphic Card" by P-LAB, 
featuring the TMS9918 Video Display Processor by Texas Instruments.

## Repo structure

```
demo/     a demo program that makes use of the library
docs/     TMS9918 and Apple-1 manuals
kickc/    target configuration files for KickC
lib/      the library files to include in your project
tetris/   a demo game
tools/    some build tools
```

## Introduction

The library is a set of C routines that make very easy
to use the TMS9918 on the Apple-1. It is intended to work
with the "Apple-1 Graphic Card" board by [P-LAB]() or any 
other video card that maps the TMS9918 in the `$CC00`-`$CC01`
memory range of the Apple-1.

The library is written in C with [KickC](https://gitlab.com/camelot/kickc/-/releases), a very efficient 6502 C compiler.

## Choice of the screen mode

The library supports screen modes 1 and 2 only (screen 0 and screen 3 not 
being very useful). Both are 256x192 pixels but there are some differences 
you should consider when evaluating which mode to use:

- Screen 1: there are 256 tiles that can be written very quickly on the 
screen, but the color choice is limited to 8 consecutive tiles for each 
color in the palette. It's commonly used for text applications with 
limited colors.

- Screen 2: you can address every pixel on the screen with a limitation 
of 2 colors per line within an 8x8 tile grid. It's commonly used for bitmapped
graphic or slow-but-colorful text.

Both screen modes support 32 sprites.

### Working with screen 1

Some example code:
```c
// *** first we set the SCREEN 1 mode

tms_init_regs(SCREEN1_TABLE);   // initializes the registers with SCREEN 1 precalculated values
screen1_prepare();              // prepares the screen to be used, loads a useful 8x8 ASCII font
tms_set_color(COLOR_VIOLET);    // sets border color to violet (see tms9918.h for the list of all colors)

// *** then we can use it

screen1_cls();                  // clears the screen 
screen1_putc('A');              // writes the character "A"
screen1_putc(CHR_BACKSPACE);    // goes back 1 character
screen1_putc('B');              // overwrites a "B" over it

// writes "A" in the bottom-right corner, causing the screen to scroll
screen1_locate(31,23);
screen1_putc("A");

// some printing
screen1_puts(CLS "Hello world");                  // CLS, REVERSE_ON, REVERSE_OFF
screen1_puts(REVERSE_ON "reverse" REVERSE_OFF);   // are macros defined in tms_screen1.h 
screen1_puts("Line1\nLine2");                     // '\n' is also supported

// simple string input from the keyboard (editing with CTRL-H is also supported)
char buffer[32];
screen1_strinput(buffer, 32);
screen1_prints("you wrote:");
screen1_prints(buffer);
```

### Working with screen 2

Some example code:
```c
// initializes the registers with SCREEN 2 precalculated values
tms_init_regs(SCREEN2_TABLE);   

// sometimes two colors need to be packed into a single byte
// you can easily do that with the FG_BG() macro:
byte mycolor = FG_BG(COLOR_BLACK,COLOR_WHITE);

// prepares the screeen to be used as a bitmap with default colors black on white
screen2_init_bitmap(mycolor);

// plots a pixel in the middle of the screen
screen2_plot(128,96);

// and erases it:   
screen2_plot_mode = PLOT_MODE_RESET;  // PLOT_MODE_INVERT is also supported
screen2_plot(128,96);
screen2_plot_mode = PLOT_MODE_SET;

// draws a diagonal line
screen2_line(0,0,255,191);

// writes a character from the embedded FONT
byte col = FB_BG(COLOR_DARK_RED,COLOR_LIGH_YELLOW);
screen2_putc('A', 31, 23, col);

// writes a string
screen2_puts(16, 12, col, "HELLO");

// note: screen2_putc() and screen2_puts() are fast but they
// can only print characters aligned within the 8x8 grid
```

### Working with VRAM directly

Some example code:
```c
// writes the value 42 at VRAM location 8000
tms_set_vram_write_addr(8000);
TMS_WRITE_DATA_PORT(42);

// and re-reads it
tms_set_vram_read_addr(8000);
byte val = TMS_READ_DATA_PORT;
```

When using the default values that came with `SCREEN1_TABLE[]` and `SCREEN2_TABLE[]`,
VRAM is organized according the following memory map:
```c
// ZONE                RANGE          NAME YOU CAN USE IN C
// =========================================================== 
// pattern table       $0000-$17FF    TMS_PATTERN_TABLE       
// sprite patterns     $1800-$19FF    TMS_SPRITE_PATTERNS  
// color table         $2000-$27FF    TMS_COLOR_TABLE      
// name table          $3800-$3AFF    TMS_NAME_TABLE      
// sprite attributes   $3B00-$3BFF    TMS_SPRITE_ATTRS        

// example: writes the bitmap value 10101010 on row 3 of pattern 4
tms_set_vram_write_addr(TMS_PATTERN_TABLE+4*8+3);
TMS_WRITE_DATA_PORT(0b10101010);
```

### Working on a more low-level

#### Setting the TMS9918 registers

```c
// you can set a TMS9918 register directly with:
tms_write_reg(7, 0x1F);

// which also saves the written value to a buffer
// because the TMS does not allow to read from 
// its registers (they are write-only)
byte oldvalue = tms_regs_latch[7];
```

#### Working directly with the I/O chip interface

If you want to program the VDP directly you can use the following utility functions:

```c
TMS_WRITE_CTRL_PORT(value);       // writes a byte to the control port ($CC01)
TMS_WRITE_DATA_PORT(value);       // writes a byte to the data port ($CC00)
byte value = TMS_READ_CTRL_PORT;  // reads the status register ($CC01)
byte value = TMS_READ_DATA_PORT;  // reads a byte from the data port ($CC00)
```

### Apple-1 utility functions

There are also utility functions to interact with the Apple-1
screen and keyboard:

```c
// prints hex "F3" on the Apple-1 screen
woz_print_hex(0xF3)

// prints "A" on the Apple-1 screen
woz_putc('A');

// prints "HELLO" on the Apple-1 screen
woz_puts("HELLO");

// gets a key from the keyboard (waits for it)
byte k = apple1_getkey();

// non blocking keyboard read: do something until RETURN is hit
while(1) {
   if(apple1_iskeypressed()) {
      if(apple1_readkey()==0x0d) break;
   }
   else do_something_else();
}                
```

### Building the source code

To link the library, simply `#include` the `.h` files 
from the `lib/` directory in your C source files.

Compile your sources with the KickC compiler, the `tools/`
directory contains a `build.bat` script example for Windows.

There are three configurations you can target with the switches `-t target -targetdir thisrepopath/kickc` of the KickC compiler:

- `apple1`
- `apple1_jukebox`
- `vic20`

#### Target "apple1"

With this target, the compiled program will start at `$0280` in 
the free RAM of the Apple-1 (please make sure you have enough RAM).

(TODO: add reference to `hexdump.js`)

#### Target "apple1_jukebox"

This target is for expansion cards that provide a ROM storage in 
the range `$4000`-`$7FFF`, as:
- the "CodeTank" EEPROM daughterboard of the "Apple-1 Graphic Card"
- "Juke-Box Card" FLASH

In this target configuration, the program is split into two segments:
- the `Code` that resides in ROM at `$4000`
- the `Data` that resides in RAM at `$0280`

The split is required because the program needs to write on the `Data` segment (e.g. when changing the value of a variable).

The only issue is that the "Data" segment needs to be initialized 
with the correct startup values (for example, the value that 
a global `int` variable takes before it's used).

The "Data" initialization needs to be done manually in the C program 
by explicitly calling `apple1_eprom_init()` in `main()`. The function 
will copy the ROM portion $7582-$7FFF into the "Data" segment at $0280-$0FFF.

The initialization values in the ROM range $7582-$7FFF are generated with 
the build script `mkeprom.js` which creates a fixed-length 16K binary file 
to be put on the EEPROM/FLASH.

Below is a recap of the memory map for this target:

$0000-$00FF zero page: holds some C program variables 
$0280-$0FFF RAM: C program "Data" segment 
$4000-$7581 ROM: C program "Code" segment 
$7582-$7FFF ROM: C program "Data" segment (startup values)

#### Target "vic20"

This target has been used during the development of the library
where a custom made VIC-20 emulator was interfaced with 
an emulated TMS9918, thus allowing running tests when 
the real Apple-1 machine was not available.
