call ..\..\tools\build sdcard
copy out\sdcard.prg ..\..\..\apple1-emu\software\sdcard.prg /y

@grep comando_load_bas out/sdcard.sym
@grep comando_asave out/sdcard.sym
@grep chksum_table out/sdcard.sym
