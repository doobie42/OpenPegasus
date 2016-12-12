/*
  Copyright (c) 2016 Doobie (doobie.org)
  
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "gui/MainApp.hxx"
#include "general/pegasus.hxx"

//extern pegasus peg;

int lineNbr = 0;
double baseE, baseX, baseY, baseZ;
double lastE=0, lastX, lastY, lastZ;
int absolutePos = 1;

//int peg->calibData.getDebugLevel() = 0;
//int peg->calibData.getDebugLevel() = DEBUG_SIMPLE;
//int peg->calibData.getDebugLevel() = DEBUG_BASIC | DEBUG_MODERATE | DEBUG_SIMPLE |DEBUG_GALVO;

// TODO: move to class
int pointsPerSlice = 0;
int nbrPoints = 0;
int nbrSlices = 0; 
int maxPPS;
float currentZ = 0;

gcode::gcode(pegasus *a_peg) :
  gcodeFile(NULL),
  peg(a_peg),
  gui(NULL) {
  printf("gcode construct\n");
  init();
  runReal = 0;
  firstXY = 1;
  }

gcode::~gcode() {

}

void gcode::init() {
  minX = 1000;
  minY = 1000;
  maxX = 0;
  maxY = 0;
  totalLayers = 0;
  totalLaser  = 0;
  analyzed    = 0;
}
char *gcode::parse_pair(char *line, char *letter, float *value) {
  // null line
  if (NULL == line) {
    return NULL;
  }
  line = peg->calibData.removeWhitespace(line);
  
  // comment
  if (*line == '\0' || *line == ';' || *line == '%')
    return NULL;
  
  // Comments can be in ()'s between commands
  if (*line == '(') {
    while (*line && ')' != *line)
      line++;
    // might have white space again
    line = peg->calibData.removeWhitespace(line + 1);
    if ('\0' == *line) return NULL;
  }
  
  // get the letter and return it as an upper case.
  *letter = toupper(*line);
  if (*(++line) == '\0') { // letter without a number; is this possible?
    printf("Error: Couldn't finish processing; missing $ str @line%d: %s\n", lineNbr, line);
    return NULL;
  }
  
  char *endOfFloat;
  //printf("Line before strtof: %s\n", line);
  if (errno != 0 ) {
    printf ("before: strtof returned an error: %d\n", errno);
    errno = 0;
  }
  *value = strtof(line, &endOfFloat);
  if (errno != 0 ) {
    printf ("strtof returned an error: %d\n", errno);
  }
  //printf("Floating: %f, end of %s\n", *value, endOfFloat);
  //exit(0);
  // do we need a check to see if number is following leter?: probably at some point
  line = endOfFloat;
  line = peg->calibData.removeWhitespace(line);
  return line;
}

gcodeStatus gcode::processM(int execute, float value, char *buffer) {
  //printf("%s\n",buffer);
  // 107
  // 649
  // 650
  char let = 0;
  float val = 0;
  int p = 0; // did we print anything wihtout a 'n that needs to be printed later
  if (peg->calibData.getDebugLevel() & DEBUG_BASIC) {p=1; printf("M%d:", (int)value);}
  if (value == 0) {
    // treat this as a stop;
    printf("Press 'r' the enter to start\n");
    while ('r' != getchar());
  }else if (value == 1) {
  } else if (value == 82) { // M82: set extruder to abs mode
  } else if (value == 84) { // M84: stop idle hold
  } else if (value == 104) { // M104: ??
  } else if (value == 106) { // M106 : fan
    while ((buffer = parse_pair(buffer, &let, &val))) {
      if (peg->calibData.getDebugLevel() & DEBUG_BASIC) {p=1;printf("[%c%f]", let, val);}
      switch (let) {
      case 'S':
	break;
      default:
	printf("ERROR: Unknown: M106 element: %c%f\n", let, val);
	return GCodeError;	
      }
    }

  } else if (value == 107) { // M107: fan off
  } else if (value == 109) { // M109: ??
  } else if (value == 649) { // M649: unknown
    while ((buffer = parse_pair(buffer, &let, &val))) {
      if (peg->calibData.getDebugLevel() & DEBUG_BASIC) {p=1; printf("[%c%f]", let, val);}
    }
  } else if (value == 650) { // unknown
    while ((buffer = parse_pair(buffer, &let, &val))) {
      if (peg->calibData.getDebugLevel() & DEBUG_BASIC) {p=1; printf("[%c%f]", let, val);}
    }
  } else {
    printf("ERROR: Unknown M-cmd @line%d error: M%f %s\n", lineNbr, value, buffer);
    return GCodeError;
    //perror("ERROR in M processing\n");
  }
  if (p) printf( "\n");
  return GCodeNone;
}

int gcode::scaleAndMove(int execute, float X, float Y) {
  int XS, YS;
  int p = 0;
  if (peg->calibData.useMatrixCalib()) {
    peg->calibData.transformXY(X, Y, &XS, &YS);
  }
  //printf("X/Y=%d, %d from %f, %f\n", XS, YS, X,Y);
  if (!analyzed) {
    totalLaser++;
  } else {
    doneLaser++;
  }
  
  if (analyzed) {
    if (execute & RUN_HARDWARE) {
      int laserRepeat;
      if (doneLayers <= peg->calibData.getFirstLayers()) {
	laserRepeat = peg->calibData.getFirstLayersRepeat() * peg->calibData.getLaserRepeat();
      } else {
	laserRepeat = peg->calibData.getLaserRepeat();
      }
      peg->galvoControl.addMoveTo((ushort)XS,(ushort)YS, peg->calibData.getLaserEnable(), laserRepeat, doneLayers);
    }
    if (gui != NULL) {
      int drawX, drawY;
      float scale;
      if (maxX-minX > maxY-minY) {
	// x is bigger
	scale = 127 / (maxX-minX);
      } else {
	// y is bigger
	scale = 127 / (maxY-minY);
      }
      drawX = (X - minX) * scale;
      drawY = (Y - minY) * scale;
      //printf("draw: %d x %d!!\n", (int)drawX, (int)drawY);
      gui->getFile()->DotIt((int)drawX, (int)drawY, 1);
    } else {
      //printf("GUI is null!!\n");
    }
  }
  nbrPoints++;
  pointsPerSlice++;
  if (peg->calibData.getDebugLevel() & DEBUG_GCODE) { p = 1; printf("MOVETO: %d, %d from %f, %f\n", XS,YS, X,Y);}
  return p;
}

gcodeStatus gcode::processG(int execute, float value, char*buffer) {
  //char *str;
  //printf("%s\n",buffer);  
  // 92
  // 1
  char let = 0;
  float val = 0;
  int p = 0;
  //const char *buffer = buffer;
  if (peg->calibData.getDebugLevel() & DEBUG_BASIC) {p=1; printf("G%d:", (int)value);}
  if (value == 1 || value == 0) {
    // move X, Y, E.
    float xline = 0;
    float yline = 0;
    float eline = 0;
    //tbd:float fline = 0;
    float zline = 0;
    int hasElement =0;
    //printf("bufferG: %s\n", buffer);
    while ((buffer = parse_pair(buffer, &let, &val))) {
      if (peg->calibData.getDebugLevel() & DEBUG_BASIC) {p=1; printf("[%c%f]", let, val);}
      switch(let) {
      case 'X':
	xline = val;
	hasElement |= 1<<HAS_X;
	break;
      case 'Y':
	yline = val;
	hasElement |= 1<<HAS_Y;
	break;
      case 'Z':
	zline = val;
	hasElement |= 1<<HAS_Z;
	break;
      case 'E':
	eline = val;
	hasElement |= 1<<HAS_E;
	break;
      case 'F': //TBD: do we want/need to handle this?
	//tbd:fline = val; 
	hasElement |= 1<<HAS_F;
	break;
      default:
	printf("ERROR: Unknown: G1 element: %c%f\n", let, val);
	return GCodeError;
      }
    }
    // ignore F, possibly use E for laser time?!?
    if (hasElement & 1<<HAS_Z) {
      // move Z up; then down.
      firstXY = 1; // track the XY not move to it.
      if (currentZ != zline) {
	if (peg->calibData.getDebugLevel() & DEBUG_SIMPLE) {p=1; printf("NEW SLICE: %d (total:%d) this: %d, max:%d. ", nbrSlices, nbrPoints, pointsPerSlice, maxPPS);}
	if (pointsPerSlice > maxPPS) { maxPPS = pointsPerSlice; }
	pointsPerSlice=0;
	nbrSlices++;
	if (execute & RUN_HARDWARE) {
	  peg->galvoControl.runGalvo(doneLayers);
	}
	errno = 0;
	if (peg->calibData.getDebugLevel() & DEBUG_SIMPLE) printf("Z step=%f; was %f. ", zline, currentZ);
	//moveZ(zline*peg->calibData.ZSCALE, 0, 1, 100);
	if (execute & RUN_HARDWARE) {
	  if (peg->calibData.isUnitsMM()) {
	    peg->zaxisControl.liftMoveZ(zline*peg->calibData.getScale(2,1));
	  } else {
	    peg->zaxisControl.liftMoveZ(zline*peg->calibData.getScale(2,0));
	  }
	}
	if (analyzed && gui != NULL) {
	  gui->getFile()->DotIt(-1,-1,-1);
	}

	currentZ = zline;
	if (!analyzed) {
	  totalLayers++;
	  if (gui != NULL) {
	    gui->getFile()->setLayers(0, totalLayers);
	    gui->getFile()->setLaser(0, totalLaser);
	  }
	}
	return GCodeNewLayer;
	
      } else {
	if (peg->calibData.getDebugLevel() & DEBUG_SIMPLE) {p=1; printf("Z step=%f did not change. ", zline); }
      }
    }
    // TBD: do we want to pay attention to feed rate or assume we know better?
    if ((hasElement & (1<<HAS_X|1<<HAS_Y|1<<HAS_E)) == (1<<HAS_X|1<<HAS_Y|1<<HAS_E)) {
      if (absolutePos == 0) {
	printf("ERROR!  OH NO!  I don't support abs position yet!\n");
	exit(0);
      } else if (firstXY) { // this is a hack
	firstXY = 0;
	lastX = xline;
	lastY = yline;
      } else {
	int steps;
	float extractAmount = eline - lastE; 
	float hyp;
	int i;
	float xs = 0, Y = lastY;
	float ys = 0, X = lastX;
	hyp = sqrt((lastX-xline)*(lastX-xline) + (lastY-yline)*(lastY-yline));
	steps = hyp / ((float)peg->calibData.getSpotSize()/1000);
	//printf("Spot size=%dm hyp=%f\n", peg->calibData.getSpotSize(), hyp);
	//ys = (yline-lastY)/steps;
	//xs = (xline-lastX)/steps;
	ys = (yline-lastY)/steps;
	xs = (xline-lastX)/steps;
	//printf("Steps: %d size: %d\n", steps, peg->calibData.spotSize);
	if (peg->calibData.getDebugLevel() & DEBUG_SIMPLE) printf("extractAmount: %f = %f - %f; hyp: %f, hyp/E: %f; dX: %f = %f - %f; dY: %f = %f - %f: steps: %d\n", extractAmount, eline, lastE, hyp, hyp/extractAmount, xline-lastX, xline, lastX, yline-lastY, yline, lastY, steps);
	// X/Y are where we start from.  move on1
	for (i = 0; i < steps; i++) {
	  X+=xs;
	  Y+=ys;
	  if (peg->calibData.getDebugLevel() & DEBUG_SIMPLE) printf("\t%d => %f, %f\n", i, X,Y);
	  //#ifdef REAL_GALVO
	  p |= scaleAndMove(execute,X,Y);
	  //#endif
	  if (X < minX) minX = X;
	  if (X > maxX) maxX = X;
	  if (Y < minY) minY = Y;
	  if (Y > maxY) maxY = Y;
	}
	if (execute & RUN_HARDWARE) {
	  //TODO: fails to run laser if we do this here:
	  //peg->galvoControl.runGalvo();
	}
	lastX = xline;
	lastY = yline;
	lastE = eline;
      }
    } else if ((hasElement & (1<<HAS_X|1<<HAS_Y)) == (1<<HAS_X|1<<HAS_Y)) {
      lastX = xline;
      lastY = yline;
    } else if ((hasElement & (1<<HAS_E))) {
      //printf("Moving E to %f\n", lastE);
      lastE = eline;
    }
    
  } else if (value == 10) {
    //retract
  } else if (value == 11) {
    //unretract
  } else if (value == 20) {
    while ((buffer = parse_pair(buffer, &let, &val))) {
      if (peg->calibData.getDebugLevel() & DEBUG_BASIC) {p=1; printf("[%c%f]", let, val);}
    }
    peg->calibData.setMMUnits(0);
  } else if (value == 21) {
    while ((buffer = parse_pair(buffer, &let, &val))) {
      if (peg->calibData.getDebugLevel() & DEBUG_BASIC) {p=1; printf("[%c%f]", let, val);}
    }
    peg->calibData.setMMUnits(1);
  } else if (value == 28) {
    while ((buffer = parse_pair(buffer, &let, &val))) {
      if (peg->calibData.getDebugLevel() & DEBUG_BASIC) {p=1; printf("[%c%f]", let, val);}
    }
  } else if (value == 90) { // G90 : abs position
    absolutePos = 1;
  } else if (value == 91) { // G91 : relative position
    absolutePos = 0;
    printf("Error: relative positioning not supported yet; hopefully this is the end of the file!\n");
  } else if (value == 92) {
    // set position, X,Y,Z,E
    while ((buffer = parse_pair(buffer, &let, &val))) {
      float eline = 0;
      int hasElement;
      if (peg->calibData.getDebugLevel() & DEBUG_BASIC) {p=1; printf("[%c%f]", let, val);}
      switch(let) {
      case 'E':
	eline = val; lastE = eline;
	hasElement |= 1<<HAS_E;
	break;
      default:
	printf("ERROR: Unknown: G92 element: %c%f\n", let, val);
	return GCodeError;
      }
    }
  } else if (value == 1024) {
    for (int c = 0; c < 1; c++) {
      for (int i = 0; i < 1; i++) 
	p |= scaleAndMove(execute,90,90);    
      for (int i = 0; i < 1; i++) 
	p |= scaleAndMove(execute,45,45);    
      for (int i = 0; i < 1; i++) 
	p |= scaleAndMove(execute,45,135);    
      for (int i = 0; i < 1; i++) 
	p |= scaleAndMove(execute,135,135);    
      for (int i = 0; i < 1; i++) 
	p |= scaleAndMove(execute,135,45);    
    }
  } else {
    printf("ERROR: Unknown G-cmd @line%d error: G%f %s\n", lineNbr, value, buffer);
    return GCodeError;
    //perror("ERROR in G processing\n");
  }
  if (p)  printf( "\n");
  return GCodeNone;
}

int gcode::openFile(char *file) {
  //printf("OpenFile\n");
  //peg->calibData.printCalib();
  init();
  //printf("OpenFile post init\n");
  //peg->calibData.printCalib();
  time_t curTime;
  time_t gcodeStart = time(0);

  for (int i = 0; i < 2; i++) {
    int first = 0;
    firstXY = 1;
    gcodeStart = time(0);
    gcodeFile = fopen(file, "r");
    if (NULL == gcodeFile) {
      printf("Couldn't open gcode file: %s", file);
      return 1;
    }
    //printf("OpenFile post before analize\n");
    //peg->calibData.printCalib();
    while (peg->zaxisControl.isLidOpen()) {
      if (first == 0) {
	printf("Waiting for lid to be closed\n");
	first = 1;
      }
    }
    if (first == 1) {
      printf("Lid closed\n");
    }
    analyzeGcode();
    fclose(gcodeFile);

    curTime = time(0);
    double timeDiffSec = difftime(curTime, gcodeStart);
    if (first == 0) {
      printf("Analyzing gcode took: %f seconds.\n", timeDiffSec);
    } else {
      printf("Executing gcode took: %f seconds.\n", timeDiffSec);
    }

  }
  return 0;

}

int gcode::parseGCode(int execute, gcodeStatus stopAt) {
  char buffer[1024];
  char *tmpptr;
  //int len;
  //int first;
  char letter;
  float value;
  char *buffPtr;
  gcodeStatus status;
  while (fgets(buffer, sizeof(buffer), gcodeFile) != NULL) {
    //printf ("=>%s\n", buffer);
    lineNbr++;
    //first = 0;
    buffPtr = buffer;
    buffPtr = peg->calibData.removeWhitespace(buffPtr);
    
    if (NULL == buffPtr && !*buffPtr) {
      continue;
    }
    //len = strlen(buffPtr);
    //printf("Str: [%s] | Len: %d\n", buffPtr, len);

    if (';' == buffPtr[0]) {
      //printf ("Comment: %s\n", &buffPtr[first+1]);
    } else {
      //printf("BBuffPtr: %s\n", buffPtr);
      // * first char not a ;
      tmpptr = strchr(buffPtr, ';');
      if (NULL != tmpptr) { *tmpptr = 0; }
      //printf("BuffPtr: %s (%s)\n", buffPtr, tmpptr);
      letter = 0;
      //printf("Before: %s\n", buffPtr);
      buffPtr = parse_pair(buffPtr, &letter, &value);
      //printf("After : %s\n", buffPtr);
      //printf("%s, letter=%c, value=%f\n", buffer, letter, value);
      if (letter != 0) {
	//printf("Line3: %c\n", letter);
	if ('M' == letter) {
	  status = processM(execute, value, buffPtr);
	} else if ('G' == letter) {
	  status = processG(execute, value, buffPtr);
	}
	//printf("Line4\n");
	if (status == GCodeError) {
	  return GCodeError;
	} else if (status != GCodeNone && status == stopAt) {
	  return status;
	} else if (status == GCodeNewLayer) {
	  if (analyzed) {
	    doneLayers++;
	    //int elapsed;
	    time_t curTime = time(0);
	    double timeDiffSec = difftime(curTime, startTime);
	    double percentDne = (double)((((double)doneLayers/(double)totalLayers)+((double)doneLaser/(double)totalLaser)))/2;
	    double remain;
	    if (percentDne == 0) {
	      remain = 100000;
	    } else {
	      remain = (int)((double)timeDiffSec / percentDne) - timeDiffSec;
	    }
	    //printf("if\n");
	    if (gui == NULL) {
	      printf("\tDone layers = %d of %d %f %%\n", doneLayers, totalLayers, (float)doneLayers/(float)totalLayers*100);
	      printf("\tDone laser = %d of %d or %f %%\n", doneLaser, totalLaser, (float)doneLaser/(float)totalLaser*100);

	      printf("\tElapsed time: %02d:%02d:%02d\n", int(timeDiffSec / 60 / 60), int(timeDiffSec / 60) % 60, (int)timeDiffSec % 60);
	      printf("\tRemain  time: %02d:%02d:%02d (%f = %f / %f %%)\n", int(remain / 60 / 60), int(remain / 60) % 60, (int)remain % 60, remain, timeDiffSec, percentDne);

	    } else {
	      gui->getFile()->setLayers(doneLayers, totalLayers);
	      gui->getFile()->setLaser(doneLaser, totalLaser);

	      //todo:gui->getFile()->setElapsed(&timeDiffSec);
	      //todo:gui->getFile()->setRemain(timeDiff.tv_sec, ((float)doneLayers/(float)totalLayers), ((float)doneLaser/(float)totalLaser));

	    }
	  }
	}
      }
    }
  }
  printf("Done reading buffer\n");
  return 0;
}
/* Return 1 if the difference is negative, otherwise 0.  */
int gcode::timeval_subtract(struct timeval *result, struct timeval *t2, struct timeval *t1) {
  long int diff = (t2->tv_usec + 1000000 * t2->tv_sec) - (t1->tv_usec + 1000000 * t1->tv_sec);
  result->tv_sec = diff / 1000000;
  result->tv_usec = diff % 1000000;

  return (diff<0);
}

