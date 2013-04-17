//=============================================================================
//  MuseScore
//  Music Composition & Notation
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
