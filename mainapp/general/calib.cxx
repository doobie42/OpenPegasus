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
ushort calibration::calPoints[PTS_X][PTS_Y][2] = {{{  0,   0},{  0,  45},{  0,  90},{  0, 135},{  0, 180}},
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
                   printf("====== IN calib.cxx ======");
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
      calibPtsGalvo[x][y][XPT] = 340 * calPoints[x][y][0] + 2750;
      calibPtsGalvo[x][y][YPT] = 340 * calPoints[x][y][1] + 2750;
    }
  }

  // Z Axis
  zScaleMM          = 1000;
  zScaleIN          = 10000; // todo inches
  zLiftInitial      = 1000;
  zLiftDistance        = 2000;
  zBelowLimit       = 40;
  //zHomeTopBottom    = 20;
  spotSize          = 80;
  useGalvoFloat     = 1;
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
  } else if (!strncmp(str, "usefloat", 8)) {
    useGalvoFloat = value;
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
    calibPtsGalvo[ptX][ptY][iaxis] += amount;
    printf("adjustXY(%d, %d, %d) = %d\n", iaxis, pt, amount, calibPtsGalvo[ptX][ptY][iaxis]);
    if (showLaser) {
      peg->galvoControl.addMoveTo(calPoints[ptX][ptY][XPT], calPoints[ptX][ptY][YPT], peg->calibData.getLaserEnable(), peg->calibData.getLaserRepeat(), 0);
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
      calibPtsGalvo[x][y][iaxis] = value;
    }
  } else {
    printf("calib.%s: Unable to process %s\n", saxis, str);
  }
}

