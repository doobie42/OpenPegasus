/*
  Copyright (c) 2016 Doobie (doobie.org)
  
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef CALIBRATEZ_HXX
#define CALIBRATEZ_HXX

#include <gtkmm.h>
#include <gtkmm/button.h>
#include <gtkmm/window.h>
#include "general/pegasus.hxx"

class CalibrateZ : public Gtk::Box {
public:
  CalibrateZ(pegasus *peg);
  virtual ~CalibrateZ();

protected:
  // Handlers
  void on_button_up();
  void on_button_down();
  void on_button_home();
  void on_button_up1cm();
  void on_button_down1cm();
  void on_button_setlayer0();
  void on_button_adjPlus();
  void on_button_adjMinus();
  void updateStatus();

  pegasus *pegPtr;
  // Features

  Gtk::Box m_ZHBox1;
  Gtk::Box m_ZHBox2;
  Gtk::Box m_ZVBox1;
  Gtk::Box m_ZVBox2;
  Gtk::Box m_ZVBox3;
  Gtk::Box m_ZVBox4;
  Gtk::Label m_ZWarn;
  Gtk::Button zUp, zDown;
  Gtk::RadioButton zSpeed0, zSpeed1, zSpeed2;
  Gtk::Button zHome;
  Gtk::Button zUp1CM, zDown1CM, zAdjUp, zAdjDown;
  Gtk::RadioButton zAdj1, zAdj10, zAdj100;
  //  Gtk::RadioButtonGroup zAdjGrp;
  Gtk::Button setBase;
  Gtk::Label zPos;
  //Gtk::Scale m_Pts;
  //Glib::RefPtr<Gtk::Adjustment> m_adjustment;
  //Gtk::Scrollbar m_Scrollbar;
  bool haveHome;
  
};

#endif
