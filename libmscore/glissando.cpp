//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: glissando.cpp 5149 2011-12-29 08:38:43Z wschweer $
//
//  Copyright (C) 2008-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "glissando.h"
#include "chord.h"
#include "segment.h"
#include "note.h"
#include "style.h"
#include "score.h"
#include "sym.h"

//---------------------------------------------------------
//   Glissando
//---------------------------------------------------------

Glissando::Glissando(Score* s)
  : Element(s)
      {
      _subtype = 0;
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      _text = "gliss.";
      _showText = true;
      qreal _spatium = spatium();
      setSize(QSizeF(_spatium * 2, _spatium * 4));    // for use in palettes
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Glissando::layout()
      {
      Chord* chord = static_cast<Chord*>(parent());
      if (chord == 0) {
            return;
            }
      Note* anchor2   = chord->upNote();
      Segment* s = chord->segment();
      s = s->prev1();
      while (s) {
            if ((s->subtype() & (Segment::SegChordRestGrace)) && s->element(track()))
                  break;
            s = s->prev1();
            }
      if (s == 0) {
            qDebug("no segment for first note of glissando found\n");
            return;
            }
      ChordRest* cr = static_cast<ChordRest*>(s->element(track()));
      if (cr == 0 || cr->type() != CHORD) {
            qDebug("no first note for glissando found, track %d\n", track());
            return;
            }
      Note* anchor1 = static_cast<Chord*>(cr)->upNote();

      setPos(0.0, 0.0);

      QPointF cp1    = anchor1->pagePos();
      QPointF cp2    = anchor2->pagePos();

      // construct line from notehead to notehead
      qreal x1 = (anchor1->headWidth()) - (cp2.x() - cp1.x());
      qreal y1 = anchor1->pos().y();
      qreal x2 = anchor2->pos().x();
      qreal y2 = anchor2->pos().y();
      QLineF fullLine(x1, y1, x2, y2);

      // shorten line on each side by offsets
      qreal xo = spatium() * .5;
      qreal yo = xo;   // spatium() * .5;
      QPointF p1 = fullLine.pointAt(xo / fullLine.length());
      QPointF p2 = fullLine.pointAt(1 - (yo / fullLine.length()));

      line = QLineF(p1, p2);
      qreal lw = spatium() * .15 * .5;
      QRectF r = QRectF(line.p1(), line.p2()).normalized();
      setbbox(r.adjusted(-lw, -lw, lw, lw));
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Glissando::write(Xml& xml) const
      {
      xml.stag("Glissando");
      if (_showText && !_text.isEmpty())
            xml.tag("text", _text);
      xml.tag("subtype", _subtype);
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Glissando::read(const QDomElement& de)
      {
      _showText = false;
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "text") {
                  _showText = true;
                  _text = e.text();
                  }
            else if (e.tagName() == "subtype")
                  _subtype = e.text().toInt();
            else if (!Element::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Glissando::draw(QPainter* painter) const
      {
      painter->save();
      qreal _spatium = spatium();

      QPen pen(curColor());
      pen.setWidthF(_spatium * .15);
      pen.setCapStyle(Qt::RoundCap);
      painter->setPen(pen);

      qreal w = line.dx();
      qreal h = line.dy();

      qreal l = sqrt(w * w + h * h);
      painter->translate(line.p1());
      qreal wi = asin(-h / l) * 180.0 / M_PI;
      painter->rotate(-wi);

      if (subtype() == 0) {
            painter->drawLine(QLineF(0.0, 0.0, l, 0.0));
            }
      else if (subtype() == 1) {
            qreal mags = magS();
            QRectF b = symbols[score()->symIdx()][trillelementSym].bbox(mags);
            qreal w  = symbols[score()->symIdx()][trillelementSym].width(mags);
            int n    = lrint(l / w);
            symbols[score()->symIdx()][trillelementSym].draw(painter, mags, QPointF(0.0, b.height()*.5), n);
            }
      if (_showText) {
            const TextStyle& st = score()->textStyle(TEXT_STYLE_GLISSANDO);
            QFont f = st.fontPx(_spatium);
            QRectF r = QFontMetricsF(f).boundingRect(_text);
            if (r.width() < l) {
                  painter->setFont(f);
                  qreal x = (l - r.width()) * .5;
                  painter->drawText(QPointF(x, -_spatium * .5), _text);
                  }
            }
      painter->restore();
      }

//---------------------------------------------------------
//   space
//---------------------------------------------------------

Space Glissando::space() const
      {
      return Space(0.0, spatium() * 2.0);
      }

//---------------------------------------------------------
//   setSize
//    used for palette
//---------------------------------------------------------

void Glissando::setSize(const QSizeF& s)
      {
      line = QLineF(0.0, s.height(), s.width(), 0.0);
      setbbox(QRectF(QPointF(), s));
      }

