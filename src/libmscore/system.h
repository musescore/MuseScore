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

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

/**
 \file
 Definition of classes SysStaff and System
*/

#include "element.h"
#include "spatium.h"
#include "symbol.h"
#include "skyline.h"

namespace Ms {
class Staff;
class StaffLines;
class Clef;
class Page;
class Bracket;
class Lyrics;
class Segment;
class MeasureBase;
class Text;
class InstrumentName;
class SpannerSegment;
class VBox;
class BarLine;

//---------------------------------------------------------
//   SysStaff
///  One staff in a System.
//---------------------------------------------------------

class SysStaff
{
    QRectF _bbox;                   // Bbox of StaffLines.
    Skyline _skyline;
    qreal _yOff   { 0.0 };          // offset of top staff line within bbox
    qreal _yPos   { 0.0 };          // y position of bbox after System::layout2
    qreal _height { 0.0 };          // height of bbox after System::layout2
    qreal _continuousDist { -1.0 }; // distance for continuous mode
    bool _show  { true };           // derived from Staff or false if empty
                                    // staff is hidden
public:
    //int idx     { 0    };
    QList<InstrumentName*> instrumentNames;

    const QRectF& bbox() const { return _bbox; }
    QRectF& bbox() { return _bbox; }
    void setbbox(const QRectF& r) { _bbox = r; }
    qreal y() const { return _bbox.y() + _yOff; }
    void setYOff(qreal offset) { _yOff = offset; }
    qreal yOffset() const { return _yOff; }
    qreal yBottom() const;

    void saveLayout();
    void restoreLayout();

    qreal continuousDist() const { return _continuousDist; }
    void setContinuousDist(qreal val) { _continuousDist = val; }

    bool show() const { return _show; }
    void setShow(bool v) { _show = v; }

    const Skyline& skyline() const { return _skyline; }
    Skyline& skyline() { return _skyline; }

    SysStaff() {}
    ~SysStaff();
};

//---------------------------------------------------------
//   System
///    One row of measures for all instruments;
///    a complete piece of the timeline.
//---------------------------------------------------------

class System final : public Element
{
    SystemDivider* _systemDividerLeft    { nullptr };       // to the next system
    SystemDivider* _systemDividerRight   { nullptr };

    std::vector<MeasureBase*> ml;
    QList<SysStaff*> _staves;
    QList<Bracket*> _brackets;
    QList<SpannerSegment*> _spannerSegments;

    qreal _leftMargin              { 0.0 };     ///< left margin for instrument name, brackets etc.
    mutable bool fixedDownDistance { false };
    qreal _distance                { 0.0 };     /// temp. variable used during layout
    qreal _systemHeight            { 0.0 };

    int firstVisibleSysStaff() const;
    int lastVisibleSysStaff() const;

    int getBracketsColumnsCount();
    void setBracketsXPosition(const qreal xOffset);
    Bracket* createBracket(Ms::BracketItem* bi, int column, int staffIdx, QList<Ms::Bracket*>& bl, Measure* measure);

public:
    System(Score*);
    ~System();

    // Score Tree functions
    ScoreElement* treeParent() const override;
    ScoreElement* treeChild(int idx) const override;
    int treeChildCount() const override;

    System* clone() const override { return new System(*this); }
    ElementType type() const override { return ElementType::SYSTEM; }

    void add(Element*) override;
    void remove(Element*) override;
    void change(Element* o, Element* n) override;
    void write(XmlWriter&) const override;
    void read(XmlReader&) override;

    void scanElements(void* data, void (* func)(void*, Element*), bool all=true) override;

    void appendMeasure(MeasureBase*);
    void removeMeasure(MeasureBase*);
    void removeLastMeasure();

    Page* page() const { return (Page*)parent(); }

    void layoutSystem(qreal, const bool isFirstSystem = false, bool firstSystemIndent = false);

    void setMeasureHeight(qreal height);
    void layoutBracketsVertical();
    void layoutInstrumentNames();

    void addBrackets(Measure* measure);

    void layout2();                       ///< Called after Measure layout.
    void restoreLayout2();
    void clear();                         ///< Clear measure list.

    QRectF bboxStaff(int staff) const { return _staves[staff]->bbox(); }
    QList<SysStaff*>* staves() { return &_staves; }
    const QList<SysStaff*>* staves() const { return &_staves; }
    qreal staffYpage(int staffIdx) const;
    qreal staffCanvasYpage(int staffIdx) const;
    SysStaff* staff(int staffIdx) const;

    bool pageBreak() const;

    SysStaff* insertStaff(int);
    void removeStaff(int);
    void adjustStavesNumber(int);

    int y2staff(qreal y) const;
    int searchStaff(qreal y, int preferredStaff = -1, qreal spacingFactor = 0.5) const;
    void setInstrumentNames(bool longName, Fraction tick = { 0,1 });
    Fraction snap(const Fraction& tick, const QPointF p) const;
    Fraction snapNote(const Fraction& tick, const QPointF p, int staff) const;

    const std::vector<MeasureBase*>& measures() const { return ml; }

    MeasureBase* measure(int idx) { return ml[idx]; }
    Measure* firstMeasure() const;
    Measure* lastMeasure() const;
    Fraction endTick() const;

    MeasureBase* nextMeasure(const MeasureBase*) const;

    qreal leftMargin() const { return _leftMargin; }
    Box* vbox() const;

    const QList<Bracket*>& brackets() const { return _brackets; }

    QList<SpannerSegment*>& spannerSegments() { return _spannerSegments; }
    const QList<SpannerSegment*>& spannerSegments() const { return _spannerSegments; }

    SystemDivider* systemDividerLeft() const { return _systemDividerLeft; }
    SystemDivider* systemDividerRight() const { return _systemDividerRight; }

    Element* nextSegmentElement() override;
    Element* prevSegmentElement() override;

    qreal minDistance(System*) const;
    qreal topDistance(int staffIdx, const SkylineLine&) const;
    qreal bottomDistance(int staffIdx, const SkylineLine&) const;
    qreal minTop() const;
    qreal minBottom() const;
    qreal spacerDistance(bool up) const;
    Spacer* upSpacer(int staffIdx, Spacer* prevDownSpacer) const;
    Spacer* downSpacer(int staffIdx) const;

    qreal firstNoteRestSegmentX(bool leading = false);

    void moveBracket(int staffIdx, int srcCol, int dstCol);
    bool hasFixedDownDistance() const { return fixedDownDistance; }
    int firstVisibleStaff() const;
    int nextVisibleStaff(int) const;
    qreal distance() const { return _distance; }
    void setDistance(qreal d) { _distance = d; }

    int firstSysStaffOfPart(const Part* part) const;
    int firstVisibleSysStaffOfPart(const Part* part) const;
    int lastSysStaffOfPart(const Part* part) const;
    int lastVisibleSysStaffOfPart(const Part* part) const;
};

typedef QList<System*>::iterator iSystem;
typedef QList<System*>::const_iterator ciSystem;
}     // namespace Ms
#endif
