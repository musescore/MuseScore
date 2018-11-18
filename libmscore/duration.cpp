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

#include "duration.h"
#include "measure.h"
#include "tuplet.h"
#include "score.h"
#include "undo.h"
#include "staff.h"
#include "xml.h"
#include "property.h"

namespace Ms {

//---------------------------------------------------------
//   DurationElement
//---------------------------------------------------------

DurationElement::DurationElement(Score* s, ElementFlags f)
   : Element(s, f)
      {
      _tuplet = 0;
      }

//---------------------------------------------------------
//   DurationElement
//---------------------------------------------------------

DurationElement::DurationElement(const DurationElement& e)
   : Element(e)
      {
      _tuplet   = 0;    // e._tuplet;
      _duration = e._duration;
      }

//---------------------------------------------------------
//   DurationElement
//---------------------------------------------------------

DurationElement::~DurationElement()
      {
      }

//---------------------------------------------------------
//   topTuplet
//---------------------------------------------------------

Tuplet* DurationElement::topTuplet() const
      {
      Tuplet* t = tuplet();
      if (t) {
            while (t->tuplet())
                  t = t->tuplet();
            }
      return t;
      }

//---------------------------------------------------------
//   globalDuration
//---------------------------------------------------------

Fraction DurationElement::globalDuration() const
      {
      Fraction f(_duration);
      for (Tuplet* t = tuplet(); t; t = t->tuplet())
            f /= t->ratio();
      return f;
      }

//---------------------------------------------------------
//  actualTicks
//---------------------------------------------------------

int DurationElement::actualTicks() const
      {
      return actualFraction().ticks();
      }

//---------------------------------------------------------
//   actualFraction
//---------------------------------------------------------

Fraction DurationElement::actualFraction() const
      {
      return globalDuration() / staff()->timeStretch(tick());
      }

//---------------------------------------------------------
//   afrac
//    Absolute position of element in fractions.
//---------------------------------------------------------

Fraction DurationElement::afrac() const
      {
      Tuplet* t = tuplet();
      if (t) {
            Fraction f = t->afrac();
            for (DurationElement* de : t->elements()) {
                  if (de == this)
                        break;
                  f += de->actualFraction();
                  }
            return f.reduced();
            }
      else
            return Element::afrac();
      }

//---------------------------------------------------------
//   rfrac
//---------------------------------------------------------

Fraction DurationElement::rfrac() const
      {
      if (tuplet()) {
            if (Measure* m = measure())
                  return afrac() - m->afrac();
            }
      return Element::rfrac();
      }

//---------------------------------------------------------
//   readAddTuplet
//---------------------------------------------------------

void DurationElement::readAddTuplet(Tuplet* t)
      {
      if (t) {
            setTuplet(t);
            if (!score()->undoStack()->active())     // HACK, also added in Undo::AddElement()
                  t->add(this);
            }
      }

//---------------------------------------------------------
//   writeTupletStart
//---------------------------------------------------------

void DurationElement::writeTupletStart(XmlWriter& xml) const
      {
      if (tuplet() && tuplet()->elements().front() == this) {
            tuplet()->writeTupletStart(xml);           // recursion
            tuplet()->write(xml);
            }
      }

//---------------------------------------------------------
//   writeTupletEnd
//---------------------------------------------------------

void DurationElement::writeTupletEnd(XmlWriter& xml) const
      {
      if (tuplet() && tuplet()->elements().back() == this) {
            xml.tagE("endTuplet");
            tuplet()->writeTupletEnd(xml);           // recursion
            }
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant DurationElement::getProperty(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::DURATION:
                  return QVariant::fromValue(_duration);
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool DurationElement::setProperty(Pid propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case Pid::DURATION: {
                  Fraction f(v.value<Fraction>());
                  setDuration(f);
                  score()->setLayoutAll();
                  }
                  break;
            default:
                  return Element::setProperty(propertyId, v);
            }
      return true;
      }

}

