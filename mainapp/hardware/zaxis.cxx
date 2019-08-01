
/*
  Copyright (c) 2016 Doobie (doobie.org)
  
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// g++ -o zaxis zaxis.c gpio/GPIOManager.cpp gpio/GPIOConst.cpp -D STANDALONE

#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include "hardware/zaxis.hxx"
#include "general/pegasus.hxx"


  /*
   *        _____________
   *       /             \
   *      /               \
   *     /                 \
   *    /                   \
   *   /                     \
   *  /                       \
   * /                         \
   *
   *  3 phases of stepper motor driving:
   * acceleration
   * constant
   * deceleration
   *
   * depending the number of steps, we may need to use all 3 or just accel/decel.
   *  
   *  This really should be moved to the PRU at some point
   *
   *  We will generate a rough estimate delay step for now, but should move it int something more complex utilizing a linear speed ramp formula
   *
   * stepper control delays based on homepage.cs.uiowa.edu/~jones/step/index.html
   *    
   *
   */                           


zaxis::zaxis(pegasus *a_peg) :
  peg(a_peg) {
      printf("====== IN zaxis.cxx ======");
  zP = 0;
  inZCalibration =0;
  calibZTop = 0;
  calibZBot = 0;
  gp      = GPIO::GPIOManager::getInstance();
  Z_LIMIT = GPIO::GPIOConst::getInstance()->getGpioByKey("P8_16");
  LID     = GPIO::GPIOConst::getInstance()->getGpioByKey("P8_17");
  ABEN    = GPIO::GPIOConst::getInstance()->getGpioByKey("P8_18");
  ABEN2   = GPIO::GPIOConst::getInstance()->getGpioByKey("P8_39");
  ABSTEP  = GPIO::GPIOConst::getInstance()->getGpioByKey("P8_9");
  ABDIR   = GPIO::GPIOConst::getInstance()->getGpioByKey("P8_10");
  CUR     = GPIO::GPIOConst::getInstance()->getGpioByKey("P8_12");

  gp->exportPin(Z_LIMIT );
  gp->exportPin(LID     );
  gp->exportPin(ABEN    );
  gp->exportPin(ABEN2   );
  gp->exportPin(ABSTEP  );
  gp->exportPin(ABDIR   );
  gp->exportPin(CUR     );

  gp->setDirection(Z_LIMIT, GPIO::INPUT);
  gp->setDirection(LID,     GPIO::INPUT);
  gp->setDirection(ABEN,    GPIO::OUTPUT);
  gp->setDirection(ABEN2,   GPIO::OUTPUT);
  gp->setDirection(ABSTEP,  GPIO::OUTPUT);
  gp->setDirection(ABDIR,   GPIO::OUTPUT);
  gp->setDirection(CUR,     GPIO::OUTPUT);
  gp->setValue(ABEN,   GPIO::LOW);
  gp->setValue(ABEN2,  GPIO::LOW);
  gp->setValue(ABSTEP, GPIO::LOW);
  gp->setValue(ABDIR,  GPIO::LOW);
  gp->setValue(CUR,    GPIO::LOW);
  //initStep();
  highestZPrinted = -100000;
}

zaxis::~zaxis() {
}

void zaxis::initStep() {
  int i;
  AccelDecelDelay[0] = peg->calibData.getInitialACD()*10;
  //printf("ACD[%d] = %d\n", 0, AccelDecelDelay[0]);
  for (i = 1; i < MAX_ACCEL_DECEL_STEPS; i++) {
    /***************************************************
     * Copyright (C) 1995, Douglas W. Jones; major revision:1998.  This work may be transmitted or stored in electronic form on any computer attached to the Internet or World Wide Web so ong as this notice is included in the copy.  Individuals may make single copies for heir own use.  All other rights are reserved.
     *  a = motor angle
     *  c0 = sqrt(2*a / accel)
     *  cn = cn-1 * (sqrt(n+1) - sqrt(n)
     ***************************************************/
    AccelDecelDelay[i] = AccelDecelDelay[i-1] - (int)((float)(2*AccelDecelDelay[i-1]) / (peg->calibData.getAccConst()*((float)i)+1));
    //note this won't work since this is inited before calib.
    if (peg->calibData.DEBUG_LEVEL & DEBUG_ZAXIS) printf("AccelDecelDelay[%d]=%d dx=%d/%f = %d\n", i, AccelDecelDelay[i], 2*AccelDecelDelay[i-1], (peg->calibData.getAccConst()*(i)+1),
							    (int)((float)(2*AccelDecelDelay[i-1]) / (peg->calibData.getAccConst()*((float)i)+1)));
  }
  for (i = 1; i < MAX_ACCEL_DECEL_STEPS; i++) {
    if (1|| i < 10) {
      //printf("ACD[%d] = %d",i, AccelDecelDelay[i]);
      AccelDecelDelay[i] /= 10;
      //printf(" => %d\n", AccelDecelDelay[i]);
    }
  }
  //exit(0);
  AccelerationTime = MAX_ACCEL_DECEL_STEPS;
}

