@set TARGET=NOJUKEBOX
@call ..\..\tools\build anagram
copy out\anagram.prg ..\..\..\apple1-emu\software\anagram.prg /y
copy out\anagram.bin ..\..\..\apple1-emu\software\sdcard_image\PLAB\ANAGRAM#060280 /y

@set TARGET=NOJUKEBOX
@call ..\..\tools\build hangman
copy out\hangman.prg ..\..\..\apple1-emu\software\hangman.prg /y
copy out\hangman.bin ..\..\..\apple1-emu\software\sdcard_image\PLAB\HANGMAN#060280 /y

