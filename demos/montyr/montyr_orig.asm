;======================================================================================
;taken from https://codebase64.org/doku.php?id=magazines:chacking5#rob_hubbard_s_musicdisassembled_commented_and_explained
;======================================================================================
;
;How to use the sourcecode:
;*************************
;
;    jsr    music+0 to initialize the music number in the accumulator
;    jsr    music+3 to play the music
;    jsr    music+6 to stop the music and quieten SID
;
;  The music is supposed to run at 50Hz, or 50 times per second. Therefore
;PAL users can run the music routine off the IRQ like this:
;
;    lda    #$00     ; init music number 0
;    jsr    music+0
;    sei             ; install the irq and a raster compare
;    lda    #<irq
;    ldx    #>irq
;    sta    $314
;    stx    $315
;    lda    #$1b
;    sta    $d011
;    lda    #$01
;    sta    $d01a
;    lda    #$7f
;    sta    $dc0d
;    cli
;loop =*
;    jmp    loop     ; endless loop (music is now playing off interrupt :-)
;
;irq =*
;    lda    #$01
;    sta    $d019
;    lda    #$3c
;    sta    $d012
;
;    inc    $d020    ; play music, and show a raster for the time it takes
;    jsr    music+3
;    dec    $d020
;
;    lda    #$14
;    sta    $d018
;    jmp    $ea31
; ======================================================================================


;rob hubbard
;monty on the run music driver

;this player was used (with small mods)
;for his first approx 30 musix

.org $8000
.obj motr

 jmp initmusic
 jmp playmusic
 jmp musicoff


;====================================
;init music

initmusic =*

  lda #$00         ;music num
  ldy #$00
  asl
  sta tempstore
  asl
  clc
  adc tempstore    ;now music num*6
  tax

- lda songs,x      ;copy ptrs to this
  sta currtrkhi,y  ;music's tracks to
  inx              ;current tracks
  iny
  cpy #$06
  bne -

  lda #$00         ;clear control regs
  sta $d404
  sta $d40b
  sta $d412
  sta $d417

  lda #$0f         ;full volume
  sta $d418

  lda #$40         ;flag init music
  sta mstatus

  rts


;====================================
;music off

musicoff =*

  lda #$c0         ;flag music off
  sta mstatus
  rts


;====================================
;play music

playmusic =*

  inc counter

  bit mstatus      ;test music status
  bmi moff         ;$80 and $c0 is off
  bvc contplay     ;$40 init, else play


;==========
;init the song (mstatus $40)

  lda #$00         ;init counter
  sta counter

  ldx #3-1
- sta posoffset,x  ;init pos offsets
  sta patoffset,x  ;init pat offsets
  sta lengthleft,x ;get note right away
  sta notenum,x
  dex
  bpl -

  sta mstatus      ;signal music play
  jmp contplay


;==========
;music is off (mstatus $80 or $c0)

moff =*

  bvc +            ;if mstatus $c0 then
  lda #$00
  sta $d404        ;kill voice 1,2,3
  sta $d40b        ;control registers
  sta $d412

  lda #$0f         ;full volume still
  sta $d418

  lda #$80         ;flag no need to kill
  sta mstatus      ;sound next time

+ jmp musicend     ;end


;==========
;music is playing (mstatus otherwise)

contplay =*

  ldx #3-1         ;number of chanels

  dec speed        ;check the speed
  bpl mainloop

  lda resetspd     ;reset speed if needed
  sta speed


mainloop =*

  lda regoffsets,x ;save offset to regs
  sta tmpregofst   ;for this channel
  tay


;check whether a new note is needed

  lda speed        ;if speed not reset
  cmp resetspd     ;then skip notework
  beq checknewnote
  jmp vibrato

checknewnote =*

  lda currtrkhi,x  ;put base addr.w of
  sta $02          ;this track in $2
  lda currtrklo,x
  sta $03

  dec lengthleft,x ;check whether a new
  bmi getnewnote   ;note is needed

  jmp soundwork    ;no new note needed


;==========
;notework
;a new note is needed. get the pattern
;number/cc from this position

getnewnote =*

  ldy posoffset,x  ;get the data from
  lda ($02),y      ;the current position

  cmp #$ff         ;pos $ff restarts
  beq restart

  cmp #$fe         ;pos $fe stops music
  bne getnotedata  ;on all channels
  jmp musicend

;cc of $ff restarts this track from the
;first position

restart =*

  lda #$00         ;get note immediately
  sta lengthleft,x ;and reset pat,pos
  sta posoffset,x
  sta patoffset,x
  jmp getnewnote


;get the note data from this pattern

getnotedata =*

  tay
  lda patptl,y     ;put base addr.w of
  sta $04          ;the pattern in $4
  lda patpth,y
  sta $05

  lda #$00         ;default no portamento
  sta portaval,x

  ldy patoffset,x  ;get offset into ptn

  lda #$ff         ;default no append
  sta appendfl

;1st byte is the length of the note 0-31
;bit5 signals no release (see sndwork)
;bit6 signals appended note
;bit7 signals a new instrument
;     or portamento coming up

  lda ($04),y      ;get length of note
  sta savelnthcc,x
  sta templnthcc
  and #$1f
  sta lengthleft,x

  bit templnthcc   ;test for append
  bvs appendnote

  inc patoffset,x  ;pt to next data

  lda templnthcc   ;2nd byte needed?
  bpl getpitch

;2nd byte needed as 1st byte negative
;2nd byte is the instrument number(+ve)
;or portamento speed(-ve)

  iny
  lda ($04),y      ;get instr/portamento
  bpl +

  sta portaval,x   ;save portamento val
  jmp ++

+ sta instrnr,x    ;save instr nr

+ inc patoffset,x

;3rd byte is the pitch of the note
;get the 'base frequency' here

getpitch =*

  iny
  lda ($04),y      ;get pitch of note
  sta notenum,x
  asl              ;pitch*2
  tay
  lda frequenzlo,y ;save the appropriate
  sta tempfreq     ;base frequency
  lda frequenzhi,y
  ldy tmpregofst
  sta $d401,y
  sta savefreqhi,x
  lda tempfreq
  sta $d400,y
  sta savefreqlo,x
  jmp +

appendnote =*

  dec appendfl     ;clever eh?


;fetch all the initial values from the
;instrument data structure

