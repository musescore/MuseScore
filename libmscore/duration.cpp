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
//   ~DurationElement
//---------------------------------------------------------

DurationElement::~DurationElement()
      {
      if (_tuplet && _tuplet->contains(this)) {
            while (Tuplet* t = topTuplet()) // delete tuplets from top to bottom
                  delete t; // Tuplet destructor removes references to the deleted object
            }
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
//   globalTicks
//---------------------------------------------------------

Fraction DurationElement::globalTicks() const
      {
      Fraction f(_duration);
      for (Tuplet* t = tuplet(); t; t = t->tuplet())
            f /= t->ratio();
      return f;
      }

//---------------------------------------------------------
//   actualTicks
//---------------------------------------------------------

Fraction DurationElement::actualTicksAt(const Fraction& tick) const
      {
      // Use when tick() is unreliable, for example when pasting
      return globalTicks() / staff()->timeStretch(tick);
      }

Fraction DurationElement::actualTicks() const
      {
      return actualTicksAt(tick());
      }

//---------------------------------------------------------
//   readAddTuplet
//---------------------------------------------------------

void DurationElement::readAddTuplet(Tuplet* t)
      {
      setTuplet(t);
      if (!score()->undoStack()->active())     // HACK, also added in Undo::AddElement()
            t->add(this);
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
                  setTicks(f);
                  triggerLayout();
                  }
                  break;
            default:
                  return Element::setProperty(propertyId, v);
            }
      return true;
      }

}

