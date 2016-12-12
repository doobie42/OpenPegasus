/*
  Copyright (c) 2016 Doobie (doobie.org)
  
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef CALIB_HXX
#define CALIB_HXX


#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#define DEBUG_BASIC    0x0001
#define DEBUG_SIMPLE   0x0002
#define DEBUG_MODERATE 0x0004
#define DEBUG_GALVO    0x0008
#define DEBUG_PRU      0x0010
#define DEBUG_ZAXIS    0x0020
#define DEBUG_GCODE    0x0040

class pegasus;

// Debug levels to disable hardware components
#define DEBUG_NOZAXIS  0x4000
#define DEBUG_NOGALVO  0x8000

#define PTS_X 5
#define PTS_Y 5
#define XPT 0
#define YPT 1

class calibration {

public:
  // Debug  
  int DEBUG_LEVEL;
  calibration(pegasus *a_peg);
  int getValue(char *str);
  void processLaserCalib(char *str);
  void processDebugCalib(char *str);
  void processGeneralCalib(char *str);
  void processXCalib(char *str);
  void processYCalib(char *str);
  void processZCalib(char *str);
  void printCalib(FILE*fp);
  int readCalib(char*file, int recur);

  // Z
  //int getLiftInitial();
  int getLiftDistance();
  int getLowerPause();
  int getPrintPause();
  int getLiftPause();
  int getLiftSteps();
  int getBelowLimit() { return zBelowLimit; }

  
  char *removeWhitespace(char *line);
  int isUnitsMM() { return unitsInMM; }
  void setMMUnits(int units) { unitsInMM = units; }
  int getScale(int, int);
  void setScale(int, int);
  int getOffset(int, int);
  int getSpotSize() { return spotSize; }
  int getDebugLevel() { return DEBUG_LEVEL; }
  int getLaserEnable() { return LaserEnable; }
  ushort getLaserRepeat() { return LaserRepeat; }
  int getZScale() { return zScaleMM; }
  void adjustZScale(int i);
  int getLiftSpeed(int i);
  int getLowerSpeed(int i);
  int getLayerTime() { return zLayerTime; }
  int getLaserTime() { return laserTime; }
  int getFirstLayers() { return firstLayers; }
  int getFirstLayersRepeat() { return firstLayersRepeat; }
  int stringToNbr(char*equal);
  void processPoint(int iaxis, const char *saxis, char *str, int value);
  void transformXY(float x, float y, int *nx, int *ny);
  void findUpperLeft(float x, float y, int *ulx, int *uly);
  int useMatrixCalib() { return matrixCalib; }
  void adjustXY(int iaxis, int pt, int amount, int showLaser);
  int getInitialACD() { return zInitialACD; }
  float getAccConst() { return zAccConst; } 
  void getCalibPts(int x, int y, ushort *xp, ushort *yp) { *xp = calibPtsGalvo[x][y][XPT];  *yp = calibPtsGalvo[x][y][YPT]; }
  void setCalibPts(int x, int y, ushort xp, ushort yp) {   calibPtsGalvo[x][y][XPT]  = xp;  calibPtsGalvo[x][y][YPT]  = yp; }
  void setCalibPtsDx(int x, int y, ushort xp, ushort yp) { calibPtsGalvo[x][y][XPT] += xp;  calibPtsGalvo[x][y][YPT] += yp; }
  void writeCalib();
  int getLiftInitial() { return zLiftInitial; }
protected:
  // in mm
  static ushort calPoints[PTS_X][PTS_Y][2];
  // galvo position
  ushort calibPtsGalvo[PTS_X][PTS_Y][2];

  // auto tuned
  int ZAxisHomed; //= 0;
  int ZPosition; //= 0; // not valid until ZAxisHomed
  // gcode
  int unitsInMM; //= 1;
  
  // 
  int firstLayers;
  int firstLayersRepeat;
  // Laser
  // exposure: fixed for now, but can these come or be adjusted from gcode exposure data
  int LaserEnable;
  ushort LaserRepeat;
  int LaserVolume;
  int zLayerTime;
  int laserTime;
  // Indeve
  int useHypot;
  int spotSize;

  int xyMatrix[25][2]; // TODO:X/Y for each of 25 calibration points.

  int matrixCalib;
  // X axis
  // Need to calibrate these in a calib file; use defaults for now.
  int xScaleMM; //= 131*2;
  int yScaleMM; //= 131*2;
  int xScaleIN; //= 40000/6;

  // Y axis
  int yScaleIN; //= 40000/6;
  int xOffset; //= 10000;
  int yOffset; //= 10000;
  
  // Z Axis
  int zScaleMM      ; //= 1000;
  int zScaleIN      ; //= 1000;

  int zLiftInitial   ;
  int zLiftDistance   ; //= 2000;
  int zLiftPause    ; // Puase before lifting;
  int zLiftSteps    ; // number of steps to lift in
  int zLowerPause    ; // Pause before lower;
  int zPrintPause    ; // Pause before starting next layer;
  int zBelowLimit;
  //int zHomeTopBottom;
  int zDoneLift;
  int zMaxLift;
  int zInitialACD;
  float zAccConst;
  
  char calibFile[512];
  pegasus *peg;

};

#endif