+ ldy tmpregofst
  lda instrnr,x    ;instr num
  stx tempstore
  asl              ;instr num*8
  asl
  asl
  tax

  lda instr+2,x    ;get control reg val
  sta tempctrl
  lda instr+2,x
  and appendfl     ;implement append
  sta $d404,y

  lda instr+0,x    ;get pulse width lo
  sta $d402,y

  lda instr+1,x    ;get pulse width hi
  sta $d403,y

  lda instr+3,x    ;get attack/decay
  sta $d405,y

  lda instr+4,x    ;get sustain/release
  sta $d406,y

  ldx tempstore    ;save control reg val
  lda tempctrl
  sta voicectrl,x


;4th byte checks for the end of pattern
;if eop found, inc the position and
;reset patoffset for new pattern

  inc patoffset,x  ;preview 4th byte
  ldy patoffset,x
  lda ($04),y

  cmp #$ff         ;check for eop
  bne +

  lda #$00         ;end of pat reached
  sta patoffset,x  ;inc position for
  inc posoffset,x  ;the next time

+ jmp loopcont


;==========
;soundwork
;the instrument and effects processing
;routine when no new note was needed

soundwork =*

;release routine
;set off a release when the length of
;the note is exceeded
;bit4 of the 1st note-byte can specify
;for no release

  ldy tmpregofst

  lda savelnthcc,x ;check for no release
  and #$20         ;specified
  bne vibrato

  lda lengthleft,x ;check for length of
  bne vibrato      ;exceeded

  lda voicectrl,x  ;length exceeded so
  and #$fe         ;start the release
  sta $d404,y      ;and kill adsr
  lda #$00
  sta $d405,y
  sta $d406,y


;vibrato routine
;(does alot of work)

vibrato =*

  lda instrnr,x    ;instr num
  asl
  asl
  asl              ;instr num*8
  tay
  sty instnumby8   ;save instr num*8

  lda instr+7,y    ;get instr fx byte
  sta instrfx

  lda instr+6,y    ;get pulse speed
  sta pulsevalue

  lda instr+5,y    ;get vibrato depth
  sta vibrdepth
  beq pulsework    ;check for no vibrato

  lda counter      ;this is clever!!
  and #7           ;the counter's turned
  cmp #4           ;into an oscillating
  bcc +            ;value (01233210)
  eor #7
+ sta oscilatval

  lda notenum,x    ;get base note
  asl              ;note*2
  tay              ;get diff btw note
  sec              ;and note+1 frequency
  lda frequenzlo+2,y
  sbc frequenzlo,y
  sta tmpvdiflo
  lda frequenzhi+2,y
  sbc frequenzhi,y

- lsr              ;divide difference by
  ror tmpvdiflo    ;2 for each vibrdepth
  dec vibrdepth
  bpl -
  sta tmpvdifhi

  lda frequenzlo,y ;save note frequency
  sta tmpvfrqlo
  lda frequenzhi,y
  sta tmpvfrqhi

  lda savelnthcc,x ;no vibrato if note
  and #$1f         ;length less than 8
  cmp #8
  bcc +

  ldy oscilatval

- dey              ;depending on the osc
  bmi +            ;value, add the vibr
  clc              ;freq that many times
  lda tmpvfrqlo    ;to the base freq
  adc tmpvdiflo
  sta tmpvfrqlo
  lda tmpvfrqhi
  adc tmpvdifhi
  sta tmpvfrqhi
  jmp -

+ ldy tmpregofst   ;save the final
  lda tmpvfrqlo    ;frequencies
  sta $d400,y
  lda tmpvfrqhi
  sta $d401,y


;pulse-width timbre routine
;depending on the control/speed byte in
;the instrument datastructure, the pulse
;width is of course inc/decremented to
;produce timbre

;strangely the delay value is also the
;size of the inc/decrements

pulsework =*

  lda pulsevalue   ;check for pulsework
  beq portamento   ;needed this instr

  ldy instnumby8
  and #$1f
  dec pulsedelay,x ;pulsedelay-1
  bpl portamento

  sta pulsedelay,x ;reset pulsedelay

  lda pulsevalue   ;restrict pulse speed
  and #$e0         ;from $00-$1f
  sta pulsespeed

  lda pulsedir,x   ;pulsedir 0 is up and
  bne pulsedown    ;1 is down

  lda pulsespeed   ;pulse width up
  clc
  adc instr+0,y    ;add the pulsespeed
  pha              ;to the pulse width
  lda instr+1,y
  adc #$00
  and #$0f
  pha
  cmp #$0e         ;go pulsedown when
  bne dumpulse     ;the pulse value
  inc pulsedir,x   ;reaches max ($0exx)
  jmp dumpulse

pulsedown =*

  sec              ;pulse width down
  lda instr+0,y
  sbc pulsespeed   ;sub the pulsespeed
  pha              ;from the pulse width
  lda instr+1,y
  sbc #$00
  and #$0f
  pha
  cmp #$08         ;go pulseup when
  bne dumpulse     ;the pulse value
  dec pulsedir,x   ;reaches min ($08xx)

dumpulse =*

  stx tempstore    ;dump pulse width to
  ldx tmpregofst   ;chip and back into
  pla              ;the instr data str
  sta instr+1,y
  sta $d403,x
  pla
  sta instr+0,y
  sta $d402,x
  ldx tempstore


;portamento routine
;portamento comes from the second byte
;if it's a negative value

portamento =*

  ldy tmpregofst
  lda portaval,x   ;check for portamento
  beq drums        ;none

  and #$7e         ;toad unwanted bits
  sta tempstore

  lda portaval,x   ;bit0 signals up/down
  and #$01
  beq portup

  sec              ;portamento down
  lda savefreqlo,x ;sub portaval from
  sbc tempstore    ;current frequency
  sta savefreqlo,x
  sta $d400,y
  lda savefreqhi,x
  sbc #$00         ;(word arithmetic)
  sta savefreqhi,x
  sta $d401,y
  jmp drums

portup =*

  clc              ;portamento up
  lda savefreqlo,x ;add portval to
  adc tempstore    ;current frequency
  sta savefreqlo,x
  sta $d400,y
  lda savefreqhi,x
  adc #$00
  sta savefreqhi,x
  sta $d401,y


;bit0 instrfx are the drum routines
;the actual drum timbre depends on the
;crtl register value for the instrument:
;ctrlreg 0 is always noise
;ctrlreg x is noise for 1st vbl and x
;from then on

;see that the drum is made by rapid hi
;to low frequency slide with fast attack
;and decay

drums =*

  lda instrfx      ;check if drums
  and #$01         ;needed this instr
  beq skydive

  lda savefreqhi,x ;don't bother if freq
  beq skydive      ;can't go any lower

  lda lengthleft,x ;or if the note has
  beq skydive      ;finished

  lda savelnthcc,x ;check if this is the
  and #$1f         ;first vbl for this
  sec              ;instrument-note
  sbc #$01
  cmp lengthleft,x
  ldy tmpregofst
  bcc firstime

  lda savefreqhi,x ;not the first time
  dec savefreqhi,x ;so dec freqhi for
  sta $d401,y      ;drum sound

  lda voicectrl,x  ;if ctrlreg is 0 then
  and #$fe         ;noise is used always
  bne dumpctrl

