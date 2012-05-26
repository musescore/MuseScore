//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "shadownote.h"
#include "score.h"
#include "drumset.h"
#include "sym.h"
#include "mscore.h"

//---------------------------------------------------------
//   ShadowNote
//---------------------------------------------------------

ShadowNote::ShadowNote(Score* s)
   : Element(s)
      {
      _line = 1000;
      sym   = 0;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void ShadowNote::draw(QPainter* painter) const
      {
      if (!visible() || sym == 0)
            return;

      QPointF ap(pagePos());
      QRect r(abbox().toRect());

      painter->translate(ap);
      qreal lw = point(score()->styleS(ST_ledgerLineWidth));
      InputState ps = score()->inputState();
      int voice;
      if (ps.drumNote() != -1 && ps.drumset() && ps.drumset()->isValid(ps.drumNote()))
            voice = ps.drumset()->voice(ps.drumNote());
      else
            voice = ps.voice();

      QPen pen(MScore::selectColor[voice].light(140), lw);
      painter->setPen(pen);

      sym->draw(painter, magS());

      qreal ms = spatium();

      qreal x1 = sym->width(magS())*.5 - ms;
      qreal x2 = x1 + 2 * ms;

      ms *= .5;
      if (_line < 100 && _line > -100) {
            for (int i = -2; i >= _line; i -= 2) {
                  qreal y = ms * (i - _line);
                  painter->drawLine(QLineF(x1, y, x2, y));
                  }
            for (int i = 10; i <= _line; i += 2) {
                  qreal y = ms * (i - _line);
                  painter->drawLine(QLineF(x1, y, x2, y));
                  }
            }
      painter->translate(-ap);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void ShadowNote::layout()
      {
      if (sym == 0) {
            setbbox(QRectF());
            return;
            }
      QRectF b = sym->bbox(magS());
      qreal _spatium = spatium();
      qreal x  = b.width()/2 - _spatium;
      qreal lw = point(score()->styleS(ST_ledgerLineWidth));

      if (_line < 100 && _line > -100) {
            QRectF r(0, -lw/2.0, 2 * _spatium, lw);
            for (int i = -2; i >= _line; i -= 2)
                  b |= r.translated(QPointF(x, _spatium * .5 * (i - _line)));
            for (int i = 10; i <= _line; i += 2)
                  b |= r.translated(QPointF(x, _spatium * .5 * (i - _line)));
            }
      setbbox(b);
      }


