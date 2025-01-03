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

#include "measure.h"
#include "mscore.h"
#include "score.h"
#include "staff.h"
#include "stafftypechange.h"
#include "system.h"
#include "xml.h"

namespace Ms {

//---------------------------------------------------------
//   StaffTypeChange
//---------------------------------------------------------

StaffTypeChange::StaffTypeChange(Score* score)
   : Element(score, ElementFlag::HAS_TAG)
      {
      lw = spatium() * 0.3;
      }

StaffTypeChange::StaffTypeChange(const StaffTypeChange& lb)
   : Element(lb)
      {
      lw = lb.lw;
      m_ownsStaffType = lb.m_ownsStaffType;
      if (lb.m_ownsStaffType && lb.m_staffType)
            m_staffType = new StaffType(*lb.m_staffType);
      else
            m_staffType = lb.m_staffType;
      }

StaffTypeChange::~StaffTypeChange()
      {
      if (m_ownsStaffType)
            delete m_staffType;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void StaffTypeChange::write(XmlWriter& xml) const
      {
      xml.stag(this);
      if (m_staffType)
            m_staffType->write(xml);
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
                  StaffType* st = new StaffType();
                  st->read(e);
                  // Measure::add() will replace this with a pointer to a copy in the staff
                  setStaffType(st, true);
                  }
            else if (!Element::readProperties(e))
                  e.unknown();
            }
      }

