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
      p->drawText(drawingRect, alignFlags(), _text);
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
      if (frameWidth() != 0.0) {
            QPen pen(color, frameWidth() * MScore::DPMM);
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
//   alignFlags
//---------------------------------------------------------

int SimpleText::alignFlags() const
      {
      int flags = Qt::TextDontClip;
      Align align = textStyle().align();
      if (align & ALIGN_HCENTER)
            flags |= Qt::AlignHCenter;
      else if (align & ALIGN_RIGHT)
            flags |= Qt::AlignRight;
      else
            flags |= Qt::AlignLeft;
      if (align & ALIGN_VCENTER)
            flags |= Qt::AlignVCenter;
      else if (align & ALIGN_BOTTOM)
            flags |= Qt::AlignBottom;
      else if (flags & ALIGN_BASELINE)
            ;
      else
            flags |= Qt::AlignTop;
      if (_layoutToParentWidth)
            flags |= Qt::TextWordWrap;
      return flags;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void SimpleText::layout()
      {
      if (_text.isEmpty()) {
            setPos(QPointF());
            setbbox(QRectF());
            return;
            }

      const TextStyle& s(textStyle());

      QPointF o(s.offset(spatium()));
      if (_layoutToParentWidth && parent()) {
            Element* e = parent();
            if ((type() == MARKER || type() == JUMP) && e->parent())
                  e = e->parent();
            qreal w = e->width();
            qreal h = e->height();
            drawingRect = QRectF(0.0, 0.0, w, h);
            QPointF ro = s.reloff() * .01;
            setPos(o + QPointF(ro.x() * w, ro.y() * h));
            }
      else {
            drawingRect = QRectF();
            setPos(o);
            }
      QFontMetricsF fm(s.fontPx(spatium()));
      setbbox(fm.boundingRect(drawingRect, alignFlags(), _text));
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
      qreal w = (paddingWidth() + frameWidth() * .5) * MScore::DPMM;
      frame.adjust(-w, -w, w, w);
      w = frameWidth() * MScore::DPMM;
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
//   frameWidth
//---------------------------------------------------------

qreal SimpleText::frameWidth() const
      {
      return textStyle().frameWidth();
      }

//---------------------------------------------------------
//   hasFrame
//---------------------------------------------------------

bool SimpleText::hasFrame() const
      {
      return textStyle().hasFrame();
      }

//---------------------------------------------------------
//   paddingWidth
//---------------------------------------------------------

qreal SimpleText::paddingWidth() const
      {
      return textStyle().paddingWidth();
      }

//---------------------------------------------------------
//   frameColor
//---------------------------------------------------------

QColor SimpleText::frameColor() const
      {
      return textStyle().frameColor();
      }

//---------------------------------------------------------
//   backgroundColor
//---------------------------------------------------------

QColor SimpleText::backgroundColor() const
      {
      return textStyle().backgroundColor();
      }

//---------------------------------------------------------
//   frameRound
//---------------------------------------------------------

int SimpleText::frameRound() const
      {
      return textStyle().frameRound();
      }

//---------------------------------------------------------
//   circle
//---------------------------------------------------------

bool SimpleText::circle() const
      {
      return textStyle().circle();
      }
