//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "simpletext.h"
#include "score.h"
#include "segment.h"
#include "measure.h"
#include "system.h"

//---------------------------------------------------------
//   SimpleText
//---------------------------------------------------------

SimpleText::SimpleText(Score* s)
   : Element(s)
      {
      _textStyle           = s->textStyle(TEXT_STYLE_DEFAULT);
      _layoutToParentWidth = false;
      }

SimpleText::SimpleText(const SimpleText& st)
   : Element(st)
      {
      _text                = st._text;
      _textStyle           = st._textStyle;
      _layoutToParentWidth = st._layoutToParentWidth;
      frame                = st.frame;
      }

SimpleText::~SimpleText()
      {
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void SimpleText::draw(QPainter* p) const
      {
      drawFrame(p);
      p->setFont(textStyle().fontPx(spatium()));
      p->setBrush(Qt::NoBrush);
      p->setPen(textColor());
      foreach(const TLine& t, _text)
            p->drawText(t.pos, t.text);
      }

//---------------------------------------------------------
//   drawFrame
//---------------------------------------------------------

void SimpleText::drawFrame(QPainter* painter) const
      {
      if (!textStyle().hasFrame())
            return;

      QColor color(frameColor());
      if (!visible())
            color = Qt::gray;
      else if (selected())
            color = Qt::blue;
      if (frameWidth().val() != 0.0) {
            QPen pen(color, frameWidth().val() * spatium());
            painter->setPen(pen);
            }
      else
            painter->setPen(Qt::NoPen);
      QColor bg(backgroundColor());
      painter->setBrush(bg.alpha() ? QBrush(bg) : Qt::NoBrush);
      if (circle())
            painter->drawArc(frame, 0, 5760);
      else {
            int r2 = frameRound() * lrint((frame.width() / frame.height()));
            if (r2 > 99)
                  r2 = 99;
            painter->drawRoundRect(frame, frameRound(), r2);
            }
      }

//---------------------------------------------------------
//   textColor
//---------------------------------------------------------

QColor SimpleText::textColor() const
      {
      if (!score()->printing()) {
            QColor color;
            if (selected())
                  return MScore::selectColor[0];
            if (!visible())
                  return Qt::gray;
            }
      return textStyle().foregroundColor();
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void SimpleText::layout()
      {
      int n = _text.size();
      if (!n) {
            setPos(QPointF());
            setbbox(QRectF());
            return;
            }

      const TextStyle& s(textStyle());

      QFontMetricsF fm(s.fontPx(spatium()));
      QPointF o(s.offset(spatium()));

      QRectF bb;
      qreal lh = lineHeight();
      qreal ly = .0;
      for (int i = 0; i < n; ++i) {
            TLine* t = &_text[i];

            QRectF r(fm.tightBoundingRect(t->text));

            t->pos.ry() = ly;
            if (align() & ALIGN_BOTTOM)
                  t->pos.ry() += -r.bottom();
            else if (align() & ALIGN_VCENTER)
                  t->pos.ry() +=  -(r.top() + r.bottom()) * .5;
            else if (align() & ALIGN_BASELINE)
                  ;
            else  // ALIGN_TOP
                  t->pos.ry() += -r.top();
            if (align() & ALIGN_RIGHT)
                  t->pos.rx() = -r.right();
            else if (align() & ALIGN_HCENTER)
                  t->pos.rx() = -(r.left() + r.right()) * .5;
            else  // ALIGN_LEFT
                  t->pos.rx() = -r.left();
            bb |= r.translated(t->pos);
            ly += lh;
            }
      if (_layoutToParentWidth && parent()) {
            //
            // layout to parent frame
            //
            Element* e = parent();
            QPointF ro(s.reloff() * .01);
            o += QPointF(ro.x() * e->width(), ro.y() * e->height());
            }

      setPos(o);
      setbbox(bb);

      if (hasFrame())
            layoutFrame();
      }

//---------------------------------------------------------
//   layoutFrame
//---------------------------------------------------------

void SimpleText::layoutFrame()
      {
      frame = bbox();
      if (circle()) {
            if (frame.width() > frame.height()) {
                  frame.setY(frame.y() + (frame.width() - frame.height()) * -.5);
                  frame.setHeight(frame.width());
                  }
            else {
                  frame.setX(frame.x() + (frame.height() - frame.width()) * -.5);
                  frame.setWidth(frame.height());
                  }
            }
      qreal _spatium = spatium();
      qreal w = (paddingWidth() + frameWidth() * .5f).val() * _spatium;
      frame.adjust(-w, -w, w, w);
      w = frameWidth().val() * _spatium;
      setbbox(frame.adjusted(-w, -w, w, w));
      }

//---------------------------------------------------------
//   lineSpacing
//---------------------------------------------------------

qreal SimpleText::lineSpacing() const
      {
      return QFontMetricsF(textStyle().font(spatium())).lineSpacing();
      }

//---------------------------------------------------------
//   lineHeight
//---------------------------------------------------------

qreal SimpleText::lineHeight() const
      {
      return QFontMetricsF(textStyle().font(spatium())).height();
      }

//---------------------------------------------------------
//   baseLine
//---------------------------------------------------------

qreal SimpleText::baseLine() const
      {
      return QFontMetricsF(textStyle().font(spatium())).ascent();
      }

//---------------------------------------------------------
//   setText
//---------------------------------------------------------

void SimpleText::setText(const QString& s)
      {
      QStringList sl = s.split('\n');
      _text.clear();
      foreach(QString s, sl)
            _text.append(TLine(s));
      }

//---------------------------------------------------------
//   getText
//---------------------------------------------------------

QString SimpleText::getText() const
      {
      QString s;
      int n = _text.size();
      if (n == 0)
            return s;
      s = _text[0].text;
      for (int i = 1; i < n; ++i) {
            s += '\n';
            s += _text[i].text;
            }
      return s;
      }

