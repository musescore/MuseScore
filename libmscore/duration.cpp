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

DurationElement::DurationElement(Score* s)
   : Element(s)
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
//   readProperties
//---------------------------------------------------------

bool DurationElement::readProperties(XmlReader& e)
      {
      if (e.name() == "Tuplet") {
            int i = e.readInt();
            Tuplet* t = e.findTuplet(i);
            if (!t) {
                  qDebug("DurationElement:read(): Tuplet id %d not found", i);
                  t = score()->searchTuplet(e, i);
                  if (t) {
                        qDebug("   ...found outside measure, input file corrupted?");
                        e.addTuplet(t);
                        }
                  }
            if (t) {
                  setTuplet(t);
                  if (!score()->undoStack()->active())     // HACK, also added in Undo::AddElement()
                        t->add(this);
                  }
            return true;
            }
      if (Element::readProperties(e))
            return true;
      return false;
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void DurationElement::writeProperties(Xml& xml) const
      {
      Element::writeProperties(xml);
      if (tuplet())
            xml.tag("Tuplet", tuplet()->id());
      }

//---------------------------------------------------------
//   writeTuplet
//---------------------------------------------------------

void DurationElement::writeTuplet(Xml& xml)
      {
      if (tuplet() && tuplet()->elements().front() == this) {
            tuplet()->writeTuplet(xml);           // recursion
            tuplet()->setId(xml.tupletId++);
            tuplet()->write(xml);
            }
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant DurationElement::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::DURATION:
                  return QVariant::fromValue(_duration);
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool DurationElement::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case P_ID::DURATION: {
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

