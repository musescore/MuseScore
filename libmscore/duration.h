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

   public:
      DurationElement(Score* = 0, ElementFlags = ElementFlag::MOVABLE | ElementFlag::ON_STAFF);
      DurationElement(const DurationElement& e);
      ~DurationElement();

      virtual Measure* measure() const    { return (Measure*)(parent()); }

      void readAddTuplet(Tuplet* t);
      void writeTupletStart(XmlWriter& xml) const;
      void writeTupletEnd(XmlWriter& xml) const;

      void setTuplet(Tuplet* t)           { _tuplet = t;      }
      Tuplet* tuplet() const              { return _tuplet;   }
      Tuplet* topTuplet() const;
      virtual Beam* beam() const          { return 0;         }

      Fraction actualTicksAt(const Fraction& tick) const;
      Fraction actualTicks() const;

      //Length expressed as a fraction of a whole note
      virtual Fraction ticks() const { return _duration; }
      Fraction globalTicks() const;
      void setTicks(const Fraction& f) { _duration = f;    }

      virtual QVariant getProperty(Pid propertyId) const override;
      virtual bool setProperty(Pid propertyId, const QVariant&) override;
      };


}     // namespace Ms
#endif

