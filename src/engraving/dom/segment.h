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

#include "engravingitem.h"

#include "types.h"

namespace mu::engraving {
class Factory;
class Measure;
class Segment;
class ChordRest;
class Spanner;
class System;

//------------------------------------------------------------------------
//   @@ Segment
//    A segment holds all vertical aligned staff elements.
//    Segments are typed and contain only Elements of the same type.
//
//    All Elements in a segment start at the same tick. The Segment can store one EngravingItem for
//    each voice in each staff in the score.
//    Some elements (Clef, KeySig, TimeSig etc.) are assumed to always have voice zero
//    and can be found in _elist[staffIdx * VOICES];

//    Segments are children of Measures and store Clefs, KeySigs, TimeSigs,
//    BarLines and ChordRests.
//
//   @P annotations     array[EngravingItem]    the list of annotations (read only)
//   @P next            Segment           the next segment in the whole score; null at last score segment (read-only)
//   @P nextInMeasure   Segment           the next segment in measure; null at last measure segment (read-only)
//   @P prev            Segment           the previous segment in the whole score; null at first score segment (read-only)
//   @P prevInMeasure   Segment           the previous segment in measure; null at first measure segment (read-only)
//   @P segmentType     enum (Segment.All, .Ambitus, .BarLine, .Breath, .ChordRest, .Clef, .EndBarLine, .Invalid, .KeySig, .KeySigAnnounce, .StartRepeatBarLine, .TimeSig, .TimeSigAnnounce)
//   @P tick            int               midi tick position (read only)
//------------------------------------------------------------------------

struct CrossBeamType
{
    bool upDown = false; // This chord is stem-up, next chord is stem-down
    bool downUp = false; // This chord is stem-down, next chord is stem-up
    bool canBeAdjusted = true;
    void reset()
    {
        upDown = false;
        downUp = false;
        canBeAdjusted = true;
    }
};

struct Spring
{
    double springConst = 0.0;
    double width = 0.0;
    double preTension = 0.0;
    Segment* segment = nullptr;
    Spring(double sc, double w, double pt, Segment* s)
        : springConst(sc), width(w), preTension(pt),  segment(s) {}
};

class Segment final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, Segment)
    DECLARE_CLASSOF(ElementType::SEGMENT)

protected:
    EngravingItem* getElement(staff_idx_t staff) const;       //??

public:

    ~Segment();

    void setParent(Measure* parent);

    // Score Tree functions
    EngravingObject* scanParent() const override;
    EngravingObjectList scanChildren() const override;

    Segment* clone() const override { return new Segment(*this); }

    void setScore(Score*) override;

    inline bool isActive() const { return !isTimeTickType() && enabled() && visible(); }

    Segment* next() const { return m_next; }
    Segment* next(SegmentType) const;
    Segment* nextActive() const;
    Segment* nextEnabled() const;
    Segment* nextInStaff(staff_idx_t staffIdx, SegmentType t = SegmentType::ChordRest) const;
    void setNext(Segment* e) { m_next = e; }

    Segment* prev() const { return m_prev; }
    Segment* prev(SegmentType) const;
    Segment* prevActive() const;
    Segment* prevEnabled() const;
    void setPrev(Segment* e) { m_prev = e; }

    // don’t stop at measure boundary:
    Segment* next1() const;
    Segment* next1enabled() const;
    Segment* next1MM() const;
    Segment* next1MMenabled() const;
    Segment* next1(SegmentType) const;
    Segment* next1ChordRestOrTimeTick() const;
    Segment* next1WithElemsOnStaff(staff_idx_t staffIdx, SegmentType segType = SegmentType::ChordRest) const;
    Segment* next1MM(SegmentType) const;

    Segment* prev1() const;
    Segment* prev1ChordRestOrTimeTick() const;
    Segment* prev1WithElemsOnStaff(staff_idx_t staffIdx, SegmentType segType = SegmentType::ChordRest) const;
    Segment* prev1enabled() const;
    Segment* prev1MM() const;
    Segment* prev1MMenabled() const;
    Segment* prev1(SegmentType) const;
    Segment* prev1MM(SegmentType) const;

    Segment* nextCR(track_idx_t track = muse::nidx, bool sameStaff = false) const;

    ChordRest* nextChordRest(track_idx_t track, bool backwards = false, bool stopAtMeasureBoundary = false) const;

    EngravingItem* element(track_idx_t track) const;

    // a variant of the above function, specifically designed to be called from QML
    //@ returns the element at track 'track' (null if none)
    EngravingItem* elementAt(track_idx_t track) const;

    const std::vector<EngravingItem*>& elist() const { return m_elist; }
    std::vector<EngravingItem*>& elist() { return m_elist; }

    void removeElement(track_idx_t track);
    void setElement(track_idx_t track, EngravingItem* el);
    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;

    Measure* measure() const { return toMeasure(explicitParent()); }
    System* system() const { return toSystem(explicitParent()->explicitParent()); }
    double x() const override { return ldata()->pos().x(); }

    RectF contentRect() const;

    void insertStaff(staff_idx_t staff);
    void removeStaff(staff_idx_t staff);

    void add(EngravingItem*) override;
    void remove(EngravingItem*) override;
    void swapElements(track_idx_t i1, track_idx_t i2);

    void sortStaves(std::vector<staff_idx_t>& dst);
    const char* subTypeName() const;

    static const char* subTypeName(SegmentType);
    static SegmentType segmentType(ElementType type);

    SegmentType segmentType() const { return m_segmentType; }
    void setSegmentType(SegmentType t);

    bool empty() const { return flag(ElementFlag::EMPTY); }
    bool written() const { return flag(ElementFlag::WRITTEN); }
    void setWritten(bool val) const { setFlag(ElementFlag::WRITTEN, val); }
    bool endOfMeasureChange() const { return flag(ElementFlag::END_OF_MEASURE_CHANGE); }         // Key/time sigs which should be placed at the end of the measure
    void setEndOfMeasureChange(bool val) const { setFlag(ElementFlag::END_OF_MEASURE_CHANGE, val); }

    void fixStaffIdx();

    double stretch() const { return m_stretch; }
    void setStretch(double v) { m_stretch = v; }

    Fraction rtick() const override { return m_tick; }
    void setRtick(const Fraction& v) { assert(v >= Fraction(0, 1)); m_tick = v; }
    Fraction tick() const override;

    Fraction ticks() const { return m_ticks; }
    void setTicks(const Fraction& v) { m_ticks = v; }

    double widthInStaff(staff_idx_t staffIdx, SegmentType nextSegType = SegmentType::ChordRest) const;
    Fraction ticksInStaff(staff_idx_t staffIdx) const;

    bool splitsTuplet() const;

    const std::vector<EngravingItem*>& annotations() const { return m_annotations; }
    void clearAnnotations();
    void removeAnnotation(EngravingItem* e);
    bool hasAnnotationOrElement(ElementType type, track_idx_t minTrack, track_idx_t maxTrack) const;
    EngravingItem* findAnnotation(ElementType type, track_idx_t minTrack, track_idx_t maxTrack) const;
    std::vector<EngravingItem*> findAnnotations(ElementType type, track_idx_t minTrack, track_idx_t maxTrack) const;
    bool hasElements() const;
    bool hasElements(track_idx_t minTrack, track_idx_t maxTrack) const;
    bool hasElements(staff_idx_t staffIdx) const;
    bool allElementsInvisible() const;

    Spatium extraLeadingSpace() const { return m_extraLeadingSpace; }
    void setExtraLeadingSpace(Spatium v) { m_extraLeadingSpace = v; }

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

    bool operator<(const Segment&) const;
    bool operator>(const Segment&) const;

    String accessibleExtraInfo() const override;

