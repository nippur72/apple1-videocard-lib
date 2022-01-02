#!/bin/sh

TARGET=$1
TMS9918=../..

mkdir -p out

echo ======================== VIC20 ===================================================
kickc.sh -includedir ${TMS9918}/lib -targetdir ${TMS9918}/kickc/ -t vic20_8k ${TARGET}.c -o out/${TARGET}_vic20.prg -e
cp out/${TARGET}.prg out/${TARGET}_vic20.prg

echo ======================== APPLE 1 JUKEBOX =================================================
kickc.sh -includedir ${TMS9918}/lib -targetdir ${TMS9918}/kickc/ -t apple1_jukebox ${TARGET}.c -o out/${TARGET}_apple1.prg -e
node ${TMS9918}/tools/mkeprom out out/${TARGET}_apple1.bin

echo ======================== APPLE 1 =================================================
kickc.sh -includedir ${TMS9918}/lib -targetdir ${TMS9918}/kickc/ -t apple1 ${TARGET}.c -o out/${TARGET}_apple1.prg -e
cp out/${TARGET}.prg out/${TARGET}_apple1.prg

# clean up files
rm -f out/apple1_codeseg.bin
rm -f out/apple1_dataseg.bin
rm -f out/*.vs
rm -f out/*.klog
rm -f out/*.vs
rm -f out/*.dbg
rm -f out/${TARGET}.prg
