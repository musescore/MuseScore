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

   private:
      int _stick;
      int _etick;

      void layoutSlurs(System* system);
      void layoutBeamAndArticulation(System* system);
      void layoutTuplet(System* system);
      void layoutSpannerSegments(System* system);
      void layoutTempoTextAndFermata(System* system);
      void layoutJumpAndMarker(System* system);
      void layoutRehearsalMarkAndStaffTextAndFretDiagram(System* system);
      void layoutGraceNote(System* system);

   public:
      Score* score             { 0    };
      bool startWithLongNames  { true };
      bool firstSystem         { true };
      Page* page               { 0 };
      int curPage              { 0 };      // index in Score->page()s
      int tick                 { 0 };
      Fraction sig;

      QList<System*> systemList;          // reusable systems

      System* prevSystem       { 0 };     // used during page layout
      System* curSystem        { 0 };

      MeasureBase* systemOldMeasure;
      bool rangeDone           { false };

      MeasureBase* prevMeasure { 0 };
      MeasureBase* curMeasure  { 0 };
      MeasureBase* nextMeasure { 0 };
      int measureNo            { 0 };
      int endTick;

      void layoutLinear();
      void layoutMeasureLinear(MeasureBase*);

      void layoutPageMode(System* system);

      void layout();
      int adjustMeasureNo(MeasureBase*);
      void getEmptyPage();
      void collectPage();

      void computeMeasureShape(System* system);
      };

//---------------------------------------------------------
//   VerticalAlignRange
//---------------------------------------------------------

enum class VerticalAlignRange {
      SEGMENT, MEASURE, SYSTEM
      };


}     // namespace Ms
#endif

