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

#include "effects/effect.h"
#include "effects/effectgui.h"
#include "noeffect.h"

static const std::vector<ParDescr> noeffectPd;

//---------------------------------------------------------
//   gui
//---------------------------------------------------------

EffectGui* NoEffect::gui()
      {
      EffectGui* eg = new EffectGui;
      eg->setEffect(this);
      QUrl url;
      eg->init(url, 44100.0);
      return eg;
      }

//---------------------------------------------------------
//   parDescr
//---------------------------------------------------------

const std::vector<ParDescr>& NoEffect::parDescr() const
      {
      return noeffectPd;
      }