firstime =*

  lda savefreqhi,x ;noise is used for
  sta $d401,y      ;the first vbl also
  lda #$80         ;(set noise)

dumpctrl =*

  sta $d404,y


;bit1 instrfx is the skydive
;a long portamento-down from the note
;to zerofreq

skydive =*

  lda instrfx      ;check if skydive
  and #$02         ;needed this instr
  beq octarp

  lda counter      ;every 2nd vbl
  and #$01
  beq octarp

  lda savefreqhi,x ;check if skydive
  beq octarp        ;already complete

  dec savefreqhi,x ;decr and save the
  ldy tmpregofst   ;high byte freq
  sta $d401,y


;bit2 instrfx is an octave arpeggio
;pretty tame huh?

octarp =*

  lda instrfx      ;check if arpt needed
  and #$04
  beq loopcont

  lda counter      ;only 2 arpt values
  and #$01
  beq +

  lda notenum,x    ;odd, note+12
  clc
  adc #$0c
  jmp ++

+ lda notenum,x    ;even, note

+ asl              ;dump the corresponding
  tay              ;frequencies
  lda frequenzlo,y
  sta tempfreq
  lda frequenzhi,y
  ldy tmpregofst
  sta $d401,y
  lda tempfreq
  sta $d400,y


;==========
;end of dbf loop

loopcont =*

  dex              ;dbf mainloop
  bmi musicend
  jmp mainloop

musicend =*

  rts


;====================================
;frequenz data
;====================================

frequenzlo .byt $16
frequenzhi .byt $01
 .byt $27,$01,$38,$01,$4b,$01
 .byt $5f,$01,$73,$01,$8a,$01,$a1,$01
 .byt $ba,$01,$d4,$01,$f0,$01,$0e,$02
 .byt $2d,$02,$4e,$02,$71,$02,$96,$02
 .byt $bd,$02,$e7,$02,$13,$03,$42,$03
 .byt $74,$03,$a9,$03,$e0,$03,$1b,$04
 .byt $5a,$04,$9b,$04,$e2,$04,$2c,$05
 .byt $7b,$05,$ce,$05,$27,$06,$85,$06
 .byt $e8,$06,$51,$07,$c1,$07,$37,$08
 .byt $b4,$08,$37,$09,$c4,$09,$57,$0a
 .byt $f5,$0a,$9c,$0b,$4e,$0c,$09,$0d
 .byt $d0,$0d,$a3,$0e,$82,$0f,$6e,$10
 .byt $68,$11,$6e,$12,$88,$13,$af,$14
 .byt $eb,$15,$39,$17,$9c,$18,$13,$1a
 .byt $a1,$1b,$46,$1d,$04,$1f,$dc,$20
 .byt $d0,$22,$dc,$24,$10,$27,$5e,$29
 .byt $d6,$2b,$72,$2e,$38,$31,$26,$34
 .byt $42,$37,$8c,$3a,$08,$3e,$b8,$41
 .byt $a0,$45,$b8,$49,$20,$4e,$bc,$52
 .byt $ac,$57,$e4,$5c,$70,$62,$4c,$68
 .byt $84,$6e,$18,$75,$10,$7c,$70,$83
 .byt $40,$8b,$70,$93,$40,$9c,$78,$a5
 .byt $58,$af,$c8,$b9,$e0,$c4,$98,$d0
 .byt $08,$dd,$30,$ea,$20,$f8,$2e,$fd


regoffsets .byt $00,$07,$0e
tmpregofst .byt $00
posoffset  .byt $00,$00,$00
patoffset  .byt $00,$00,$00
lengthleft .byt $00,$00,$00
savelnthcc .byt $00,$00,$00
voicectrl  .byt $00,$00,$00
notenum    .byt $00,$00,$00
instrnr    .byt $00,$00,$00
appendfl   .byt $00
templnthcc .byt $00
tempfreq   .byt $00
tempstore  .byt $00
tempctrl   .byt $00
vibrdepth  .byt $00
pulsevalue .byt $00
tmpvdiflo  .byt $00
tmpvdifhi  .byt $00
tmpvfrqlo  .byt $00
tmpvfrqhi  .byt $00
oscilatval .byt $00
pulsedelay .byt $00,$00,$00
pulsedir   .byt $00,$00,$00
speed      .byt $00
resetspd   .byt $01
instnumby8 .byt $00
mstatus    .byt $c0
savefreqhi .byt $00,$00,$00
savefreqlo .byt $00,$00,$00
portaval   .byt $00,$00,$00
instrfx    .byt $00
pulsespeed .byt $00
counter    .byt $00
currtrkhi  .byt $00,$00,$00
currtrklo  .byt $00,$00,$00


;====================================
;monty on the run main theme
;====================================

songs =*
 .byt <montymaintr1
 .byt <montymaintr2
 .byt <montymaintr3
 .byt >montymaintr1
 .byt >montymaintr2
 .byt >montymaintr3


;====================================
;pointers to the patterns

;low pointers
patptl =*
 .byt <ptn00
 .byt <ptn01
 .byt <ptn02
 .byt <ptn03
 .byt <ptn04
 .byt <ptn05
 .byt <ptn06
 .byt <ptn07
 .byt <ptn08
 .byt <ptn09
 .byt <ptn0a
 .byt <ptn0b
 .byt <ptn0c
 .byt <ptn0d
 .byt <ptn0e
 .byt <ptn0f
 .byt <ptn10
 .byt <ptn11
 .byt <ptn12
 .byt <ptn13
 .byt <ptn14
 .byt <ptn15
 .byt <ptn16
 .byt <ptn17
 .byt <ptn18
 .byt <ptn19
 .byt <ptn1a
 .byt <ptn1b
 .byt <ptn1c
 .byt <ptn1d
 .byt <ptn1e
 .byt <ptn1f
 .byt <ptn20
 .byt <ptn21
 .byt <ptn22
 .byt <ptn23
 .byt <ptn24
 .byt <ptn25
 .byt <ptn26
 .byt <ptn27
 .byt <ptn28
 .byt <ptn29
 .byt <ptn2a
 .byt <ptn2b
 .byt <ptn2c
 .byt <ptn2d
 .byt 0
 .byt <ptn2f
 .byt <ptn30
 .byt <ptn31
 .byt <ptn32
 .byt <ptn33
 .byt <ptn34
 .byt <ptn35
 .byt <ptn36
 .byt <ptn37
 .byt <ptn38
 .byt <ptn39
 .byt <ptn3a
 .byt <ptn3b

