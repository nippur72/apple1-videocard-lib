../../tools/build.sh tapemon
node ../../tools/prg2bin.js -i out/tapemon_apple1.prg -o out/tapemon.0280.bin

../../tools/build.sh tapetoggle
node ../../tools/prg2woz.js -i out/tapetoggle_apple1.prg -o out/tapetoggle.0280.woz
