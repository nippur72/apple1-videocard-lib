; LIFE FOR APPLE-1
; UNKNOWN AUTHOR
; DISASSEMBLED BY ANTONINO PORCINO, 14TH SEP 2022

ECHO      = $FFEF    ; character echo routine in Woz monitor
KEY_DATA  = $D010    ; read key
KEY_CTRL  = $D011    ; control port

; zero page variables

PTR1      = $F0   ; $F0-$F1
PTR2      = $F2   ; $F2-$F3
PTR3      = $F6   ; $F6-$F7
PTR4      = $F8   ; $F8-$F9
CELLCOUNT = $F9   ; also PTR4 high byte
RND       = $FA   ; $FA-$FB random seed
TMP       = $F6   ; also PTR3 low byte

;
; board geometry is 40x24 = 960 bytes
; there are two boards stored in memory:
; primary board:   $2440-$27FF
; secondary board: $2840-$2BFF
;
; the boards contain the actual characters that are printed on 
; the screen: live cell ("*"), dead cell (" ") and top/bottom border ("-")
;

BOARD    = $2440       ; address of the primary board in memory
BOARDEND = $2800       ; end address of the board, for bound checking

ROW0  = $2440          ; address of topmost row in the board (row 0)
ROW1  = $2468          ; address of second row
ROW22 = $27B0          ; address of 23nd row
ROW23 = $27D8          ; address of bottom row in the board (row 23)

DEADCELL  = $A0          ; " " (space character)
ALIVECELL = $AA          ; "*"
FRAMECELL = $AD          ; "-"

   ORG $2000

;
; clears the primary board ($2400-$27FF) 
; bug: the area between $2400-$243F is cleared as well
;
          LDX #$00
          LDA #DEADCELL ; dead cell character
L2004     STA $2400,X   ; bug: this should be $2440
          STA $2500,X
          STA $2600,X
          STA $2700,X
          INX
          BNE L2004

;
; prints the welcome message (56 characters)
; (X is zero from previous BNE)
;

L2013     LDA MESSAGE,X
          JSR ECHO
          INX
          CPX #56      ; prints 56 characters of the message
          BNE L2013

; Asks for user's full name; read and print from keyboard
; until ENTER is pressed. Typed characters are simply discarded,
; the purpose of asking user's full name is to increment the random
; seed generator in $FA-$FB (RND) at user's typing speed.
;
; At the end of the typing, the RND variable will contain
; a fairly good random integer that can be used as a
; seed generator. This because the Apple-1 does not have
; any other way of generating random numbers
;

L201E     JSR RDKEY
          CMP #$8D     ; enter key
          BNE L201E

;
; randomly calculate how many cells to draw at the start of the game
; it's a random number from 32 to 95, stored into "CELLCOUNT" variable
;

          JSR GETRAND    ; gets a random number (0-255) in A
          AND #$3F       ; ranges it $00-$3F    
          ADC #$20       ; ranges it $20-$5F   (carry is not cleared though)
          STA CELLCOUNT  ; number of cells to generate betwen $20-$5F (32-95)

;
; fills the board with the number live cells in "CELLCOUNT"
; bug: also writes out of the screen area between $2400 and $243F
;
          LDY #$00       ; keeps Y to 0 for "STA(),Y" instruction
L2030     JSR GETRAND    ; gets a random number in A
          AND #$03       ; ranges it 0-3
          CLC 
          ADC #BOARD/256 ; board high byte $24 + rnd(0-3)
          STA PTR1+1     ; high byte now ranges from $24 to $27
          LDA RND        ; gets random number directly from seed
          STA PTR1       ; PTR1 now points randomly to $2400-$27FF
          LDA #ALIVECELL ; alive cell character ("*")
          STA (PTR1),Y   ; writes on the board (set cell "ON")
          DEC CELLCOUNT  ; decrements cell counter
          BNE L2030      ; loops until all cells are written (counter reches 0)

;
;$2046
; main game loop
;
MAINLOOP  LDA #BOARD/256  ;
          STA PTR4+1      ;
          LDA #BOARD%256  ;
          STA PTR4        ; sets PTR4 to $2440 (primary board)

;
; draws a line of dashes ("-") on top and bottom rows
;

          LDX #39        ; counts 40 characters backward
          LDA #FRAMECELL ; board frame character ("-")
L2052     STA ROW23,X    ; writes bottom line of the board
          STA ROW0 ,X    ; writes top    line of the board
          DEX            ; decrements char counter
          BPL L2052      ; loops if X >= 0