void StaffTypeChange::setStaffType(StaffType* st, bool owned)
      {
      if (m_staffType && m_ownsStaffType)
            delete m_staffType;

      m_staffType = st;
      m_ownsStaffType = owned && (st != nullptr);
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
      qreal _spatium = score()->spatium();
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
      qreal lineDist = 0.35;         // line distance for the icon 'staff lines'
      // draw icon rectangle
      painter->setPen(QPen(selected() ? MScore::selectColor[0] : MScore::layoutBreakColor,
         lw, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
      painter->setBrush(Qt::NoBrush);
      painter->drawRect(0, 0, w, h);

      // draw icon contents
      int lines = 5;
      if (staffType()) {
            if (staffType()->stemless())     // a single notehead represents a stemless staff
                  drawSymbol(SymId::noteheadBlack, painter, QPoint(w * 0.5 - 0.33 * _spatium, h * 0.5), 0.5);
            if (staffType()->invisible())    // no lines needed. It's done.
                  return;
            // show up to 6 lines
            lines = qMin(staffType()->lines(),6);
            }
      // calculate starting point Y for the lines from half the icon height (2.5) so staff lines appear vertically centered
      qreal startY = 1.25 - (lines - 1) * lineDist * 0.5;
      painter->setPen(QPen(selected() ? MScore::selectColor[0] : MScore::layoutBreakColor,
         2.5, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
      for (int i=0; i < lines; i++) {
            int y = (startY + i * lineDist) * _spatium;
            painter->drawLine(0, y, w, y);
            }
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant StaffTypeChange::getProperty(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::STEP_OFFSET:
                  return m_staffType->stepOffset();
            case Pid::STAFF_LINES:
                  return m_staffType->lines();
            case Pid::LINE_DISTANCE:
                  return m_staffType->lineDistance();
            case Pid::STAFF_SHOW_BARLINES:
                  return m_staffType->showBarlines();
            case Pid::STAFF_SHOW_LEDGERLINES:
                  return m_staffType->showLedgerLines();
            case Pid::STAFF_STEMLESS:
                  return m_staffType->stemless();
            case Pid::HEAD_SCHEME:
                  return int(m_staffType->noteHeadScheme());
            case Pid::STAFF_GEN_CLEF:
                  return m_staffType->genClef();
            case Pid::STAFF_GEN_TIMESIG:
                  return m_staffType->genTimesig();
            case Pid::STAFF_GEN_KEYSIG:
                  return m_staffType->genKeysig();
            case Pid::MAG:
                  return m_staffType->userMag();
            case Pid::SMALL:
                  return m_staffType->isSmall();
            case Pid::STAFF_INVISIBLE:
                  return m_staffType->invisible();
            case Pid::STAFF_COLOR:
                  return m_staffType->color();
            case Pid::STAFF_YOFFSET:
                  return m_staffType->yoffset();
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool StaffTypeChange::setProperty(Pid propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case Pid::STEP_OFFSET:
                  m_staffType->setStepOffset(v.toInt());
                  break;
            case Pid::STAFF_LINES:
                  m_staffType->setLines(v.toInt());
                  break;
            case Pid::LINE_DISTANCE:
                  m_staffType->setLineDistance(v.value<Spatium>());
                  break;
            case Pid::STAFF_SHOW_BARLINES:
                  m_staffType->setShowBarlines(v.toBool());
                  break;
            case Pid::STAFF_SHOW_LEDGERLINES:
                  m_staffType->setShowLedgerLines(v.toBool());
                  break;
            case Pid::STAFF_STEMLESS:
                  m_staffType->setStemless(v.toBool());
                  break;
            case Pid::HEAD_SCHEME:
                  m_staffType->setNoteHeadScheme(NoteHead::Scheme(v.toInt()));
                  break;
            case Pid::STAFF_GEN_CLEF:
                  m_staffType->setGenClef(v.toBool());
                  break;
            case Pid::STAFF_GEN_TIMESIG:
                  m_staffType->setGenTimesig(v.toBool());
                  break;
            case Pid::STAFF_GEN_KEYSIG:
                  m_staffType->setGenKeysig(v.toBool());
                  break;
            case Pid::MAG: {
                  qreal _spatium = spatium();
                  m_staffType->setUserMag(v.toDouble());
                  Staff* _staff = staff();
                  if (_staff)
                        _staff->localSpatiumChanged(_spatium, spatium(), tick());
                  }
                  break;
            case Pid::SMALL: {
                  qreal _spatium = spatium();
                  m_staffType->setSmall(v.toBool());
                  Staff* _staff = staff();
                  if (_staff)
                        _staff->localSpatiumChanged(_spatium, spatium(), tick());
                  }
                  break;
            case Pid::STAFF_INVISIBLE:
                  m_staffType->setInvisible(v.toBool());
                  break;
            case Pid::STAFF_COLOR:
                  m_staffType->setColor(v.value<QColor>());
                  break;
            case Pid::STAFF_YOFFSET:
                  m_staffType->setYoffset(v.value<Spatium>());
                  break;
            default:
                  if (!Element::setProperty(propertyId, v))
                        return false;
                  break;
            }
      if (parent())
            staff()->staffTypeListChanged(measure()->tick());
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant StaffTypeChange::propertyDefault(Pid id) const
      {
      switch (id) {
            case Pid::STEP_OFFSET:
                  return 0;
            case Pid::STAFF_LINES:
                  return 5;
            case Pid::LINE_DISTANCE:
                  return Spatium(1.0);
            case Pid::STAFF_SHOW_BARLINES:
                  return true;
            case Pid::STAFF_SHOW_LEDGERLINES:
                  return true;
            case Pid::STAFF_STEMLESS:
                  return false;
            case Pid::HEAD_SCHEME:
                  return int(NoteHead::Scheme::HEAD_NORMAL);
            case Pid::STAFF_GEN_CLEF:
                  return true;
            case Pid::STAFF_GEN_TIMESIG:
                  return true;
            case Pid::STAFF_GEN_KEYSIG:
                  return true;
            case Pid::MAG:
                  return 1.0;
            case Pid::SMALL:
                  return false;
            case Pid::STAFF_INVISIBLE:
                  return false;
            case Pid::STAFF_COLOR:
                  return QColor(Qt::black);
            case Pid::STAFF_YOFFSET:
                  return Spatium(0.0);
            default:
                  return Element::propertyDefault(id);
            }
      }

}

