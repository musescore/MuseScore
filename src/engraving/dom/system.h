/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#pragma once

/**
 \file
 Definition of classes SysStaff and System
*/

#include "engravingitem.h"

namespace mu::engraving {
class Box;
class Bracket;
class InstrumentName;
class MeasureBase;
class Page;
class SpannerSegment;
class StaffVisibilityIndicator;
class SystemLock;

//---------------------------------------------------------
//   SysStaff
///  One staff in a System.
//---------------------------------------------------------

class SysStaff
{
public:
    SysStaff() {}
    ~SysStaff();

    //int idx     { 0    };
    std::vector<InstrumentName*> instrumentNames;

    const RectF& bbox() const { return m_bbox; }
    RectF& bbox() { return m_bbox; }
    void setbbox(const RectF& r) { m_bbox = r; }
    void setbbox(double x, double y, double w, double h) { m_bbox.setRect(x, y, w, h); }
    double y() const { return m_bbox.y() + m_yOff; }
    void setYOff(double offset) { m_yOff = offset; }
    double yOffset() const { return m_yOff; }
    double yBottom() const;

    void saveLayout();
    void restoreLayout();

    double continuousDist() const { return m_continuousDist; }
    void setContinuousDist(double val) { m_continuousDist = val; }

    bool show() const { return m_show; }
    void setShow(bool v) { m_show = v; }

    const Skyline& skyline() const { return m_skyline; }
    Skyline& skyline() { return m_skyline; }

private:
    RectF m_bbox;               // Bbox of StaffLines.
    Skyline m_skyline;
    double m_yOff = 0.0;            // offset of top staff line within bbox
    double m_yPos = 0.0;            // y position of bbox after System::layout2
    double m_height = 0.0;          // height of bbox after System::layout2
    double m_continuousDist = -1.0; // distance for continuous mode
    bool m_show = true;             // derived from Staff or false if empty
                                    // staff is hidden
};

//---------------------------------------------------------
//   System
///    One row of measures for all instruments;
///    a complete piece of the timeline.
//---------------------------------------------------------

class System final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, System)
    DECLARE_CLASSOF(ElementType::SYSTEM)

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

    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;

    void appendMeasure(MeasureBase*);
    void removeMeasure(MeasureBase*);
    void removeLastMeasure();

    Page* page() const { return (Page*)explicitParent(); }

    void clear(); ///< Clear measure list.

    std::vector<SysStaff*>& staves() { return m_staves; }
    const std::vector<SysStaff*>& staves() const { return m_staves; }
    double staffYpage(staff_idx_t staffIdx) const;
    double staffCanvasYpage(staff_idx_t staffIdx) const;
    SysStaff* staff(size_t staffIdx) const;

    bool pageBreak() const;

    SysStaff* insertStaff(int);
    void removeStaff(int);
    void adjustStavesNumber(size_t nstaves);

    staff_idx_t searchStaff(double y, staff_idx_t preferredStaff = muse::nidx, double spacingFactor = 0.5) const;
    Fraction snap(const Fraction& tick, const PointF p) const;
    Fraction snapNote(const Fraction& tick, const PointF p, int staff) const;

    const std::vector<MeasureBase*>& measures() const { return m_ml; }
    std::vector<MeasureBase*>& measures() { return m_ml; }

    MeasureBase* measure(int idx) { return m_ml[idx]; }
    MeasureBase* first() const { return m_ml.front(); }
    MeasureBase* last() const { return m_ml.back(); }
    Measure* firstMeasure() const;
    Measure* lastMeasure() const;
    Fraction endTick() const;

    MeasureBase* nextMeasure(const MeasureBase*) const;

    double leftMargin() const { return m_leftMargin; }
    void setLeftMargin(double val) { m_leftMargin = val; }

    double systemHeight() const { return m_systemHeight; }
    void setSystemHeight(double h) { m_systemHeight = h; }

    Box* vbox() const;

    const std::vector<Bracket*>& brackets() const { return m_brackets; }
    std::vector<Bracket*>& brackets() { return m_brackets; }

    std::list<SpannerSegment*>& spannerSegments() { return m_spannerSegments; }
    const std::list<SpannerSegment*>& spannerSegments() const { return m_spannerSegments; }

    SystemDivider* systemDividerLeft() const { return m_systemDividerLeft; }
    SystemDivider* systemDividerRight() const { return m_systemDividerRight; }

    EngravingItem* nextSegmentElement() override;
    EngravingItem* prevSegmentElement() override;

    double topDistance(staff_idx_t staffIdx, const SkylineLine&) const;
    double bottomDistance(staff_idx_t staffIdx, const SkylineLine&) const;
    double minTop() const;
    double minBottom() const;
    double spacerDistance(bool up) const;
    Spacer* upSpacer(staff_idx_t staffIdx, Spacer* prevDownSpacer) const;
    Spacer* downSpacer(staff_idx_t staffIdx) const;

    double firstNoteRestSegmentX(bool leading = false) const;
    double endingXForOpenEndedLines() const;
    ChordRest* lastChordRest(track_idx_t track) const;
    ChordRest* firstChordRest(track_idx_t track) const;

    bool hasFixedDownDistance() const { return m_fixedDownDistance; }
    void setFixedDownDistance(bool val) const { m_fixedDownDistance = val; }

    staff_idx_t firstVisibleStaff() const;
    staff_idx_t nextVisibleStaff(staff_idx_t) const;
    staff_idx_t prevVisibleStaff(staff_idx_t) const;
    staff_idx_t lastVisibleStaff() const;

    double distance() const { return m_distance; }
    void setDistance(double d) { m_distance = d; }

    staff_idx_t firstSysStaffOfPart(const Part* part) const;
    staff_idx_t firstVisibleSysStaffOfPart(const Part* part) const;
    staff_idx_t lastSysStaffOfPart(const Part* part) const;
    staff_idx_t lastVisibleSysStaffOfPart(const Part* part) const;

