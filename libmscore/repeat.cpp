//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: repeat.cpp 5427 2012-03-07 12:41:34Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "repeat.h"
#include "sym.h"
#include "score.h"
#include "system.h"
#include "measure.h"
#include "mscore.h"

//---------------------------------------------------------
//   RepeatMeasure
//---------------------------------------------------------

RepeatMeasure::RepeatMeasure(Score* score)
   : Rest(score)
      {
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void RepeatMeasure::draw(QPainter* painter) const
      {
      painter->setBrush(QBrush(curColor()));
      painter->setPen(Qt::NoPen);
      painter->drawPath(path);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void RepeatMeasure::layout()
      {
      qreal sp  = spatium();

      qreal y   = sp;
      qreal w   = sp * 2.0;
      qreal h   = sp * 2.0;
      qreal lw  = sp * .30;  // line width
      qreal r   = sp * .15;  // dot radius

      path       = QPainterPath();

      path.moveTo(w - lw, y);
      path.lineTo(w,  y);
      path.lineTo(lw,  h+y);
      path.lineTo(0.0, h+y);
      path.closeSubpath();
      path.addEllipse(QRectF(w * .25 - r, y+h * .25 - r, r * 2.0, r * 2.0 ));
      path.addEllipse(QRectF(w * .75 - r, y+h * .75 - r, r * 2.0, r * 2.0 ));

      setbbox(path.boundingRect());
      }

//---------------------------------------------------------
//   duration
//---------------------------------------------------------

Fraction RepeatMeasure::duration() const
      {
      if (measure())
            return measure()->len();
      return Fraction(0, 1);
      }

//---------------------------------------------------------
//   Marker
//---------------------------------------------------------

Marker::Marker(Score* s)
   : Text(s)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE | ELEMENT_ON_STAFF);
      setTextStyle(s->textStyle(TEXT_STYLE_REPEAT));
      }

//---------------------------------------------------------
//   setMarkerType
//---------------------------------------------------------

void Marker::setMarkerType(MarkerType t)
      {
      _markerType = t;
      switch(t) {
            case MARKER_SEGNO:
                  setText(symbols[score()->symIdx()][segnoSym].toString());
                  setLabel("segno");
                  break;

            case MARKER_VARSEGNO:
                  setText(symbols[score()->symIdx()][varsegnoSym].toString());
                  setLabel("varsegno");
                  break;

            case MARKER_CODA:
                  setText(symbols[score()->symIdx()][codaSym].toString());
                  setLabel("codab");
                  break;

            case MARKER_VARCODA:
                  setText(symbols[score()->symIdx()][varcodaSym].toString());
                  setLabel("varcoda");
                  break;

            case MARKER_CODETTA:
                  setText(symbols[score()->symIdx()][codaSym].toString());
                  setLabel("codetta");
                  break;

            case MARKER_FINE:
                  setText("Fine");
                  setLabel("fine");
                  break;

            case MARKER_TOCODA:
                  setText("To Coda");
                  setLabel("coda");
                  break;

            case MARKER_USER:
                  break;

            default:
                  qDebug("unknown marker type %d\n", t);
                  break;
            }
      }

//---------------------------------------------------------
//   styleChanged
//---------------------------------------------------------

void Marker::styleChanged()
      {
      setMarkerType(_markerType);
      }

//---------------------------------------------------------
//   adjustReadPos
//---------------------------------------------------------

void Marker::adjustReadPos()
      {
      if (!readPos().isNull()) {
            QPointF uo;
            if (score()->mscVersion() <= 114) {
                  // rebase from Measure to Segment
                  uo = userOff();
                  uo.rx() -= segment()->pos().x();
                  // 1.2 is always HCENTER aligned
                  if ((align() & ALIGN_HMASK) == 0)    // ALIGN_LEFT
                        uo.rx() -= bbox().width() * .5;
                  }
            else
                  uo = readPos() - ipos();
            setUserOff(uo);
            setReadPos(QPointF());
            }
      }

//---------------------------------------------------------
//   markerType
//---------------------------------------------------------

