/*
  Copyright (c) 2016 Doobie (doobie.org)
  
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include "CalibrateZ.hxx"
#include <string>     // std::string, std::to_string
#include <iostream>

CalibrateZ::CalibrateZ(pegasus *peg) :
  Gtk::Box(Gtk::ORIENTATION_HORIZONTAL),
  pegPtr(peg),
  m_ZHBox1( Gtk::ORIENTATION_HORIZONTAL),
  m_ZHBox2( Gtk::ORIENTATION_HORIZONTAL),
  m_ZVBox1( Gtk::ORIENTATION_VERTICAL),
  m_ZVBox2( Gtk::ORIENTATION_VERTICAL),
  m_ZVBox3( Gtk::ORIENTATION_VERTICAL),
  m_ZVBox4( Gtk::ORIENTATION_VERTICAL),
  m_ZWarn("Careful moving down/up\nTo ensure you don't\nexceed the limits\nof the printer.\n\n"),
  zUp("Up"),
  zDown("Down"),
  zSpeed0("Speed F"),
  zSpeed1("Speed M"),
  zSpeed2("Speed S"),
  zHome("Home"),
  zUp1CM("Up 1cm"),
  zDown1CM("Down 1cm"),
  zAdjUp("Adj+"),
  zAdjDown("Adj-"),
  zAdj1("Adj 1"),
  zAdj10("Adj 10"),
  zAdj100("Adj 100"),
  setBase("Set Layer 0"),
  zPos("Z=Need to hit hime"),
  haveHome(false)

  //m_adjustment( Gtk::Adjustment::create(0.0, 0.0, 101.0, 0.1, 1.0, 1.0) ),
  //m_Pts(m_adjustment, Gtk::ORIENTATION_HORIZONTAL),
  //m_Scrollbar(m_adjustment)
{
  Gtk::RadioButton::Group group = zAdj1.get_group();
  zAdj10.set_group(group);
  zAdj100.set_group(group);
  Gtk::RadioButton::Group group2 = zSpeed0.get_group();
  zSpeed1.set_group(group2);
  zSpeed2.set_group(group2);
  
  set_border_width(1);

  add(m_ZHBox1);
  
  add(m_ZHBox2);

  m_ZHBox2.add(m_ZVBox1);
  m_ZHBox2.add(m_ZVBox2);
  m_ZHBox2.add(m_ZVBox3);
  m_ZHBox2.add(m_ZVBox4);

  m_ZVBox1.add(m_ZWarn);
  m_ZVBox1.add(zPos);
  
  m_ZVBox2.add(zUp);
  m_ZVBox2.add(zDown);
  m_ZVBox2.add(zSpeed0);
  m_ZVBox2.add(zSpeed1);
  m_ZVBox2.add(zSpeed2);

  m_ZVBox3.add(zHome);
  m_ZVBox3.add(zUp1CM);
  m_ZVBox3.add(zDown1CM);
  m_ZVBox3.add(setBase);
  
  m_ZVBox4.add(zAdjUp);
  m_ZVBox4.add(zAdjDown);

  m_ZVBox4.add(zAdj1);
  m_ZVBox4.add(zAdj10);
  m_ZVBox4.add(zAdj100);
  
  
  zUp.signal_clicked().connect(sigc::mem_fun(*this, &CalibrateZ::on_button_up));
  zDown.signal_clicked().connect(sigc::mem_fun(*this, &CalibrateZ::on_button_down));
  
  zUp1CM.signal_clicked().connect(sigc::mem_fun(*this, &CalibrateZ::on_button_up1cm));
  zDown1CM.signal_clicked().connect(sigc::mem_fun(*this, &CalibrateZ::on_button_down1cm));

  zHome.signal_clicked().connect(sigc::mem_fun(*this, &CalibrateZ::on_button_home)); 

  setBase.signal_clicked().connect(sigc::mem_fun(*this, &CalibrateZ::on_button_setlayer0)); 

  zAdjUp.signal_clicked().connect(sigc::mem_fun(*this, &CalibrateZ::on_button_adjPlus)); 

  zAdjDown.signal_clicked().connect(sigc::mem_fun(*this, &CalibrateZ::on_button_adjMinus)); 
 
  //updateStatus();
}

void CalibrateZ::updateStatus() {
  int zPosPt;
  int Adj;
  float distance;
  char strMsg[48];

  Adj = pegPtr->calibData.getZScale();
  zPosPt = pegPtr->zaxisControl.getZ();
  distance = (float)zPosPt / (float)Adj;
  printf("Adj=%d, zP=%d\n", Adj, zPosPt);
  snprintf(strMsg, 48, "Calib Z=%d\nZ @%f mm\nSteps/MM= =%d\nSteps=%d", pegPtr->calibData.getBelowLimit(), distance, Adj, zPosPt);
  zPos.set_text(strMsg);

  //zPos.invalidate ();
  //zPos.update ();
}

void CalibrateZ::on_button_home() { 
  std::cout << "home button " << std::endl;
  pegPtr->zaxisControl.moveZHome();
  updateStatus();
}
void CalibrateZ::on_button_setlayer0() {
  std::cout << "setlayer0 button " << std::endl;  
  pegPtr->zaxisControl.setZ(0);
  updateStatus();
}
void CalibrateZ::on_button_up() {
  int steps = zAdj1.get_active() ? 1 :
    zAdj10.get_active() ? 10 :
    zAdj100.get_active() ? 100 : 0;
  std::cout << "up button " << std::endl;
  pegPtr->zaxisControl.moveZ(steps, 1, 1, 1.0);
  updateStatus();
}

void CalibrateZ::on_button_down() {
  std::cout << "down button " << std::endl;  
  int steps = zAdj1.get_active() ? 1 :
    zAdj10.get_active() ? 10 :
    zAdj100.get_active() ? 100 : 0;
  std::cout << "up button " << std::endl;
  pegPtr->zaxisControl.moveZ(-steps, 1, 1, 1.0);
  updateStatus();
}

void CalibrateZ::on_button_up1cm() { 
  int steps = 10 * pegPtr->calibData.getScale(2,1);
  std::cout << "up 1cm button " << std::endl;
  pegPtr->zaxisControl.moveZ(steps, 1, 1, 1.0);
  updateStatus();
}

void CalibrateZ::on_button_down1cm() {
  int steps = 10 * pegPtr->calibData.getScale(2,1);
  std::cout << "down 1cm button " << std::endl;  
  updateStatus();
  pegPtr->zaxisControl.moveZ(-steps, 1, 1, 1.0);
}

void CalibrateZ::on_button_adjPlus() {
  std::cout << "adjPlus button " << std::endl;  
  int steps = zAdj1.get_active() ? 1 :
    zAdj10.get_active() ? 10 :
    zAdj100.get_active() ? 100 : 0;
  int newScale = pegPtr->calibData.getScale(2, 1) + steps;
  if (newScale > 1000) {
    newScale = 1000;
  }
  pegPtr->calibData.setScale(2, newScale);
  updateStatus();
}

void CalibrateZ::on_button_adjMinus() {
  std::cout << "adjMinus button " << std::endl;  
  int steps = zAdj1.get_active() ? 1 :
    zAdj10.get_active() ? 10 :
    zAdj100.get_active() ? 100 : 0;
  int newScale = pegPtr->calibData.getScale(2, 1) - steps;
  if (newScale < 10) {
    newScale = 10;
  }  
  pegPtr->calibData.setScale(2, newScale);
  updateStatus();
}

CalibrateZ::~CalibrateZ() {

}
