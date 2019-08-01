/*
  Copyright (c) 2016 Doobie (doobie.org)
  
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h> //abs
// Driver header file
#ifdef REAL_HARDWARE
#include <prussdrv.h>
#include <pruss_intc_mapping.h>
#else
#warning "Not running with real hardware!"
#endif
#include "err.h"
#include <math.h>
#include "general/pegasus.hxx"
#include "galvo.hxx"


galvo::galvo(pegasus *a_peg) : 
  peg(a_peg) {
  printf("====== IN galvo.cxx ======");

  printf("galvo construct\n");
#ifndef REAL_HARDWARE
  printf("Galvo not enable via real hardware!\n");
#endif
  pkt.nbrPkts = 0;
}

galvo::~galvo() {

}

#define SHARED_MEMORY 0x0

void galvo::writeDataToPRU(int packets, sGalvo *pkts, unsigned int loops, unsigned int nbrOfLoops) {
  unsigned int memSize = packets*sizeof(sGalvo);
  unsigned int done = (loops+1 == nbrOfLoops) ? 0xFFFFFFFF : 0x0;
  // nbr of packets
  if (peg->calibData.DEBUG_LEVEL & DEBUG_PRU) {
    printf("writeDataToPRU: prepping to send: %d\n", packets);
  }
#ifdef PRUMEM
  // for using shared memory; would be nice maybe?
  pruDataMem[0] = packets;
  // packet burst
  memcpy (&pruDataMem[1], pkts, memSize);
  // are there done?
  pruDataMem[memSize+1] = done;
#else
  prussdrv_pru_write_memory(PRUSS0_PRU0_DATARAM, SHARED_MEMORY, (unsigned int*) &packets, sizeof(int));
  if (peg->calibData.DEBUG_LEVEL & DEBUG_PRU) {
    printf("Sending: %d %d\n", packets, memSize);
    printf("%08x %08x %08x %08x %08x %08x %0x8 %08x\n", ((int*)pkts)[0], ((int*)pkts)[1], ((int*)pkts)[2], ((int*)pkts)[3], ((int*)pkts)[4], ((int*)pkts)[5], ((int*)pkts)[6], ((int*)pkts)[7]);
  }
  prussdrv_pru_write_memory(PRUSS0_PRU0_DATARAM, SHARED_MEMORY+1, (unsigned int*)pkts, memSize);
  prussdrv_pru_write_memory(PRUSS0_PRU0_DATARAM, SHARED_MEMORY+1+(memSize/4), (unsigned int*) &done, sizeof(int));
#endif 
  // here are the packets!
  prussdrv_pru_send_event(ARM_PRU0_INTERRUPT);

  // wait for PRU response
  prussdrv_pru_wait_event (PRU_EVTOUT_0);
  // clear PRU response
  prussdrv_pru_clear_event (PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);
  // TBD: when I upgrade I can fix this, but the version of pruss has a bug with events that have the event occuring twice (https://github.com/beagleboard/am335x_pru_package/issues/28)
  // wait for PRU response
  //__sync_synchronize();return;
#ifdef KERNEL_3_8
  prussdrv_pru_wait_event (PRU_EVTOUT_0);
#endif
  // clear PRU response
  prussdrv_pru_clear_event (PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);
}

void galvo::sendDataToPRU() {
  // we can only send 8kb of data to the PRU at a time.  There are 2 shorts, one int per packet
  unsigned int packetsTransfered = 0;
  unsigned int nbrOfTransfers = (pkt.nbrPkts / MAX_PRU_PACKETS);
  unsigned int loops = 0;
  unsigned int packetsOnTransfer = 0;
  
  if (pkt.nbrPkts != (nbrOfTransfers * MAX_PRU_PACKETS)) {
    nbrOfTransfers++;
  }
  if (peg->calibData.DEBUG_LEVEL & DEBUG_PRU) {
    printf("SendDataToPRU: %d packets to send in %d transfers of a maximum size of %d\n", pkt.nbrPkts, nbrOfTransfers, MAX_PRU_PACKETS);
  }
  
  for (loops = 0; loops < nbrOfTransfers; loops++) {
    if ((pkt.nbrPkts - packetsTransfered) > MAX_PRU_PACKETS) {
      packetsOnTransfer = MAX_PRU_PACKETS;
    } else {
      packetsOnTransfer = pkt.nbrPkts - packetsTransfered;
    }
    if (peg->calibData.DEBUG_LEVEL & DEBUG_PRU) printf("SendDataToGalvo: sending packet bundle %d with %d packets starting with %d.\n", loops, packetsOnTransfer, packetsTransfered);
    writeDataToPRU(packetsOnTransfer, &pkt.myPackets[packetsTransfered], loops, nbrOfTransfers);
    packetsTransfered += packetsOnTransfer;
  }
  
  if (loops != nbrOfTransfers) {
    printf ("Error: invalid numer of transfers: expected %d but saw %d\n", nbrOfTransfers, loops);
    exit(0);
  }
}

void galvo::enablePRU() {
  // addresses	
  unsigned int ret;
  
  // initialize pru
  if (peg->calibData.DEBUG_LEVEL & DEBUG_PRU) printf("\nINFO: Enabling PRU\r\n");
#ifdef REAL_HARDWARE
  tpruss_intc_initdata intc = PRUSS_INTC_INITDATA;
  
  /* Initialize the PRU */
  if ((ret = prussdrv_init()) != 0) {
    fprintf(stderr, "pru_setup(): path is NULL\n");
    exit(-1);
  }
  
  /* Open PRU Interrupt */
  if ((ret = prussdrv_open(PRU_EVTOUT_0))) {
    printf("prussdrv_open open failed\n");
    errx(EXIT_FAILURE, "prussdrv_open failed\n");
  }
  
  //printf("resetting\n");
  prussdrv_pru_reset(0);
  
  /* Get the interrupt initialized */
  if((ret = prussdrv_pruintc_init(&intc)) != 0) {
    fprintf(stderr, "prussdrv_pruintc_init() failed\n");
    exit(ret);
  }
  
  /* Initialize example */
  if (peg->calibData.DEBUG_LEVEL & DEBUG_PRU) printf("\tINFO: loading code: runlaser.bin\r\n");
  if ((ret = prussdrv_exec_program (0, "./bin/runlaser.bin")) < 0) {
    printf("prussdrv_exec_program() failed\n");
    exit(ret);
  }
