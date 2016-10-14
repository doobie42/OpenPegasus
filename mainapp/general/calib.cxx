/*
  Copyright (c) 2016 Doobie (doobie.org)
  
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include "general/pegasus.hxx"

int cpts[PTS_X][PTS_Y][2] = {{{  0,   0},{  0,  45},{  0,  90},{  0, 135},{  0, 180}},
			     {{ 45,   0},{ 45,  45},{ 45,  90},{ 45, 135},{ 45, 180}},
			     {{ 90,   0},{ 90,  45},{ 90,  90},{ 90, 135},{ 90, 180}},
			     {{135,   0},{135,  45},{135,  90},{135, 135},{135, 180}},
			     {{180,   0},{180,  45},{180,  90},{180, 135},{180, 180}}};

			     /*int cpts[PTS_X][PTS_Y][2] = {{{0,   0}, {45,   0}, {90,   0}, {135,   0}, {180,   0}},
			     {{0,  45}, {45,  45}, {90,  45}, {135,  45}, {180,  45}},
			     {{0,  90}, {45,  90}, {90,  90}, {135,  90}, {180,  90}},
			     {{0, 135}, {45, 135}, {90, 135}, {135, 135}, {180, 135}},
			     {{0, 180}, {45, 180}, {90, 180}, {135, 180}, {180, 180}}};
			     */
calibration::calibration(pegasus *a_peg) :
  peg(a_peg) {

  DEBUG_LEVEL = 0; //DEBUG_SIMPLE |DEBUG_BASIC;// | 0x0FFF; 
  // |DEBUG_NOZAXIS |DEBUG_NOGALVO;
  ZAxisHomed = 0;
  ZPosition = 0; // not valid until ZAxisHomed
  // gcode
  unitsInMM = 1;
  
  // exposure: fixed for now, but can these come or be adjusted from gcode exposure data
  LaserEnable = 0x00FFFF;
  LaserRepeat = 10;
  
  // estimates for Times of running
  zLayerTime = 4; // how much time per layer (z movement)
  laserTime = 10000; // estimate of how many laser points per second
  
  // Need to calibrate these in a calib file; use defaults for now.
  xScaleMM = 131*2;
  yScaleMM = 131*2;
  xScaleIN = 40000/6;
  yScaleIN = 40000/6;
  xOffset = 10000;
  yOffset = 10000;
  
  for (int x = 0; x < PTS_X; x++) {
    for (int y = 0; y < PTS_Y; y++) {
      calibpts[x][y][0] = 340*cpts[x][y][0]+2750;
      calibpts[x][y][1] = 340*cpts[x][y][1]+2750;
    }
  }
  // Z Axis
  zScaleMM          = 1000;
  zScaleIN          = 10000; // todo inches
  zLiftInitial      = 1000;
  zLiftTicks        = 2000;
  zBelowLimit       = 40;
  zHomeTopBottom    = 20;
  spotSize          = 80;
  useHypot          = 1;
  zLiftPause        = 1000000;
  zLiftSteps        = 1;
  firstLayers       = -1;
  firstLayersRepeat = 1;

  zInitialACD       = 5000;
  zAccConst         = 3.3;
}

int calibration::stringToNbr(char*equal) {
  if (equal[0] == '0' && equal[1] == 'x') {
    //printf("hex of: %s\n", equal);
    // hex
    return strtol( equal, NULL, 16 );
  } else {
    //printf("aoit of: %s\n", equal);
    return atoi(equal);
  }

}
int calibration::getValue(char *str) {
  char *equal;
  equal = strchr(str, '=');
  if (equal == NULL) {
    printf("ERROR: calib has no value\n");
    return 0;
  }
  equal++;
  equal = removeWhitespace(equal);
  if (*equal == 0) {
    return 0;
  }
  return stringToNbr(equal);
}