;high pointers
patpth =*
 .byt >ptn00
 .byt >ptn01
 .byt >ptn02
 .byt >ptn03
 .byt >ptn04
 .byt >ptn05
 .byt >ptn06
 .byt >ptn07
 .byt >ptn08
 .byt >ptn09
 .byt >ptn0a
 .byt >ptn0b
 .byt >ptn0c
 .byt >ptn0d
 .byt >ptn0e
 .byt >ptn0f
 .byt >ptn10
 .byt >ptn11
 .byt >ptn12
 .byt >ptn13
 .byt >ptn14
 .byt >ptn15
 .byt >ptn16
 .byt >ptn17
 .byt >ptn18
 .byt >ptn19
 .byt >ptn1a
 .byt >ptn1b
 .byt >ptn1c
 .byt >ptn1d
 .byt >ptn1e
 .byt >ptn1f
 .byt >ptn20
 .byt >ptn21
 .byt >ptn22
 .byt >ptn23
 .byt >ptn24
 .byt >ptn25
 .byt >ptn26
 .byt >ptn27
 .byt >ptn28
 .byt >ptn29
 .byt >ptn2a
 .byt >ptn2b
 .byt >ptn2c
 .byt >ptn2d
 .byt 0
 .byt >ptn2f
 .byt >ptn30
 .byt >ptn31
 .byt >ptn32
 .byt >ptn33
 .byt >ptn34
 .byt >ptn35
 .byt >ptn36
 .byt >ptn37
 .byt >ptn38
 .byt >ptn39
 .byt >ptn3a
 .byt >ptn3b


;====================================
;tracks
;====================================

;track1
montymaintr1 =*
 .byt $11,$14,$17,$1a,$00,$27,$00,$28
 .byt $03,$05,$00,$27,$00,$28,$03,$05
 .byt $07,$3a,$14,$17,$00,$27,$00,$28
 .byt $2f,$30,$31,$31,$32,$33,$33,$34
 .byt $34,$34,$34,$34,$34,$34,$34,$35
 .byt $35,$35,$35,$35,$35,$36,$12,$37
 .byt $38,$09,$2a,$09,$2b,$09,$0a,$09
 .byt $2a,$09,$2b,$09,$0a,$0d,$0d,$0f
 .byt $ff

;track2
montymaintr2 =*
 .byt $12,$15,$18,$1b,$2d,$39,$39
 .byt $39,$39,$39,$39,$2c,$39,$39,$39
 .byt $39,$39,$39,$2c,$39,$39,$39,$01
 .byt $01,$29,$29,$2c,$15,$18,$39,$39
 .byt $39,$39,$39,$39,$39,$39,$39,$39
 .byt $39,$39,$39,$39,$39,$39,$39,$39
 .byt $39,$39,$39,$39,$39,$39,$39,$39
 .byt $39,$39,$39,$39,$39,$01,$01,$01
 .byt $29,$39,$39,$39,$01,$01,$01,$29
 .byt $39,$39,$39,$39,$ff

;track3
montymaintr3 =*
 .byt $13,$16,$19
 .byt $1c,$02,$02,$1d,$1e,$02,$02,$1d
 .byt $1f,$04,$04,$20,$20,$06,$02,$02
 .byt $1d,$1e,$02,$02,$1d,$1f,$04,$04
 .byt $20,$20,$06,$08,$08,$08,$08,$21
 .byt $21,$21,$21,$22,$22,$22,$23,$22
 .byt $24,$25,$3b,$26,$26,$26,$26,$26
 .byt $26,$26,$26,$26,$26,$26,$26,$26
 .byt $26,$26,$26,$02,$02,$1d,$1e,$02
 .byt $02,$1d,$1f,$2f,$2f,$2f,$2f,$2f
 .byt $2f,$2f,$2f,$2f,$2f,$2f,$2f,$2f
 .byt $0b,$0b,$1d,$1d,$0b,$0b,$1d,$0b
 .byt $0b,$0b,$0c,$0c,$1d,$1d,$1d,$10
 .byt $0b,$0b,$1d,$1d,$0b,$0b,$1d,$0b
 .byt $0b,$0b,$0c,$0c,$1d,$1d,$1d,$10
 .byt $0b,$1d,$0b,$1d,$0b,$1d,$0b,$1d
 .byt $0b,$0c,$1d,$0b,$0c,$23,$0b,$0b
 .byt $ff


;====================================
;patterns
;====================================

ptn00 =*
 .byt $83,$00,$37,$01,$3e,$01,$3e,$03
 .byt $3d,$03,$3e,$03,$43,$03,$3e,$03
 .byt $3d,$03,$3e,$03,$37,$01,$3e,$01
 .byt $3e,$03,$3d,$03,$3e,$03,$43,$03
 .byt $42,$03,$43,$03,$45,$03,$46,$01
 .byt $48,$01,$46,$03,$45,$03,$43,$03
 .byt $4b,$01,$4d,$01,$4b,$03,$4a,$03
 .byt $48,$ff

ptn27 =*
 .byt $1f,$4a,$ff

ptn28 =*
 .byt $03,$46,$01,$48,$01,$46,$03,$45
 .byt $03,$4a,$0f,$43,$ff

ptn03 =*
 .byt $bf,$06
 .byt $48,$07,$48,$01,$4b,$01,$4a,$01
 .byt $4b,$01,$4a,$03,$4b,$03,$4d,$03
 .byt $4b,$03,$4a,$3f,$48,$07,$48,$01
 .byt $4b,$01,$4a,$01,$4b,$01,$4a,$03
 .byt $4b,$03,$4d,$03,$4b,$03,$48,$3f
 .byt $4c,$07,$4c,$01,$4f,$01,$4e,$01
 .byt $4f,$01,$4e,$03,$4f,$03,$51,$03
 .byt $4f,$03,$4e,$3f,$4c,$07,$4c,$01
 .byt $4f,$01,$4e,$01,$4f,$01,$4e,$03
 .byt $4f,$03,$51,$03,$4f,$03,$4c,$ff

