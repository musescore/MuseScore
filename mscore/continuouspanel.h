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

//---------------------------------------------------------
//   ContinuousPanel
//---------------------------------------------------------

class ContinuousPanel {
      ScoreView* _sv;
      Score* _score;
      QRectF _rect;
      bool _visible;
      const Measure* _currentMeasure;
      int _currentMeasureTick;
      int _currentMeasureNo;
      int _mmRestCount;       // Used for showing mmRest interval in the panel
      Fraction _currentTimeSig;
      qreal _offsetPanel;
      qreal _x;
      qreal _y;
      qreal _width;           // Actual panel width (final or transitional)
      qreal _oldWidth;        // The last final panel width
      qreal _newWidth;        // New panel width
      qreal _measureWidth;
      qreal _height;
      qreal _heightName;
      qreal _widthName;
      qreal _widthClef;
      qreal _widthKeySig;
      qreal _widthTimeSig;
      qreal _leftMarginTotal; // Sum of all elments left margin
      qreal _panelRightPadding;  // Extra space for the panel after last element
      qreal _xPosTimeSig;     // X position of the time signature (because it is centered
      qreal _xPosMeasure;     // Position of the coming measure

   protected:
      void findElementWidths(const QList<Element*>& el);
      void draw(QPainter& painter, const QList<Element*>& el);

   public:
      ContinuousPanel(ScoreView* sv);

      QRectF rect() const            { return _rect;     }
      void setRect(const QRectF& r)  { _rect = r;        }
      bool visible() const           { return _visible;  }
      void setVisible(bool val)      { _visible = val;   }
      void setScore(Score* s)        { _score = s;       }
      qreal width()                  { return _width;    }

      void paint(const QRect& r, QPainter& p);
      };

}

#endif

