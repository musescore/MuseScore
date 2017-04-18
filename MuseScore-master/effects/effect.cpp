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

#include "effect.h"

namespace Ms {

//---------------------------------------------------------
//   parameter
//---------------------------------------------------------

const ParDescr* Effect::parameter(int idx) const
      {
      return &parDescr()[idx];
      }

const ParDescr* Effect::parameter(const QString& name) const
      {
      for (const ParDescr& pd : parDescr()) {
            if (name == pd.name)
                  return &pd;
            }
      return 0;
      }

void Effect::setValue(const QString& name, double value)
      {
      const ParDescr* p = parameter(name);
      if (p == 0)
            return;
      setValue(p->id, value);
      }

void Effect::setValue(int id, double value)
      {
      const ParDescr* p = parameter(id);
      if (p == 0)
            return;
      double v;
      if (p->log)
            v = exp(p->min + value * (p->max - p->min));
      else
            v = p->min + value * (p->max - p->min);

      setNValue(p->id, v);
      }

//---------------------------------------------------------
//   value
//    return normalized value 0-1.0
//---------------------------------------------------------

double Effect::value(const QString& name) const
      {
      const ParDescr* p = parameter(name);
      if (p == 0)
            return 0.0;
      return value(p->id);
      }

double Effect::value(int idx) const
      {
      double v = nvalue(idx);
      const ParDescr* p = parameter(idx);
      if (p->log)
            v = (log(v) - p->min)/(p->max - p->min);
      else
            v = (v - p->min)/(p->max - p->min);
      return v;
      }
}