void calibration::processLaserCalib(char *str) {
  int value = getValue(str);
  str = removeWhitespace(str);
  
  if (!strncmp(str, "pattern", 7)) {
    LaserEnable = value;
  } else if (!strncmp(str, "repeat", 6)) {
    LaserRepeat = (ushort)value;
  } else if (!strncmp(str, "spotsize", 8)) {
    spotSize = value;
  } else if (!strncmp(str, "usehypot", 8)) {
    useHypot = value;
  } else if (!strncmp(str, "time", 4)) {
    laserTime = value;
  } else if (!strncmp(str, "firstlayersrepeat", 17)) {
    firstLayersRepeat = value;
    printf("Overridig firstLayersRepeat: %d\n", value);
  } else if (!strncmp(str, "firstlayers", 11)) {
    firstLayers = value;
  } else {
    printf ("ERROR: Couldn't process laser line: %s\n", str);
  }
}

char *calibration::removeWhitespace(char *line) {
  while (*line && ((' ' == *line) || ('\t' == *line || ('\n' == *line) || ('\r' == *line)))) {
    line++;
  }
  return line;
}


void calibration::processDebugCalib(char *str) {
  int value = getValue(str);
  str = removeWhitespace(str);
  if (!strncmp(str, "value", 5)) {
    DEBUG_LEVEL = value;
  } else{
    printf ("ERROR: Couldn't process debug line: %s\n", str);
  }
}

void calibration::processXCalib(char *str) {
  int value = getValue(str);
  str = removeWhitespace(str);
  if (!strncmp(str, "scalemm", 7)) {
    xScaleMM = value;
  } else if (!strncmp(str, "scalein", 7)) {
    xScaleIN = value;
  } else if (!strncmp(str, "offset", 6)) {
    xOffset = value;
  } else if (!strncmp(str, "point", 5)) {
    processPoint(XPT, "x", str, value);
  } else{
    printf ("ERROR: Couldn't process X line: %s\n", str);
  }
}
void calibration::processYCalib(char *str) {
  int value = getValue(str);
  str = removeWhitespace(str);
  if (!strncmp(str, "scalemm", 7)) {
    yScaleMM = value;
  } else if (!strncmp(str, "scalein", 7)) {
    yScaleIN = value;
  } else if (!strncmp(str, "offset", 6)) {
    yOffset = value;
  } else if (!strncmp(str, "point", 5)) {
    processPoint(YPT, "y", str, value);
  } else{
    printf ("ERROR: Couldn't process Y line: %s\n", str);
  }
}

void calibration::processGeneralCalib(char *str) {
  int value = getValue(str);
  str = removeWhitespace(str);
  if (!strncmp(str, "matrix", 6)) {
    matrixCalib = value;
  } else {
    printf ("ERROR: Couldn't process general line: %s\n", str);
  }
}

void calibration::adjustXY(int iaxis, int pt, int amount, int showLaser) {
  if (pt >= 0 && pt < PTS_X*PTS_Y) {
    int ptX = pt % PTS_X;
    int ptY = pt / PTS_Y;
    calibpts[ptX][ptY][iaxis] += amount;
    printf("adjustXY(%d, %d, %d) = %d\n", iaxis, pt, amount, calibpts[ptX][ptY][iaxis]);
    if (showLaser) {
      peg->galvoControl.addMoveTo(cpts[ptX][ptY][XPT], cpts[ptX][ptY][YPT], peg->calibData.getLaserEnable(), peg->calibData.getLaserRepeat(), 0);
    }
  } else {

  }
}

void calibration::processPoint(int iaxis, const char *saxis, char *str, int value) {
  if (strlen(str) > 5) {
    int pt = stringToNbr(&str[5]);
    if (pt < 0 || pt > (PTS_X * PTS_Y)) {
      printf("calib.%s: point out of range: %s => 0 !< %d !< %d\n", saxis, str, pt, PTS_X*PTS_Y);
    } else {
      int x = pt % PTS_X;
      int y = pt / PTS_Y;
      calibpts[x][y][iaxis] = value;
    }
  } else {
    printf("calib.%s: Unable to process %s\n", saxis, str);
  }
}