;
; prints the whole board from $2440 to $2800
; Y is already zero from above
;
L205B     LDA (PTR4),Y        ; reads character from board
          JSR ECHO            ; prints it by the echo routine in Woz monitor
          INC PTR4            ;
          BNE L205B           ;
          INC PTR4+1          ; increments pointer PTR4
          LDA PTR4+1
          CMP #BOARDEND/256   ; checks end of the board
          BNE L205B

;
; reads the keyboard between each game epoch
;
          JSR RDKEY

;
; copy primary board into secondary board (copies $2440-$27FF into $2840-$2BFF)
;
          LDY #$00
          JSR SETPTRS        ; sets board pointers: PTR1=$2440 (primary), PTR2=$2840 (secondary)
L2074     LDA (PTR1),Y       ; reads from primary
          STA (PTR2),Y       ; and writes into secondary
          INC PTR1
          INC PTR2
          BNE L2074
          INC PTR1+1
          INC PTR2+1         ; increments pointers PTR1, PTR2
          LDA PTR1+1
          CMP #BOARDEND/256  ; checks end of the board
          BNE L2074

;
; once the primary board is copied onto secondary, the primary is used to count
; the number of nearby cells around a live cell, so it needs to be filled with 0's
;
          JSR ZEROBOARD   

;
; scans the secondary board: for every live cell, increment by one 
; the nearby cells in the primary board
;
; nearby cells are scanned using Y-indexing with the following Y values:
;
;   -41 -40 -39 
;    -1   *  +1           
;    39  40  41
;

L208B     LDY #$00        ; keeps Y to 0 for "LDA(),Y" instruction
          LDA (PTR2),Y    ; reads cell in the secondary board
          CMP #ALIVECELL  ; is cell alive?
          BNE L209D       ; if no goes next cell
          LDY #39         ; yes it's alive, starts Y indexing from 39
          JSR SUM6CELLS   ; sums cells with Y offsets: +/-39, +/-40, +/-41 (row above and below the live cell)
          LDY #1        
          JSR SUMYCELLS   ; sums cells with Y offsets: +/-1 (left/right of live cell)

          ; increments PTR1 and PTR2 to next cell
L209D     INC PTR1        
          INC PTR2
          BNE L208B
          INC PTR1+1
          INC PTR2+1
          LDA PTR1+1
          CMP #BOARDEND/256    ; checks end of the board
          BNE L208B

;
; sums the primary board accumulated score from bottom row into top, and top into bottom
; this actaully "wraps" the playfield making it like a toroid (sort of)
;
; does: ROW1  = ROW1  + ROW23
;       ROW22 = ROW22 + ROW0
;
          CLC            ; keeps carry clear for ADC below
          LDX #39        ; counts 40 characters (1 row)
L20B0     LDA ROW23,X    ; reads 24th row
          ADC ROW1,X
          STA ROW1,X     ; and sums into 2nd
          LDA ROW0,X     ; reads first row
          ADC ROW22,X    ; and sums into row 23rd (note: numbers are small, carry always 0, no need to CLC again)
          STA ROW22,X
          DEX
          BPL L20B0