#ifndef ENGRAVING_NO_ACCESSIBILITY
    AccessibleItemPtr createAccessible() override;
#endif

    EngravingItem* firstInNextSegments(staff_idx_t activeStaff) const;   //<
    EngravingItem* lastInPrevSegments(staff_idx_t activeStaff) const;     //<
    EngravingItem* firstElementForNavigation(staff_idx_t staff) const;          //<  These methods are used for navigation
    EngravingItem* lastElementForNavigation(staff_idx_t staff) const;           //<  for next-element and prev-element
    EngravingItem* firstElementOfSegment(staff_idx_t activeStaff) const;
    EngravingItem* nextElementOfSegment(EngravingItem* e, staff_idx_t activeStaff) const;
    EngravingItem* prevElementOfSegment(EngravingItem* e, staff_idx_t activeStaff) const;
    EngravingItem* lastElementOfSegment(staff_idx_t activeStaff) const;
    EngravingItem* nextAnnotation(EngravingItem* e) const;
    EngravingItem* prevAnnotation(EngravingItem* e) const;
    EngravingItem* firstAnnotation(staff_idx_t activeStaff) const;
    EngravingItem* lastAnnotation(staff_idx_t activeStaff) const;
    Spanner* firstSpanner(staff_idx_t activeStaff) const;
    Spanner* lastSpanner(staff_idx_t activeStaff) const;
    bool notChordRestType() const;
    using EngravingItem::nextElement;
    EngravingItem* nextElement(staff_idx_t activeStaff);
    using EngravingItem::prevElement;
    EngravingItem* prevElement(staff_idx_t activeStaff);

    EngravingItem* firstElement(staff_idx_t staffIdx) const;

    std::vector<Shape> shapes() { return m_shapes; }
    const std::vector<Shape>& shapes() const { return m_shapes; }
    const Shape& staffShape(staff_idx_t staffIdx) const { return m_shapes[staffIdx]; }
    Shape& staffShape(staff_idx_t staffIdx) { return m_shapes[staffIdx]; }
    void createShapes();
    void createShape(staff_idx_t staffIdx);
    double minRight() const;
    double minLeft() const;

    double widthOffset() const { return m_widthOffset; }
    void clearWidthOffset() { m_widthOffset = 0.0; }
    void addWidthOffset(double w) { m_widthOffset += w; }
    void setWidthOffset(double w) { m_widthOffset = w; }

    double elementsTopOffsetFromSkyline(staff_idx_t staffIndex) const;
    double elementsBottomOffsetFromSkyline(staff_idx_t staffIndex) const;

    //! spacing is additional width of segment, for example accidental needs this spacing to avoid overlapping
    void setSpacing(double);
    double spacing() const;

    // some helper function
    ChordRest* cr(track_idx_t track) const { return toChordRest(m_elist[track]); }
    bool isType(const SegmentType t) const { return int(m_segmentType) & int(t); }
    bool isJustType(const SegmentType t) const { return m_segmentType == t; }
    bool isBeginBarLineType() const { return m_segmentType == SegmentType::BeginBarLine; }
    bool isClefType() const { return m_segmentType == SegmentType::Clef; }
    bool isHeaderClefType() const { return m_segmentType == SegmentType::HeaderClef; }
    bool isKeySigType() const { return m_segmentType == SegmentType::KeySig; }
    bool isAmbitusType() const { return m_segmentType == SegmentType::Ambitus; }
    bool isTimeSigType() const { return m_segmentType == SegmentType::TimeSig; }
    bool hasTimeSigAboveStaves() const;
    bool makeSpaceForTimeSigAboveStaves() const;
    bool hasTimeSigAcrossStaves() const;
    bool isStartRepeatBarLineType() const { return m_segmentType == SegmentType::StartRepeatBarLine; }
    bool isBarLineType() const { return m_segmentType == SegmentType::BarLine; }
    bool isBreathType() const { return m_segmentType == SegmentType::Breath; }
    bool isChordRestType() const { return m_segmentType == SegmentType::ChordRest; }
    bool isClefRepeatAnnounceType() const { return m_segmentType == SegmentType::ClefRepeatAnnounce; }
    bool isKeySigRepeatAnnounceType() const { return m_segmentType == SegmentType::KeySigRepeatAnnounce; }
    bool isTimeSigRepeatAnnounceType() const { return m_segmentType == SegmentType::TimeSigRepeatAnnounce; }
    bool isEndBarLineType() const { return m_segmentType == SegmentType::EndBarLine; }
    bool isKeySigAnnounceType() const { return m_segmentType == SegmentType::KeySigAnnounce; }
    bool isTimeSigAnnounceType() const { return m_segmentType == SegmentType::TimeSigAnnounce; }
    bool isCourtesySegment() const
    {
        return m_segmentType & (SegmentType::CourtesyTimeSigType | SegmentType::CourtesyKeySigType | SegmentType::CourtesyClefType);
    }

    bool isTimeTickType() const { return m_segmentType == SegmentType::TimeTick; }
    bool isRightAligned() const { return isClefType() || isBreathType(); }
    bool isMMRestSegment() const { return isChordRestType() && m_elist.front() && m_elist.front()->isMMRest(); }

    static constexpr SegmentType CHORD_REST_OR_TIME_TICK_TYPE = SegmentType::ChordRest | SegmentType::TimeTick;
    static constexpr SegmentType durationSegmentsMask = CHORD_REST_OR_TIME_TICK_TYPE; // segment types which may have non-zero tick length

    bool canWriteSpannerStartEnd(track_idx_t track, const Spanner* spanner) const;

    Fraction shortestChordRest() const;

    bool hasAccidentals() const;

    EngravingItem* preAppendedItem(track_idx_t track) { return m_preAppendedItems[track]; }
    void preAppend(EngravingItem* item, track_idx_t track) { m_preAppendedItems[track] = item; }
    void clearPreAppended(track_idx_t track) { m_preAppendedItems[track] = nullptr; }
    void addPreAppendedToShape();

    bool goesBefore(const Segment* nextSegment) const;

    void checkEmpty() const;

    double xPosInSystemCoords() const;
    void setXPosInSystemCoords(double x);

    bool isTupletSubdivision() const;
    bool isInsideTupletOnStaff(staff_idx_t staffIdx) const;

