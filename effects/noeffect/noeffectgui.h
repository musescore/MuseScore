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

#ifndef __NOEFFECTGUI_H__
#define __NOEFFECTGUI_H__

#include "effects/effectgui.h"

namespace Ms {

class NoEffect;

//---------------------------------------------------------
//   gui
//---------------------------------------------------------

class NoEffectGui : public EffectGui {
      Q_OBJECT

   public:
      NoEffectGui(NoEffect*, QWidget* parent = 0);
      };
}

#endif


