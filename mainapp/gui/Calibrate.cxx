/*
  Copyright (c) 2016 Doobie (doobie.org)
  
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include "Calibrate.hxx"
#include "calib.hxx"
#include <iostream>

Calibrate::Calibrate(pegasus *peg) :
  Gtk::Box(Gtk::ORIENTATION_HORIZONTAL),
  pegPtr(peg),
  m_VBox1 (Gtk::ORIENTATION_VERTICAL),
  m_VBox2 (Gtk::ORIENTATION_VERTICAL),
  m_VBox3 (Gtk::ORIENTATION_VERTICAL),
  m_HBox1 (Gtk::ORIENTATION_HORIZONTAL),
 
  pt("1"),
  spd("16"),
  ptLbl("Point"),
  spdLbl("Speed"),
  prevPt("<"),
  nextPt(">"),
  slower("-"),
  faster("+"),
  up("up"),
  down("down"),
  left("left"),
  right("right"),
  ok("OK")
  /*
    m_ZHBox1( Gtk::ORIENTATION_HORIZONTAL),
    m_ZHBox2( Gtk::ORIENTATION_HORIZONTAL),
    m_ZVBox1( Gtk::ORIENTATION_VERTICAL),
    m_ZVBox2( Gtk::ORIENTATION_VERTICAL),
    m_ZVBox3( Gtk::ORIENTATION_VERTICAL),
    m_ZVBox4( Gtk::ORIENTATION_VERTICAL),
    zUp("Up"),
    zDown("Down"),
    zSpeed0("Speed 0"),
    zSpeed1("Speed 1"),
    zSpeed2("Speed 2"),
    zHome("Home"),
    zUp1CM("Up 1cm"),
    zDown1CM("Down 1cm"),
    zAdjUp("Adj+"),
    zAdjDown("Adj-"),
    zAdj1("Adj 1"),
    zAdj10("Adj 10"),
    zAdj100("Adj 100"),
    setBase("Set Layer 0"),
    zPos("Z=Need home or cal")*/

  //m_adjustment( Gtk::Adjustment::create(0.0, 0.0, 101.0, 0.1, 1.0, 1.0) ),
  //m_Pts(m_adjustment, Gtk::ORIENTATION_HORIZONTAL),
  //m_Scrollbar(m_adjustment)
{
  /*Gtk::RadioButton::Group group = zAdj1.get_group();
  zAdj10.set_group(group);
  zAdj100.set_group(group);
  Gtk::RadioButton::Group group2 = zSpeed0.get_group();
  zSpeed1.set_group(group2);
  zSpeed2.set_group(group2);*/
  Pt = 1;
  Spd = 16;
  set_border_width(1);
  add(m_HBox1);
  add(m_VBox1);
  add(m_VBox2);
  add(m_VBox3);

  m_VBox1.add(prevPt);
  m_VBox1.add(left);
  m_VBox1.add(slower);
  m_VBox1.add(ptLbl);
  m_VBox1.add(pt);

  m_VBox2.add(up);
  m_VBox2.add(ok);
  m_VBox2.add(down);

  m_VBox3.add(nextPt);
  m_VBox3.add(right);
  m_VBox3.add(faster);
  m_VBox3.add(spdLbl);
  m_VBox3.add(spd);
  
  prevPt.signal_clicked().connect(sigc::mem_fun(*this,
						&Calibrate::on_button_prev) );
  nextPt.signal_clicked().connect(sigc::mem_fun(*this,
						&Calibrate::on_button_next) );
  left.signal_clicked().connect(sigc::mem_fun(*this,
					      &Calibrate::on_button_left) );
  right.signal_clicked().connect(sigc::mem_fun(*this,
					       &Calibrate::on_button_right) );
  up.signal_clicked().connect(sigc::mem_fun(*this,
					    &Calibrate::on_button_up) );
  ok.signal_clicked().connect(sigc::mem_fun(*this,
					    &Calibrate::on_button_ok) );
  down.signal_clicked().connect(sigc::mem_fun(*this,
					      &Calibrate::on_button_down) );
  
  faster.signal_clicked().connect(sigc::mem_fun(*this,
						&Calibrate::on_button_faster) );
  
  slower.signal_clicked().connect(sigc::mem_fun(*this,
						&Calibrate::on_button_slower) );

}

void Calibrate::on_button_prev() { 
  std::cout << "prev button " << std::endl;
  char str[5];
  Pt--;
  if (Pt < 0) { Pt = PTS_X*PTS_Y; }
  sprintf(str,"%d", Pt);
  pt.set_text(str);
  pegPtr->calibData.adjustXY(XPT, Pt, 0, 1); // use this to show where the laser is
}

void Calibrate::on_button_next() {
  char str[5];
  std::cout << "next button " << std::endl;  
  Pt++;
  if (Pt >= PTS_X*PTS_Y ) { Pt = 0; }
  sprintf(str,"%d", Pt);
  pt.set_text(str);
  pegPtr->calibData.adjustXY(XPT, Pt, 0, 1); // use this to show where the laser is
}

void Calibrate::on_button_up() { 
  std::cout << "up button " << std::endl;
  //* Y--
  pegPtr->calibData.adjustXY(YPT, Pt, -Spd, 1);
}

void Calibrate::on_button_ok() { 
  std::cout << "ok button " << std::endl;
  pegPtr->calibData.adjustXY(XPT, Pt, 0, 1); // use this to show where the laser is
}

void Calibrate::on_button_down() {
  //* Y++
  std::cout << "down button " << std::endl;  
  pegPtr->calibData.adjustXY(YPT, Pt, +Spd, 1);
}
void Calibrate::on_button_left() { 
  //* X--
  std::cout << "left button " << std::endl;
  pegPtr->calibData.adjustXY(XPT, Pt, -Spd, 1);
}
void Calibrate::on_button_right() {
  //* X++
  std::cout << "right button " << std::endl;  
  pegPtr->calibData.adjustXY(XPT, Pt, +Spd, 1);
}
void Calibrate::on_button_slower() { 
  std::cout << "slower button " << std::endl;
  char str[5];
  Spd/=2;
  if (Spd < 1) { Spd = 1; }
  sprintf(str,"%d", Spd);
  spd.set_text(str);
}
void Calibrate::on_button_faster() {
  std::cout << "faster button " << std::endl;  
  char str[5];
  Spd*=2;
  if (Spd > 128) { Spd = 128; }
  sprintf(str,"%d", Spd);
  spd.set_text(str);
}

void Calibrate::on_change_point_value_changed() {
  //const double val = m_adjustment->get_value();
  //std::cout << "Slider moved to " << val << std::endl;
}

Calibrate::~Calibrate() {

}
