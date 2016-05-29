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

#include "tremolobar.h"
#include "score.h"
#include "undo.h"
#include "staff.h"
#include "chord.h"
#include "note.h"
#include "xml.h"

namespace Ms {

//---------------------------------------------------------
//   TremoloBar
//---------------------------------------------------------

TremoloBar::TremoloBar(Score* s)
   : Element(s)
      {
      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE | ElementFlag::ON_STAFF);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TremoloBar::layout()
      {
      qreal _spatium = spatium();

      setPos(0.0, 0.0);
      if (staff() && !staff()->isTabStaff()) {
            setbbox(QRectF());
            if (!parent()) {
                  noteWidth = -_spatium*2;
                  notePos   = QPointF(0.0, _spatium*3);
                  }
            }

      _lw = _spatium * 0.1;
      Note* note = 0;
      if (note == 0) {
            noteWidth = 0.0;
            notePos = QPointF();
            }
      else {
            noteWidth = note->width();
            notePos = note->pos();
            }
//      int n    = _points.size();
//      int pt   = 0;
//      qreal x = noteWidth * .5;
//      qreal y = notePos.y() - _spatium;
//      qreal x2, y2;

      QRectF bb (0, 0, _spatium, -_spatium * 5);
#if 0
      for (int pt = 0; pt < n; ++pt) {
            if (pt == (n-1))
                  break;
            x = x2;
            y = y2;
            }
#endif
      bb.adjust(-_lw, -_lw, _lw, _lw);
      setbbox(bb);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TremoloBar::draw(QPainter* painter) const
      {
      QPen pen(curColor(), _lw, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
      painter->setPen(pen);
      painter->setBrush(QBrush(Qt::black));

      qreal _spatium = spatium();
      const TextStyle* st = &score()->textStyle(TextStyleType::BENCH);
      QFont f = st->fontPx(_spatium);
      painter->setFont(f);

      int n    = _points.size();

      int previousTime  = _points[0].time;
      int previousPitch = _points[0].pitch;
      /* we place the tremolo bars starting slightly before the
       *  notehead, and end it slightly after, drawing above the
       *  note. The values specified in Guitar Pro are very large, too
       *  large for the scale used in Musescore. We used the
       *  timeFactor and pitchFactor below to reduce these values down
       *  consistently to values that make sense to draw with the
       *  Musescore scale. */
      qreal timeFactor  = 10.0 / _userMag;
      qreal pitchFactor = 25.0 / _userMag;
      for (int pt = 1; pt < n; ++pt) {
            painter->drawLine(QLineF(previousTime/timeFactor, -previousPitch/pitchFactor-_spatium*3,
                                     _points[pt].time/timeFactor, -_points[pt].pitch/pitchFactor-_spatium*3));
            previousTime = _points[pt].time;
            previousPitch = _points[pt].pitch;
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void TremoloBar::write(Xml& xml) const
      {
      xml.stag("TremoloBar");
      writeProperty(xml, P_ID::MAG);
      for (const PitchValue& v : _points) {
            xml.tagE(QString("point time=\"%1\" pitch=\"%2\" vibrato=\"%3\"")
               .arg(v.time).arg(v.pitch).arg(v.vibrato));
            }
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void TremoloBar::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            if (e.name() == "point") {
                  PitchValue pv;
                  pv.time    = e.intAttribute("time");
                  pv.pitch   = e.intAttribute("pitch");
                  pv.vibrato = e.intAttribute("vibrato");
                  _points.append(pv);
                  e.readNext();
                  }
            else if (e.name() == "mag")
                  _userMag = e.readDouble(0.1, 10.0);
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   undoSetUserMag
//---------------------------------------------------------

void TremoloBar::undoSetUserMag(qreal val)
      {
      score()->undoChangeProperty(this, P_ID::MAG, val);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant TremoloBar::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::MAG:            return userMag();
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant TremoloBar::propertyDefault(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::MAG:            return 1.0;
            default:
                  return Element::propertyDefault(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool TremoloBar::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case P_ID::MAG:
                  setUserMag(v.toDouble());
                  break;
            default:
                  return Element::setProperty(propertyId, v);
            }
      score()->setLayoutAll(true);
      return true;
      }

}

