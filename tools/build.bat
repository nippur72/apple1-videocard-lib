@rem === BUILD SCRIPT ===

@SET TARGET=%1

@echo ======================== VIC20 ===================================================
call kickc -targetdir ..\kickc\ -t VIC20_8K -D=VIC20 %TARGET%.c -o out\%TARGET%_vic20.prg -e
copy out\%TARGET%.prg out\%TARGET%_vic20.prg

@echo ======================== APPLE 1 =================================================
call kickc -targetdir ..\kickc\ -t apple1 -D=APPLE1 %TARGET%.c -o out\%TARGET%_apple1.prg -e

@rem builds the apple1 eprom file
call node ..\tools\mkeprom out out\%TARGET%_apple1.bin

@rem clean up files
@del out\apple1_codeseg.bin
@del out\apple1_dataseg.bin
@del out\*.vs
@del out\*.klog
@del out\*.vs
@del out\*.dbg
@del out\%TARGET%.prg
