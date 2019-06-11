#include "zaxis.hp"

.origin 0
.entrypoint SETUP

SETUP:
      //enableInterrupt
      
      enableInterrupt
      
      mov rResetInterrupt, ARM_PRU0_INTERRUPT
      waitForHostInterrupt
READY_PINS:
      // todo wait for bbb
      // TODO:clear step pin (GPIO2[5]).
      mov rStepAccel, 0
      mov rStepDecel, 0

      // <const> <Acc><DeAcc> <direction> <accel table...>  .... <accel table>
      lbco rStepConst,    CONST_PRUSHAREDRAM, 0, 4
      lbco rScratch,      CONST_PRUSHAREDRAM, 4, 4
      mov  rStepAccel.w0, rScratch.w0
      mov  rStepDecel.w0, rScratch.w2
      lbco rDirection.b0, CONST_PRUSHAREDRAM, 8, 1
      
      mov rStepCnt, 9

      qbne CLEARDIRECTION, rDirection, 1
SETDIRECTION:
      // TODO:set dir pin
     JMP prepLoop
	     
CLEARDIRECTION:
     // TODO:clear dir pin
	       
prepLoop:
	 mov rStepDelay, 0x400
waitLoop:
	 sub1qbne rStepDelay, waitLoop, 0

accelSteps:
	 qbeq constSteps, rStepAccel, 0
	 mStepZ

	 add rStepCnt, rStepCnt, 4
	 lbco rStepDelay, CONST_PRUSHAREDRAM, rStepCnt, 1
accelDelay:
	 sub1qbne rStepDelay, accelDelay, 0

	 sub1qbne rStepAccel, accelSteps, 0

	 
constSteps:
	 qbeq decelSteps, rStepConst, 0
	 mStepZ

	 add rStepCnt, rStepCnt, 4
	 lbco rStepDelay, CONST_PRUSHAREDRAM, rStepCnt, 1
constDelay:
	 sub1qbne rStepDelay, constDelay, 0
         mStepZ
	   
	 sub1qbne rStepConst, constSteps, 0

	 
decelSteps:
	 qbeq doneSteps, rStepDecel, 0
	 mStepZ

	 sub rStepCnt, rStepCnt, 4
	 lbco rStepDelay, CONST_PRUSHAREDRAM, rStepCnt, 1
decelDelay:
	 sub1qbne rStepDelay, decelDelay, 0

	 sub1qbne rStepDecel, decelSteps, 0

	 jmp doneSteps
	   
ZLIMIT_HIT:
	   
doneSteps:
     sendHostInterrupt
     HALT
