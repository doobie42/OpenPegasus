/*
  Copyright (c) 2016 Doobie (doobie.org)
  
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef FILE_HXX
#define FILE_HXX
#include <pthread.h>
#include <gtkmm.h>
#include "general/pegasus.hxx"
#include "gui/Graphics.hxx"

class File : public Gtk::Box {
public:
  File(pegasus *peg);
  void DotIt(int x, int y,int c) { graphics.DotIt(x,y, c); }
  void setLayers(int done, int total);
  void setLaser(int done, int total);
  void setElapsed(struct timeval *elapse);
  void setRemain(int sec, float layer, float laser);
  void ClrDraw() {graphics.ClrDraw(); }
protected:
  Graphics graphics;
  pegasus *pegPtr;
  Gtk::Box m_VBox1, m_VBox2, m_VBox3, m_VBox4;
  Gtk::Label lLayer, lLaser, lTime, lRemainTime;
  Gtk::Label nLayer, nLaser, nTime, nRemainTime;
  Gtk::Button File1, File2;
  Gtk::CheckButton DontRun;  
  
  pthread_t thread;

  int lastX, lastY;
  static void *dofile(void *p);
  static void *dofile2(void *p);
  void DoFile1();
  void DoFile2();
};

#endif
