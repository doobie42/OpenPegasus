/*
  Copyright (c) 2016 Doobie (doobie.org)
  
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef NO_GUI
#include <gtkmm.h>
#endif
#include <gui/MainApp.hxx>
#include "general/pegasus.hxx"


int main(int argc, char *argv[])
{
    printf("====== IN main.cxx ======");
  if (argc == 3) {
    pegasus peg
#ifndef NO_GUI
		(NULL)
#endif
      ;
    peg.calibData.readCalib(argv[1], 0);
    peg.gcodeProcessor.SetRunReal(1);
    if (strcmp(argv[2], "z") == 0) {
      peg.zaxisControl.tuneZ();
    } else if (strcmp(argv[2], "zup") == 0) {
      peg.zaxisControl.moveZ(3000,0,0,1);
        printf(peg.zaxisControl.isZLimit());

    } else if (strcmp(argv[2], "zcal") == 0) {
      peg.zaxisControl.zCalib();
    }else if (strcmp(argv[2], "galvo") == 0) {
      peg.galvoControl.tuneGalvo();
    } else {
      peg.gcodeProcessor.openFile(argv[2]);
    }
  } else {
#ifndef NO_GUI
    Glib::RefPtr<Gtk::Application> app =
      Gtk::Application::create(argc, argv,
			       "org.doobie.pegasus.touch");
    
    MainApp mainwin;
    mainwin.set_default_size(480, 250);
    
    return app->run(mainwin);
#endif
      
  }
}
