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

#include "element.h"
#include "durationtype.h"

class Tuplet;
class Beam;
class Spanner;

//---------------------------------------------------------
//   @@ DurationElement
///   Virtual base class for Chord, Rest and Tuplet.
//
//   @P duration int     duration in ticks
//---------------------------------------------------------

class DurationElement : public Element {
      Q_OBJECT
      Q_PROPERTY(int duration READ durationTicks WRITE setDuration);

      Fraction _duration;
      Tuplet* _tuplet;

   public:
      DurationElement(Score* s);
      DurationElement(const DurationElement& e);
      ~DurationElement();

      virtual Measure* measure() const    { return (Measure*)(parent()); }

      bool readProperties(XmlReader& e);
      void writeProperties(Xml& xml) const;
      void writeTuplet(Xml& xml);

      void setTuplet(Tuplet* t)           { _tuplet = t;      }
      Tuplet* tuplet() const              { return _tuplet;   }
      virtual Beam* beam() const          { return 0;         }
      virtual int tick() const = 0;
      int actualTicks() const;

      virtual Fraction duration() const   { return _duration; }
      Fraction globalDuration() const;
      void setDuration(const Fraction& f) { _duration = f;    }
      void setDuration(int ticks)         { _duration = Fraction::fromTicks(ticks); }
      int durationTicks() const           { return _duration.ticks(); }
      };

#endif

