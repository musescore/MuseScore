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
      setLineWidth(score()->styleS(Sid::tremoloBarLineWidth));
      lineWidthStyle = PropertyFlags::STYLED;
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

void TremoloBar::write(XmlWriter& xml) const
      {
      xml.stag("TremoloBar");
      writeProperty(xml, Pid::MAG);
      writeProperty(xml, Pid::LINE_WIDTH);
      writeProperty(xml, Pid::PLAY);
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
      undoChangeProperty(Pid::MAG, val);
      }

//---------------------------------------------------------
//   undoSetLineWidth
//---------------------------------------------------------

void TremoloBar::undoSetLineWidth(Spatium val)
      {
      undoChangeProperty(Pid::LINE_WIDTH, val);
      }

//---------------------------------------------------------
//   undoSetPlay
//---------------------------------------------------------

void TremoloBar::undoSetPlay(bool val)
      {
      undoChangeProperty(Pid::PLAY, val);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant TremoloBar::getProperty(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::LINE_WIDTH:
                  return lineWidth();
            case Pid::MAG:
                  return userMag();
            case Pid::PLAY:
                  return play();
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant TremoloBar::propertyDefault(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::LINE_WIDTH:
                  return score()->styleV(Sid::voltaLineWidth);
            case Pid::MAG:
                  return 1.0;
            case Pid::PLAY:
                  return true;
            default:
                  return Element::propertyDefault(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool TremoloBar::setProperty(Pid propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case Pid::LINE_WIDTH:
                  lineWidthStyle = PropertyFlags::UNSTYLED;
                  setLineWidth(v.value<Spatium>());
                  break;
            case Pid::MAG:
                  setUserMag(v.toDouble());
                  break;
            case Pid::PLAY:
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

PropertyFlags& TremoloBar::propertyFlags(Pid id)
      {
      switch (id) {
            case Pid::LINE_WIDTH:
                  return lineWidthStyle;

            default:
                  return Element::propertyFlags(id);
            }
      }

//---------------------------------------------------------
//   resetProperty
//---------------------------------------------------------

void TremoloBar::resetProperty(Pid id)
      {
      switch (id) {
            case Pid::LINE_WIDTH:
                  setProperty(id, propertyDefault(id));
                  lineWidthStyle = PropertyFlags::STYLED;
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
      if (lineWidthStyle == PropertyFlags::STYLED)
            setLineWidth(score()->styleS(Sid::voltaLineWidth));
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void TremoloBar::reset()
      {
      if (lineWidthStyle == PropertyFlags::UNSTYLED)
            undoChangeProperty(Pid::LINE_WIDTH, propertyDefault(Pid::LINE_WIDTH), PropertyFlags::STYLED);
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

