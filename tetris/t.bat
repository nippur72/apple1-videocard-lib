@rem === BUILD TETRIS ===

@SET TARGET=tetris


@rem     @echo ======================== VIC20 ===================================================
@rem     call kickc -t VIC20_8K -D=VIC20 %TARGET%.c -o %TARGET%_vic20.prg -e
@rem     copy %TARGET%.prg %TARGET%_vic20.prg


@echo ======================== APPLE 1 =================================================
call kickc -t apple1 -D=APPLE1 %TARGET%.c -o %TARGET%_apple1.prg -e -Xassembler="-symbolfile"
copy %TARGET%.prg %TARGET%_apple1.prg

@rem call node hexdump > test_apple1.woz
call node bindump 

@rem @del *.klog

@del *.vs
@del *.dbg
@del tetris.prg
@del tetris_apple1.prg
