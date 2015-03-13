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

#include "instrchange.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "part.h"
#include "undo.h"
#include "mscore.h"
#include "xml.h"
#include "measure.h"
#include "system.h"

namespace Ms {

//---------------------------------------------------------
//   InstrumentChange
//---------------------------------------------------------

InstrumentChange::InstrumentChange(Score* s)
   : Text(s)
      {
      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE | ElementFlag::ON_STAFF);
      setTextStyleType(TextStyleType::INSTRUMENT_CHANGE);
      _instrument = new Instrument();
      }

InstrumentChange::InstrumentChange(const Instrument& i, Score* s)
   : Text(s)
      {
      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE | ElementFlag::ON_STAFF);
      setTextStyleType(TextStyleType::INSTRUMENT_CHANGE);
      _instrument = new Instrument(i);
      }

InstrumentChange::InstrumentChange(const InstrumentChange& is)
   : Text(is)
      {
      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE | ElementFlag::ON_STAFF);
      setTextStyleType(TextStyleType::INSTRUMENT_CHANGE);
      _instrument = new Instrument(*is._instrument);
      }

InstrumentChange::~InstrumentChange()
      {
      delete _instrument;
      }

void InstrumentChange::setInstrument(const Instrument& i)
      {
      delete _instrument;
      _instrument = new Instrument(i);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void InstrumentChange::write(Xml& xml) const
      {
      xml.stag("InstrumentChange");
      _instrument->write(xml);
      Text::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void InstrumentChange::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "Instrument")
                  _instrument->read(e);
            else if (!Text::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant InstrumentChange::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            default:
                  return Text::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant InstrumentChange::propertyDefault(P_ID propertyId) const
      {
      switch (propertyId) {
            default:
                  return Text::propertyDefault(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool InstrumentChange::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch (propertyId) {
            default:
                  return Text::setProperty(propertyId, v);
            }
      return true;
      }

//---------------------------------------------------------
//   dragAnchor
//---------------------------------------------------------

QLineF InstrumentChange::dragAnchor() const
      {
      qreal xp = 0.0;
      for (Element* e = parent(); e; e = e->parent())
            xp += e->x();
      qreal yp = segment()->measure()->system()->staffYpage(staffIdx());
      QPointF p(xp, yp);

      return QLineF(p, canvasPos());
      }

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

QRectF InstrumentChange::drag(EditData* ed)
      {
      QRectF f = Element::drag(ed);

      //
      // move anchor
      //
      Qt::KeyboardModifiers km = qApp->keyboardModifiers();
      if (km != (Qt::ShiftModifier | Qt::ControlModifier)) {
            int si;
            Segment* seg = 0;
            if (_score->pos2measure(ed->pos, &si, 0, &seg, 0) == nullptr)
                  return f;
            if (seg && (seg != segment() || staffIdx() != si)) {
                  QPointF pos1(canvasPos());
                  score()->undo(new ChangeParent(this, seg, si));
                  setUserOff(QPointF());
                  layout();
                  QPointF pos2(canvasPos());
                  setUserOff(pos1 - pos2);
                  ed->startMove = pos2;
                  }
            }
      return f;
      }

}

