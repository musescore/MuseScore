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

#include "freeverb.h"
#include "effects/effectgui.h"

//---------------------------------------------------------
//   gui
//---------------------------------------------------------

EffectGui* Freeverb::gui()
      {
      EffectGui* eg = new EffectGui(this);
      QUrl url("qrc:/freeverb/freeverb.qml");
      eg->init(url);
      return eg;
      }

