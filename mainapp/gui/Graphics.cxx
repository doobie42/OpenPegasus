/*
  Copyright (c) 2016 Doobie (doobie.org)
  
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include "gui/Graphics.hxx"

Graphics::Graphics() {
  //lastX = -1;
  //lastY = -1;
  set_size_request (DRAWING_X+2, DRAWING_Y+2);
  force_redraw();
  Gtk::Allocation allocation = get_allocation();
  const int width = allocation.get_width();
  const int height = allocation.get_height();
  printf("Init Size: %dx%d\n", width, height);
  ClrOnFirst = true;
}

Graphics::~Graphics() {
}

bool Graphics::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
  Gtk::Allocation allocation = get_allocation();
  const int width = allocation.get_width();
  const int height = allocation.get_height();
  //printf("ondraw size %dx%d\n", width, height);
  if (ClrOnFirst) {
    ClrDraw();
    ClrOnFirst = false;
  }
  cr->set_line_width(1.0);
  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      if (drawing[x][y] != 0) {
	if((x == lastX) && (y == lastY)) {
	  cr->set_source_rgb(0, 0.8, 0.8);
	} else {
	  cr->set_source_rgb(.8, 0.0, 0.0);
	}
	cr->move_to(x,   y);
	cr->line_to(x+1, y);
      }
    }
  }
  cr->stroke();
  return true;
}

void Graphics::ClrDraw() { 
  Gtk::Allocation allocation = get_allocation();
  const int width = allocation.get_width();
  const int height = allocation.get_height();
  //  printf("clrdraw size %dx%d\n", width, height);
  //printf("Clr draw\n");
  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      if ( (x==0) || (y == 0) || (x == DRAWING_X+1) || (y == DRAWING_Y+1)) {
	drawing[x][y] = 1;
      } else {
	drawing[x][y] = 0;
      }
    }
  }
  lastX = -1;
  lastY = -1;
  force_redraw();
}
// force the redraw of the image
void Graphics::force_redraw()
{
  Glib::RefPtr< Gdk::Window > win = get_window();
  if (win) {
    Gdk::Rectangle r(0, 0, get_allocation().get_width(), get_allocation().get_height());
    win->invalidate_rect(r, false);
  } else {
    printf("Window is null\n");
  }
  //queue_draw();
}// force the redraw of the image

void Graphics::DotIt(int x, int y, int c) {
  if (x == -1 && y == -1) {
    //printf("Clearing drawing area\n");
    //usleep(1000000);
    ClrDraw();
    return ;
  }
  if (lastX-1 != x || lastY-1 != y) {
    if (x >= DRAWING_X) { x = DRAWING_X-1; }
    if (x < 0) { x= 0; }
    if (y > DRAWING_Y) { y = DRAWING_Y-1; }
    if (y < 0) { y= 0; }

    drawing[x+1][y+1]=c; 
    lastX = x+1;
    lastY = y+1; 
    //printf("Drawing area: %d %d\n",x, y);
    force_redraw();
    //queue_draw();
    //usleep(50);
  }
}
