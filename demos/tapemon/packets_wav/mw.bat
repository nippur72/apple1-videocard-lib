call node mkpackets

call node ../../../tools/wavconv/prg2wav.js -i packets_32.bin  -o packets_32 -b 00
call node ../../../tools/wavconv/prg2wav.js -i packets_64.bin  -o packets_64 -b 00
call node ../../../tools/wavconv/prg2wav.js -i packets_128.bin -o packets_128 -b 00
call node ../../../tools/wavconv/prg2wav.js -i packets_255.bin -o packets_255 -b 00

del packets_32.bin
del packets_64.bin
del packets_128.bin
del packets_255.bin

call node ../../../tools/wavconv/prg2wav.js -i single_tone_0.bin -o single_tone_0 -b 00
call node ../../../tools/wavconv/prg2wav.js -i single_tone_255.bin -o single_tone_255 -b 00

del single_tone_0.bin
del single_tone_255.bin

