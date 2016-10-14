/*
  Copyright (c) 2016 Doobie (doobie.org)
  
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef CALIBRATE_HXX
#define CALIBRATE_HXX

#include <gtkmm.h>
#include <gtkmm/button.h>
#include <gtkmm/window.h>
#include "general/pegasus.hxx"

class Calibrate : public Gtk::Box {
public:
  Calibrate(pegasus *peg);
  virtual ~Calibrate();

protected:
  // Handlers
  void on_change_point_value_changed();
  void on_button_prev();
  void on_button_next();
  void on_button_slower();
  void on_button_faster();
  void on_button_ok();
  void on_button_up();
  void on_button_down();
  void on_button_left();
  void on_button_right();
  
  /*void on_button_zUp();
  void on_button_zDown();

  void on_button_zSpeed0();
  void on_button_zSpeed1();
  void on_button_zSpeed2();
  void on_button_zHome();
  void on_button_zUp1CM();
  void on_button_zDown1CM();
  void on_button_setBase();*/
  
  
  pegasus *pegPtr;
  // Features
  Gtk::Box m_VBox1, m_VBox2, m_VBox3, m_HBox1;
  Gtk::Label pt, spd;
  Gtk::Label ptLbl, spdLbl;
  Gtk::Button prevPt, nextPt, slower, faster;
  
  Gtk::Button up, down, left, right;
  Gtk::Button ok;

  /*
    Gtk::Box m_ZHBox1;
  Gtk::Box m_ZHBox2;
  Gtk::Box m_ZVBox1;
  Gtk::Box m_ZVBox2;
  Gtk::Box m_ZVBox3;
  Gtk::Box m_ZVBox4;
  Gtk::Button zUp, zDown;
  Gtk::RadioButton zSpeed0, zSpeed1, zSpeed2;
  Gtk::Button zHome;
  Gtk::Button zUp1CM, zDown1CM, zAdjUp, zAdjDown;
  Gtk::RadioButton zAdj1, zAdj10, zAdj100;
  //  Gtk::RadioButtonGroup zAdjGrp;
  Gtk::Button setBase;
  Gtk::Label zPos;
  */
  //Gtk::Scale m_Pts;
  //Glib::RefPtr<Gtk::Adjustment> m_adjustment;
  //Gtk::Scrollbar m_Scrollbar;
  int Pt;
  int Spd;
};

#endif
