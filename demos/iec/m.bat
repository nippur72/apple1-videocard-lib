@echo off

set DASM=C:\Users\Nino1\Desktop\USB\compilers\dasm\dasm.exe
set ASMPROC=call asmproc
rem set ASMPROC=node ..\..\asmproc\dist\asmproc

%ASMPROC% -i iec.lm -o iec.asm -t dasm -d APPLE1
if %errorlevel% == -1 goto fine

%DASM% iec.asm -f1 -liec.lst -oiec.prg
if %errorlevel% == -1 goto fine

call node ..\..\tools\prg2bin.js -i iec.prg -o iec#060280

echo "ALL OK!"

:fine

