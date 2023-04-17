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

#ifndef __SEGMENT_H__
#define __SEGMENT_H__

#include "engravingitem.h"

#include "shape.h"
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

    SegmentType _segmentType { SegmentType::Invalid };
    Fraction _tick;    // { Fraction(0, 1) };
    Fraction _ticks;   // { Fraction(0, 1) };
    Spatium _extraLeadingSpace;
    double _stretch;
    double _widthOffset = 0.0; // part of the segment width that will not be stretched during system justification

    Segment* _next = nullptr;                       // linked list of segments inside a measure
    Segment* _prev = nullptr;

    std::vector<EngravingItem*> _annotations;
    std::vector<EngravingItem*> _elist;         // EngravingItem storage, size = staves * VOICES.
    std::vector<EngravingItem*> _preAppendedItems; // Container for items appended to the left of this segment (example: grace notes), size = staves * VOICES.
    std::vector<Shape> _shapes;           // size = staves
    double m_spacing{ 0 };

    CrossBeamType _crossBeamType; // Will affect segment-to-segment horizontal spacing

    friend class Factory;
    Segment(Measure* m = 0);
    Segment(Measure*, SegmentType, const Fraction&);
    Segment(const Segment&);

    void init();
    void checkEmpty() const;
    void checkElement(EngravingItem*, track_idx_t track);
    void setEmpty(bool val) const { setFlag(ElementFlag::EMPTY, val); }

protected:
    EngravingItem* getElement(staff_idx_t staff);       //??

public:

    ~Segment();

    void setParent(Measure* parent);

    // Score Tree functions
    EngravingObject* scanParent() const override;
    EngravingObjectList scanChildren() const override;

    Segment* clone() const override { return new Segment(*this); }

    void setScore(Score*) override;

    Segment* next() const { return _next; }
    Segment* next(SegmentType) const;
    Segment* nextActive() const;
    Segment* nextEnabled() const;
    Segment* nextInStaff(staff_idx_t staffIdx, SegmentType t = SegmentType::ChordRest) const;
    void setNext(Segment* e) { _next = e; }

    Segment* prev() const { return _prev; }
    Segment* prev(SegmentType) const;
    Segment* prevActive() const;
    Segment* prevEnabled() const;
    void setPrev(Segment* e) { _prev = e; }

    // don’t stop at measure boundary:
    Segment* next1() const;
    Segment* next1enabled() const;
    Segment* next1MM() const;
    Segment* next1MMenabled() const;
    Segment* next1(SegmentType) const;
    Segment* next1MM(SegmentType) const;

    Segment* prev1() const;
    Segment* prev1enabled() const;
    Segment* prev1MM() const;
    Segment* prev1MMenabled() const;
    Segment* prev1(SegmentType) const;
    Segment* prev1MM(SegmentType) const;

    Segment* nextCR(track_idx_t track = mu::nidx, bool sameStaff = false) const;

    ChordRest* nextChordRest(track_idx_t track, bool backwards = false) const;

    EngravingItem* element(track_idx_t track) const;

    // a variant of the above function, specifically designed to be called from QML
    //@ returns the element at track 'track' (null if none)
    EngravingItem* elementAt(track_idx_t track) const;

    const std::vector<EngravingItem*>& elist() const { return _elist; }
    std::vector<EngravingItem*>& elist() { return _elist; }

    void removeElement(track_idx_t track);
    void setElement(track_idx_t track, EngravingItem* el);
    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;

    Measure* measure() const { return toMeasure(explicitParent()); }
    System* system() const { return toSystem(explicitParent()->explicitParent()); }
    double x() const override { return ipos().x(); }

    mu::RectF contentRect() const;

    void insertStaff(staff_idx_t staff);
    void removeStaff(staff_idx_t staff);

    void add(EngravingItem*) override;
    void remove(EngravingItem*) override;
    void swapElements(track_idx_t i1, track_idx_t i2);

    void sortStaves(std::vector<staff_idx_t>& dst);
    const char* subTypeName() const;

    static const char* subTypeName(SegmentType);
    static SegmentType segmentType(ElementType type);

    SegmentType segmentType() const { return _segmentType; }
    void setSegmentType(SegmentType t);

    bool empty() const { return flag(ElementFlag::EMPTY); }
    bool written() const { return flag(ElementFlag::WRITTEN); }
    void setWritten(bool val) const { setFlag(ElementFlag::WRITTEN, val); }

    void fixStaffIdx();

    double stretch() const { return _stretch; }
    void setStretch(double v) { _stretch = v; }
    double computeDurationStretch(Segment* prevSeg, Fraction minTicks, Fraction maxTicks);

    Fraction rtick() const override { return _tick; }
    void setRtick(const Fraction& v) { assert(v >= Fraction(0, 1)); _tick = v; }
    Fraction tick() const override;

    Fraction ticks() const { return _ticks; }
    void setTicks(const Fraction& v) { _ticks = v; }

    double widthInStaff(staff_idx_t staffIdx, SegmentType t = SegmentType::ChordRest) const;
    Fraction ticksInStaff(staff_idx_t staffIdx) const;

    bool splitsTuplet() const;

    const std::vector<EngravingItem*>& annotations() const { return _annotations; }
    void clearAnnotations();
    void removeAnnotation(EngravingItem* e);
    bool hasAnnotationOrElement(ElementType type, track_idx_t minTrack, track_idx_t maxTrack) const;
    EngravingItem* findAnnotation(ElementType type, track_idx_t minTrack, track_idx_t maxTrack) const;
    std::vector<EngravingItem*> findAnnotations(ElementType type, track_idx_t minTrack, track_idx_t maxTrack) const;
    bool hasElements() const;
    bool hasElements(track_idx_t minTrack, track_idx_t maxTrack) const;
    bool allElementsInvisible() const;

    Spatium extraLeadingSpace() const { return _extraLeadingSpace; }
    void setExtraLeadingSpace(Spatium v) { _extraLeadingSpace = v; }

    void write(XmlWriter&) const override;

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

    bool operator<(const Segment&) const;
    bool operator>(const Segment&) const;

    String accessibleExtraInfo() const override;

