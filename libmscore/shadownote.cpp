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

#include "shadownote.h"
#include "score.h"
#include "drumset.h"
#include "sym.h"
#include "mscore.h"

namespace Ms {

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

      QPen pen(MScore::selectColor[voice].lighter(140), lw);
      painter->setPen(pen);

      sym->draw(painter, magS());

      qreal ms = spatium();

      qreal x1 = sym->width(magS())*.5 - (ms * mag());
      qreal x2 = x1 + 2 * ms * mag();

      ms *= .5;
      if (_line < 100 && _line > -100 && !ps.rest) {
            for (int i = -2; i >= _line; i -= 2) {
                  qreal y = ms * mag() * (i - _line);
                  painter->drawLine(QLineF(x1, y, x2, y));
                  }
            for (int i = 10; i <= _line; i += 2) {
                  qreal y = ms * mag() * (i - _line);
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
      qreal lw = point(score()->styleS(ST_ledgerLineWidth));

      qreal x1 = sym->width(magS())*.5 - (_spatium * mag()) - lw * .5;
      qreal x2 = x1 + 2 * _spatium * mag() + lw * .5;

      InputState ps = score()->inputState();
      QRectF r(x1, -lw * .5, x2 - x1, lw);
      if (_line < 100 && _line > -100 && !ps.rest) {
            for (int i = -2; i >= _line; i -= 2)
                  b |= r.translated(QPointF(0, _spatium * .5 * (i - _line)));
            for (int i = 10; i <= _line; i += 2)
                  b |= r.translated(QPointF(0, _spatium * .5 * (i - _line)));
            }
      setbbox(b);
      }


}