void calibration::processZCalib(char *str) {
  int value = getValue(str);
  str = removeWhitespace(str);
  if (!strncmp(str, "liftdistance", 12)) {
    zLiftTicks = value;
  } else if (!strncmp(str, "liftpause", 9)) {
    zLiftPause = value;
  } else if (!strncmp(str, "liftsteps", 9)) {
    zLiftSteps = value;
  } else if (!strncmp(str, "lowerpause", 9)) {
    zLowerPause = value;
  } else if (!strncmp(str, "printpause", 10)) {
    zPrintPause = value;
  } else if (!strncmp(str, "scalemm", 7)) {
    zScaleMM = value;
  } else if (!strncmp(str, "scalein", 7)) {
    zScaleIN = value;
  } else if (!strncmp(str, "belowlimit", 10)) {
    zBelowLimit = value;
  } else if (!strncmp(str, "hometopbottom", 10)) {
    zHomeTopBottom = value;
  } else if (!strncmp(str, "maxlift", 7)) {
    zMaxLift = value;
  } else if (!strncmp(str, "initialacd", 10)) {
    zInitialACD = value;
  } else if (!strncmp(str, "accconst", 8)) {
    zAccConst = (float)value/100;
  } else if (!strncmp(str, "donelift", 8)) {
    zDoneLift = value;
  } else if (!strncmp(str, "time", 4)) {
    zLayerTime = value;
  } else{
    printf ("ERROR: Couldn't process Z line: %s\n", str);
  }
}

int calibration::getLiftDistance() {
  return zLiftTicks;
}

int calibration::getLowerPause() {
  return zLowerPause;
}
int calibration::getLiftSteps() {
  return zLiftSteps;
}
int calibration::getPrintPause() {
  return zPrintPause;
}

int calibration::getLiftPause() {
  return zLiftPause;
}

int calibration::getScale(int axis, int MM) {
  if (MM) {
    if (axis == 0) { return xScaleMM; }
    if (axis == 1) { return yScaleMM; }
    if (axis == 2) { return zScaleMM; }
  } else {
    if (axis == 0) { return xScaleIN; }
    if (axis == 1) { return yScaleIN; }
    if (axis == 2) { return zScaleIN; }
  }
  printf("calibration::getScale(): Invalid axis/MM:%d/%d\n", axis, MM);
  return 0;
}

void calibration::setScale(int axis, int steps) {
  if (axis == 0) { xScaleMM = steps; }
  else if (axis == 1) { yScaleMM = steps; }
  else if (axis == 2) { zScaleMM = steps; }
}

int calibration::getOffset(int axis, int MM) {
  if (MM) {
    if (axis == 0) { return xOffset; }
    if (axis == 1) { return yOffset; }
  } else {
    if (axis == 0) { return xOffset; }
    if (axis == 1) { return yOffset; }
  }
  printf("calibration::getOffset(): Invalid axis/MM:%d/%d\n", axis, MM);
  return 0;
}

int calibration::getLiftInitial() { 
  return zLiftInitial;
}

int calibration::getBelowLimit() {
  return zBelowLimit;
}

void calibration::printCalib() {
  printf("Debug: 0x%x\n", DEBUG_LEVEL);
  
  printf("Laser.Pattern: 0x%x\n", LaserEnable);
  printf("Laser.Repeat: %d\n", LaserRepeat);
  printf("Laser.FirstLayers: %d\n", firstLayers);
  printf("Laser.FirstLayersRepeat: %d\n", firstLayersRepeat);

  printf("MatrixCalib: %d\n", matrixCalib);
  printf("X ScaleMM: %d\n", xScaleMM);
  printf("X ScaleIN: %d\n", xScaleIN);
  printf("X Offset: %d\n", xOffset);

  printf("Y ScaleMM: %d\n", yScaleMM);
  printf("Y ScaleIN: %d\n", yScaleIN);
  printf("Y Offset: %d\n", yOffset);

  printf("Z ScaleMM: %d\n", zScaleMM);
  printf("Z ScaleIN: %d\n", zScaleIN);
  printf("Z LiftPause: %d\n", zLiftPause);
  printf("Z LiftSteps: %d\n", zLiftSteps);
  printf("Z LiftSlow: %d\n", zLiftTicks);
  printf("Z Lift Initial: %d\n", zLiftInitial);
  printf("Z BelowLimit: %d\n", zBelowLimit);
  printf("Z Initial ACD: %d\n", zInitialACD);
  printf("Z Const ACD: %f\n", zAccConst);

  printf("SpotSize: %d\n", spotSize);
  printf("UseHypot: %d\n", useHypot);

  for (int y = 0; y < PTS_Y; y++) {
    for (int x = 0; x < PTS_X; x++) {
      printf("%5d, %5d", cpts[x][y][XPT], cpts[x][y][YPT]);
      if (x+1 != PTS_X) {
	printf(" | ");
      }
    }
    printf("\n");
  }
  printf("\n");
  for (int y = 0; y < PTS_Y; y++) {
    for (int x = 0; x < PTS_X; x++) {
      printf("%5d, %5d", calibpts[x][y][XPT], calibpts[x][y][YPT]);
      if (x+1 != PTS_X) {
	printf(" | ");
      }
    }
    printf("\n");
  }
  // test transforms
  int testPts[][2] = {{10,25},
		      {45,65},
		      {55,35},
		      {175,65},
		      {45,145}};
  int nx = 0;
  int ny = 0;
  for (int i=0; i < 5; i++) {
    printf("==> %d\n", i);
    transformXY(testPts[i][XPT], testPts[i][YPT], &nx, &ny);
    printf("%3d, %3d => %5d, %5d\n", testPts[i][XPT], testPts[i][YPT], nx, ny);
  }
}