MarkerType Marker::markerType(const QString& s) const
      {
      if (s == "segno")
            return MARKER_SEGNO;
      else if (s == "varsegno")
            return MARKER_VARSEGNO;
      else if (s == "codab")
            return MARKER_CODA;
      else if (s == "varcoda")
            return MARKER_VARCODA;
      else if (s == "codetta")
            return MARKER_CODETTA;
      else if (s == "fine")
            return MARKER_FINE;
      else if (s == "coda")
            return MARKER_TOCODA;
      else
            return MARKER_USER;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Marker::read(XmlReader& e)
      {
      MarkerType mt = MARKER_SEGNO;

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "label") {
                  QString s(e.readElementText());
                  setLabel(s);
                  mt = markerType(s);
                  }
            else if (!Text::readProperties(e))
                  e.unknown();
            }
      switch (mt) {
            case MARKER_SEGNO:
            case MARKER_VARSEGNO:
            case MARKER_CODA:
            case MARKER_VARCODA:
            case MARKER_CODETTA:
                  setTextStyleType(TEXT_STYLE_REPEAT_LEFT);
                  break;

            case MARKER_FINE:
            case MARKER_TOCODA:
                  setTextStyleType(TEXT_STYLE_REPEAT_RIGHT);
                  break;

            case MARKER_USER:
                  setTextStyleType(TEXT_STYLE_REPEAT);
                  break;
            }
      setMarkerType(mt);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Marker::write(Xml& xml) const
      {
      xml.stag(name());
      Text::writeProperties(xml);
      xml.tag("label", _label);
      xml.etag();
      }

//---------------------------------------------------------
//   Jump
//---------------------------------------------------------

Jump::Jump(Score* s)
   : Text(s)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      setTextStyle(s->textStyle(TEXT_STYLE_REPEAT));
      }

//---------------------------------------------------------
//   setJumpType
//---------------------------------------------------------

void Jump::setJumpType(int t)
      {
      switch(t) {
            case JUMP_DC:
                  setText("D.C.");
                  setJumpTo("start");
                  setPlayUntil("end");
                  break;

            case JUMP_DC_AL_FINE:
                  setText("D.C. al Fine");
                  setJumpTo("start");
                  setPlayUntil("fine");
                  break;

            case JUMP_DC_AL_CODA:
                  setText("D.C. al Coda");
                  setJumpTo("start");
                  setPlayUntil("coda");
                  setContinueAt("codab");
                  break;

            case JUMP_DS_AL_CODA:
                  setText("D.S. al Coda");
                  setJumpTo("segno");
                  setPlayUntil("coda");
                  setContinueAt("codab");
                  break;

            case JUMP_DS_AL_FINE:
                  setText("D.S. al Fine");
                  setJumpTo("segno");
                  setPlayUntil("fine");
                  break;

            case JUMP_DS:
                  setText("D.S.");
                  setJumpTo("segno");
                  setPlayUntil("end");
                  break;

            case JUMP_USER:
                  break;

            default:
                  qDebug("unknown jump type\n");
                  break;
            }
      }

//---------------------------------------------------------
//   jumpType
//---------------------------------------------------------

int Jump::jumpType() const
      {
      if (_jumpTo == "start" && _playUntil == "end" && _continueAt == "")
            return JUMP_DC;
      else if (_jumpTo == "start" && _playUntil == "fine" && _continueAt == "")
            return JUMP_DC_AL_FINE;
      else if (_jumpTo == "start" && _playUntil == "coda" && _continueAt == "codab")
            return JUMP_DC_AL_CODA;
      else if (_jumpTo == "segno" && _playUntil == "coda" && _continueAt == "codab")
            return JUMP_DS_AL_CODA;
      else if (_jumpTo == "segno" && _playUntil == "fine" && _continueAt == "")
            return JUMP_DS_AL_FINE;
      else if (_jumpTo == "segno" && _playUntil == "end" && _continueAt == "")
            return JUMP_DS;
      return JUMP_USER;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Jump::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "jumpTo")
                  _jumpTo = e.readElementText();
            else if (tag == "playUntil")
                  _playUntil = e.readElementText();
            else if (tag == "continueAt")
                  _continueAt = e.readElementText();
            else if (!Text::readProperties(e))
                  e.unknown();
            }
      setTextStyle(score()->textStyle(TEXT_STYLE_REPEAT_RIGHT));
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Jump::write(Xml& xml) const
      {
      xml.stag(name());
      Text::writeProperties(xml);
      xml.tag("jumpTo", _jumpTo);
      xml.tag("playUntil", _playUntil);
      xml.tag("continueAt", _continueAt);
      xml.etag();
      }

