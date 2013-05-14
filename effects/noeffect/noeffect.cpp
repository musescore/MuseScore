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

namespace Ms {

static const std::vector<ParDescr> noeffectPd;

//---------------------------------------------------------
//   gui
//---------------------------------------------------------

EffectGui* NoEffect::gui()
      {
      EffectGui* eg = new EffectGui(this);
      QUrl url("qrc:/noeffect/noeffect.qml");
      eg->init(url);
      return eg;
      }

//---------------------------------------------------------
//   parDescr
//---------------------------------------------------------

const std::vector<ParDescr>& NoEffect::parDescr() const
      {
      return noeffectPd;
      }

void NoEffect::process(int n, float* src, float* dst)
      {
      memcpy(dst, src, n * 2 * sizeof(float));
      }

}