private:

    friend class Factory;
    Segment(Measure* m = 0);
    Segment(Measure*, SegmentType, const Fraction&);
    Segment(const Segment&);

    void init();
    void checkElement(EngravingItem*, track_idx_t track);
    void setEmpty(bool val) const { setFlag(ElementFlag::EMPTY, val); }

    SegmentType m_segmentType = SegmentType::Invalid;
    Fraction m_tick;    // { Fraction(0, 1) };
    Fraction m_ticks;   // { Fraction(0, 1) };
    Spatium m_extraLeadingSpace;
    double m_stretch = 1.0;
    double m_widthOffset = 0.0; // part of the segment width that will not be stretched during system justification

    Segment* m_next = nullptr;                       // linked list of segments inside a measure
    Segment* m_prev = nullptr;

    std::vector<EngravingItem*> m_annotations;
    std::vector<EngravingItem*> m_elist;         // EngravingItem storage, size = staves * VOICES.
    std::vector<EngravingItem*> m_preAppendedItems; // Container for items appended to the left of this segment (example: grace notes), size = staves * VOICES.
    std::vector<Shape> m_shapes;           // size = staves
    double m_spacing = 0;
};
} // namespace mu::engraving

#ifndef NO_QT_SUPPORT
Q_DECLARE_METATYPE(mu::engraving::SegmentType)
#endif
