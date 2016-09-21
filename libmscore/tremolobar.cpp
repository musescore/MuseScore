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
      setLineWidth(score()->styleS(StyleIdx::tremoloBarLineWidth));
      lineWidthStyle = PropertyStyle::STYLED;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TremoloBar::layout()
      {
      qreal _spatium = spatium();
      if (parent())
            setPos(0.0, -_spatium * 3.0);
      else
            setPos(QPointF());

      /* we place the tremolo bars starting slightly before the
       *  notehead, and end it slightly after, drawing above the
       *  note. The values specified in Guitar Pro are very large, too
       *  large for the scale used in Musescore. We used the
       *  timeFactor and pitchFactor below to reduce these values down
       *  consistently to values that make sense to draw with the
       *  Musescore scale. */

      qreal timeFactor  = _userMag / 1.0;
      qreal pitchFactor = -_spatium * .02;

      polygon.clear();
      for (auto v : _points)
            polygon << QPointF(v.time * timeFactor, v.pitch * pitchFactor);

      qreal w = _lw.val() * _spatium;
      setbbox(polygon.boundingRect().adjusted(-w, -w, w, w));
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TremoloBar::draw(QPainter* painter) const
      {
      QPen pen(curColor(), _lw.val() * spatium(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
      painter->setPen(pen);
      painter->drawPolyline(polygon);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void TremoloBar::write(Xml& xml) const
      {
      xml.stag("TremoloBar");
      writeProperty(xml, P_ID::MAG);
      writeProperty(xml, P_ID::LINE_WIDTH);
      writeProperty(xml, P_ID::PLAY);
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
            auto tag = e.name();
            if (tag == "point") {
                  PitchValue pv;
                  pv.time    = e.intAttribute("time");
                  pv.pitch   = e.intAttribute("pitch");
                  pv.vibrato = e.intAttribute("vibrato");
                  _points.append(pv);
                  e.readNext();
                  }
            else if (tag == "mag")
                  _userMag = e.readDouble(0.1, 10.0);
            else if (tag == "lineWidth")
                  setLineWidth(Spatium(e.readDouble()));
            else if (tag == "play")
                  setPlay(e.readInt());
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
//   undoSetLineWidth
//---------------------------------------------------------

void TremoloBar::undoSetLineWidth(Spatium val)
      {
      undoChangeProperty(P_ID::LINE_WIDTH, val);
      }

//---------------------------------------------------------
//   undoSetPlay
//---------------------------------------------------------

void TremoloBar::undoSetPlay(bool val)
      {
      undoChangeProperty(P_ID::PLAY, val);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant TremoloBar::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::LINE_WIDTH:
                  return lineWidth();
            case P_ID::MAG:
                  return userMag();
            case P_ID::PLAY:
                  return play();
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
            case P_ID::LINE_WIDTH:
                  return score()->style(StyleIdx::voltaLineWidth);
            case P_ID::MAG:
                  return 1.0;
            case P_ID::PLAY:
                  return true;
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
            case P_ID::LINE_WIDTH:
                  lineWidthStyle = PropertyStyle::UNSTYLED;
                  setLineWidth(v.value<Spatium>());
                  break;
            case P_ID::MAG:
                  setUserMag(v.toDouble());
                  break;
            case P_ID::PLAY:
                  setPlay(v.toBool());
                  score()->setPlaylistDirty();
                  break;
            default:
                  return Element::setProperty(propertyId, v);
            }
      score()->setLayoutAll();
      return true;
      }

//---------------------------------------------------------
//   propertyStyle
//---------------------------------------------------------

PropertyStyle TremoloBar::propertyStyle(P_ID id) const
      {
      switch (id) {
            case P_ID::LINE_WIDTH:
                  return lineWidthStyle;

            default:
                  return Element::propertyStyle(id);
            }
      }

//---------------------------------------------------------
//   resetProperty
//---------------------------------------------------------

void TremoloBar::resetProperty(P_ID id)
      {
      switch (id) {
            case P_ID::LINE_WIDTH:
                  setProperty(id, propertyDefault(id));
                  lineWidthStyle = PropertyStyle::STYLED;
                  break;

            default:
                  return Element::resetProperty(id);
            }
      }

//---------------------------------------------------------
//   styleChanged
//    reset all styled values to actual style
//---------------------------------------------------------

void TremoloBar::styleChanged()
      {
      if (lineWidthStyle == PropertyStyle::STYLED)
            setLineWidth(score()->styleS(StyleIdx::voltaLineWidth));
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void TremoloBar::reset()
      {
      if (lineWidthStyle == PropertyStyle::UNSTYLED)
            undoChangeProperty(P_ID::LINE_WIDTH, propertyDefault(P_ID::LINE_WIDTH), PropertyStyle::STYLED);
      Element::reset();
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void TremoloBar::spatiumChanged(qreal oldValue, qreal newValue)
      {
      _lw *= (newValue / oldValue);
      Element::spatiumChanged(oldValue, newValue);
      }

}

