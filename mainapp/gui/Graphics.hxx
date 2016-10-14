/*
  Copyright (c) 2016 Doobie (doobie.org)
  
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef GRAPHICS_HXX
#define GRAPHICS_HXX
#include <gtkmm/drawingarea.h>
#include <cairomm/context.h>
#include <glibmm/main.h>

#define DRAWING_X 128
#define DRAWING_Y 128
class Graphics : public Gtk::DrawingArea {
public:
  Graphics();
  virtual ~Graphics();
  void ClrDraw();
  void force_redraw();
  void DotIt(int x,int y, int c);
protected:
  int drawing[DRAWING_X+4][DRAWING_Y+4];
  
  bool on_draw( const Cairo::RefPtr<Cairo::Context>& cr);
  int lastX, lastY;
  bool ClrOnFirst;
};

#endif
