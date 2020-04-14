//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2013-15 Werner Schweer & others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __TEXTCURSOR_H__
#define __TEXTCURSOR_H__

#include "libmscore/fraction.h"

namespace Ms {

class ScoreView;

//---------------------------------------------------------
//   CursorType
//---------------------------------------------------------

enum class CursorType : char {
      LOOP_IN,
      LOOP_OUT,
      POS
      };

//---------------------------------------------------------
//   PositionCursor
//---------------------------------------------------------

class PositionCursor {
      ScoreView*  _sv;
      QRectF      _rect;
      bool        _visible    { false };
      QColor      _color;
      Fraction    _tick       { 0,1 };
      CursorType  _type       { CursorType::POS };

   public:
      PositionCursor(ScoreView* sv) : _sv(sv)  {}

      void        setType(CursorType t);
      QRectF      rect() const                  { return _rect;     }
      void        setRect(const QRectF& r)      { _rect = r;        }
      bool        visible() const               { return _visible;  }
      void        setVisible(bool val)          { _visible = val;   }
      QColor      color() const                 { return _color;    }
      void        setColor(const QColor& c)     { _color = c;       }
      Fraction    tick() const                  { return _tick;     }
      void        setTick(const Fraction& val)  { _tick = val;      }

      void update(QWidget*);
      void move(const Fraction& tick);
      void paint(QPainter*);
      QRectF bbox() const;
      };

}

#endif