#endif

}

void galvo::disablePRU() {
#ifdef REAL_HARDWARE
  if (prussdrv_pru_disable (0) != 0) {
    fprintf(stderr, "prussdrv_pru_disable failed: %d\n", errno);
  }
#endif
}

void galvo::runGalvo(int layer) {
  //printf("sizeof(pkt.myPackets)=%d, nbr=%d\n", sizeof(pkt.myPackets),pkt.nbrPkts);
#ifdef OLD_LOOPS
  int loops;
  if (layer <= peg->calibData.getFirstLayers()) {
    loops = peg->calibData.getFirstLayersRepeat();
  } else {
    loops = 1;
  }
#endif
  if ((peg->calibData.DEBUG_LEVEL & DEBUG_GALVO)) {
#ifdef OLD_LOOPS
    printf("galvo::runGalvo(%d), loops = %d, numPkt=%d (%d, %d)\n", layer, loops, pkt.nbrPkts, peg->calibData.getFirstLayers(), peg->calibData.getFirstLayersRepeat());
#else
    printf("galvo::runGalvo(%d), numPkt=%d (%d, %d)\n", layer, pkt.nbrPkts, peg->calibData.getFirstLayers(), peg->calibData.getFirstLayersRepeat());
#endif
  }
  if (pkt.nbrPkts > 0) {
    if (!(peg->calibData.DEBUG_LEVEL & DEBUG_NOGALVO)) {
      //#ifdef OLD_LOOPS
      //for (int i = 0; i < loops; i++) {
      //if (i != 0) {
      //#endif
      enablePRU();
      //#ifdef OLD_LOOPS
      //}
      //#endif
      sendDataToPRU();
      //#ifdef OLD_LOOPS
      //}
      //#endif
      disablePRU();
      peg->zaxisControl.setCurrentZPrinted();
    } else {
      printf("Skipping GALVO run\n");
    }
    pkt.nbrPkts = 0;
  }
  if (!(peg->calibData.DEBUG_LEVEL & DEBUG_NOGALVO)) {

  }
}

