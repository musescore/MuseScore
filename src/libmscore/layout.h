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

#include <set>
#include <QList>

#include "system.h"

namespace Ms {
class Segment;
class Page;

//---------------------------------------------------------
//   VerticalStretchData
//    helper class for spreading staves over a page
//---------------------------------------------------------

class VerticalGapData
{
private:
    bool _fixedHeight        { false };
    bool _fixedSpacer        { false };
    qreal _factor               { 1.0 };
    qreal _normalisedSpacing    { 0.0 };
    qreal _maxActualSpacing     { 0.0 };
    qreal _addedNormalisedSpace { 0.0 };
    qreal _fillSpacing          { 0.0 };
    qreal _lastStep             { 0.0 };
    void  updateFactor(qreal factor);

public:
    System* system   { nullptr };
    SysStaff* sysStaff { nullptr };
    Staff* staff    { nullptr };

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

class VerticalGapDataList : public QList<VerticalGapData*>
{
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
    Score* score             { 0 };
    bool startWithLongNames  { true };
    bool firstSystem         { true };
    bool firstSystemIndent   { true };
    Page* page               { 0 };
    int curPage              { 0 };        // index in Score->page()s
    Fraction tick            { 0, 1 };

    QList<System*> systemList;            // reusable systems
    std::set<Spanner*> processedSpanners;

    System* prevSystem       { 0 };       // used during page layout
    System* curSystem        { 0 };

    MeasureBase* systemOldMeasure { 0 };
    MeasureBase* pageOldMeasure   { 0 };
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
