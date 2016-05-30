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
   : Element(s), _notehead(SymId::noSym),
        _flag(SymId::noSym)
      {
      _line = 1000;
      }

bool ShadowNote::isValid() const
      {
      return _notehead != SymId::noSym;
      }

bool ShadowNote::noFlag() const
      {
      return _flag == SymId::noSym;
      }

void ShadowNote::setSym(SymId id)
      {
      setSymbols(TDuration::DurationType::V_INVALID, id);
      }

void ShadowNote::setSymbols(TDuration::DurationType type, SymId noteSymbol)
      {
      // clear symbols
      _notehead = SymId::noSym;
      _flag = SymId::noSym;
      _notehead = noteSymbol;

      switch(type) {
            case TDuration::DurationType::V_LONG:
                  _flag = SymId::flagInternalUp;
                  break;
            case TDuration::DurationType::V_BREVE:
                  _flag = SymId::noSym;
                  break;
            case TDuration::DurationType::V_WHOLE:
                  _flag = SymId::noSym;
                  break;
            case TDuration::DurationType::V_HALF:
                  _flag = SymId::flagInternalUp;
                  break;
            case TDuration::DurationType::V_QUARTER:
                  _flag = SymId::flagInternalUp;
                  break;
            case TDuration::DurationType::V_EIGHTH:
                  _flag = SymId::flag8thUp;
                  break;
            case TDuration::DurationType::V_16TH:
                  _flag = SymId::flag16thUp;
                  break;
            case TDuration::DurationType::V_32ND:
                  _flag = SymId::flag32ndUp;
                  break;
            case TDuration::DurationType::V_64TH:
                  _flag = SymId::flag64thUp;
                  break;
            case TDuration::DurationType::V_128TH:
                  _flag = SymId::flag128thUp;
                  break;
            default:
                  _flag = SymId::noSym;
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void ShadowNote::draw(QPainter* painter) const
      {
      if (!visible() || !isValid())
            return;

      QPointF ap(pagePos());
      painter->translate(ap);
      InputState ps = score()->inputState();
      int voice;
      if (ps.drumNote() != -1 && ps.drumset() && ps.drumset()->isValid(ps.drumNote()))
            voice = ps.drumset()->voice(ps.drumNote());
      else
            voice = ps.voice();

      qreal lw = score()->styleP(StyleIdx::stemWidth);
      QPen pen(MScore::selectColor[voice].lighter(SHADOW_NOTE_LIGHT), lw, Qt::SolidLine, Qt::RoundCap);
      painter->setPen(pen);

      drawSymbol(_notehead, painter);

      qreal noteheadWidth = symWidth(_notehead);
      if (!noFlag()) {
            QPointF pos;
            pos.rx() = noteheadWidth - (lw / 2);
            qreal yOffset = symStemUpSE(_notehead).y() * magS();
            if(_flag != SymId::flagInternalUp) {
                  pos.ry() -= symHeight(_flag);
                  painter->drawLine(QLineF(pos.x(), yOffset, pos.x(), pos.y() - yOffset - lw/2));
                  pos.rx() -= (lw / 2); // flag offset?
                  drawSymbol(_flag, painter, pos, 1);
                  }
            else {
                  painter->drawLine(QLineF(pos.x(), yOffset, pos.x(), -3 * spatium() * mag() + yOffset));
                  }
            }

      qreal ms = spatium();
      qreal x1 = noteheadWidth * .5 - (ms * mag());
      qreal x2 = x1 + 2 * ms * mag();
      ms *= .5;

      lw = score()->styleP(StyleIdx::ledgerLineWidth);
      QPen penL(MScore::selectColor[voice].lighter(SHADOW_NOTE_LIGHT), lw);
      painter->setPen(penL);

      if (_line < 100 && _line > -100 && !ps.rest()) {
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
      if (!isValid()) {
            setbbox(QRectF());
            return;
            }
      qreal _spatium = spatium();
      QRectF b;
      QRectF noteheadBbox = symBbox(_notehead);
      if(noFlag()) {
            b = noteheadBbox.toRect();
            }
      else {
            qreal x = noteheadBbox.x();
            qreal height = noteheadBbox.height();
            qreal width = noteheadBbox.width();
            qreal y = 0;
            if (_flag != SymId::flagInternalUp) {
                  QRectF flagBbox = symBbox(_flag);
                  qreal lw = score()->styleP(StyleIdx::stemWidth);
                  y -= flagBbox.height();
                  height += flagBbox.height() + lw / 2;
                  width += flagBbox.width();
                  }
            else {
                  y -= 4 * spatium() * mag();
                  height += (-y);
                  }

            b.setRect(x, y, width, height);
            }

      qreal lw = point(score()->styleS(StyleIdx::ledgerLineWidth));

      qreal x1 = (noteheadBbox.width()) * .5 - (_spatium * mag()) - lw * .5;

      qreal x2 = x1 + 2 * _spatium * mag() + lw * .5;

      InputState ps = score()->inputState();
      QRectF r(x1, -lw * .5, x2 - x1, lw);
      if (_line < 100 && _line > -100 && !ps.rest()) {
            for (int i = -2; i >= _line; i -= 2)
                  b |= r.translated(QPointF(0, _spatium * .5 * (i - _line)));
            for (int i = 10; i <= _line; i += 2)
                  b |= r.translated(QPointF(0, _spatium * .5 * (i - _line)));
            }
      setbbox(b);
      }
}

