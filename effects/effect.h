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

#ifndef __EFFECT_H__
#define __EFFECT_H__

#include <vector>
#include "libmscore/synthesizerstate.h"

namespace Ms {

class EffectGui;

//---------------------------------------------------------
//   ParDescr
//    parameter description
//---------------------------------------------------------

struct ParDescr {
      int id;                 // index in parameter list
      const char* name;
      bool log;
      float min;
      float max;
      float init;
      };

//---------------------------------------------------------
//   Effect
//---------------------------------------------------------

class Effect : public QObject {
      Q_OBJECT

   protected:
      EffectGui* _gui { nullptr };

   public slots:
      virtual void setValue(const QString& name, double value);
      virtual void setValue(int idx, double value);
      virtual void setNValue(int idx, double value) = 0;

   public:
      Effect() : QObject() { }
      virtual ~Effect() {}
      virtual void process(int frames, float*, float*) = 0;
      virtual const char* name() const = 0;
      virtual void init(float /*sampleRate*/) {}
      virtual const std::vector<ParDescr>& parDescr() const = 0;

      Q_INVOKABLE qreal value(const QString& name) const;
      virtual double value(int idx) const;
      virtual double nvalue(int /*idx*/) const { return .0; }

      virtual const ParDescr* parameter(int idx) const;
      virtual const ParDescr* parameter(const QString&) const;

      virtual SynthesizerGroup state() const { return SynthesizerGroup(); }
      virtual void setState(const SynthesizerGroup&) {}

      virtual EffectGui* gui() { return _gui; }
      };
}
#endif

