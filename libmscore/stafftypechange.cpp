//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2016 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "stafftypechange.h"
#include "score.h"
#include "mscore.h"
#include "xml.h"
#include "measure.h"
#include "system.h"
#include "staff.h"

namespace Ms {

//---------------------------------------------------------
//   StaffTypeChange
//---------------------------------------------------------

StaffTypeChange::StaffTypeChange(Score* score)
   : Element(score)
      {
      setFlag(ElementFlag::HAS_TAG, true);
      lw = spatium() * 0.3;
      }

StaffTypeChange::StaffTypeChange(const StaffTypeChange& lb)
   : Element(lb)
      {
      lw = lb.lw;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void StaffTypeChange::write(XmlWriter& xml) const
      {
      xml.stag(name());
      if (_staffType)
            _staffType->write(xml);
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void StaffTypeChange::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "StaffType") {
                  StaffType st;
                  st.read(e);
                  if (staff())
                        _staffType = staff()->setStaffType(measure()->tick(), &st);
                  else
                        _staffType = new StaffType(st);     // drag&drop operation
                  }
            else if (!Element::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void StaffTypeChange::spatiumChanged(qreal, qreal)
      {
      lw = spatium() * 0.3;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void StaffTypeChange::layout()
      {
      qreal _spatium = spatium();
      setbbox(QRectF(-lw*.5, -lw*.5, _spatium * 2.5 + lw, _spatium*2.5 + lw));
      if (measure()) {
            qreal y = -1.5 * _spatium - height() + measure()->system()->staff(staffIdx())->y();
            setPos(_spatium * .8, y);
            }
      else
            setPos(0.0, 0.0);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void StaffTypeChange::draw(QPainter* painter) const
      {
      if (score()->printing() || !score()->showUnprintable())
            return;
      qreal _spatium = score()->spatium();
      qreal h  = _spatium * 2.5;
      qreal w  = _spatium * 2.5;
      painter->setPen(QPen(selected() ? MScore::selectColor[0] : MScore::layoutBreakColor,
         lw, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
      painter->setBrush(Qt::NoBrush);
      painter->drawRect(0, 0, w, h);
      QFont f("FreeSans", 12.0 * _spatium * MScore::pixelRatio / SPATIUM20);
      painter->setFont(f);
      painter->drawText(QRectF(0.0, 0.0, w, h), Qt::AlignCenter, QString("S"));
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant StaffTypeChange::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::STEP_OFFSET:
                  return _staffType->stepOffset();
            case P_ID::STAFF_LINES:
                  return _staffType->lines();
            case P_ID::LINE_DISTANCE:
                  return _staffType->lineDistance();
            case P_ID::STAFF_SHOW_BARLINES:
                  return _staffType->showBarlines();
            case P_ID::STAFF_SHOW_LEDGERLINES:
                  return _staffType->showLedgerLines();
            case P_ID::STAFF_SLASH_STYLE:
                  return _staffType->slashStyle();
            case P_ID::STAFF_NOTEHEAD_SCHEME:
                  return int(_staffType->noteHeadScheme());
            case P_ID::STAFF_GEN_CLEF:
                  return _staffType->genClef();
            case P_ID::STAFF_GEN_TIMESIG:
                  return _staffType->genTimesig();
            case P_ID::STAFF_GEN_KEYSIG:
                  return _staffType->genKeysig();
            case P_ID::MAG:
                  return _staffType->userMag();
            case P_ID::SMALL:
                  return _staffType->small();
            case P_ID::STAFF_YOFFSET:
                  return _staffType->yoffset();
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool StaffTypeChange::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case P_ID::STEP_OFFSET:
                  _staffType->setStepOffset(v.toInt());
                  break;
            case P_ID::STAFF_LINES:
                  _staffType->setLines(v.toInt());
                  break;
            case P_ID::LINE_DISTANCE:
                  _staffType->setLineDistance(v.value<Spatium>());
                  break;
            case P_ID::STAFF_SHOW_BARLINES:
                  _staffType->setShowBarlines(v.toBool());
                  break;
            case P_ID::STAFF_SHOW_LEDGERLINES:
                  _staffType->setShowLedgerLines(v.toBool());
                  break;
            case P_ID::STAFF_SLASH_STYLE:
                  _staffType->setSlashStyle(v.toBool());
                  break;
            case P_ID::STAFF_NOTEHEAD_SCHEME:
                  _staffType->setNoteHeadScheme(NoteHeadScheme(v.toInt()));
                  break;
            case P_ID::STAFF_GEN_CLEF:
                  _staffType->setGenClef(v.toBool());
                  break;
            case P_ID::STAFF_GEN_TIMESIG:
                  _staffType->setGenTimesig(v.toBool());
                  break;
            case P_ID::STAFF_GEN_KEYSIG:
                  _staffType->setGenKeysig(v.toBool());
                  break;
            case P_ID::MAG:
                  _staffType->setUserMag(v.toDouble());
                  break;
            case P_ID::SMALL:
                  _staffType->setSmall(v.toBool());
                  break;
            case P_ID::STAFF_YOFFSET:
                  _staffType->setYoffset(v.value<Spatium>());
                  break;
            default:
                  if (!Element::setProperty(propertyId, v))
                        return false;
                  break;
            }
      staff()->staffTypeListChanged(measure()->tick());
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant StaffTypeChange::propertyDefault(P_ID id) const
      {
      switch (id) {
            case P_ID::STEP_OFFSET:
                  return 0;
            case P_ID::STAFF_LINES:
                  return 5;
            case P_ID::LINE_DISTANCE:
                  return Spatium(1.0);
            case P_ID::STAFF_SHOW_BARLINES:
                  return true;
            case P_ID::STAFF_SHOW_LEDGERLINES:
                  return true;
            case P_ID::STAFF_SLASH_STYLE:
                  return false;
            case P_ID::STAFF_NOTEHEAD_SCHEME:
                  return int(NoteHeadScheme::HEAD_NORMAL);
            case P_ID::STAFF_GEN_CLEF:
                  return true;
            case P_ID::STAFF_GEN_TIMESIG:
                  return true;
            case P_ID::STAFF_GEN_KEYSIG:
                  return true;
            case P_ID::MAG:
                  return 1.0;
            case P_ID::SMALL:
                  return false;
            case P_ID::STAFF_YOFFSET:
                  return Spatium(0.0);
            default:
                  return Element::propertyDefault(id);
            }
      }

}

