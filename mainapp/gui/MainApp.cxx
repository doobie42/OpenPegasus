/*
  Copyright (c) 2016 Doobie (doobie.org)
  
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include "MainApp.hxx"
#include <iostream>

MainApp::MainApp()
  :
  m_VBox(Gtk::ORIENTATION_VERTICAL),
  //m_Label1("Contents of tab 1"),
  m_Label2("Contents of tab 2"),
  //m_Label3("Contents of tab 3"),
  mCalibrateGalvo(&Peg),
  mCalibrateZ(&Peg),
  mFile(&Peg),
  Peg(this)
  //m_Button_Quit("Quit")

{
  set_title("Open Pegasus Touch");
  set_border_width(1);
  //if (!g_thread_supported ()){ g_thread_init(NULL); }
  //gdk_threads_init();
  //gdk_threads_enter();

  set_default_size(400, 200);
  Peg.gcodeProcessor.setGui(this);

  add(m_VBox);

  //Add the Notebook, with the button underneath:
  m_Notebook.set_border_width(10);
  m_VBox.pack_start(m_Notebook);
  //m_VBox.pack_start(m_ButtonBox, Gtk::PACK_SHRINK);

  //m_ButtonBox.pack_start(m_Button_Quit, Gtk::PACK_SHRINK);
  //m_Button_Quit.signal_clicked().connect(sigc::mem_fun(*this,
  //&MainApp::on_button_quit) );

  //Add the Notebook pages:
  m_Notebook.append_page(mFile, "Files");
  //m_Notebook.append_page(mSetting, "Settings");
  m_Notebook.append_page(m_Label2, "Settings");
  m_Notebook.append_page(mCalibrateGalvo, "Cal-Galvo");
  m_Notebook.append_page(mCalibrateZ, "Cal-ZAxis");
  
  m_Notebook.signal_switch_page().connect(sigc::mem_fun(*this,
							&MainApp::on_notebook_switch_page) );
  
  printf("Reading calib.txt\n");
  Peg.calibData.readCalib("calib.txt", 0);
  printf("Results of reading calib.txt\n");
  Peg.calibData.printCalib(stdout);
  show_all_children();

}

MainApp::~MainApp()
{
}

//void MainApp::on_button_quit()
//{
//hide();
//}

void MainApp::on_notebook_switch_page(Gtk::Widget* /* page */, guint page_num)
{
  std::cout << "Switched to tab with index " << page_num << std::endl;

  //You can also use m_Notebook.get_current_page() to get this index.
}