void zaxis::enableZ(int en) {
  gp->setValue(ABEN2, en ? GPIO::HIGH : GPIO::LOW);
}

void zaxis::initZAxis() {
}

int zaxis::isZLimit() {
  if ((peg->calibData.DEBUG_LEVEL & DEBUG_NOZAXIS)) {
    return 1;
  } else {
    return (gp->getValue(Z_LIMIT));
  }
}

int zaxis::isLidOpen() {
  if ((peg->calibData.DEBUG_LEVEL & DEBUG_NOZAXIS)) {
    return 0;
  } else {
    return (gp->getValue(LID));
  }
}

void zaxis::moveZAbs(int z, int ovr) {
  if (peg->calibData.DEBUG_LEVEL & DEBUG_ZAXIS) printf("I'm at %d, moving to %d, by moveZ=%d\n", zP, z, z-zP);
  moveZ(z-zP, ovr, 1, 1);
}


int zaxis::getZ() {
  return zP;
}

void zaxis::liftMoveZ(int newPoint) {
  printf("Starting at: %d move to %d\n", zP, newPoint);
  // first lift UP at slow speed; then fast
  if ((peg->calibData.DEBUG_LEVEL & DEBUG_NOZAXIS)) {
  } else {
    int extraLift = 0;
    if (peg->calibData.getLiftSteps() > 1) {
      for (int i = 0; i < peg->calibData.getLiftSteps(); i++) {
	usleep(peg->calibData.getLiftPause() / peg->calibData.getLiftSteps());
	moveZ(peg->calibData.getLiftDistance()/peg->calibData.getLiftSteps(), 0, 1, 1); 
      }
    } else {
      usleep(peg->calibData.getLiftPause());
      moveZ(peg->calibData.getLiftDistance(), 0, 1, 1);
    }
    usleep(peg->calibData.getLowerPause());
    while (isLidOpen()) {
      if (extraLift == 0) {
	moveZ(8000, 0, 1, 1);      
	extraLift = 1;
      }
    }
    if (extraLift) {
      moveZ(-8000, 0, 1, 1);
    }
    moveZAbs(newPoint, 1);
    usleep(peg->calibData.getPrintPause());
  }
}

int zaxis::moveZ(int step, int ovr, int cnt, float speedDiv) {
  int speedStep, down;

  int stepCntPerAccel_Const_Decel[3] = {0,0,0};
  int absStep = abs(step);
  int delayTime;

  if (step < 0) {
    gp->setValue(ABDIR,GPIO::HIGH);
    down = 1;
    if (zP - step < highestZPrinted) {
      printf ("ERROR!  Cannot move %d because we are at %d and we already printed at %d; check your gcode!\n", step, zP, highestZPrinted);
      return 0;
    }
  } else {
    gp->setValue(ABDIR,GPIO::LOW);
    down = 0;
  }
  //usleep(100);
  if (peg->calibData.DEBUG_LEVEL & DEBUG_ZAXIS) printf("MoveZ: %d\n", step);

  if (absStep/2 > AccelerationTime) {
    // We have enough steps to acceleration, constant rate, and deceleration
    stepCntPerAccel_Const_Decel[0] = AccelerationTime;
    stepCntPerAccel_Const_Decel[2] = AccelerationTime;
    stepCntPerAccel_Const_Decel[1] = absStep - AccelerationTime - AccelerationTime;
  } else if (absStep%2== 0){
    // not enough for ACD; but have even number of steps
    stepCntPerAccel_Const_Decel[0] = absStep/2;
    stepCntPerAccel_Const_Decel[2] = absStep/2;
    stepCntPerAccel_Const_Decel[1] = 0;
  } else {
    // not enough for ACD; but have odd number of steps, need to add 1 to either accel or decel, does it matter?  maybe const?
    stepCntPerAccel_Const_Decel[0] = absStep/2;
    stepCntPerAccel_Const_Decel[2] = absStep/2;
    stepCntPerAccel_Const_Decel[1] = 1;
  }
  for (int speed = 0; speed < 3; speed++) {
    for (speedStep = 0; speedStep < stepCntPerAccel_Const_Decel[speed]; speedStep++) {
      if (down) {
	// need to check limit
	if (ovr == 0 && isZLimit()) {
	  if (peg->calibData.DEBUG_LEVEL & DEBUG_ZAXIS) printf( "**** zlimit hit at %d\n", speedStep);
	  return 1;
	}
	if (zP == calibZBot && calibZBot != 0 && !isZLimit()) {
	  return 1; // don't let it go below calibZBot once detected and if limit sensor is open
	}
	if (cnt) {
	  zP --;
	}
      } else {
	if (cnt) {
	  zP++;
	}
      }
      if ((peg->calibData.DEBUG_LEVEL & DEBUG_NOZAXIS)) {
	//usleep(2*delay); // fake delays so logs doesn't stream so fast
      } else {
	if (speed == 0) {
	  // Acceeration
	  delayTime = AccelDecelDelay[speedStep];
	} else if (speed == 1) {
	  // Constant speed
	  delayTime = AccelDecelDelay[AccelerationTime - 1];  // subtract 1 because it stores the number of steps; not the entry.
	} else if (speed == 2) { 
	  // Deceleration: this is done in opposite order
	  delayTime = AccelDecelDelay[AccelerationTime - speedStep];
	}
	gp->setValue(ABSTEP, GPIO::HIGH);
	if (speedDiv != 1) { // avoid the * if it is 1
	  usleep(delayTime * speedDiv);
	} else {
	  usleep(delayTime);
	}
	gp->setValue(ABSTEP, GPIO::LOW);
	if (speedDiv != 1) { // avoid the * if it is 1
	  usleep(delayTime * speedDiv);
	} else {
	  usleep(delayTime);
	}
      }
    }
  }
  return 0;
}

