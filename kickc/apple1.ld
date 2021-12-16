// Apple 1
.file [name="%O", type="prg", segments="Program"]
.segmentdef Program [segments="Code, Data"]
.segmentdef Code [start=%P,outBin="apple1_codeseg.bin"]
.segmentdef Data [start=$0280,min=$0280,max=$0fff,outBin="apple1_dataseg.bin"]

