cl65 -D APPLE1 --target none  --start-addr $4000 -O test.c -o test_apple1.prg
cl65 --listing test.lst -D VIC20 --target vic20 -O test.c -o test_vic20.prg


