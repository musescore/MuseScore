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
class Score;
class MasterScore;
class System;
class MeasureBase;
class Fraction;
class Spanner;

//---------------------------------------------------------
//   VerticalStretchData
//    helper class for spreading staves over a page
//---------------------------------------------------------

class VerticalGapData {
   private:
      bool  _fixedHeight          { false };
      bool  _fixedSpacer          { false };
      qreal _factor               { 1.0   };
      qreal _normalisedSpacing    { 0.0   };
      qreal _maxActualSpacing     { 0.0   };
      qreal _addedNormalisedSpace { 0.0   };
      qreal _fillSpacing          { 0.0   };
      qreal _lastStep             { 0.0   };
      void  updateFactor(qreal factor);

   public:
      System*   system   { nullptr };
      SysStaff* sysStaff { nullptr };
      Staff*    staff    { nullptr };

      VerticalGapData(bool first, System* sys, Staff* st, SysStaff* sst, Spacer* nextSpacer, qreal y);

      void addSpaceBetweenSections();
      void addSpaceAroundVBox(bool above);
      void addSpaceAroundNormalBracket();
      void addSpaceAroundCurlyBracket();
      void insideCurlyBracket();

      qreal factor() const;
      qreal spacing() const;
      qreal actualAddedSpace() const;

      qreal addSpacing(qreal step);
      bool isFixedHeight() const;
      void undoLastAddSpacing();
      qreal addFillSpacing(qreal step, qreal maxFill);
      };

//---------------------------------------------------------
//   VerticalStretchDataList
//    helper class for spreading staves over a page
//---------------------------------------------------------

class VerticalGapDataList : public QList<VerticalGapData*> {
   public:
      void deleteAll();
      qreal sumStretchFactor() const;
      qreal smallest(qreal limit=-1.0) const;
      };

//---------------------------------------------------------
//   LayoutContext
//    temp values used during layout
//---------------------------------------------------------

struct LayoutContext {
    Score* mainScore         { nullptr };   // the score that holds the pages.
    Score* currentScore      { nullptr };   // the score whose measures/systems are being laid out.
                                            // these 2 are the same, unless we have a multi-movement score.
    int movementIndex        { -1 };        // points to the movement being laid out.
      bool startWithLongNames  { true };
      bool firstSystem         { true };
      bool firstSystemIndent   { true };
      Page* page               { nullptr };
      int curPage              { 0 };      // index in Score->pages()
      int systemIdx            { -1 };
      bool continuing          { false };     // used to continue adding systems (to the next page)
                                            // if the page is full
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

   private:
      static void layoutPage(Page* page, qreal restHeight, qreal footerPadding);
      static void checkDivider(bool left, System* s, qreal yOffset, bool remove = false);
      static void distributeStaves(Page* page, qreal footerPadding);
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

