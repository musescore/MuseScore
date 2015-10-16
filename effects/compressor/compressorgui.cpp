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
            _gui->setGeometry(0, 0, 640, 79);
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
      connect(cg.rms,        SIGNAL(valueChanged(double,int)),                SLOT(valueChanged(double,int)));
      connect(cg.rms,        SIGNAL(valueChanged(double,int)), cg.rmsSpinBox, SLOT(setValue(double)));
      connect(cg.rmsSpinBox, SIGNAL(valueChanged(double)),     cg.rms,        SLOT(setValue(double)));

      connect(cg.attack,        SIGNAL(valueChanged(double,int)),                   SLOT(valueChanged(double,int)));
      connect(cg.attack,        SIGNAL(valueChanged(double,int)), cg.attackSpinBox, SLOT(setValue(double)));
      connect(cg.attackSpinBox, SIGNAL(valueChanged(double)),     cg.attack,        SLOT(setValue(double)));

      connect(cg.release,        SIGNAL(valueChanged(double,int)),                    SLOT(valueChanged(double,int)));
      connect(cg.release,        SIGNAL(valueChanged(double,int)), cg.releaseSpinBox, SLOT(setValue(double)));
      connect(cg.releaseSpinBox, SIGNAL(valueChanged(double)),     cg.release,        SLOT(setValue(double)));

      connect(cg.threshold,        SIGNAL(valueChanged(double,int)),                      SLOT(valueChanged(double,int)));
      connect(cg.threshold,        SIGNAL(valueChanged(double,int)), cg.thresholdSpinBox, SLOT(setValue(double)));
      connect(cg.thresholdSpinBox, SIGNAL(valueChanged(double)),     cg.threshold,        SLOT(setValue(double)));

      connect(cg.ratio,        SIGNAL(valueChanged(double,int)),                  SLOT(valueChanged(double,int)));
      connect(cg.ratio,        SIGNAL(valueChanged(double,int)), cg.ratioSpinBox, SLOT(setValue(double)));
      connect(cg.ratioSpinBox, SIGNAL(valueChanged(double)),     cg.ratio,        SLOT(setValue(double)));

      connect(cg.knee,        SIGNAL(valueChanged(double,int)),                 SLOT(valueChanged(double,int)));
      connect(cg.knee,        SIGNAL(valueChanged(double,int)), cg.kneeSpinBox, SLOT(setValue(double)));
      connect(cg.kneeSpinBox, SIGNAL(valueChanged(double)),     cg.knee,        SLOT(setValue(double)));

      connect(cg.gain,        SIGNAL(valueChanged(double,int)),                 SLOT(valueChanged(double,int)));
      connect(cg.gain,        SIGNAL(valueChanged(double,int)), cg.gainSpinBox, SLOT(setValue(double)));
      connect(cg.gainSpinBox, SIGNAL(valueChanged(double)),     cg.gain,        SLOT(setValue(double)));
      }

//---------------------------------------------------------
// updateValues
//---------------------------------------------------------

void CompressorGui::updateValues()
      {
      cg.rmsSpinBox->setValue(effect()->nvalue(0));
      cg.attackSpinBox->setValue(effect()->nvalue(1));
      cg.releaseSpinBox->setValue(effect()->nvalue(2));
      cg.thresholdSpinBox->setValue(effect()->nvalue(3));
      cg.ratioSpinBox->setValue(effect()->nvalue(4));
      cg.kneeSpinBox->setValue(effect()->nvalue(5));
      cg.gainSpinBox->setValue(effect()->nvalue(6));
      }

#include "compressorgui.moc"

}