ptn05 =*
 .byt $83,$04,$26,$03,$29,$03,$28,$03
 .byt $29,$03,$26,$03,$35,$03,$34,$03
 .byt $32,$03,$2d,$03,$30,$03,$2f,$03
 .byt $30,$03,$2d,$03,$3c,$03,$3b,$03
 .byt $39,$03,$30,$03,$33,$03,$32,$03
 .byt $33,$03,$30,$03,$3f,$03,$3e,$03
 .byt $3c,$03,$46,$03,$45,$03,$43,$03
 .byt $3a,$03,$39,$03,$37,$03,$2e,$03
 .byt $2d,$03,$26,$03,$29,$03,$28,$03
 .byt $29,$03,$26,$03,$35,$03,$34,$03
 .byt $32,$03,$2d,$03,$30,$03,$2f,$03
 .byt $30,$03,$2d,$03,$3c,$03,$3b,$03
 .byt $39,$03,$30,$03,$33,$03,$32,$03
 .byt $33,$03,$30,$03,$3f,$03,$3e,$03
 .byt $3c,$03,$34,$03,$37,$03,$36,$03
 .byt $37,$03,$34,$03,$37,$03,$3a,$03
 .byt $3d

ptn3a =*
 .byt $03,$3e,$07,$3e,$07,$3f,$07
 .byt $3e,$03,$3c,$07,$3e,$57,$ff

ptn07 =*
 .byt $8b
 .byt $00,$3a,$01,$3a,$01,$3c,$03,$3d
 .byt $03,$3f,$03,$3d,$03,$3c,$0b,$3a
 .byt $03,$39,$07,$3a,$81,$06,$4b,$01
 .byt $4d,$01,$4e,$01,$4d,$01,$4e,$01
 .byt $4d,$05,$4b,$81,$00,$3a,$01,$3c
 .byt $01,$3d,$03,$3f,$03,$3d,$03,$3c
 .byt $03,$3a,$03,$39,$1b,$3a,$0b,$3b
 .byt $01,$3b,$01,$3d,$03,$3e,$03,$40
 .byt $03,$3e,$03,$3d,$0b,$3b,$03,$3a
 .byt $07,$3b,$81,$06,$4c,$01,$4e,$01
 .byt $4f,$01,$4e,$01,$4f,$01,$4e,$05
 .byt $4c,$81,$00,$3b,$01,$3d,$01,$3e
 .byt $03,$40,$03,$3e,$03,$3d,$03,$3b
 .byt $03,$3a,$1b,$3b,$8b,$05,$35,$03
 .byt $33,$07,$32,$03,$30,$03,$2f,$0b
 .byt $30,$03,$32,$0f,$30,$0b,$35,$03
 .byt $33,$07,$32,$03,$30,$03,$2f,$1f
 .byt $30,$8b,$00,$3c,$01,$3c,$01,$3e
 .byt $03,$3f,$03,$41,$03,$3f,$03,$3e
 .byt $0b,$3d,$01,$3d,$01,$3f,$03,$40
 .byt $03,$42,$03,$40,$03,$3f,$03,$3e
 .byt $01,$3e,$01,$40,$03,$41,$03,$40
 .byt $03,$3e,$03,$3d,$03,$3e,$03,$3c
 .byt $03,$3a,$01,$3a,$01,$3c,$03,$3d
 .byt $03,$3c,$03,$3a,$03,$39,$03,$3a
 .byt $03,$3c,$ff

ptn09 =*
 .byt $83,$00,$32,$01,$35,$01,$34,$03
 .byt $32,$03,$35,$03,$34,$03,$32,$03
 .byt $35,$01,$34,$01,$32,$03,$32,$03
 .byt $3a,$03,$39,$03,$3a,$03,$32,$03
 .byt $3a,$03,$39,$03,$3a,$ff

ptn2a =*
 .byt $03,$34,$01,$37,$01,$35,$03,$34
 .byt $03,$37,$03,$35,$03,$34,$03,$37
 .byt $01,$35,$01,$34,$03,$34,$03,$3a
 .byt $03,$39,$03,$3a,$03,$34,$03,$3a
 .byt $03,$39,$03,$3a,$ff

ptn2b =*
 .byt $03,$39,$03,$38,$03,$39,$03,$3a
 .byt $03,$39,$03,$37,$03,$35,$03,$34
 .byt $03,$35,$03,$34,$03,$35,$03,$37
 .byt $03,$35,$03,$34,$03,$32,$03,$31
 .byt $ff

ptn0a =*
 .byt $03
 .byt $37,$01,$3a,$01,$39,$03,$37,$03
 .byt $3a,$03,$39,$03,$37,$03,$3a,$01
 .byt $39,$01,$37,$03,$37,$03,$3e,$03
 .byt $3d,$03,$3e,$03,$37,$03,$3e,$03
 .byt $3d,$03,$3e,$03,$3d,$01,$40,$01
 .byt $3e,$03,$3d,$03,$40,$01,$3e,$01
 .byt $3d,$03,$40,$03,$3e,$03,$40,$03
 .byt $40,$01,$43,$01,$41,$03,$40,$03
 .byt $43,$01,$41,$01,$40,$03,$43,$03
 .byt $41,$03,$43,$03,$43,$01,$46,$01
 .byt $45,$03,$43,$03,$46,$01,$45,$01
 .byt $43,$03,$46,$03,$45,$03,$43,$01
 .byt $48,$01,$49,$01,$48,$01,$46,$01
 .byt $45,$01,$46,$01,$45,$01,$43,$01
 .byt $41,$01,$43,$01,$41,$01,$40,$01
 .byt $3d,$01,$39,$01,$3b,$01,$3d,$ff

ptn0d =*
 .byt $01,$3e,$01,$39,$01,$35,$01,$39
 .byt $01,$3e,$01,$39,$01,$35,$01,$39
 .byt $03,$3e,$01,$41,$01,$40,$03,$40
 .byt $01,$3d,$01,$3e,$01,$40,$01,$3d
 .byt $01,$39,$01,$3d,$01,$40,$01,$3d
 .byt $01,$39,$01,$3d,$03,$40,$01,$43
 .byt $01,$41,$03,$41,$01,$3e,$01,$40
 .byt $01,$41,$01,$3e,$01,$39,$01,$3e
 .byt $01,$41,$01,$3e,$01,$39,$01,$3e
 .byt $03,$41,$01,$45,$01,$43,$03,$43
 .byt $01,$40,$01,$41,$01,$43,$01,$40
 .byt $01,$3d,$01,$40,$01,$43,$01,$40
 .byt $01,$3d,$01,$40,$01,$46,$01,$43
 .byt $01,$45,$01,$46,$01,$44,$01,$43
 .byt $01,$40,$01,$3d,$ff