//#define STEP_SMOOTHER
//this code tries to make steps smoother on large jumps.
#define SMOOTHER_SIZE 2000
void galvo::addMoveTo(ushort X, ushort Y, int L, ushort R, int layer) {
#ifdef STEP_SMOOTHER
  if (pkt.nbrPkts > 0) {
    ushort lX = 0, lY = 0;
    int dX, dY, dH;
    ushort steps;
    printf("last pkt[%d] at %d,%d, new packet at %d, %d\n", pkt.nbrPkts,  pkt.myPackets[pkt.nbrPkts-1].Xdac, pkt.myPackets[pkt.nbrPkts-1].Ydac, X,Y);
    lX = pkt.myPackets[pkt.nbrPkts-1].Xdac;
    lY = pkt.myPackets[pkt.nbrPkts-1].Ydac;
    if (pkt.nbrPkts + 4 >= PKT_LEN) {
      // restart at the beginning.
      printf("pkt[%d] at %d,%d near end\n", pkt.nbrPkts,  pkt.myPackets[pkt.nbrPkts-1].Xdac, pkt.myPackets[pkt.nbrPkts-1].Ydac);
      //runGalvo(layer);
      pkt.nbrPkts = 0;
      pkt.myPackets[pkt.nbrPkts].Xdac = X;
      pkt.myPackets[pkt.nbrPkts].Ydac = Y;
      pkt.myPackets[pkt.nbrPkts].LaserData = 0xA << 24; // 1 repeat, no laser
      pkt.nbrPkts++;
    }
    dX = X - lX;
    dY = Y - lY;
    dH = sqrt(dX*dX + dY*dY);
    if (dH > SMOOTHER_SIZE) {
      steps = (dH)/ SMOOTHER_SIZE + 1/*rounding*/;
      printf("Distance: %d, steps=%d, dx=%d, dy=%d\n", dH, steps, dX, dY);
      dX /= steps;
      dY /= steps;
      printf("Distance: %d, steps=%d, dx=%d, dy=%d\n", dH, steps, dX, dY);
      for (int i = 1; i < steps; i++) {
	printf("\tnull: %d,%d i=%d from %d %d => %d, %d\n", dX, dY, i, lX, lY,  lX + dX * i, lY +dY*i);
	pkt.myPackets[pkt.nbrPkts].Xdac = (ushort)X;
	pkt.myPackets[pkt.nbrPkts].Ydac = Y;
	pkt.myPackets[pkt.nbrPkts].LaserData = 0xA << 24;// 1 repeat, no laser
	pkt.nbrPkts++;
	if (pkt.nbrPkts + 4 >= PKT_LEN) {
	  runGalvo(layer);
	}
	pkt.nbrPkts = 0;
      }
      //printf("Exiting after smoothing move.\n");
      //exit(0);
    }
  }
#endif
  pkt.myPackets[pkt.nbrPkts].LaserData = (((int)R & 0xFF) << 24) | (L & 0xFFFFFF);
  //printf("X:%5d, Y:%5d, L:%x, R:%3d => %08x\n", X, Y, L, R, pkt.myPackets[pkt.nbrPkts].LaserData);
  pkt.myPackets[pkt.nbrPkts].Xdac = (ushort)X;
  pkt.myPackets[pkt.nbrPkts].Ydac = (ushort)Y;
  pkt.nbrPkts++;
  if (pkt.nbrPkts >= PKT_LEN) {
    runGalvo(layer);
  }
}

void galvo::tuneGalvo() {
  char input;
  int maxPts = PTS_X*PTS_Y;
  int pt = 0;
  int x = 0;
  int y = 0;
  int speed = 64;
  input = 0;
  ushort xp, yp;
  int runLaser = 1;
  peg->calibData.getCalibPts(x, y, &xp, &yp);
  printf("pt%d (%d, %d) @ (%d, %d) speed=%d> ", pt, x, y, speed, xp, yp);
  while (input != 'q') {
    if (runLaser) {
      peg->calibData.getCalibPts(x, y, &xp, &yp);
      for (int i = 0; i < 50; i++) {
	addMoveTo(xp, yp, 0x1111, 0xff, 0);
      }
      runGalvo(0);
      runLaser = 0;
    }
    input = getchar();
    runLaser = 0;
    switch (input) {
    case 'N':
    case 'n':
      pt++;
      if (input >= maxPts) {
	input = 0;
      }
      x = pt % PTS_X;
      y = pt / PTS_X;
      runLaser = 1;
      break;
    case 'P':
    case 'p':
      pt--;
      if (input < 0) {
	input = maxPts - 1;
      }
      x = pt % PTS_X;
      y = pt / PTS_X;
      runLaser = 1;
      break;
    case 'L':
    case 'l':
      // move left;
      peg->calibData.setCalibPtsDx(x, y, -speed, 0);
      runLaser = 1;
      break;
    case 'R':
    case 'r':
      // move right;
      peg->calibData.setCalibPtsDx(x, y, +speed, 0);
      runLaser = 1;
    break;
    case 'D':
    case 'd':
      // move up;
      peg->calibData.setCalibPtsDx(x, y, 0, speed);
      runLaser = 1;
    break;
    case 'U':
    case 'u':
      // move down;
      peg->calibData.setCalibPtsDx(x, y, 0, -speed);
      runLaser = 1;
    break;
    case 'F':
    case 'f':
    case '+':
      // move faster
      speed*=2;
      break;
    case 'S':
    case 's':
    case '-':
      // mov slower
      speed /= 2;
      if (speed < 1) { speed = 1; }
      break;
    case ' ':
      runLaser = 1;
      break;
    case 'a':
      for (int r = 0; r < 2; r++) {
      for (y = 0; y < PTS_Y; y++) {
	for (x = 0; x < PTS_X; x++) {
	  for (int i = 0; i < 30; i++) {
	    peg->calibData.getCalibPts(x, y, &xp, &yp);
	    addMoveTo(xp, yp, 0x5555, 0xf, 0);
	  }
	}
      }
      }
      x = pt % PTS_X;
      y = pt / PTS_X;
      runGalvo(0);
      break;
    case 'W':
    case 'w':
      peg->calibData.writeCalib();
      break;
    case 'Q':
    case 'q':
      input = 'q';
      break;
    } // case
    if (input != '\n') {
      peg->calibData.getCalibPts(x, y, &xp, &yp);
      printf("pt%d (%d, %d) @ (%d, %d) speed=%d> ", pt, x, y, speed, xp, yp);
    }
  }
  peg->calibData.writeCalib();
}
