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

      polygon.clear();

      /* we place the tremolo bars starting slightly before the
       *  notehead, and end it slightly after, drawing above the
       *  note. The values specified in Guitar Pro are very large, too
       *  large for the scale used in Musescore. We used the
       *  timeFactor and pitchFactor below to reduce these values down
       *  consistently to values that make sense to draw with the
       *  Musescore scale. */

      qreal timeFactor  = 1.0 / _userMag;
      qreal pitchFactor = 2.5 / _userMag;

      for (auto v : _points)
            polygon << QPointF(v.time / timeFactor, -v.pitch / pitchFactor - _spatium * 3);

      QRectF bb(polygon.boundingRect());
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
      painter->drawPolyline(polygon.translated(pos()));
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
      undoChangeProperty(P_ID::MAG, val);
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
      score()->setLayoutAll();
      return true;
      }

}

