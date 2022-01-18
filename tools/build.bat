@rem === BUILD SCRIPT ===

@SET TARGET=%1
@SET TMS9918=..\..

@echo ======================== VIC20 ===================================================
call kickc -includedir %TMS9918%\lib -targetdir %TMS9918%\kickc\ -t vic20_8k %TARGET%.c -o out\%TARGET%_vic20.prg -e
copy out\%TARGET%.prg out\%TARGET%_vic20.prg

@echo ======================== APPLE 1 JUKEBOX =================================================
call kickc -includedir %TMS9918%\lib -targetdir %TMS9918%\kickc\ -t apple1_jukebox %TARGET%.c -o out\%TARGET%.prg -e
call node %TMS9918%\tools\mkeprom out out\%TARGET%_jukebox.bin

@echo ======================== APPLE 1 =================================================
call kickc -includedir %TMS9918%\lib -targetdir %TMS9918%\kickc\ -t apple1 %TARGET%.c -o out\%TARGET%.prg -e
call node %TMS9918%\tools\prg2bin -i out\%TARGET%.prg -o out\%TARGET%.bin
call node %TMS9918%\tools\prg2woz -i out\%TARGET%.prg -o out\%TARGET%.woz

@rem clean up files
@del out\apple1_codeseg.bin
@del out\apple1_dataseg.bin
@del out\*.vs
@del out\*.klog
@del out\*.vs
@del out\*.dbg
