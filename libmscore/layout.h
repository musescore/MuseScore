//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __LAYOUT_H__
#define __LAYOUT_H__

namespace Ms {

class Segment;

//---------------------------------------------------------
//   Spring
//---------------------------------------------------------

struct Spring {
      Segment* seg;
      qreal stretch;
      qreal fix;
      Spring(Segment* s, qreal str) : seg(s), stretch(str), fix(s->width()) {}
      };

//---------------------------------------------------------
//   LayoutContext
//    temp values used during layout
//---------------------------------------------------------

struct LayoutContext {
      bool startWithLongNames  { true };
      bool firstSystem         { true };
      int curPage              { 0 };      // index in Score->_pages
      int tick                 { 0 };
      Fraction sig;

      QList<System*> systemList;          // reusable systems
      System* curSystem        { 0 };
      MeasureBase* systemOldMeasure;
      System* pageOldSystem    { 0 };
      bool systemChanged       { false };
      bool pageChanged         { false };

      MeasureBase* prevMeasure { 0 };
      MeasureBase* curMeasure  { 0 };
      MeasureBase* nextMeasure { 0 };
      int measureNo            { 0 };
      bool rangeLayout         { false };
      int endTick;

      int adjustMeasureNo(MeasureBase*);
      };

typedef std::multimap<qreal, Spring, std::less<qreal> > SpringMap;

extern qreal sff(qreal x, qreal xMin, const SpringMap& springs);


}     // namespace Ms
#endif

