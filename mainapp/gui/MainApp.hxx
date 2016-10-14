/*
  Copyright (c) 2016 Doobie (doobie.org)
  
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef MAINAPP_H
#define MAINAPP_H

#include "Calibrate.hxx"
#include "CalibrateZ.hxx"
#include "Settings.hxx"
#include "File.hxx"
#include "general/pegasus.hxx"
#include <gtkmm.h>
#include <gtkmm/button.h>
#include <gtkmm/window.h>

class MainApp : public Gtk::Window
{
public:
  MainApp();
  virtual ~MainApp();
  File *getFile() { return &mFile; }
protected:
  //Child widgets:
  //Gtk::Box m_HBox;
  //Gtk::Box m_VBox, m_VBox2, m_VBox3;
  //Gtk::Frame m_Frame_Normal, m_Frame_Multi, m_Frame_Left, m_Frame_Right,
  //m_Frame_LineWrapped, m_Frame_FilledWrapped, m_Frame_Underlined;
  //Gtk::Label m_Label_Normal, m_Label_Multi, m_Label_Left, m_Label_Right,
  //m_Label_LineWrapped, m_Label_FilledWrapped, m_Label_Underlined;

  void on_notebook_switch_page(Gtk::Widget* page, guint page_num);
  //Child widgets:
  Gtk::Box m_VBox;
  Gtk::Notebook m_Notebook;
  Gtk::Label  m_Label2;

  Calibrate mCalibrateGalvo;
  CalibrateZ mCalibrateZ;
  File mFile;
  Settings mSetting;
  //  Gtk::ButtonBox m_ButtonBox;
  //Gtk::Button m_Button_Quit;
  pegasus Peg;
};

#endif // MAINAPP_H
