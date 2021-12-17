@rem === BUILD SCRIPT ===

@SET TARGET=%1
@SET TMS9918=..\..

@echo ======================== VIC20 ===================================================
call kickc -includedir %TMS9918%\lib -targetdir %TMS9918%\kickc\ -t VIC20_8K %TARGET%.c -o out\%TARGET%_vic20.prg -e
copy out\%TARGET%.prg out\%TARGET%_vic20.prg

@echo ======================== APPLE 1 =================================================
call kickc -includedir %TMS9918%\lib -targetdir %TMS9918%\kickc\ -t apple1_jukebox %TARGET%.c -o out\%TARGET%_apple1.prg -e

@rem builds the apple1 eprom file
call node %TMS9918%\tools\mkeprom out out\%TARGET%_apple1.bin

@rem clean up files
@del out\apple1_codeseg.bin
@del out\apple1_dataseg.bin
@del out\*.vs
@del out\*.klog
@del out\*.vs
@del out\*.dbg
@del out\%TARGET%.prg
