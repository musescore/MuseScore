/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

/**
 \file
 Definition of classes SysStaff and System
*/

#include "engravingitem.h"
#include "symbol.h"
#include "skyline.h"

namespace mu::engraving {
class LayoutContext;
}

namespace mu::engraving {
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
    mu::RectF _bbox;                   // Bbox of StaffLines.
    Skyline _skyline;
    qreal _yOff   { 0.0 };          // offset of top staff line within bbox
    qreal _yPos   { 0.0 };          // y position of bbox after System::layout2
    qreal _height { 0.0 };          // height of bbox after System::layout2
    qreal _continuousDist { -1.0 }; // distance for continuous mode
    bool _show  { true };           // derived from Staff or false if empty
                                    // staff is hidden
public:
    //int idx     { 0    };
    std::vector<InstrumentName*> instrumentNames;

    const mu::RectF& bbox() const { return _bbox; }
    mu::RectF& bbox() { return _bbox; }
    void setbbox(const mu::RectF& r) { _bbox = r; }
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

class System final : public EngravingItem
{
    SystemDivider* _systemDividerLeft    { nullptr };       // to the next system
    SystemDivider* _systemDividerRight   { nullptr };

    std::vector<MeasureBase*> ml;
    std::vector<SysStaff*> _staves;
    std::vector<Bracket*> _brackets;
    std::list<SpannerSegment*> _spannerSegments;

    qreal _leftMargin              { 0.0 };     ///< left margin for instrument name, brackets etc.
    mutable bool fixedDownDistance { false };
    qreal _distance                { 0.0 };     /// temp. variable used during layout
    qreal _systemHeight            { 0.0 };

    friend class mu::engraving::Factory;
    System(Page* parent);

    staff_idx_t firstVisibleSysStaff() const;
    staff_idx_t lastVisibleSysStaff() const;

    staff_idx_t firstVisibleStaffFrom(staff_idx_t startStaffIdx) const;

    size_t getBracketsColumnsCount();
    void setBracketsXPosition(const qreal xOffset);
    Bracket* createBracket(const mu::engraving::LayoutContext& ctx, mu::engraving::BracketItem* bi, size_t column, staff_idx_t staffIdx,
                           std::vector<Bracket*>& bl, Measure* measure);

    qreal systemNamesWidth();
    qreal layoutBrackets(const mu::engraving::LayoutContext& ctx);
    qreal totalBracketOffset(const mu::engraving::LayoutContext& ctx);

public:

    ~System();

    void moveToPage(Page* parent);

    // Score Tree functions
    EngravingObject* scanParent() const override;
    EngravingObjectList scanChildren() const override;

    System* clone() const override { return new System(*this); }

    void add(EngravingItem*) override;
    void remove(EngravingItem*) override;
    void change(EngravingItem* o, EngravingItem* n) override;
    void write(XmlWriter&) const override;
    void read(XmlReader&) override;

    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;

    void appendMeasure(MeasureBase*);
    void removeMeasure(MeasureBase*);
    void removeLastMeasure();

    Page* page() const { return (Page*)explicitParent(); }

    void layoutSystem(const mu::engraving::LayoutContext& ctx, qreal xo1, const bool isFirstSystem = false, bool firstSystemIndent = false);

    void setMeasureHeight(qreal height);
    void layoutBracketsVertical();
    void layoutInstrumentNames();

    void addBrackets(const mu::engraving::LayoutContext& ctx, Measure* measure);

    void layout2(const mu::engraving::LayoutContext& ctx);                       ///< Called after Measure layout.
    void restoreLayout2();
    void clear();                         ///< Clear measure list.

    mu::RectF bboxStaff(int staff) const { return _staves[staff]->bbox(); }
    std::vector<SysStaff*>& staves() { return _staves; }
    const std::vector<SysStaff*>& staves() const { return _staves; }
    qreal staffYpage(staff_idx_t staffIdx) const;
    qreal staffCanvasYpage(staff_idx_t staffIdx) const;
    SysStaff* staff(size_t staffIdx) const;

    bool pageBreak() const;

    SysStaff* insertStaff(int);
    void removeStaff(int);
    void adjustStavesNumber(size_t nstaves);

    int y2staff(qreal y) const;
    staff_idx_t searchStaff(qreal y, staff_idx_t preferredStaff = mu::nidx, qreal spacingFactor = 0.5) const;
    void setInstrumentNames(const mu::engraving::LayoutContext& ctx, bool longName, Fraction tick = { 0, 1 });
    Fraction snap(const Fraction& tick, const mu::PointF p) const;
    Fraction snapNote(const Fraction& tick, const mu::PointF p, int staff) const;

    const std::vector<MeasureBase*>& measures() const { return ml; }

    MeasureBase* measure(int idx) { return ml[idx]; }
    Measure* firstMeasure() const;
    Measure* lastMeasure() const;
    Fraction endTick() const;

    MeasureBase* nextMeasure(const MeasureBase*) const;

    qreal leftMargin() const { return _leftMargin; }
    Box* vbox() const;

    const std::vector<Bracket*>& brackets() const { return _brackets; }

    std::list<SpannerSegment*>& spannerSegments() { return _spannerSegments; }
    const std::list<SpannerSegment*>& spannerSegments() const { return _spannerSegments; }

    SystemDivider* systemDividerLeft() const { return _systemDividerLeft; }
    SystemDivider* systemDividerRight() const { return _systemDividerRight; }

    EngravingItem* nextSegmentElement() override;
    EngravingItem* prevSegmentElement() override;

    qreal minDistance(System*) const;
    qreal topDistance(staff_idx_t staffIdx, const SkylineLine&) const;
    qreal bottomDistance(staff_idx_t staffIdx, const SkylineLine&) const;
    qreal minTop() const;
    qreal minBottom() const;
    qreal spacerDistance(bool up) const;
    Spacer* upSpacer(staff_idx_t staffIdx, Spacer* prevDownSpacer) const;
    Spacer* downSpacer(staff_idx_t staffIdx) const;

    qreal firstNoteRestSegmentX(bool leading = false);
    qreal lastNoteRestSegmentX(bool trailing = false);
    ChordRest* lastChordRest(track_idx_t track);
    ChordRest* firstChordRest(track_idx_t track);

    bool hasFixedDownDistance() const { return fixedDownDistance; }
    staff_idx_t firstVisibleStaff() const;
    staff_idx_t nextVisibleStaff(staff_idx_t) const;
    qreal distance() const { return _distance; }
    void setDistance(qreal d) { _distance = d; }

    staff_idx_t firstSysStaffOfPart(const Part* part) const;
    staff_idx_t firstVisibleSysStaffOfPart(const Part* part) const;
    staff_idx_t lastSysStaffOfPart(const Part* part) const;
    staff_idx_t lastVisibleSysStaffOfPart(const Part* part) const;

    Fraction minSysTicks() const;
    Fraction maxSysTicks() const;

    double squeezableSpace() const;
};

typedef std::vector<System*>::iterator iSystem;
typedef std::vector<System*>::const_iterator ciSystem;
}     // namespace Ms
#endif
