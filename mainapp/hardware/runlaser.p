#include "lasergalvo.hp"

.origin 0
.entrypoint SETUP

;
; Shared memory:
;
;  Number of packete
;  packets (0..8kb)
;    Pattern [24 bits]
;    Repeat  [ 8 bits]
;    X
;    Y
;
;  1) Disable laser to start
;  2) Setup DAC for the laser
;  3) Wait for interrupt from host
;  4) Execute packets
;  5) Send interrupt when done
;  7) Goto 3 if not done (last piece of data indicates done [1s]  or not [0])
;  8) halt

SETUP:

      enableInterrupt
      
      mov rResetInterrupt, ARM_PRU0_INTERRUPT

      disableLaser
      ;
      ; LDAC ___________
      ; SYNC -----------
      ; SCLK ..__--__...
      ;  sclk must be    50mhz;  4 commands does that include the set/clr?; need to hook up a scope to verify
      clr r30, GALVO_LDAC
      ; SYNC should be pulled high (will pull low later; active low signal)
      set r30, GALVO_SYNC
      ;run the clk a few ticks
      clr r30, GALVO_SCLK
      nop
      nop
      nop
      nop
      set r30, GALVO_SCLK
      nop
      nop
      nop
      nop
      clr r30, GALVO_SCLK

      ; Enable internal reference and reset DAC to gain=2
      mov rScratchPad, 0x380001
      dacSPI rScratchPad

NEWBUFFER:
      mov rMemoryPointer, 0
      // clear register for usage on SPI to LDAC
      mov rDataX.b3, 0x0
      mov rDataY.b3, 0x0
      mov rDataX.b2, 0x0  ; Write to DAC-A input register
      mov rDataY.b2, 0x11 ; Write to DAC-B input register and update all DACs

BEGINDATABUFFER:

      waitForHostInterrupt
      
      ; header with how many packets we have.
      getMemoryBuffer rNumberOfPackets, 4
		
GETNEWPACKET:	

      ; get pattern and repeat count
      getMemoryBuffer rLaserData, 8

      mov rRepeatPattern, 0
      mov rRepeatPattern.b0, rLaserData.b3

PROCESSPATTERN:
      ; load lower DB15-0; maintain above C2-C0:A2:A0
      mov rDataX.w0, rGalvoData.w0
      mov rDataY.w0, rGalvoData.w2

      dacSPI rDataX
      dacSPI rDataY

RUNLASER:      
      runLaserM

      sub1qbne rRepeatPattern, RUNLASER, 0

      sub1qbne rNumberOfPackets, GETNEWPACKET, 0

      ; tell host we are done handling memory
      sendHostInterrupt

      mov rLastMePtr, rMemoryPointer ; for debug
      ; see if there is more data.
      getMemoryBuffer rMoreData, 4

      qbeq NEWBUFFER, rMoreData, 0

DONE:
     disableLaser
     HALT
