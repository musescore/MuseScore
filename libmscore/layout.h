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
class Page;

//---------------------------------------------------------
//   LayoutContext
//    temp values used during layout
//---------------------------------------------------------

struct LayoutContext {
      Score* score             { 0    };
      bool startWithLongNames  { true };
      bool firstSystem         { true };
      Page* page               { 0 };
      int curPage              { 0 };      // index in Score->page()s
      Fraction tick            { 0, 1 };

      QList<System*> systemList;          // reusable systems
      std::set<Spanner*> processedSpanners;

      System* prevSystem       { 0 };     // used during page layout
      System* curSystem        { 0 };

      MeasureBase* systemOldMeasure;
      MeasureBase* pageOldMeasure;
      bool rangeDone           { false };

      MeasureBase* prevMeasure { 0 };
      MeasureBase* curMeasure  { 0 };
      MeasureBase* nextMeasure { 0 };
      int measureNo            { 0 };
      Fraction startTick;
      Fraction endTick;

      LayoutContext() = default;
      LayoutContext(const LayoutContext&) = delete;
      LayoutContext& operator=(const LayoutContext&) = delete;
      ~LayoutContext();

      void layoutLinear();

      void layout();
      int adjustMeasureNo(MeasureBase*);
      void getNextPage();
      void collectPage();
      };

//---------------------------------------------------------
//   VerticalAlignRange
//---------------------------------------------------------

enum class VerticalAlignRange {
      SEGMENT, MEASURE, SYSTEM
      };

extern bool isTopBeam(ChordRest* cr);
extern bool notTopBeam(ChordRest* cr);
extern bool isTopTuplet(ChordRest* cr);
extern bool notTopTuplet(ChordRest* cr);

}     // namespace Ms
#endif

