//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2008-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __DURATION_H__
#define __DURATION_H__

#include "config.h"
#include "element.h"
#include "durationtype.h"

namespace Ms {

class Tuplet;
class Beam;
class Spanner;

//---------------------------------------------------------
//   @@ DurationElement
///    Virtual base class for Chord, Rest and Tuplet.
//
//   @P duration       Fraction  duration (as written)
//   @P globalDuration Fraction  played duration
//---------------------------------------------------------

class DurationElement : public Element {
      Fraction _duration;
      Tuplet* _tuplet;

#ifdef SCRIPT_INTERFACE
      Q_OBJECT
      Q_PROPERTY(FractionWrapper* duration READ durationW WRITE setDurationW)
      Q_PROPERTY(FractionWrapper* globalDuration READ globalDurW)

      void setDurationW(FractionWrapper* f)  { _duration = f->fraction(); }
      FractionWrapper* durationW() const     { return new FractionWrapper(_duration); }
      FractionWrapper* globalDurW() const    { return new FractionWrapper(globalDuration()); }
#endif

   public:
      DurationElement(Score* s);
      DurationElement(const DurationElement& e);
      ~DurationElement();

      virtual Measure* measure() const    { return (Measure*)(parent()); }

      virtual bool readProperties(XmlReader& e);
      virtual void writeProperties(Xml& xml) const;
      void writeTuplet(Xml& xml);

      void setTuplet(Tuplet* t)           { _tuplet = t;      }
      Tuplet* tuplet() const              { return _tuplet;   }
      virtual Beam* beam() const          { return 0;         }
      int actualTicks() const;
      Fraction actualFraction() const;

      virtual Fraction duration() const   { return _duration; }
      Fraction globalDuration() const;
      void setDuration(const Fraction& f) { _duration = f;    }

      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      };


}     // namespace Ms
#endif

