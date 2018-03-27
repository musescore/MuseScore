//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "stemslash.h"
#include "score.h"
#include "chord.h"

namespace Ms {

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void StemSlash::draw(QPainter* painter) const
      {
      qreal lw = score()->styleP(Sid::stemWidth);
      painter->setPen(QPen(curColor(), lw, Qt::SolidLine, Qt::FlatCap));
      painter->drawLine(line);
      }

//---------------------------------------------------------
//   setLine
//---------------------------------------------------------

void StemSlash::setLine(const QLineF& l)
      {
      line = l;
      qreal w = score()->styleP(Sid::stemWidth) * .5;
      setbbox(QRectF(line.p1(), line.p2()).normalized().adjusted(-w, -w, 2.0*w, 2.0*w));
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void StemSlash::layout()
      {
      Stem* stem = chord()->stem();
      qreal h2;
      qreal _spatium = spatium();
      qreal l = chord()->up() ? _spatium : -_spatium;
      QPointF p(stem->hookPos());
      qreal x = p.x() + _spatium * .1;
      qreal y = p.y();

      if (chord()->beam()) {
            y += l * .3;
            h2 = l * .8;
            }
      else {
            y += l * 1.2;
            h2 = l * .4;
            }
      qreal w  = chord()->upNote()->headWidth() * .7;
      setLine(QLineF(QPointF(x + w, y - h2), QPointF(x - w, y + h2)));
      adjustReadPos();
      }



}