void calibration::processZCalib(char *str) {
  int value = getValue(str);
  str = removeWhitespace(str);
  if (!strncmp(str, "liftdistance", 12)) {
    zLiftDistance = value;
  } else if (!strncmp(str, "liftpause", 9)) {
    zLiftPause = value;
  } else if (!strncmp(str, "liftinitial", 11)) {
    zLiftInitial = value;
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
    //} else if (!strncmp(str, "hometopbottom", 10)) {
    //zHomeTopBottom = value;
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
  return zLiftDistance;
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


void calibration::printCalib(FILE*fp) {
  fprintf(fp, "debug.value = 0x%x\n", DEBUG_LEVEL);
  
  fprintf(fp, "laser.pattern = 0x%x\n", LaserEnable);
  fprintf(fp, "laser.repeat = %d\n", LaserRepeat);
  fprintf(fp, "laser.firstlayers = %d\n", firstLayers);
  fprintf(fp, "laser.firstlayersrepeat = %d\n", firstLayersRepeat);
  fprintf(fp, "laser.spotsize = %d\n", spotSize);
  fprintf(fp, "laser.usehypot = %d\n", useHypot);
  fprintf(fp, "laser.float = %d\n", useGalvoFloat);

  fprintf(fp, "x.scalemm = %d\n", xScaleMM);
  fprintf(fp, "x.scalein = %d\n", xScaleIN);
  fprintf(fp, "x.offset = %d\n", xOffset);

  fprintf(fp, "y.scalemm = %d\n", yScaleMM);
  fprintf(fp, "y.scalein = %d\n", yScaleIN);
  fprintf(fp, "y.offset = %d\n", yOffset);

  fprintf(fp, "z.liftinitial = %d\n", zLiftInitial);
  fprintf(fp, "z.liftdistance = %d\n", zLiftDistance);
  fprintf(fp, "z.liftpause = %d\n", zLiftPause);
  fprintf(fp, "z.liftsteps = %d\n", zLiftSteps);
  fprintf(fp, "z.lowerpause = %d\n", zLowerPause);
  fprintf(fp, "z.printpause = %d\n", zPrintPause);
  fprintf(fp, "z.scalemm = %d\n", zScaleMM);
  fprintf(fp, "z.scalein = %d\n", zScaleIN);
  fprintf(fp, "z.belowlimit = %d\n", zBelowLimit);
  fprintf(fp, "z.donelift = %d\n", zDoneLift);
  fprintf(fp, "z.maxlift = %d\n", zMaxLift);
  fprintf(fp, "z.initialacd = %d\n", zInitialACD);
  fprintf(fp, "z.accconst = %d\n", int(zAccConst*100));
  fprintf(fp, "z.time = %d\n", zLayerTime);

  fprintf(fp, "general.matrixcalib = %d\n", matrixCalib);

  int i = 0;
  for (int y = 0; y < PTS_Y; y++) {
    for (int x = 0; x < PTS_X; x++) {
      fprintf(fp, "x.point%d = %5d\n", i, calibPtsGalvo[x][y][XPT]);
      fprintf(fp, "y.point%d = %5d\n", i, calibPtsGalvo[x][y][YPT]);
      i++;
    }
  }
  return;
  
  for (int y = 0; y < PTS_Y; y++) {
    for (int x = 0; x < PTS_X; x++) {
      printf("%5d, %5d", calibPtsGalvo[x][y][XPT], calibPtsGalvo[x][y][YPT]);
      if (x+1 != PTS_X) {
	printf(" | ");
      }
    }
    printf("\n");
  }
  // test transforms
  float testPts[][2] = {{10,25},
			{45,65},
			{55,35},
			{175,65},
			{45,145}};

  int nx = 0;
  int ny = 0;
  int px = 0, py=0;
  int dx, dy;
  for (int i=0; i < 5; i++) {

    printf("==> %d\n", i);
    transformXY(0, testPts[i][XPT], testPts[i][YPT], &nx, &ny); 
    dx = abs(nx-px);
    dy = abs(ny-py);
    printf("\tf: %f, %f => %5d, %5d (dx=%5x, %5x, hypot=%d)\n", testPts[i][XPT], testPts[i][YPT], nx, ny, dx, dy, (int)sqrt(dx*dx + dy*dy));
    transformXY(1, testPts[i][XPT], testPts[i][YPT], &nx, &ny); 
    dx = abs(nx-px);
    dy = abs(ny-py);
    printf("\td:%f, %f => %5d, %5d (dx=%5x, %5x, hypot=%d)\n", testPts[i][XPT], testPts[i][YPT], nx, ny, dx, dy, (int)sqrt(dx*dx + dy*dy));
    px=nx;
    py=ny;
  }
  exit(0);
}

void calibration::findUpperLeft(float x, float y, int *ulx, int *uly) {
  for (int yc = 0; yc < (PTS_Y-1); yc++) {
    for (int xc = 0; xc < (PTS_X-1); xc++) {
      if ( (calPoints[xc][yc][XPT] <= x) && (x < calPoints[xc+1][yc+1][XPT]) &&
	   (calPoints[xc][yc][YPT] <= y) && (y < calPoints[xc+1][yc+1][YPT])) {
	*ulx = xc;
	*uly = yc;
	//printf("UL: %d %d\n", *ulx, *uly);
	return ;
      } else {
	/*printf("(%d, %d) = %3d %3d | %3d %3d\n", xc, yc,
	       calPoints[xc][yc][XPT], calPoints[xc][yc][YPT], 
	       calPoints[xc+1][yc+1][XPT], calPoints[xc+1][yc+1][YPT]);*/
      }
    }
  }
  printf("Couldn't find upper left for %f, %f\n", x,y);
  exit(0);
}

void calibration::transformXY(int floatMath, float x, float y, int *nx, int *ny) {
  if (useMatrixCalib()) {
    int upperLeftX;
    int upperLeftY;
    
    findUpperLeft(x, y, &upperLeftX, &upperLeftY);
    // use floating point, note it is slower, but works, non-float doesn't work right now (precision issues).
    floatMath = 0; // TBD
#define PRECISION (long long)10000
    /*printf("%f, %f @ (%d,%d) => (%d, %d) -> (%d, %d)\n", 
      x,y, upperLeftX, upperLeftY,
      calibPtsGalvo[upperLeftX][upperLeftY][XPT],
      calibPtsGalvo[upperLeftX][upperLeftY][YPT],
      calibPtsGalvo[upperLeftX+1][upperLeftY+1][XPT],
      calibPtsGalvo[upperLeftX+1][upperLeftY+1][YPT]);
      printf("%f, %f => (%d, %d) -> (%d, %d)\n", 
      x,y, 
      calPoints[upperLeftX][upperLeftY][XPT],
      calPoints[upperLeftX][upperLeftY][YPT],
      calPoints[upperLeftX+1][upperLeftY][XPT],
      calPoints[upperLeftX][upperLeftY+1][YPT]);
    */
    long xLeftSlope   = calibPtsGalvo[upperLeftX  ][upperLeftY  ][XPT] - calibPtsGalvo[upperLeftX  ][upperLeftY+1][XPT];
    long xRightSlope  = calibPtsGalvo[upperLeftX+1][upperLeftY  ][XPT] - calibPtsGalvo[upperLeftX+1][upperLeftY+1][XPT];
    long yTopSlope    = calibPtsGalvo[upperLeftX  ][upperLeftY  ][YPT] - calibPtsGalvo[upperLeftX+1][upperLeftY  ][YPT];
    long yBottomSlope = calibPtsGalvo[upperLeftX  ][upperLeftY+1][YPT] - calibPtsGalvo[upperLeftX+1][upperLeftY+1][YPT];
    
    /*printf("xLS,xRS=(%ld, %ld)  yTS,yBS=(%ld, %ld)\n", xLeftSlope, xRightSlope, yTopSlope, yBottomSlope);
      printf("%d %d | %d %d\n", calPoints[upperLeftX+1][upperLeftY  ][XPT], calPoints[upperLeftX  ][upperLeftY  ][XPT],
      calPoints[upperLeftX  ][upperLeftY+1][YPT], calPoints[upperLeftX  ][upperLeftY  ][YPT]);
    */
    if (floatMath) {
      float xRatio = (float)((x - calPoints[upperLeftX][upperLeftY][XPT])) / (calPoints[upperLeftX+1][upperLeftY  ][XPT] - calPoints[upperLeftX  ][upperLeftY  ][XPT]);
      float yRatio = (float)((y - calPoints[upperLeftX][upperLeftY][YPT])) / (calPoints[upperLeftX  ][upperLeftY+1][YPT] - calPoints[upperLeftX  ][upperLeftY  ][YPT]);
      //printf("xR,yR=(%f, %f)\n", xRatio, yRatio);
      
      float xLeftSlopePt   = xLeftSlope   * yRatio;
      float xRightSlopePt  = xRightSlope  * yRatio;
      float yTopSlopePt    = yTopSlope    * xRatio;
      float yBottomSlopePt = yBottomSlope * xRatio;
      //printf("xLSR,xRSR=(%ld, %ld)  yTSR,yBSR=(%ld, %ld)\n", xLeftSlopePt, xRightSlopePt, yTopSlopePt, yBottomSlopePt);
      
      float xLeftPt   = xLeftSlopePt   + calibPtsGalvo[upperLeftX  ][upperLeftY  ][XPT];
      float xRightPt  = xRightSlopePt  + calibPtsGalvo[upperLeftX+1][upperLeftY  ][XPT];
      float yTopPt    = yTopSlopePt    + calibPtsGalvo[upperLeftX  ][upperLeftY  ][YPT];
      float yBottomPt = yBottomSlopePt + calibPtsGalvo[upperLeftX  ][upperLeftY+1][YPT];
      //printf("xLR,xRR=(%ld, %ld)  yTR,yBR=(%ld, %ld)\n", xLeftPt, xRightPt, yTopPt, yBottomPt);
      *nx = (int)(((xRightPt - xLeftPt) * xRatio) + xLeftPt);
      *ny = (int)(((yBottomPt - yTopPt) * yRatio) + yTopPt);
    } else {
      long xRatio = (long)(PRECISION*(x - calPoints[upperLeftX][upperLeftY][XPT])) / (calPoints[upperLeftX+1][upperLeftY  ][XPT] - calPoints[upperLeftX  ][upperLeftY  ][XPT]);
      long yRatio = (long)(PRECISION*(y - calPoints[upperLeftX][upperLeftY][YPT])) / (calPoints[upperLeftX  ][upperLeftY+1][YPT] - calPoints[upperLeftX  ][upperLeftY  ][YPT]);
      //printf("xR,yR=(%ld, %ld)\n", xRatio, yRatio);
      
      long long xLeftSlopePt   = xLeftSlope   * yRatio;
      long long xRightSlopePt  = xRightSlope  * yRatio;
      long long yTopSlopePt    = yTopSlope    * xRatio;
      long long yBottomSlopePt = yBottomSlope * xRatio;
      //printf("xLSR,xRSR=(%ld, %ld)  yTSR,yBSR=(%ld, %ld)\n", xLeftSlopePt, xRightSlopePt, yTopSlopePt, yBottomSlopePt);
      
      long long xLeftPt   = xLeftSlopePt   + calibPtsGalvo[upperLeftX  ][upperLeftY  ][XPT]*PRECISION;
      long long xRightPt  = xRightSlopePt  + calibPtsGalvo[upperLeftX+1][upperLeftY  ][XPT]*PRECISION;
      long long yTopPt    = yTopSlopePt    + calibPtsGalvo[upperLeftX  ][upperLeftY  ][YPT]*PRECISION;
      long long yBottomPt = yBottomSlopePt + calibPtsGalvo[upperLeftX  ][upperLeftY+1][YPT]*PRECISION;
      //printf("xLR,xRR=(%ld, %ld)  yTR,yBR=(%ld, %ld)\n", xLeftPt, xRightPt, yTopPt, yBottomPt);
      *nx = (int)(((xRightPt - xLeftPt) * xRatio)/PRECISION + xLeftPt)/PRECISION;
      *ny = (int)(((yBottomPt - yTopPt) * yRatio)/PRECISION + yTopPt)/PRECISION;
    }
  } else {

    if (peg->calibData.isUnitsMM()) {
      //printf("X MM: %d %d\n",  peg->calibData.getScale(0,1), peg->calibData.getOffset(0,1));
      //printf("Y MM: %d %d\n",  peg->calibData.getScale(1,1), peg->calibData.getOffset(1,1));
      *ny  = (int)(y * peg->calibData.getScale(1,1) + peg->calibData.getOffset(1,1));
      *nx  = (int)(x * peg->calibData.getScale(0,1) + peg->calibData.getOffset(0,1));
    } else { // not used....
      *ny  = (int)(y * peg->calibData.getScale(1,0) + peg->calibData.getOffset(1,0));
      *nx  = (int)(x * peg->calibData.getScale(0,0) + peg->calibData.getOffset(0,0));
    }
  }
}

int calibration::readCalib(char*file, int recur) {
  char buffer[1024];
  char *buffPtr;
  FILE *calib = fopen(file,"r");
  int lineNbr = 0;
  snprintf(calibFile, 512, file);  // save name for later.
  if (NULL == calib) {
    printf("Couldn't open calib (%s) file for reading", file);
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
      } else if (!strncmp(buffPtr, "`include ", 8)) {
	printf("Reading sub calib file: %s\n", &buffPtr[9]);
	tmpptr = &buffPtr[9];
	while (*tmpptr != 0) {
	  if ((*tmpptr == '\n' ||
	       (*tmpptr == '\r'))) {
	    *tmpptr = 0;
	  } else {
	    tmpptr++;
	  }
	}
	readCalib(&buffPtr[9], 1);
      } else if (!strncmp(buffPtr, "general", 7)) {
	processGeneralCalib(&buffPtr[8]);
      } else {
	printf( "Unknown calib: %s\n", buffPtr);
      }
    }
  }
  if (!recur) {
    peg->zaxisControl.initStep();
  }
  return 0;
}

void calibration::adjustZScale(int i) {
  zScaleMM += i;
  if (zScaleMM < 1) { zScaleMM = 1; }
  if (zScaleMM > 10000) { zScaleMM = 10000; } // TODO: is this too much or too little?
}

void calibration::writeCalib() {
  char newFile[512];
  snprintf(newFile, 512, "%s.new", calibFile);
  FILE *calib = fopen(newFile, "w");
  if (NULL == calib) {
    printf("Couldn't open calib (%s) file for writing\n", calibFile);
    return ;
  } else {
    printCalib(calib);
    fclose(calib);
  }
}