#ifndef ENGRAVING_NO_ACCESSIBILITY
    AccessibleItemPtr createAccessible() override;
#endif

    void setBracketsXPosition(const double xOffset);
    size_t getBracketsColumnsCount();

    void resetShortestLongestChordRest();

    StaffVisibilityIndicator* staffVisibilityIndicator() const { return m_staffVisibilityIndicator; }

    bool isLocked() const;
    const SystemLock* systemLock() const;

    const std::vector<SystemLockIndicator*> lockIndicators() const { return m_lockIndicators; }
    void addLockIndicator(SystemLockIndicator* sli);
    void deleteLockIndicators();

private:
    friend class Factory;

    System(Page* parent);

    staff_idx_t firstVisibleSysStaff() const;
    staff_idx_t lastVisibleSysStaff() const;

    staff_idx_t firstVisibleStaffFrom(staff_idx_t startStaffIdx) const;

    SystemDivider* m_systemDividerLeft = nullptr;       // to the next system
    SystemDivider* m_systemDividerRight = nullptr;

    std::vector<MeasureBase*> m_ml;
    std::vector<SysStaff*> m_staves;
    std::vector<Bracket*> m_brackets;
    std::list<SpannerSegment*> m_spannerSegments;
    std::vector<SystemLockIndicator*> m_lockIndicators;

    StaffVisibilityIndicator* m_staffVisibilityIndicator = nullptr;

    double m_leftMargin = 0.0;      // left margin for instrument name, brackets etc.
    mutable bool m_fixedDownDistance = false;
    double m_distance = 0.0;        // temp. variable used during layout
    double m_systemHeight = 0.0;
};

typedef std::vector<System*>::iterator iSystem;
typedef std::vector<System*>::const_iterator ciSystem;
} // namespace mu::engraving