ptn0f =*
 .byt $01,$3e,$01
 .byt $39,$01,$35,$01,$39,$01,$3e,$01
 .byt $39,$01,$35,$01,$39,$01,$3e,$01
 .byt $39,$01,$35,$01,$39,$01,$3e,$01
 .byt $39,$01,$35,$01,$39,$01,$3e,$01
 .byt $3a,$01,$37,$01,$3a,$01,$3e,$01
 .byt $3a,$01,$37,$01,$3a,$01,$3e,$01
 .byt $3a,$01,$37,$01,$3a,$01,$3e,$01
 .byt $3a,$01,$37,$01,$3a,$01,$40,$01
 .byt $3d,$01,$39,$01,$3d,$01,$40,$01
 .byt $3d,$01,$39,$01,$3d,$01,$40,$01
 .byt $3d,$01,$39,$01,$3d,$01,$40,$01
 .byt $3d,$01,$39,$01,$3d,$01,$41,$01
 .byt $3e,$01,$39,$01,$3e,$01,$41,$01
 .byt $3e,$01,$39,$01,$3e,$01,$41,$01
 .byt $3e,$01,$39,$01,$3e,$01,$41,$01
 .byt $3e,$01,$39,$01,$3e,$01,$43,$01
 .byt $3e,$01,$3a,$01,$3e,$01,$43,$01
 .byt $3e,$01,$3a,$01,$3e,$01,$43,$01
 .byt $3e,$01,$3a,$01,$3e,$01,$43,$01
 .byt $3e,$01,$3a,$01,$3e,$01,$43,$01
 .byt $3f,$01,$3c,$01,$3f,$01,$43,$01
 .byt $3f,$01,$3c,$01,$3f,$01,$43,$01
 .byt $3f,$01,$3c,$01,$3f,$01,$43,$01
 .byt $3f,$01,$3c,$01,$3f,$01,$45,$01
 .byt $42,$01,$3c,$01,$42,$01,$45,$01
 .byt $42,$01,$3c,$01,$42,$01,$48,$01
 .byt $45,$01,$42,$01,$45,$01,$4b,$01
 .byt $48,$01,$45,$01,$48,$01,$4b,$01
 .byt $4a,$01,$48,$01,$4a,$01,$4b,$01
 .byt $4a,$01,$48,$01,$4a,$01,$4b,$01
 .byt $4a,$01,$48,$01,$4a,$01,$4c,$01
 .byt $4e,$03,$4f,$ff

ptn11 =*
 .byt $bf,$06,$56,$1f,$57,$1f,$56,$1f
 .byt $5b,$1f,$56,$1f,$57,$1f,$56,$1f
 .byt $4f,$ff

ptn12 =*
 .byt $bf,$0c,$68,$7f,$7f,$7f,$7f,$7f
 .byt $7f,$7f,$ff

ptn13 =*
 .byt $bf,$08,$13,$3f,$13,$3f,$13,$3f
 .byt $13,$3f,$13,$3f,$13,$3f,$13,$1f
 .byt $13,$ff

ptn14 =*
 .byt $97,$09,$2e,$03,$2e,$1b,$32,$03
 .byt $32,$1b,$31,$03,$31,$1f,$34,$43
 .byt $17,$32,$03,$32,$1b,$35,$03,$35
 .byt $1b,$34,$03,$34,$0f,$37,$8f,$0a
 .byt $37,$43,$ff

ptn15 =*
 .byt $97,$09,$2b,$03,$2b,$1b,$2e,$03
 .byt $2e,$1b,$2d,$03,$2d,$1f,$30,$43
 .byt $17,$2e,$03,$2e,$1b,$32,$03,$32
 .byt $1b,$31,$03,$31,$0f,$34,$8f,$0a
 .byt $34,$43,$ff

ptn16 =*
 .byt $0f,$1f,$0f,$1f,$0f,$1f,$0f,$1f
 .byt $0f,$1f,$0f,$1f,$0f,$1f,$0f,$1f
 .byt $0f,$1f,$0f,$1f,$0f,$1f,$0f,$1f
 .byt $0f,$1f,$0f,$1f,$0f,$1f,$0f,$1f
 .byt $ff

ptn17 =*
 .byt $97,$09,$33,$03,$33,$1b,$37,$03
 .byt $37,$1b,$36,$03,$36,$1f,$39,$43
 .byt $17,$37,$03,$37,$1b,$3a,$03,$3a
 .byt $1b,$39,$03,$39,$2f,$3c,$21,$3c
 .byt $21,$3d,$21,$3e,$21,$3f,$21,$40
 .byt $21,$41,$21,$42,$21,$43,$21,$44
 .byt $01,$45,$ff

ptn18 =*
 .byt $97,$09,$30,$03,$30,$1b,$33,$03
 .byt $33,$1b,$32,$03,$32,$1f,$36,$43
 .byt $17,$33,$03,$33,$1b,$37,$03,$37
 .byt $1b,$36,$03,$36,$2f,$39,$21,$39
 .byt $21,$3a,$21,$3b,$21,$3c,$21,$3d
 .byt $21,$3e,$21,$3f,$21,$40,$21,$41
 .byt $01,$42,$ff

ptn19 =*
 .byt $0f,$1a,$0f,$1a,$0f,$1a,$0f,$1a
 .byt $0f,$1a,$0f,$1a,$0f,$1a,$0f,$1a
 .byt $0f,$1a,$0f,$1a,$0f,$1a,$0f,$1a
 .byt $0f,$1a,$0f,$1a,$0f,$1a,$0f,$1a
 .byt $ff

ptn1a =*
 .byt $1f,$46,$bf,$0a,$46,$7f,$7f,$ff

ptn1b =*
 .byt $1f,$43,$bf,$0a,$43,$7f,$ff

ptn1c =*
 .byt $83,$02,$13,$03,$13,$03,$1e,$03
 .byt $1f,$03,$13,$03,$13,$03,$1e,$03
 .byt $1f,$03,$13,$03,$13,$03,$1e,$03
 .byt $1f,$03,$13,$03,$13,$03,$1e,$03
 .byt $1f,$03,$13,$03,$13,$03,$1e,$03
 .byt $1f,$03,$13,$03,$13,$03,$1e,$03
 .byt $1f,$03,$13,$03,$13,$03,$1e,$03
 .byt $1f,$03,$13,$03,$13,$03,$1e,$03
 .byt $1f,$ff

ptn29 =*
 .byt $8f,$0b,$38,$4f,$ff

ptn2c =*
 .byt $83,$0e,$32,$07,$32,$07,$2f,$07
 .byt $2f,$03,$2b,$87,$0b,$46,$83,$0e
 .byt $2c,$03,$2c,$8f,$0b,$32,$ff

ptn2d =*
 .byt $43,$83,$0e,$32,$03,$32,$03,$2f
 .byt $03,$2f,$03,$2c,$87,$0b,$38,$ff

