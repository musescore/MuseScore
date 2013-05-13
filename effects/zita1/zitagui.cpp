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

namespace Ms {

//---------------------------------------------------------
//   gui
//---------------------------------------------------------

EffectGui* ZitaReverb::gui()
      {
      if (_gui)
            return _gui;
      _gui = new EffectGui(this);
      _gui->setGeometry(0, 0, 644, 79);
      QUrl url("qrc:/zita1/zita.qml");
      _gui->init(url);
      return _gui;
      }

}

