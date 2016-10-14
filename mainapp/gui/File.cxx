/*
  Copyright (c) 2016 Doobie (doobie.org)
  
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include <string>
#include <iostream>
#include "gui/File.hxx"
#include <iostream>

File::File(pegasus *peg) :
  Gtk::Box(Gtk::ORIENTATION_HORIZONTAL),
  pegPtr(peg),
  m_VBox1 (Gtk::ORIENTATION_VERTICAL),
  m_VBox2 (Gtk::ORIENTATION_VERTICAL),
  m_VBox3 (Gtk::ORIENTATION_VERTICAL),
  m_VBox4 (Gtk::ORIENTATION_VERTICAL),
  lLayer("Layers"),
  lLaser("Laser"),
  lTime("Elapsed"),
  lRemainTime("Remain"),
  nLayer("TBD"),
  nLaser("TBD"),
  nTime("TBD"),
  nRemainTime("TBD"),
  File1("20mm_hollow_cube"),
  File2("tbd"),
  DontRun("Run galvo")
{

  add(m_VBox1);
  add(m_VBox2);
  add(m_VBox3);
  add(m_VBox4);
  
  m_VBox1.add(File1);
  m_VBox1.add(File2);
  m_VBox1.add(DontRun);

  m_VBox3.add(lLayer);
  m_VBox3.add(lLaser);
  m_VBox3.add(lTime);
  m_VBox3.add(lRemainTime);

  m_VBox4.add(nLayer);
  m_VBox4.add(nLaser);
  m_VBox4.add(nTime);
  m_VBox4.add(nRemainTime);

  m_VBox2.add(graphics);
  File1.signal_clicked().connect(sigc::mem_fun(*this,
					      &File::DoFile1) );
  File2.signal_clicked().connect(sigc::mem_fun(*this,
					      &File::DoFile2) );

}
void File::setLayers(int done, int total) {
  char strMsg[48];
  snprintf(strMsg, 48, "%d/%d, %02.02f", done, total, (float)done/(float)total*100);
  nLayer.set_text(strMsg);
}
void File::setLaser(int done, int total) {
  char strMsg[48];
  snprintf(strMsg, 48, "%dk/%dk, %02.02f", done/1000, total/1000, (float)done/(float)total*100);
  nLaser.set_text(strMsg);
}
void File::setElapsed(struct timeval *elapse)  {
  char strMsg[48];
  snprintf(strMsg, 48, "%02d:%02d:%02d", int(elapse->tv_sec / 60 / 60), int(elapse->tv_sec / 60) % 60, (int)(elapse->tv_sec % 60));
  nTime.set_text(strMsg);
}

void File::setRemain(int sec, float layer, float laser) {
  char strMsg[48];
  int remainSec = sec / ((layer+laser)/2);
  snprintf(strMsg, 48, "%02d:%02d:%02d", int(remainSec/60/60), int(remainSec/60)%60, remainSec% 60);
  nRemainTime.set_text(strMsg);  
}

void *File::dofile (void *p) {
  //((File*)p)->pegPtr->calibData.printCalib();
  if (p == NULL || ((File*)p)->pegPtr == NULL) {
    printf("Pointers are null in dofile!\n");
  } else {
    //printf("Gcode ptr: 0x%08x\n", &((File*)p)->pegPtr->gcodeProcessor);
    ((File*)p)->pegPtr->gcodeProcessor.openFile("/home/fsl/gcode/20mm_hollow_cube.gcode");
  }
  pthread_exit(NULL);
}

void *File::dofile2 (void *p) {
  if (p == NULL || ((File*)p)->pegPtr == NULL) {
    printf("Pointers are null in dofile!\n");
  } else {
    ((File*)p)->pegPtr->gcodeProcessor.openFile("/home/fsl/gcode/Calibration_Steps_Pyramid.gcode");
  }
  pthread_exit(NULL);
}

void File::DoFile1() {
  std::cout << "File1: The Button was clicked: state="
	    << (DontRun.get_active() ? "true" : "false")
	    << std::endl;
  if (DontRun.get_active()) { 
    pegPtr->gcodeProcessor.SetRunReal(1);
  } else {
    pegPtr->gcodeProcessor.SetRunReal(0);
  }
  //printf("Before file:\n");
  //pegPtr->calibData.printCalib();
  pthread_create(&thread, NULL, File::dofile, this);
  //usleep(10000);
  //printf("after file:\n");
  //pegPtr->calibData.printCalib();
  
}


void File::DoFile2() {
  std::cout << "File2: The Button was clicked: state="
	    << (DontRun.get_active() ? "true" : "false")
	    << std::endl;
  if (DontRun.get_active()) { 
    pegPtr->gcodeProcessor.SetRunReal(1);
  } else {
    pegPtr->gcodeProcessor.SetRunReal(0);

  }
  pthread_create(&thread, NULL, File::dofile2, this);


}

