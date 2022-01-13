@set JAVAEXE="c:\Program Files\Java\jdk1.8.0_161\bin\java.exe"
@set JAVAEXE="C:\Program Files\Java\jre1.8.0_311\bin\java.exe"

@set KICKASSJAR="C:\Users\Nino1\Desktop\USB\Retro stuff 2\KickAssembler\KickAss.jar"

@set PRG2BIN=call node ..\..\tools\prg2bin.js
@set PRG2WOZ=call node ..\..\tools\prg2woz.js

%JAVAEXE% -jar %KICKASSJAR% montyr_apple1.asm -define SID_CC00
%PRG2BIN% -i montyr_apple1.prg -o montyr.sid_cc00.0280R.bin 
%PRG2WOZ% -i montyr_apple1.prg -o montyr.sid_cc00.0280R.woz

%JAVAEXE% -jar %KICKASSJAR% montyr_apple1.asm -define SID_C800
%PRG2BIN% -i montyr_apple1.prg -o montyr.sid_c800.0280R.bin
%PRG2WOZ% -i montyr_apple1.prg -o montyr.sid_c800.0280R.woz 