#ifndef ENGRAVING_NO_ACCESSIBILITY
    AccessibleItemPtr createAccessible() override;
#endif

    EngravingItem* firstInNextSegments(staff_idx_t activeStaff);   //<
    EngravingItem* lastInPrevSegments(staff_idx_t activeStaff);     //<
    EngravingItem* firstElement(staff_idx_t staff);                //<  These methods are used for navigation
    EngravingItem* lastElement(staff_idx_t staff);                 //<  for next-element and prev-element
    EngravingItem* firstElementOfSegment(Segment* s, staff_idx_t activeStaff);
    EngravingItem* nextElementOfSegment(Segment* s, EngravingItem* e, staff_idx_t activeStaff);
    EngravingItem* prevElementOfSegment(Segment* s, EngravingItem* e, staff_idx_t activeStaff);
    EngravingItem* lastElementOfSegment(Segment* s, staff_idx_t activeStaff);
    EngravingItem* nextAnnotation(EngravingItem* e);
    EngravingItem* prevAnnotation(EngravingItem* e);
    EngravingItem* firstAnnotation(Segment* s, staff_idx_t activeStaff);
    EngravingItem* lastAnnotation(Segment* s, staff_idx_t activeStaff);
    Spanner* firstSpanner(staff_idx_t activeStaff);
    Spanner* lastSpanner(staff_idx_t activeStaff);
    bool notChordRestType(Segment* s);
    using EngravingItem::nextElement;
    EngravingItem* nextElement(staff_idx_t activeStaff);
    using EngravingItem::prevElement;
    EngravingItem* prevElement(staff_idx_t activeStaff);

    std::vector<Shape> shapes() { return _shapes; }
    const std::vector<Shape>& shapes() const { return _shapes; }
    const Shape& staffShape(staff_idx_t staffIdx) const { return _shapes[staffIdx]; }
    Shape& staffShape(staff_idx_t staffIdx) { return _shapes[staffIdx]; }
    void createShapes();
    void createShape(staff_idx_t staffIdx);
    double minRight() const;
    double minLeft(const Shape&) const;
    double minLeft() const;
    double minHorizontalDistance(Segment*, bool isSystemGap) const;
    double minHorizontalCollidingDistance(Segment* ns) const;

    double widthOffset() const { return _widthOffset; }
    void setWidthOffset(double w) { _widthOffset = w; }

    static void stretchSegmentsToWidth(std::vector<Spring>& springs, double width);

    double elementsTopOffsetFromSkyline(staff_idx_t staffIndex) const;
    double elementsBottomOffsetFromSkyline(staff_idx_t staffIndex) const;

    /*! \brief callulate width of segment and additional spacing of segment depends on duration of segment
     *  \return pair of {spacing, width}
     */
    std::pair<double, double> computeCellWidth(const std::vector<int>& visibleParts) const;

    /*! \brief get among all ChordRests of segment the ChordRest with minimum ticks,
    * take into account visibleParts
    */
    static ChordRest* ChordRestWithMinDuration(const Segment* seg, const std::vector<int>& visibleParts);

    //! spacing is additional width of segment, for example accidental needs this spacing to avoid overlapping
    void setSpacing(double);
    double spacing() const;

    // some helper function
    ChordRest* cr(track_idx_t track) const { return toChordRest(_elist[track]); }
    bool isType(const SegmentType t) const { return int(_segmentType) & int(t); }
    bool isBeginBarLineType() const { return _segmentType == SegmentType::BeginBarLine; }
    bool isClefType() const { return _segmentType == SegmentType::Clef; }
    bool isHeaderClefType() const { return _segmentType == SegmentType::HeaderClef; }
    bool isKeySigType() const { return _segmentType == SegmentType::KeySig; }
    bool isAmbitusType() const { return _segmentType == SegmentType::Ambitus; }
    bool isTimeSigType() const { return _segmentType == SegmentType::TimeSig; }
    bool isStartRepeatBarLineType() const { return _segmentType == SegmentType::StartRepeatBarLine; }
    bool isBarLineType() const { return _segmentType == SegmentType::BarLine; }
    bool isBreathType() const { return _segmentType == SegmentType::Breath; }
    bool isChordRestType() const { return _segmentType == SegmentType::ChordRest; }
    bool isEndBarLineType() const { return _segmentType == SegmentType::EndBarLine; }
    bool isKeySigAnnounceType() const { return _segmentType == SegmentType::KeySigAnnounce; }
    bool isTimeSigAnnounceType() const { return _segmentType == SegmentType::TimeSigAnnounce; }
    bool isRightAligned() const { return isClefType() || isBreathType(); }

    Fraction shortestChordRest() const;
    void computeCrossBeamType(Segment* nextSeg);
    CrossBeamType crossBeamType() const { return _crossBeamType; }

    bool hasAccidentals() const;

    EngravingItem* preAppendedItem(int track) { return _preAppendedItems[track]; }
    void preAppend(EngravingItem* item, int track) { _preAppendedItems[track] = item; }
    void clearPreAppended(int track) { _preAppendedItems[track] = nullptr; }
    void addPreAppendedToShape();

    static constexpr SegmentType durationSegmentsMask = SegmentType::ChordRest;   // segment types which may have non-zero tick length
};

//---------------------------------------------------------
//   nextActive
//---------------------------------------------------------

inline Segment* Segment::nextActive() const
{
    Segment* ns = next();
    while (ns && !(ns->enabled() && ns->visible())) {
        ns = ns->next();
    }
    return ns;
}

//---------------------------------------------------------
//   nextEnabled
//---------------------------------------------------------

inline Segment* Segment::nextEnabled() const
{
    Segment* ns = next();
    while (ns && !ns->enabled()) {
        ns = ns->next();
    }
    return ns;
}

//---------------------------------------------------------
//   prevActive
//---------------------------------------------------------

inline Segment* Segment::prevActive() const
{
    Segment* ps = prev();
    while (ps && !(ps->enabled() && ps->visible())) {
        ps = ps->prev();
    }
    return ps;
}

//---------------------------------------------------------
//   prevEnabled
//---------------------------------------------------------

inline Segment* Segment::prevEnabled() const
{
    Segment* ps = prev();
    while (ps && !ps->enabled()) {
        ps = ps->prev();
    }
    return ps;
}
} // namespace mu::engraving

#ifndef NO_QT_SUPPORT
Q_DECLARE_METATYPE(mu::engraving::SegmentType);
#endif

#endif
