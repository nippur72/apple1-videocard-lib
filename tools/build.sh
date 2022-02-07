#!/bin/sh

FNAME=$1
TMS9918=../..

mkdir -p out

echo ======================== APPLE 1 JUKEBOX =================================================
kickc.sh -includedir ${TMS9918}/lib -targetdir ${TMS9918}/kickc/ -t apple1_jukebox ${FNAME}.c -o out/${FNAME}.prg -e
node ${TMS9918}/tools/mkeprom out out/${FNAME}_jukebox.bin

echo ======================== APPLE 1 =================================================
kickc.sh -includedir ${TMS9918}/lib -targetdir ${TMS9918}/kickc/ -t apple1 ${FNAME}.c -o out/${FNAME}.prg -e
node ${TMS9918}/tools/prg2bin -i out/${FNAME}.prg -o out/${FNAME}.bin
node ${TMS9918}/tools/prg2woz -i out/${FNAME}.prg -o out/${FNAME}.woz

# clean up files
rm -f out/apple1_codeseg.bin
rm -f out/apple1_dataseg.bin
rm -f out/*.vs
rm -f out/*.klog
rm -f out/*.vs
rm -f out/*.dbg

