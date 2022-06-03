@rem === BUILD SCRIPT ===

@SET FNAME=%1
@SET TMS9918=..\..

@IF "%TARGET%" == "NOJUKEBOX" GOTO NOJUKEBOX

@echo ======================== APPLE 1 JUKEBOX =================================================
call kickc -includedir %TMS9918%\lib -targetdir %TMS9918%\kickc\ -t apple1_jukebox %FNAME%.c -o out\%FNAME%.prg -e
call node %TMS9918%\tools\mkeprom out out\%FNAME%_jukebox.bin

:NOJUKEBOX
@echo ======================== APPLE 1 =================================================
call kickc -includedir %TMS9918%\lib -targetdir %TMS9918%\kickc\ -t apple1 %FNAME%.c -o out\%FNAME%.prg -e -Xassembler="-symbolfile"
call node %TMS9918%\tools\prg2bin -i out\%FNAME%.prg -o out\%FNAME%.bin
call node %TMS9918%\tools\prg2woz -i out\%FNAME%.prg -o out\%FNAME%.woz

@rem clean up files
@del out\apple1_codeseg.bin
@del out\apple1_dataseg.bin
@del out\*.vs
@del out\*.klog
@del out\*.vs
@del out\*.dbg
