/*
  Copyright (c) 2016 Doobie (doobie.org)
  
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef GCODE_H
#define GCODE_H

#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>

#include "general/calib.hxx"

class MainApp;
class pegasus;

#define RUN_HARDWARE 1

enum hasBit {
  HAS_X = 0,
  HAS_Y,
  HAS_Z,
  HAS_E,
  HAS_F
};

enum gcodeStatus { 
  GCodeEndOfFile,
  GCodeNewLayer,
  GCodeNone,
  GCodeError
};

class gcode {
public:
  gcode(pegasus *a_peg);
  ~gcode();
  void setGui(MainApp*ptr) { gui = ptr; }
  int openFile(char *file);
  void analyzeGcode();
  int getEstPrintTime() { return estPrintTime; }
  int getTotalLayers() { return totalLayers; }
  int getDoneLayers() { return doneLayers; }
  int getTotalLaser() { return totalLaser; }
  int getDoneLaser() { return doneLaser; }
  int runLayer();
  void SetRunReal(int i) { runReal = i; printf("Run Real set to: %d\n", runReal); }
  int timeval_subtract(struct timeval *result, struct timeval *t2, struct timeval *t1);
protected:
  void init();
  FILE *gcodeFile;
  int analyzed;
  pegasus *peg;
  MainApp *gui;
  char *parse_pair(char*, char*, float*);
  int scaleAndMove(int execute, float X, float Y);
  gcodeStatus processG(int execute, float value, char*buffer);
  gcodeStatus processM(int execute, float value, char*buffer);
  int parseGCode(int execute, gcodeStatus stopAt);
  calibration *calibPtr;
  int estPrintTime;
  int doneLaser;
  int doneLayers;
  int totalLayers;
  int totalLaser;
#ifdef OLD_TIME
  struct timeval startTime;
#else
  time_t startTime;
#endif  
  float minX ;
  float minY ;
  float maxX ;
  float maxY ;  
  int runReal;
  int firstXY;
};

#endif
