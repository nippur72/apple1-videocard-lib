#!/bin/sh

KICKASSJAR=~/sw/kickass/KickAss.jar

PRG2BIN='node ../../tools/prg2bin.js'
PRG2WOZ='node ../../tools/prg2woz.js'

java -jar ${KICKASSJAR} montyr_apple1.asm -define SID_CC00
${PRG2BIN} -i montyr_apple1.prg -o montyr.sid_cc00.0280R.bin 
${PRG2WOZ} -i montyr_apple1.prg -o montyr.sid_cc00.0280R.woz
cp montyr_apple1.prg montyr.sid_cc00.0280R.prg

java -jar ${KICKASSJAR} montyr_apple1.asm -define SID_C800
${PRG2BIN} -i montyr_apple1.prg -o montyr.sid_c800.0280R.bin
${PRG2WOZ} -i montyr_apple1.prg -o montyr.sid_c800.0280R.woz
cp montyr_apple1.prg montyr.sid_c800.0280R.prg
