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

#include "synthesizergui.h"
#include "synthesizer.h"

//---------------------------------------------------------
//   SynthesizerGui
//---------------------------------------------------------

SynthesizerGui::SynthesizerGui()
   : QWidget(0)
      {
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void SynthesizerGui::init(Synthesizer* s, double sr)
      {
      _synthesizer = s;
      if (_synthesizer)
            _synthesizer->init(sr);
      }
