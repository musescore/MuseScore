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

#include "zita.h"
#include "effects/effectgui.h"

//---------------------------------------------------------
//   gui
//---------------------------------------------------------

EffectGui* ZitaReverb::gui()
      {
      if (_gui)
            return _gui;
      _gui = new EffectGui;
      _gui->setGeometry(0, 0, 644, 79);
      _gui->setEffect(this);
      QUrl url("qrc:/effects/zita1/zita.qml");
      _gui->init(url, 44100);
      return _gui;
      }
