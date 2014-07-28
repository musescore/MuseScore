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
      _tuplet   = e._tuplet;
      _duration = e._duration;
      }

//---------------------------------------------------------
//   DurationElement
//---------------------------------------------------------

DurationElement::~DurationElement()
      {
      if (tuplet() && tuplet()->elements().front() == this)
            delete tuplet();
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
            // setTuplet(0);
            int i = e.readInt();
            Tuplet* t = e.findTuplet(i);
            if (t) {
                  setTuplet(t);
                  if (!score()->undo()->active()) {  // HACK, also added in Undo::AddElement()
                        t->add(this);
                        }
                  }
            else {
                  qDebug("DurationElement:read(): Tuplet id %d not found", i);
                  Tuplet* t = score()->searchTuplet(e, i);
                  if (t) {
                        qDebug("   ...found outside measure, input file corrupted?");
                        setTuplet(t);
                        if (!score()->undo()->active())     // HACK
                              t->add(this);
                        e.addTuplet(t);
                        }
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

}

