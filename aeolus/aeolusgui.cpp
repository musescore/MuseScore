//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "aeolusgui.h"
#include "aeolus.h"
#include "mscore/preferences.h"

//---------------------------------------------------------
//   gui
//---------------------------------------------------------

SynthesizerGui* Aeolus::gui()
      {
      if (_gui == 0)
            _gui = new AeolusGui(this);
      return _gui;
      }

//---------------------------------------------------------
//   AeolusGui
//---------------------------------------------------------

AeolusGui::AeolusGui(Synthesizer* s)
   : SynthesizerGui(s)
      {
      setupUi(this);

      reverbDelay->setId(A_REVSIZE);
      reverbTime->setId(A_REVTIME);
      position->setId(A_STPOSIT);

      connect (reverbDelay, SIGNAL(valueChanged(double,int)), SLOT(valueHasChanged(double,int)));
      connect (reverbTime,  SIGNAL(valueChanged(double,int)), SLOT(valueHasChanged(double,int)));
      connect (position,    SIGNAL(valueChanged(double,int)), SLOT(valueHasChanged(double,int)));
      }

//---------------------------------------------------------
//   synthesizerChanged
//---------------------------------------------------------

void AeolusGui::synthesizerChanged()
      {
      reverbDelay->setValue(synthesizer()->value(A_REVSIZE));
      reverbTime->setValue(synthesizer()->value(A_REVTIME));
      position->setValue(synthesizer()->value(A_STPOSIT));
      }

//---------------------------------------------------------
//   valueHasChanged
//---------------------------------------------------------

void AeolusGui::valueHasChanged(double val, int id)
      {
      synthesizer()->setValue(id, val);
      emit valueChanged();
      }

