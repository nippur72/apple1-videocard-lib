@echo off

call ..\..\tools\build checksum
call ..\..\tools\build ramchk
call ..\..\tools\build ramfill

copy out\checksum.prg ..\..\..\apple1-emu\software\checksum.prg /y
copy out\ramchk.prg   ..\..\..\apple1-emu\software\ramchk.prg /y
copy out\ramfill.prg  ..\..\..\apple1-emu\software\ramfill.prg /y

copy out\checksum.bin ..\..\..\apple1-emu\software\sdcard_image\PLAB\CHKSUM#060280 /y
copy out\ramchk.bin   ..\..\..\apple1-emu\software\sdcard_image\PLAB\RAMCHK#060280 /y
copy out\ramfill.bin  ..\..\..\apple1-emu\software\sdcard_image\PLAB\RAMFILL#060280 /y

