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
      }

//---------------------------------------------------------
//   synthesizerChanged
//---------------------------------------------------------

void AeolusGui::synthesizerChanged()
      {
      }


