//=============================================================================
//  MuseSynth
//  Music Software Synthesizer
//
//  Copyright (C) 2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "compressor.h"
#include "effects/effectgui.h"
#include "ui_compressor_gui.h"

namespace Ms {

//---------------------------------------------------------
//   gui
//---------------------------------------------------------

class CompressorGui : public EffectGui {
      Q_OBJECT

      Ui::CompressorGui cg;

      virtual void updateValues();

   public:
      CompressorGui(Compressor*, QWidget* parent = 0);
      };

//---------------------------------------------------------
//   gui
//---------------------------------------------------------

EffectGui* Compressor::gui()
      {
      if (!_gui) {
            _gui = new CompressorGui(this);
            _gui->setGeometry(0, 0, 644, 79);
            }
      return _gui;
      }

//---------------------------------------------------------
//    CompressorGui
//---------------------------------------------------------

CompressorGui::CompressorGui(Compressor* effect, QWidget* parent)
   : EffectGui(effect, parent)
      {
      cg.setupUi(this);
      connect(cg.rms,       SIGNAL(valueChanged(double,int)), SLOT(valueChanged(double,int)));
      connect(cg.attack,    SIGNAL(valueChanged(double,int)), SLOT(valueChanged(double,int)));
      connect(cg.release,   SIGNAL(valueChanged(double,int)), SLOT(valueChanged(double,int)));
      connect(cg.threshold, SIGNAL(valueChanged(double,int)), SLOT(valueChanged(double,int)));
      connect(cg.ratio,     SIGNAL(valueChanged(double,int)), SLOT(valueChanged(double,int)));
      connect(cg.knee,      SIGNAL(valueChanged(double,int)), SLOT(valueChanged(double,int)));
      connect(cg.gain,      SIGNAL(valueChanged(double,int)), SLOT(valueChanged(double,int)));
      }

//---------------------------------------------------------
// updateValues
//---------------------------------------------------------

void CompressorGui::updateValues()
      {
      cg.rms->setValue(effect()->nvalue(0));
      cg.attack->setValue(effect()->nvalue(1));
      cg.release->setValue(effect()->nvalue(2));
      cg.threshold->setValue(effect()->nvalue(3));
      cg.ratio->setValue(effect()->nvalue(4));
      cg.knee->setValue(effect()->nvalue(5));
      cg.gain->setValue(effect()->nvalue(6));
      }

#include "compressorgui.moc"

}