;
; the actual GAME OF LIFE computation (Conway's rules)
; at this point the secondary board contains the board ("*" or " ") of the last epoch;
; primary board contains the number of nearby cells for each cell that is alive
;

          JSR SETPTRS       ; starts from top of the boards
L20C8     LDY #$00          ; keeps Y to 0 for "LDA (),Y"
          LDA (PTR1),Y      ; reads number of nearby cells
          CMP #2            ; two cells nearby?
          BNE L20D4         ; if not checks for three
          LDA (PTR2),Y      ; yes two cells nearby: cells survives to next round
          BNE WRITECELL
L20D4     CMP #3            ; three cells nearby?
          BNE SETDEAD       ; if not then it's dead cell
          LDA #ALIVECELL    ; else it's alive (does not matter if it was alive or not)
          BNE WRITECELL     ; cheap jump to WRITECELL
SETDEAD   LDA #DEADCELL     ; sets next cell dead
WRITECELL STA (PTR1),Y      ; writes the cell on the board

          ; increments pointers to next cell
          INC PTR1
          INC PTR2
          BNE L20C8
          INC PTR1+1
          INC PTR2+1
          LDA PTR2+1
          CMP #$2C          ; checsk end of the secondary board (which ends at $2C00)
          BNE L20C8

          JMP MAINLOOP      ; loops forever to the next epoch of the game

;
; subroutine that set pointers to point to primary and secondary boards:
; PTR1 = $2440   (primary board)
; PTR2 = $2840   (secondary board)
;
; $20F3
SETPTRS   LDA #$24
          STA PTR1+1
          LDA #$28
          STA PTR2+1
          LDA #$40
          STA PTR1
          STA PTR2
          RTS

; $2102
;
; subroutine that reads a key and displays it on the terminal
; returns key in A
; increments a 16 bit counter that works like a random number seed generator
;         
RDKEY     INC RND
          BNE L2108
          INC RND+1        ; increments RND as a 16 bit number
L2108     LDA KEY_CTRL     ; Apple-1 keyboard status port
          BPL RDKEY        ; if key not pressed reads it again
          LDA KEY_DATA     ; reads the actual key code from Apple-1 keyboard data port
          JSR ECHO         ; displays it in on screen
          RTS

; $2114
;
; this subroutine loops over Y=39, 40, 41
; and calls another soubroutine that increments 
; cell at +/- Y values
;
SUM6CELLS JSR SUMYCELLS
          INY
          CPY #42          ; is it 42? (starts at 39, ends at 41)
          BNE SUM6CELLS    ; no, loops to next Y value
          RTS

;$211D
;
; subroutine that increments PTR1 at +/- Y
; e.g.: INC (PTR1),+Y
;       INC (PTR1),-Y

SUMYCELLS 
          ; increments cell at (PTR1),Y
          CLC             
          LDA (PTR1),Y    
          ADC #$01        ; increments by one
          STA (PTR1),Y    

          STY CELLCOUNT   ; saves Y into unused location CELLCOUNT

          ; PTR3 = PTR1 - Y
          SEC
          LDA PTR1
          SBC CELLCOUNT
          STA PTR3
          LDA PTR1+1
          SBC #$00
          STA PTR3+1      

          ; increments cell at (PTR3),Y e.g. (PTR1-Y)
          LDY #$00
          LDA (PTR3),Y
          CLC
          ADC #$01       ; increments by one
          STA (PTR3),Y
          LDY CELLCOUNT  ; restores Y
          RTS
;          
;$213F
;
; advances the random number generator 
; and loads a random number into A
; uses a sort of a linear feedback shift register
;
GETRAND   LDA RND+1
          STA TMP
          LDA RND
          ASL A
          ROL TMP
          ASL A
          ROL TMP      ; [TMP|A] = [TMP|A] << 2 
          CLC
          ADC RND       
          PHA
          LDA TMP
          ADC RND+1
          STA RND+1     
          PLA
          ADC #$11
          STA RND
          LDA RND+1
          ADC #$36
          STA RND+1    ; RND = RND + $3611 + C
          RTS

;$2161
;
;fills the primary board ($2440-$27FF) with $00
;
ZEROBOARD JSR SETPTRS        ; initialized the pointers
          LDA #$00           ; value to fill
          TAY                ; keeps Y to 0 for STA(),Y
L2167     STA (PTR1),Y
          INC PTR1
          BNE L2167
          INC PTR1+1
          LDX PTR1+1
          CPX #BOARDEND/256  ; check end of the board
          BNE L2167
          JSR SETPTRS        ; reset correct pointers on exit
          RTS

L2179:
   DB $A0 ; unused byte

;
; $2180
; welcome message in Woz-ascii code
;
MESSAGE:
   DB $A0,$A0,$A0,$A0,$A0,$A0              ; "      "
   DB $8D,$8D,$8D,$8D                      ; "\n\n\n\n"
   DB $C3,$CF,$CE,$D7,$C1,$D9,$A7,$D3,$A0  ; "CONWAY'S "
   DB $C7,$C1,$CD,$C5,$A0                  ; "GAME "
   DB $CF,$C6,$A0,$CC,$C9,$C6,$C5,$8D,$8D  ; "OF LIFE\n\n"
   DB $D0,$CC,$C5,$C1,$D3,$C5,$A0          ; "PLEASE "
   DB $D4,$D9,$D0,$C5,$A0                  ; "TYPE "
   DB $D9,$CF,$D5,$D2,$A0                  ; "YOUR "
   DB $C6,$D5,$CC,$CC,$A0                  ; "FULL "
   DB $CE,$C1,$CD,$C5,$AE,$8D,$8D          ; "NAME.\n\n"

; $21B9
                            
