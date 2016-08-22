//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2016 Werner Schweer
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
      bool rangeDone           { false };
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

//---------------------------------------------------------
//   VerticalAlignRange
//---------------------------------------------------------

enum class VerticalAlignRange {
      SEGMENT, MEASURE, SYSTEM
      };


}     // namespace Ms
#endif

