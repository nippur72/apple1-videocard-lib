# apple1-videocard-lib

Library and demos for the Apple-1 TMS9918 video card by P-LAB.


# Building for the "Juke Box" EPROM expansion

The demo contained in the repo are meant to be launched from the Apple-1 "Juke Box"
EPROM expansion at address $4000. The executables are built according to the
following memory map:

$0000-$00FF zero page: holds most of C program variables
$0280-$0FFF low RAM space: C program "Data" segment
$4000-$7581 EPROM (Juke Box): C program "Code" segment
$7582-$7FFF EPROM (Juke Box): C program "Data" segment (startup values)

The build script "mkeprom.js" creates a 16K binary file to be placed on the
EPROM where the "Data" segment startup values are put at the end of the file itself.
The user program need to manually copy such data from EPROM to low RAM once after
the `main()` is launched. That can be easily done by simply calling the library
function `apple1_eprom_init()`.