void calibration::findUpperLeft(float x, float y, int *ulx, int *uly) {
  for (int yc = 0; yc < (PTS_Y-1); yc++) {
    for (int xc = 0; xc < (PTS_X-1); xc++) {
      if ( (cpts[xc][yc][XPT] <= x) && (x < cpts[xc+1][yc+1][XPT]) &&
	   (cpts[xc][yc][YPT] <= y) && (y < cpts[xc+1][yc+1][YPT])) {
	*ulx = xc;
	*uly = yc;
	return ;
      } else {
	/*printf("(%d, %d) = %3d %3d | %3d %3d\n", xc, yc,
	       cpts[xc][yc][XPT], cpts[xc][yc][YPT], 
	       cpts[xc+1][yc+1][XPT], cpts[xc+1][yc+1][YPT]);*/
      }
    }
  }
  printf("Couldn't find upper left for %f, %f\n", x,y);
  exit(0);
}

void calibration::transformXY(float x, float y, int *nx, int *ny) {
  int upperLeftX;
  int upperLeftY;
  findUpperLeft(x, y, &upperLeftX, &upperLeftY);
#define PRECISION (long)100
  
  /*printf("%f, %f @ (%d,%d) => (%d, %d) -> (%d, %d)\n", 
	   x,y, upperLeftX, upperLeftY,
	   calibpts[upperLeftX][upperLeftY][XPT],
	   calibpts[upperLeftX][upperLeftY][YPT],
	   calibpts[upperLeftX+1][upperLeftY+1][XPT],
	   calibpts[upperLeftX+1][upperLeftY+1][YPT]);
    printf("%f, %f => (%d, %d) -> (%d, %d)\n", 
	   x,y, 
	   cpts[upperLeftX][upperLeftY][XPT],
	   cpts[upperLeftX][upperLeftY][YPT],
	   cpts[upperLeftX+1][upperLeftY][XPT],
	   cpts[upperLeftX][upperLeftY+1][YPT]);
  */
  long xLeftSlope   = calibpts[upperLeftX  ][upperLeftY  ][XPT] - calibpts[upperLeftX  ][upperLeftY+1][XPT];
  long xRightSlope  = calibpts[upperLeftX+1][upperLeftY  ][XPT] - calibpts[upperLeftX+1][upperLeftY+1][XPT];
  long yTopSlope    = calibpts[upperLeftX  ][upperLeftY  ][YPT] - calibpts[upperLeftX+1][upperLeftY  ][YPT];
  long yBottomSlope = calibpts[upperLeftX  ][upperLeftY+1][YPT] - calibpts[upperLeftX+1][upperLeftY+1][YPT];
  
  /*printf("xLS,xRS=(%ld, %ld)  yTS,yBS=(%ld, %ld)\n", xLeftSlope, xRightSlope, yTopSlope, yBottomSlope);
    printf("%d %d | %d %d\n", cpts[upperLeftX+1][upperLeftY  ][XPT], cpts[upperLeftX  ][upperLeftY  ][XPT],
	                      cpts[upperLeftX  ][upperLeftY+1][YPT], cpts[upperLeftX  ][upperLeftY  ][YPT]);
  */
  long xRatio = (long)(PRECISION*(x - cpts[upperLeftX][upperLeftY][XPT])) / (cpts[upperLeftX+1][upperLeftY  ][XPT] - cpts[upperLeftX  ][upperLeftY  ][XPT]);
  long yRatio = (long)(PRECISION*(y - cpts[upperLeftX][upperLeftY][YPT])) / (cpts[upperLeftX  ][upperLeftY+1][YPT] - cpts[upperLeftX  ][upperLeftY  ][YPT]);

  //printf("xR,yR=(%ld, %ld)\n", xRatio, yRatio);
  
  long xLeftSlopePt   = xLeftSlope   * yRatio;
  long xRightSlopePt  = xRightSlope  * yRatio;
  long yTopSlopePt    = yTopSlope    * xRatio;
  long yBottomSlopePt = yBottomSlope * xRatio;
  //printf("xLSR,xRSR=(%ld, %ld)  yTSR,yBSR=(%ld, %ld)\n", xLeftSlopePt, xRightSlopePt, yTopSlopePt, yBottomSlopePt);

  long xLeftPt   = xLeftSlopePt   + calibpts[upperLeftX  ][upperLeftY  ][XPT]*PRECISION;
  long xRightPt  = xRightSlopePt  + calibpts[upperLeftX+1][upperLeftY  ][XPT]*PRECISION;
  long yTopPt    = yTopSlopePt    + calibpts[upperLeftX  ][upperLeftY  ][YPT]*PRECISION;
  long yBottomPt = yBottomSlopePt + calibpts[upperLeftX  ][upperLeftY+1][YPT]*PRECISION;
  //printf("xLR,xRR=(%ld, %ld)  yTR,yBR=(%ld, %ld)\n", xLeftPt, xRightPt, yTopPt, yBottomPt);

  *nx = (int)(((xRightPt - xLeftPt) * xRatio)/PRECISION + xLeftPt)/PRECISION; 
  *ny = (int)(((yBottomPt - yTopPt) * yRatio)/PRECISION + yTopPt)/PRECISION;
}