int zTune[] = {(int)(MAX_ACCEL_DECEL_STEPS / 10),
	       (int)(MAX_ACCEL_DECEL_STEPS / 7),
	       (int)(MAX_ACCEL_DECEL_STEPS / 5),
	       (int)(MAX_ACCEL_DECEL_STEPS / 2),
	       (int)(MAX_ACCEL_DECEL_STEPS),
	       (int)(MAX_ACCEL_DECEL_STEPS + 1),
	       (int)(MAX_ACCEL_DECEL_STEPS * 1.25),
	       (int)(MAX_ACCEL_DECEL_STEPS * 1.5),
	       (int)(MAX_ACCEL_DECEL_STEPS * 2)};
	  
void zaxis::tuneZ() {
  printf("Seeking 'home'\n");
  moveZHome();
  printf("Found home, measure position and press Any Key to Continue\n");  
  for (unsigned int z = 0 ; z < sizeof(zTune)/sizeof(int); z++) {
    getchar();
    printf("Attempt %d, distance=%d ticks or %f mm (accelpts=%d)\n", z, zTune[z], (float)zTune[z]/peg->calibData.getScale(2,1), MAX_ACCEL_DECEL_STEPS);
    for (int i = 0; i < 10; i++) {
      moveZAbs(zTune[z], 1);
      usleep(peg->calibData.getLiftPause());
      moveZAbs(0, 1);
      if (i % 10 == 0) {
	printf ("\r%3d of 25.", i);
      }
      usleep(peg->calibData.getLiftPause());
    }
    printf("\nMeasure the home position now; press any key to run again.\n");
  }
}

void zaxis::zCalib() {
  printf("Seeking 'home'\n");
  moveZHome();
  char input = 0;
  int position = 0;
  int speed = 16;
  printf("Position: %d speed=%d> ", position, speed);
  while (input != 'q') {
    input = getchar();
    switch (input) {
    case 'U':
    case 'u':
      position += speed;
      moveZ(speed, 1, 1, 1);
      break;
    case 'D':
    case 'd':
      position -= speed;
      moveZ(-speed, 1, 1, 1);
      break;
    case 'H':
    case 'h':
      moveZ(2000, 0, 0, 1);
      moveZHome();
      position = 0;
      break;
    case 'R':
    case 'r':
      moveZ(2000, 0, 0, 1);
      moveZHome();
      moveZ(position, 0, 0, 1);
      
      break;
    case '+':
      speed *= 2;
      if (speed > 128) { speed = 128; }
      break;
    case '-':
      speed /= 2;
      if (speed < 1) { speed = 1; }
      break;
    }
    if (input != '\n') {
      printf("Position: %d speed=%d> ", position, speed);
    }
  }
}

void zaxis::moveZHome() {
  if ((peg->calibData.DEBUG_LEVEL & DEBUG_NOZAXIS)) {
    printf("No Zaxis\n");
    return;
  }
  printf("========= Alex check 1 =========");
  moveZ(peg->calibData.getLiftInitial(), 0, 0, 1);
  printf("========= Alex check 2 =========");

  while (isZLimit()) {
    printf("========= Alex check 3 =========");

    if (peg->calibData.DEBUG_LEVEL & DEBUG_ZAXIS) printf("Moving away from z limit");
    moveZ(peg->calibData.getLiftInitial(), 0, 0, 1);
  }
  if (peg->calibData.DEBUG_LEVEL & DEBUG_ZAXIS) printf("Seeking zLimit\n");
  while (moveZ(-peg->calibData.getLiftInitial(), 0, 0, 1) == 0) {
    zP = 0;
  }
  if (peg->calibData.DEBUG_LEVEL & DEBUG_ZAXIS) printf("Found zLimit\n");
  moveZ(-peg->calibData.getBelowLimit(), 1, 0, 1);
  zP = 0;
}

void zaxis::setZ(int i) {
  zP = i;
}