ptn39 =*
 .byt $83,$01
 .byt $43,$01,$4f,$01,$5b,$87,$03,$2f
 .byt $83,$01,$43,$01,$4f,$01,$5b,$87
 .byt $03,$2f,$83,$01,$43,$01,$4f,$01
 .byt $5b,$87,$03,$2f,$83,$01,$43,$01
 .byt $4f,$01,$5b,$87,$03,$2f,$83,$01
 .byt $43,$01,$4f,$01,$5b,$87,$03,$2f
 .byt $83,$01,$43,$01,$4f,$01,$5b,$87
 .byt $03,$2f

ptn01 =*
 .byt $83,$01,$43,$01,$4f,$01,$5b,$87
 .byt $03,$2f,$83,$01,$43,$01,$4f,$01
 .byt $5b,$87,$03,$2f,$ff

ptn02 =*
 .byt $83,$02,$13,$03,$13,$03,$1f,$03
 .byt $1f,$03,$13,$03,$13,$03,$1f,$03
 .byt $1f,$ff

ptn1d =*
 .byt $03,$15,$03,$15,$03,$1f,$03,$21
 .byt $03,$15,$03,$15,$03,$1f,$03,$21
 .byt $ff

ptn1e =*
 .byt $03,$1a,$03,$1a,$03,$1c,$03,$1c
 .byt $03,$1d,$03,$1d,$03,$1e,$03,$1e
 .byt $ff

ptn1f =*
 .byt $03,$1a,$03,$1a,$03,$24,$03,$26
 .byt $03,$13,$03,$13,$07,$1f,$ff

ptn04 =*
 .byt $03,$18,$03,$18,$03,$24,$03,$24
 .byt $03,$18,$03,$18,$03,$24,$03,$24
 .byt $03,$20,$03,$20,$03,$2c,$03,$2c
 .byt $03,$20,$03,$20,$03,$2c,$03,$2c
 .byt $ff

ptn20 =*
 .byt $03,$19,$03,$19,$03
 .byt $25,$03,$25,$03,$19,$03,$19,$03
 .byt $25,$03,$25,$03,$21,$03,$21,$03
 .byt $2d,$03,$2d,$03,$21,$03,$21,$03
 .byt $2d,$03,$2d,$ff

ptn06 =*
 .byt $03,$1a,$03,$1a
 .byt $03,$26,$03,$26,$03,$1a,$03,$1a
 .byt $03,$26,$03,$26,$03,$15,$03,$15
 .byt $03,$21,$03,$21,$03,$15,$03,$15
 .byt $03,$21,$03,$21,$03,$18,$03,$18
 .byt $03,$24,$03,$24,$03,$18,$03,$18
 .byt $03,$24,$03,$24,$03,$1f,$03,$1f
 .byt $03,$2b,$03,$2b,$03,$1f,$03,$1f
 .byt $03,$2b,$03,$2b,$03,$1a,$03,$1a
 .byt $03,$26,$03,$26,$03,$1a,$03,$1a
 .byt $03,$26,$03,$26,$03,$15,$03,$15
 .byt $03,$21,$03,$21,$03,$15,$03,$15
 .byt $03,$21,$03,$21,$03,$18,$03,$18
 .byt $03,$24,$03,$24,$03,$18,$03,$18
 .byt $03,$24,$03,$24,$03,$1c,$03,$1c
 .byt $03,$28,$03,$28,$03,$1c,$03,$1c
 .byt $03,$28,$03,$28

ptn3b =*
 .byt $83,$04,$36,$07
 .byt $36,$07,$37,$07,$36,$03,$33,$07
 .byt $32,$57,$ff

ptn08 =*
 .byt $83,$02,$1b,$03,$1b,$03,$27,$03
 .byt $27,$03,$1b,$03,$1b,$03,$27,$03
 .byt $27,$ff

ptn21 =*
 .byt $03,$1c,$03,$1c,$03,$28,$03,$28
 .byt $03,$1c,$03,$1c,$03,$28,$03,$28
 .byt $ff

ptn22 =*
 .byt $03,$1d,$03,$1d,$03,$29,$03,$29
 .byt $03,$1d,$03,$1d,$03,$29,$03,$29
 .byt $ff

ptn23 =*
 .byt $03,$18,$03,$18,$03,$24,$03,$24
 .byt $03,$18,$03,$18,$03,$24,$03,$24
 .byt $ff

ptn24 =*
 .byt $03,$1e,$03,$1e,$03,$2a,$03,$2a
 .byt $03,$1e,$03,$1e,$03,$2a,$03,$2a
 .byt $ff

ptn25 =*
 .byt $83,$05,$26,$01,$4a,$01,$34,$03
 .byt $29,$03,$4c,$03,$4a,$03,$31,$03
 .byt $4a,$03,$24,$03,$22,$01,$46,$01
 .byt $30,$03,$25,$03,$48,$03,$46,$03
 .byt $2d,$03,$46,$03,$24,$ff

ptn0b =*
 .byt $83,$02,$1a,$03,$1a,$03,$26,$03
 .byt $26,$03,$1a,$03,$1a,$03,$26,$03
 .byt $26,$ff

ptn0c =*
 .byt $03,$13,$03,$13,$03,$1d,$03,$1f
 .byt $03,$13,$03,$13,$03,$1d,$03,$1f
 .byt $ff

ptn26 =*
 .byt $87,$02,$1a,$87,$03,$2f,$83,$02
 .byt $26,$03,$26,$87,$03,$2f,$ff

ptn10 =*
 .byt $07,$1a,$4f,$47,$ff

ptn0e =*
 .byt $03,$1f,$03,$1f,$03,$24,$03,$26
 .byt $07,$13,$47,$ff

ptn30 =*
 .byt $bf,$0f,$32,$0f,$32,$8f,$90,$30
 .byt $3f,$32,$13,$32,$03,$32,$03,$35
 .byt $03,$37,$3f,$37,$0f,$37,$8f,$90
 .byt $30,$3f,$32,$13,$32,$03,$2d,$03
 .byt $30,$03,$32,$ff

ptn31 =*
 .byt $0f,$32
 .byt $af,$90,$35,$0f,$37,$a7,$99,$37
 .byt $07,$35,$3f,$32,$13,$32,$03,$32
 .byt $a3,$e8,$35,$03,$37,$0f,$35,$af
 .byt $90,$37,$0f,$37,$a7,$99,$37,$07
 .byt $35,$3f,$32,$13,$32,$03,$2d,$a3
 .byt $e8,$30,$03,$32,$ff

