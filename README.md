# apple1-videocard-lib

Library and demos for the Apple-1 TMS9918 video card by P-LAB.

# USAGE (Windows)

- `env_kick.bat` sets KickC compiler path variable (run it once)
- `c.bat` compiles `test.c` for both VIC-20 and Apple-1
- `test_vic20.prg` runs on the "hybrid" VIC-20 emulator
- `node hexdump` puts `test_apple1.prg` into WOZMON format for the Apple-1

Note: Apple-1 start address is $4000, with TMS address range $CC00-$CC01