void gcode::analyzeGcode() {
  if (analyzed) {
    printf("Analyzed...running now....real: %d\n", runReal);
#ifdef OLD_TIME
    gettimeofday(&startTime, NULL);
#else
    startTime = time(0);
#endif
    parseGCode(runReal, GCodeEndOfFile);
    
  } else {
    printf("Analyzing...%d\n", runReal);   
    //peg->calibData.printCalib();
    if (runReal) {
      peg->zaxisControl.enableZ(1);
      //printf("after Z enable\n");
      //peg->calibData.printCalib();
      peg->zaxisControl.initZAxis();
      //printf("init\n");
      //peg->calibData.printCalib();
      peg->zaxisControl.moveZHome();
      //printf("Zhome\n");
      //peg->calibData.printCalib();
      //peg->zaxisControl.setZ(0);
    }
    //printf("parse:\n");
    //peg->calibData.printCalib();

    parseGCode(0, GCodeEndOfFile);
  }
  printf("Total Layers: %d\n", totalLayers);
  printf("Total Laser : %d\n", totalLaser);
  printf("MinX=%f, maxX=%f, size=%f\n", minX, maxX, maxX-minY);
  printf("MinY=%f, maxY=%f, size=%f\n", minY, maxY, maxY-minY);
  analyzed = 1;
  doneLayers = 0;
  doneLaser = 0;
  if (runReal) {
    peg->zaxisControl.enableZ(0);
  }
}

/*
#ifndef ZAXIS_TESTING
#ifndef GALVO_TESTING

main(int argc, char *argv[]) {
  if (argc != 3) {
    printf ("Usage: %s <calib file> <gcode file>\n", argv[0]);
    exit(0);
  }
  
  printf("Calib File: %s\n", argv[1]);
  printf("Gcode File: %s\n", argv[2]);
  peg->calibData.readCalib(argv[1]);
  peg->calibData.printCalib();
#ifdef REAL_Z

#endif
  //moveZHome();
  printf("Press 'r' the enter to start\n");
  while ('r' != getchar());
  exit(peg->gcodeProcessor.parseGCode(argv[2]));

}
#endif
#endif
*/