ptn32 =*
 .byt $07,$32,$03
 .byt $39,$13,$3c,$a7,$9a,$37,$a7,$9b
 .byt $38,$07,$37,$03,$35,$03,$32,$03
 .byt $39,$1b,$3c,$a7,$9a,$37,$a7,$9b
 .byt $38,$07,$37,$03,$35,$03,$32,$03
 .byt $39,$03,$3c,$03,$3e,$03,$3c,$07
 .byt $3e,$03,$3c,$03,$39,$a7,$9a,$37
 .byt $a7,$9b,$38,$07,$37,$03,$35,$03
 .byt $32,$af,$90,$3c,$1f,$3e,$43,$03
 .byt $3e,$03,$3c,$03,$3e,$ff

ptn33 =*
 .byt $03,$3e
 .byt $03,$3e,$a3,$e8,$3c,$03,$3e,$03
 .byt $3e,$03,$3e,$a3,$e8,$3c,$03,$3e
 .byt $03,$3e,$03,$3e,$a3,$e8,$3c,$03
 .byt $3e,$03,$3e,$03,$3e,$a3,$e8,$3c
 .byt $03,$3e,$af,$91,$43,$1f,$41,$43
 .byt $03,$3e,$03,$41,$03,$43,$03,$43
 .byt $03,$43,$a3,$e8,$41,$03,$43,$03
 .byt $43,$03,$43,$a3,$e8,$41,$03,$43
 .byt $03,$45,$03,$48,$a3,$fd,$45,$03
 .byt $44,$01,$43,$01,$41,$03,$3e,$03
 .byt $3c,$03,$3e,$2f,$3e,$bf,$98,$3e
 .byt $43,$03,$3e,$03,$3c,$03,$3e,$ff

ptn34 =*
 .byt $03,$4a,$03,$4a,$a3,$f8,$48,$03
 .byt $4a,$03,$4a,$03,$4a,$a3,$f8,$48
 .byt $03,$4a,$ff

ptn35 =*
 .byt $01,$51,$01,$54,$01
 .byt $51,$01,$54,$01,$51,$01,$54,$01
 .byt $51,$01,$54,$01,$51,$01,$54,$01
 .byt $51,$01,$54,$01,$51,$01,$54,$01
 .byt $51,$01,$54,$ff

ptn36 =*
 .byt $01,$50,$01,$4f
 .byt $01,$4d,$01,$4a,$01,$4f,$01,$4d
 .byt $01,$4a,$01,$48,$01,$4a,$01,$48
 .byt $01,$45,$01,$43,$01,$44,$01,$43
 .byt $01,$41,$01,$3e,$01,$43,$01,$41
 .byt $01,$3e,$01,$3c,$01,$3e,$01,$3c
 .byt $01,$39,$01,$37,$01,$38,$01,$37
 .byt $01,$35,$01,$32,$01,$37,$01,$35
 .byt $01,$32,$01,$30,$ff

ptn37 =*
 .byt $5f,$5f,$5f
 .byt $47,$83,$0e,$32,$07,$32,$07,$2f
 .byt $03,$2f,$07,$2f,$97,$0b,$3a,$5f
 .byt $5f,$47,$8b,$0e,$32,$03,$32,$03
 .byt $2f,$03,$2f,$47,$97,$0b,$3a,$5f
 .byt $5f,$47,$83,$0e,$2f,$0b,$2f,$03
 .byt $2f,$03,$2f,$87,$0b,$30,$17,$3a
 .byt $5f,$8b,$0e,$32,$0b,$32,$0b,$2f
 .byt $0b,$2f,$07,$2c,$07,$2c,$ff

ptn38 =*
 .byt $87
 .byt $0b,$34,$17,$3a,$5f,$5f,$84,$0e
 .byt $32,$04,$32,$05,$32,$04,$2f,$04
 .byt $2f,$05,$2f,$47,$97,$0b,$3a,$5f
 .byt $5f,$84,$0e,$32,$04,$32,$05,$32
 .byt $04,$2f,$04,$2f,$05,$2f,$ff

ptn2f =*
 .byt $03,$1a,$03,$1a,$03
 .byt $24,$03,$26,$03,$1a,$03,$1a,$03
 .byt $18,$03,$19,$03,$1a,$03,$1a,$03
 .byt $24,$03,$26,$03,$1a,$03,$1a,$03
 .byt $18,$03,$19,$03,$18,$03,$18,$03
 .byt $22,$03,$24,$03,$18,$03,$18,$03
 .byt $16,$03,$17,$03,$18,$03,$18,$03
 .byt $22,$03,$24,$03,$18,$03,$18,$03
 .byt $16,$03,$17,$03,$13,$03,$13,$03
 .byt $1d,$03,$1f,$03,$13,$03,$13,$03
 .byt $1d,$03,$1e,$03,$13,$03,$13,$03
 .byt $1d,$03,$1f,$03,$13,$03,$13,$03
 .byt $1d,$03,$1e,$03,$1a,$03,$1a,$03
 .byt $24,$03,$26,$03,$1a,$03,$1a,$03
 .byt $18,$03,$19,$03,$1a,$03,$1a,$03
 .byt $24,$03,$26,$03,$1a,$03,$1a,$03
 .byt $18,$03,$19,$ff


;====================================
;instruments
;====================================

instr =*
 .byt $80,$09,$41,$48,$60,$03,$81,$00
 .byt $00,$08,$81,$02,$08,$00,$00,$01
 .byt $a0,$02,$41,$09,$80,$00,$00,$00
 .byt $00,$02,$81,$09,$09,$00,$00,$05
 .byt $00,$08,$41,$08,$50,$02,$00,$04
 .byt $00,$01,$41,$3f,$c0,$02,$00,$00
 .byt $00,$08,$41,$04,$40,$02,$00,$00
 .byt $00,$08,$41,$09,$00,$02,$00,$00
 .byt $00,$09,$41,$09,$70,$02,$5f,$04
 .byt $00,$09,$41,$4a,$69,$02,$81,$00
 .byt $00,$09,$41,$40,$6f,$00,$81,$02
 .byt $80,$07,$81,$0a,$0a,$00,$00,$01
 .byt $00,$09,$41,$3f,$ff,$01,$e7,$02
 .byt $00,$08,$41,$90,$f0,$01,$e8,$02
 .byt $00,$08,$41,$06,$0a,$00,$00,$01
 .byt $00,$09,$41,$19,$70,$02,$a8,$00
 .byt $00,$02,$41,$09,$90,$02,$00,$00
 .byt $00,$00,$11,$0a,$fa,$00,$00,$05
 .byt $00,$08,$41,$37,$40,$02,$00,$00
 .byt $00,$08,$11,$07,$70,$02,$00,$00

.end
