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

#include "system.h"

namespace Ms {

class Segment;
class Page;

//---------------------------------------------------------
//   VerticalStretchData
//    helper class for spreading staves over a page
//---------------------------------------------------------

class VerticalGapData {
   private:
      bool      _systemSpace    { false   };
      bool      _fixedHeight    { false   };
      qreal     _spacing        { 0.0     };
      qreal     _maxSpacing     { 0.0     };
      qreal     _addedSpace     { 0.0     };
      qreal     _copyAddedSpace { 0.0     };
   public:
      System*   system          { nullptr };
      SysStaff* sysStaff        { nullptr };
      Staff*    staff           { nullptr };
      qreal     factor          { 1.0     };

      VerticalGapData(System* sys, Staff* st, SysStaff* sst, qreal y);

      void setVBox();
      void addSpaceBetweenSections();
      void addSpaceAroundVBox(bool above);
      void addSpaceAroundNormalBracket();
      void addSpaceAroundCurlyBracket();
      void insideCurlyBracket();
      bool isSystemGap() const;

      qreal normalisedSpacing() const;
      qreal actualSpacing() const;
      qreal addedSpace() const;

      qreal nextYPos(bool first) const;
      qreal yBottom() const;
      qreal addSpacing(qreal step);
      qreal addNormalisedSpacing(qreal step);
      bool canAddSpace() const;
      void restore();
      };

//---------------------------------------------------------
//   VerticalStretchDataList
//    helper class for spreading staves over a page
//---------------------------------------------------------

class VerticalGapDataList : public QList<VerticalGapData*> {
   public:
      ~VerticalGapDataList();
      qreal sumStretchFactor() const;
      qreal smallest(qreal limit=-1.0) const;
      };

//---------------------------------------------------------
//   LayoutContext
//    temp values used during layout
//---------------------------------------------------------

struct LayoutContext {
      Score* score             { 0    };
      bool startWithLongNames  { true };
      bool firstSystem         { true };
      bool firstSystemIndent   { true };
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

      LayoutContext(Score* s);
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

