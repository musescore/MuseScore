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
#include "shadownotesymbol.h"
#include "score.h"
#include "drumset.h"
#include "sym.h"
#include "mscore.h"

namespace Ms {

//---------------------------------------------------------
//   ShadowNote
//---------------------------------------------------------

ShadowNote::ShadowNote(Score* s)
   : Element(s),
     symbols()
      {
      _line = 1000;
      }

void ShadowNote::setSymbols(TDuration::DurationType type, SymId noteSymbol)
      {
      this->symbols.setSymbols(type,noteSymbol);
      }

void ShadowNote::setSym(SymId id)
      {
      this->symbols.setSymbols(TDuration::DurationType::V_INVALID, id);
      }



//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void ShadowNote::draw(QPainter* painter) const
      {


      if (!visible() || symbols.isValid()==false )
            return;

      QPointF ap(pagePos());
#if 0 // yet(?) unused
      QRect r(abbox().toRect());
#endif

      painter->translate(ap);
      qreal lw = point(score()->styleS(StyleIdx::ledgerLineWidth));
      InputState ps = score()->inputState();
      int voice;
      if (ps.drumNote() != -1 && ps.drumset() && ps.drumset()->isValid(ps.drumNote()))
            voice = ps.drumset()->voice(ps.drumNote());
      else
            voice = ps.voice();

      QPen pen(MScore::selectColor[voice].lighter(SHADOW_NOTE_LIGHT), lw);
      painter->setPen(pen);

      qreal width=0.0;

      if(this->symbols.noFlag()) {
            drawSymbol(this->symbols.getNoteId(), painter);
            }
      else {
            QList<SymId> symbolList;
            symbolList.append(symbols.getNoteId());
            symbolList.append(symbols.getFlagId());
            drawShadowSymbols(symbolList, painter);
            }

      width = symWidth(symbols.getNoteId());

      qreal ms = spatium();

      qreal x1 = width * .5 - (ms * mag());
      qreal x2 = x1 + 2 * ms * mag();

      ms *= .5;
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
      if (!this->symbols.isValid()) {
            setbbox(QRectF());
            return;
            }


      QRectF b(0,0,0,0);
      qreal x(0),y(0),height(0),width(0);
      SymId symNoteHead=symbols.getNoteId();
      SymId symFlag=symbols.getFlagId();

      if(symbols.noFlag()) {
            b.setRect(symBbox(symNoteHead).x(),symBbox(symNoteHead).y(),symBbox(symNoteHead).width(),symBbox(symNoteHead).height());
            }
      else {
           x=symBbox(symNoteHead).x();
           height+=symBbox(symNoteHead).height();
           width+=symBbox(symNoteHead).width();
           height += ((symBbox(symNoteHead).height()/4)+1);

           if(symFlag!=SymId::flagInternalUp) {
                y-=symBbox(symFlag).height();
                height+=symBbox(symFlag).height();
                width+=symBbox(symFlag).width();
                }
          else {
               y-=20;
               height+=(-y);
               }

          b.setRect(x,y,width,height);
          }

      qreal _spatium = spatium();
      qreal lw = point(score()->styleS(StyleIdx::ledgerLineWidth));

      qreal x1;

      x1 = (symWidth(symNoteHead)) * .5 - (_spatium * mag()) - lw * .5 ;

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

