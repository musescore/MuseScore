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

#ifndef __SIMPLETEXT_H__
#define __SIMPLETEXT_H__

#include "element.h"
#include "style.h"
#include "elementlayout.h"

class MuseScoreView;
struct SymCode;

//---------------------------------------------------------
//   SimpleText
//---------------------------------------------------------

class SimpleText : public Element {
      QString _text;
      bool _layoutToParentWidth;
      QRectF drawingRect;
      QRectF frame;           // set by layout()

      int alignFlags() const;

   protected:
      TextStyle _textStyle;

      void drawFrame(QPainter* painter) const;
      QColor textColor() const;
      void layoutFrame();

   public:
      SimpleText(Score*);
      SimpleText(const SimpleText&);
      ~SimpleText();

      SimpleText &operator=(const SimpleText&);

      void setTextStyle(const TextStyle& st)  { _textStyle = st;   }
      const TextStyle& textStyle() const      { return _textStyle; }
      TextStyle& textStyle()                  { return _textStyle; }

      void setText(const QString& s)        { _text = s;    }
      QString getText() const               { return _text; }

      virtual void draw(QPainter*) const;

      virtual void layout();
      qreal lineSpacing() const;
      qreal lineHeight() const;
      virtual qreal baseLine() const;

      bool isEmpty() const                { return _text.isEmpty(); }
      void clear()                        { _text.clear();          }

      bool layoutToParentWidth() const    { return _layoutToParentWidth; }
      void setLayoutToParentWidth(bool v) { _layoutToParentWidth = v;   }

      qreal frameWidth() const;
      bool hasFrame() const;
      qreal paddingWidth() const;
      QColor frameColor() const;
      int frameRound() const;
      bool circle() const;
      };

#endif
