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

#ifndef __NOEFFECT_H__
#define __NOEFFECT_H__

#include "effects/effect.h"

class EffectGui;

//---------------------------------------------------------
//   NoEffect
//---------------------------------------------------------

class NoEffect : public Effect
      {
      Q_OBJECT

   public:
      NoEffect() {}

      virtual void init(float) {}
      virtual void process(int, float*, float*);

      virtual void setValue(int, double) {}
      virtual void setNValue(int, double) {}
      virtual double value(int) const { return 0.0; }
      virtual double nvalue(int) const { return 0.0; }

      virtual const char* name() const { return "NoEffect"; }
      virtual EffectGui* gui();
      virtual const std::vector<ParDescr>& parDescr() const;
      };

#endif

