/*
  Copyright (c) 2016 Doobie (doobie.org)
  
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef _Z_AXIS
#define _Z_AXIS
#include "GPIO/GPIOManager.h"
#include "GPIO/GPIOConst.h"

class pegasus;

#define MAX_ACCEL_DECEL_STEPS 1000

class zaxis {
public:
  zaxis(pegasus *a_peg);
  ~zaxis();

  void liftMoveZ(int);
  void initZAxis();
  void moveZHome();
  int moveZ(int z, int ovr, int cnt, float speedDiv);
  int isZLimit();
  int isLidOpen();
  void moveZAbs(int z, int ovr);
  void enableZ(int);
  //void calibrateZAxis();
  int getZ();
  void setZ(int i);
  void setZPrinted(int i) { if (i > highestZPrinted) {highestZPrinted = i;} }
  void setCurrentZPrinted() { if (zP > highestZPrinted) {highestZPrinted = zP;} }
  void initStep();
  void tuneZ();
protected: 
private:
  int Z_LIMIT;
  int LID;
  int ABEN;
  int ABEN2;
  int ABSTEP;
  int ABDIR;
  int CUR;
  int zP;
  int inZCalibration;
  int calibZTop;
  int calibZBot;
  int highestZPrinted;
  GPIO::GPIOManager* gp;
  pegasus *peg;
  unsigned int AccelDecelDelay[MAX_ACCEL_DECEL_STEPS];
  int AccelerationTime;
};
#endif
