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

#ifndef __CONTINUOUSPANEL_H__
#define __CONTINUOUSPANEL_H__

namespace Ms {

class ScoreView;
class Score;

//---------------------------------------------------------
//   ContinuousPanel
//---------------------------------------------------------

class ContinuousPanel {
      ScoreView* _sv;
      Score* _score;
      QRectF _rect;
      bool _active;            // Used to active or desactive the panel
      bool _visible;           // False if beginning of the score is visible
      qreal _width;           // Actual panel width (final or transitional)

   public:
      ContinuousPanel(ScoreView* sv);

      QRectF rect() const            { return _rect;     }
      void setRect(const QRectF& r)  { _rect = r;        }
      bool active() const            { return _active;   }
      void setActive(bool val)       { _active = val;    }
      bool visible()                 { return _active ? _visible : false; }
      void setScore(Score* s)        { _score = s;       }
      qreal width()                  { return _width;    }

      void paint(const QRect& r, QPainter& p);
      };

}

#endif

