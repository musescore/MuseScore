//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "staffstate.h"
#include "score.h"
#include "instrtemplate.h"
#include "segment.h"
#include "staff.h"
#include "part.h"
#include "mscore.h"
#include "xml.h"

namespace Ms {

//---------------------------------------------------------
//   StaffState
//---------------------------------------------------------

StaffState::StaffState(Score* score)
   : Element(score)
      {
      _staffStateType = StaffStateType::INSTRUMENT;
      _instrument = new Instrument;
      }

StaffState::StaffState(const StaffState& ss)
   : Element(ss)
      {
      _instrument = new Instrument(*ss._instrument);
      }

StaffState::~StaffState()
      {
      delete _instrument;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void StaffState::write(Xml& xml) const
      {
      xml.stag(name());
      xml.tag("subtype", int(_staffStateType));
      if (staffStateType() == StaffStateType::INSTRUMENT)
            _instrument->write(xml, nullptr);
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void StaffState::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype")
                  _staffStateType = StaffStateType(e.readInt());
            else if (tag == "Instrument")
                  _instrument->read(e, nullptr);
            else if (!Element::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void StaffState::draw(QPainter* painter) const
      {
      if (score()->printing())
            return;
      QPen pen(selected() ? MScore::selectColor[0] : MScore::layoutBreakColor,
         lw, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
      painter->setPen(pen);
      painter->setBrush(Qt::NoBrush);
      painter->drawPath(path);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void StaffState::layout()
      {
      qreal _spatium = spatium();
      path      = QPainterPath();
      lw        = _spatium * 0.3;
      qreal h  = _spatium * 4;
      qreal w  = _spatium * 2.5;
//      qreal w1 = w * .6;

      switch(staffStateType()) {
            case StaffStateType::INSTRUMENT:
                  path.lineTo(w, 0.0);
                  path.lineTo(w, h);
                  path.lineTo(0.0, h);
                  path.lineTo(0.0, 0.0);
                  path.moveTo(w * .5, h - _spatium * .5);
                  path.lineTo(w * .5, _spatium * 2);
                  path.moveTo(w * .5, _spatium * .8);
                  path.lineTo(w * .5, _spatium * 1.0);
                  break;

            case StaffStateType::TYPE:
                  path.lineTo(w, 0.0);
                  path.lineTo(w, h);
                  path.lineTo(0.0, h);
                  path.lineTo(0.0, 0.0);
                  break;

            case StaffStateType::VISIBLE:
                  path.lineTo(w, 0.0);
                  path.lineTo(w, h);
                  path.lineTo(0.0, h);
                  path.lineTo(0.0, 0.0);
                  break;

            case StaffStateType::INVISIBLE:
                  path.lineTo(w, 0.0);
                  path.lineTo(w, h);
                  path.lineTo(0.0, h);
                  path.lineTo(0.0, 0.0);
                  break;

            default:
                  qDebug("unknown layout break symbol");
                  break;
            }
      QRectF bb(0, 0, w, h);
      bb.adjust(-lw, -lw, lw, lw);
      setbbox(bb);
      setPos(0.0, _spatium * -6.0);
      }

//---------------------------------------------------------
//   setStaffStateType
//---------------------------------------------------------

void StaffState::setStaffStateType(const QString& s)
      {
      if (s == "instrument")
            setStaffStateType(StaffStateType::INSTRUMENT);
      else if (s == "type")
            setStaffStateType(StaffStateType::TYPE);
      else if (s == "visible")
            setStaffStateType(StaffStateType::VISIBLE);
      else if (s == "invisible")
            setStaffStateType(StaffStateType::INVISIBLE);
      }

//---------------------------------------------------------
//   staffStateTypeName
//---------------------------------------------------------

QString StaffState::staffStateTypeName() const
      {
      switch(staffStateType()) {
            case StaffStateType::INSTRUMENT:
                  return "instrument";
            case StaffStateType::TYPE:
                  return "type";
            case StaffStateType::VISIBLE:
                  return "visible";
            case StaffStateType::INVISIBLE:
                  return "invisible";
            default:
                  return "??";
            }
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool StaffState::acceptDrop(const DropData&) const
      {
      return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* StaffState::drop(const DropData& data)
      {
      Element* e = data.element;
      score()->undoChangeElement(this, e);
      return e;
      }

}

