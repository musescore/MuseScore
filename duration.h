//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: duration.h 5151 2011-12-29 09:10:31Z wschweer $
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
//   DurationElement
//---------------------------------------------------------

class DurationElement : public Element {
      Fraction _duration;
      Tuplet* _tuplet;

   public:
      DurationElement(Score* s);
      DurationElement(const DurationElement& e);
      ~DurationElement();

      virtual Measure* measure() const    { return (Measure*)(parent()); }

      bool readProperties(const QDomElement& e, QList<Tuplet*>*, const QList<Spanner*>*);
      void writeProperties(Xml& xml) const;
      void writeTuplet(Xml& xml);

      void setTuplet(Tuplet* t)           { _tuplet = t;      }
      Tuplet* tuplet() const              { return _tuplet;   }
      virtual Beam* beam() const          { return 0;         }
      virtual int tick() const = 0;
      int actualTicks() const;

      Fraction duration() const           { return _duration; }
      Fraction globalDuration() const;
      void setDuration(const Fraction& f) { _duration = f;    }
      };

#endif

