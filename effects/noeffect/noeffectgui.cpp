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

#include "noeffect.h"
#include "noeffectgui.h"

namespace Ms {

//---------------------------------------------------------
//   gui
//---------------------------------------------------------

EffectGui* NoEffect::gui()
      {
      if (_gui)
            return _gui;
      _gui = new NoEffectGui(this);
      _gui->setGeometry(0, 0, 644, 79);
      return _gui;
      }

//---------------------------------------------------------
//   NoEffectGui
//---------------------------------------------------------

NoEffectGui::NoEffectGui(NoEffect* effect, QWidget* parent)
   : EffectGui(effect, parent)
      {

      }







}

