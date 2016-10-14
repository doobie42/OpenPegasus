/*
  Copyright (c) 2016 Doobie (doobie.org)
  
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef GALVO_H
#define GALVO_H

#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h> //abs

// max number of packets per run.
#define PKT_LEN 98000  // any more and we run out of memory running the gui

/*****************************************************************************
* Explicit External Declarations                                             *
*****************************************************************************/

/*****************************************************************************
* Local Macro Declarations                                                   *
*****************************************************************************/

#define AM33XX

/*****************************************************************************
* Global Function Definitions                                                *
*****************************************************************************/

typedef struct {
  unsigned int LaserData; //32:24 = repeat, 23:0 bit on/off for laser
  unsigned short Xdac; //16 bit dac value
  unsigned short Ydac; //16 bit dac value
} sGalvo;

// TBD: can we bump this up to 1998 ? struct is 4 bytes, plus 4 bytes for header and 4 bytes for footer
#define MAX_PRU_PACKETS 1000 // ((8096-64) / sizeof(sGalvo))   we have 8kb, can wedo more?

// at some point if we can enable shared memory in the PRU we can avoid the data transfers to the PRU; this structure maps to what the data should look like
typedef struct {
  unsigned int nbrPkts;
  sGalvo myPackets[PKT_LEN];
  unsigned int done;
} sPacket;

class pegasus;

class galvo {
public:
  galvo(pegasus *a_peg);
  ~galvo();
  void addMoveTo(ushort X, ushort Y, int L, ushort R, int layer);
  void runGalvo(int layer);
  
protected:
  pegasus *peg;
  sPacket pkt;

  void writeDataToPRU(int packets, sGalvo *, unsigned int loops, unsigned int nbrOfLoops);
  void sendDataToPRU();
  void enablePRU();
  void disablePRU();

  void SendDataToGalvo();
#ifdef PRUMEM
  static void *pruDataMem;
#endif
  
};
#endif // GALVO_H