int calibration::readCalib(char*file) {
  char buffer[1024];
  char *buffPtr;
  FILE *calib = fopen(file,"r");
  int lineNbr = 0;
  if (NULL == calib) {
    perror("Couldn't open calib file");
    return 1;
  }
  while (fgets(buffer, sizeof(buffer), calib) != NULL) {
    lineNbr++;
    buffPtr = buffer;
    buffPtr = removeWhitespace(buffPtr);
    if (';' == buffPtr[0]) {
      // just comment
    } else {
      char *tmpptr = strchr(buffPtr, ';');
      if (NULL != tmpptr) { *tmpptr = 0; }
      //printf("calib:%s\n", buffPtr);
      if (buffPtr[0] == 0) {
      } else if (!strncmp(buffPtr, "laser", 5)) {
	processLaserCalib(&buffPtr[6]);
      } else if (!strncmp(buffPtr, "debug", 5)) {
	processDebugCalib(&buffPtr[6]);
      } else if (!strncmp(buffPtr, "z", 1)) {
	processZCalib(&buffPtr[2]);
      } else if (!strncmp(buffPtr, "x", 1)) {
	processXCalib(&buffPtr[2]);
      } else if (!strncmp(buffPtr, "y", 1)) {
	processYCalib(&buffPtr[2]);
      } else if (!strncmp(buffPtr, "general", 7)) {
	processGeneralCalib(&buffPtr[8]);
      } else {
	printf( "Unknown calib: %s\n", buffPtr);
      }
    }
  }
  peg->zaxisControl.initStep();
  return 0;
}

void calibration::adjustZScale(int i) {
  zScaleMM += i;
  if (zScaleMM < 1) { zScaleMM = 1; }
  if (zScaleMM > 10000) { zScaleMM = 10000; } // TODO: is this too much or too little?
}

