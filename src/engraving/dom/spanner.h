/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include <deque>

#include "draw/types/color.h"

#include "../types/types.h"

#include "engravingitem.h"

namespace mu::engraving {
class Spanner;

//---------------------------------------------------------
//   @@ SpannerSegment
//!    parent: the Spanner it belongs to; placed on a System during layout
//---------------------------------------------------------

class SpannerSegment : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, SpannerSegment)

public:

    ~SpannerSegment() override;

    virtual double mag() const override;
    virtual Fraction tick() const override;

    //! The spanner that owns this segment - derived from the ownership parent,
    //! so that it can never go stale.
    Spanner* spanner() const { return toSpanner(ownershipParent()); }
    void setSpanner(Spanner* val);

    void setSpannerSegmentType(SpannerSegmentType s) { m_spannerSegmentType = s; }
    SpannerSegmentType spannerSegmentType() const { return m_spannerSegmentType; }
    bool isSingleType() const { return spannerSegmentType() == SpannerSegmentType::SINGLE; }
    bool isBeginType() const { return spannerSegmentType() == SpannerSegmentType::BEGIN; }
    bool isSingleBeginType() const { return isSingleType() || isBeginType(); }
    bool isSingleEndType() const { return isSingleType() || isEndType(); }
    bool isMiddleType() const { return spannerSegmentType() == SpannerSegmentType::MIDDLE; }
    bool isEndType() const { return spannerSegmentType() == SpannerSegmentType::END; }

    int subtype() const override;
    TranslatableString subtypeUserName() const override;

    //! The system this segment is currently laid out on; null if not placed.
    System* system() const { return m_system; }
    //! Records the placement only. Reserved for System itself, which maintains its own
    //! segment list; everyone else must use moveToSystem(), so that a non-null system()
    //! always implies that the segment is in that system's segment list - otherwise the
    //! pointer is left dangling when the system is destroyed.
    void setSystem(System* s) { m_system = s; }
    //! Detach from the current system's segment list and attach to the given one.
    void moveToSystem(System* s);
    EngravingItem* layoutParent() const override;

    //! A segment is owned by its spanner; a system merely places it, see
    //! setSystem()/moveToSystem(). This overload hides EngravingItem::setOwnershipParent,
    //! so that no other parent can be set by accident.
    void setOwnershipParent(Spanner* spanner);

    const PointF& userOff2() const { return m_offset2; }
    void setUserOff2(const PointF& o) { m_offset2 = o; }
    void setUserXoffset2(double x) { m_offset2.setX(x); }
    void setUserYoffset2(double y) { m_offset2.setY(y); }
    real_t& rUserXoffset2() { return m_offset2.rx(); }
    real_t& rUserYoffset2() { return m_offset2.ry(); }

    void setPos2(const PointF& p) { m_p2 = p; }
    //TODO: rename to spanSegPosWithUserOffset()
    PointF pos2() const { return m_p2 + m_offset2; }
    //TODO: rename to spanSegPos()
    const PointF& ipos2() const { return m_p2; }
    PointF& rpos2() { return m_p2; }
    real_t& rxpos2() { return m_p2.rx(); }
    real_t& rypos2() { return m_p2.ry(); }

    bool isEditable() const override { return true; }

    muse::ByteArray mimeData(const PointF& dragOffset) const override;

    void spatiumChanged(double ov, double nv) override;

    PropertyValue getProperty(Pid id) const override;
    bool setProperty(Pid id, const PropertyValue& v) override;
    PropertyValue propertyDefault(Pid id) const override;
    virtual EngravingObject* propertyDelegate(Pid) const override;
    void undoChangeProperty(Pid id, const PropertyValue&, PropertyFlags ps) override;
    using EngravingObject::undoChangeProperty;

    PointF defaultPos() const override;

    Sid getPropertyStyle(Pid id) const override;
    void resetProperty(Pid id) override;
    void styleChanged() override;
    void reset() override;

    void setSelected(bool f) override;
    void setVisible(bool f) override;
    void setColor(const Color& col) override;
    void setZ(int val) override;

    bool collectForDrawing() const override;

    EngravingItem* nextSegmentElement() override;
    EngravingItem* prevSegmentElement() override;
    String accessibleInfo() const override;
    void triggerLayout() const override;

    std::list<EngravingObject*> linkListForPropertyPropagation() const override;
    bool isPropertyLinkedToMaster(Pid id) const override;

    virtual bool isUserModified() const override;

    bool allowTimeAnchor() const override;

protected:

    SpannerSegment(const ElementType& type, Spanner*, ElementFlags f = ElementFlag::ON_STAFF | ElementFlag::MOVABLE);
    SpannerSegment(const SpannerSegment&);

    PointF m_p2;
    PointF m_offset2;

private:
    String formatBarsAndBeats() const override;
    String formatStartBarsAndBeats(const Segment* segment) const;
    String formatEndBarsAndBeats(const Segment* segment) const;

    System* m_system = nullptr;   // current layout placement; not owned, not copied
    SpannerSegmentType m_spannerSegmentType = SpannerSegmentType::SINGLE;
};

//----------------------------------------------------------------------------------
//   @@ Spanner
///   Virtual base class for slurs, ties, lines etc.
//
//    @P anchor         enum (Spanner.CHORD, Spanner.MEASURE, Spanner.NOTE, Spanner.SEGMENT)
//    @P endElement     EngravingItem           the element the spanner end is anchored to (read-only)
//    @P startElement   EngravingItem           the element the spanner start is anchored to (read-only)
//    @P tick           int               tick start position
//    @P tick2          int               tick end position
//----------------------------------------------------------------------------------

class Spanner : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, Spanner)
public:
    enum class Anchor : unsigned char {
        SEGMENT, MEASURE, CHORDREST, NOTE
    };

    ~Spanner() override;

    virtual double mag() const override;

    virtual void setScore(Score* s) override;

    virtual Fraction tick() const override { return m_tick; }
    Fraction tick2() const { return m_tick + m_ticks; }
    Fraction ticks() const { return m_ticks; }

    void setTick(const Fraction&);
    void setTick2(const Fraction&);
    void setTicks(const Fraction&);

    bool isVoiceSpecific() const;
    track_idx_t track2() const;
    void setTrack2(track_idx_t v);
    track_idx_t effectiveTrack2() const;

    bool playSpanner() const { return m_playSpanner; }
    void setPlaySpanner(bool p) { m_playSpanner = p; }

    virtual Anchor anchor() const = 0;

    const std::vector<SpannerSegment*>& spannerSegments() const { return m_segments; }
    void setSpannerSegments(const std::vector<SpannerSegment*>& s) { m_segments = s; }
    SpannerSegment* frontSegment() { return m_segments.front(); }
    const SpannerSegment* frontSegment() const { return m_segments.front(); }
    SpannerSegment* backSegment() { return m_segments.back(); }
    const SpannerSegment* backSegment() const { return m_segments.back(); }
    SpannerSegment* segmentAt(int n) { return m_segments[n]; }
    const SpannerSegment* segmentAt(int n) const { return m_segments[n]; }
    size_t nsegments() const { return m_segments.size(); }
    bool segmentsEmpty() const { return m_segments.empty(); }
    void eraseSpannerSegments();
    bool eitherEndVisible() const;

    virtual void triggerLayout() const override;
    virtual void add(EngravingItem*) override;
    virtual void remove(EngravingItem*) override;
    virtual void removed() override;
    EngravingItemList accessibleChildren() const override;
    virtual void scanElements(std::function<void(EngravingItem*)>) override {}
    bool removeSpannerBack();
    virtual void removeUnmanaged();
    virtual void insertTimeUnmanaged(const Fraction& tick, const Fraction& len);

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue& v) override;
    PropertyValue propertyDefault(Pid propertyId) const override;

    virtual void computeStartElement();
    void computeEndElement();

    static Note* endElementFromSpanner(Spanner* sp, EngravingItem* newStart);
    static Note* startElementFromSpanner(Spanner* sp, EngravingItem* newEnd);
    void setNoteSpan(Note* startNote, Note* endNote);

    EngravingItem* startElement() const { return m_startElement; }
    EngravingItem* endElement() const { return m_endElement; }

    Measure* startMeasure() const;
    Measure* endMeasure() const;

    Measure* findStartMeasure() const;
    Measure* findEndMeasure() const;

    void setStartElement(EngravingItem* e);
    virtual void setEndElement(EngravingItem* e);

    ChordRest* startCR();
    ChordRest* endCR();

    Chord* startChord();
    Chord* endChord();

    ChordRest* findStartCR() const;
    ChordRest* findEndCR() const;

    Chord* findStartChord() const;
    Chord* findEndChord() const;

    Segment* startSegment() const;
    Segment* endSegment() const;

    bool elementAppliesToTrack(const track_idx_t refTrack) const override;

    virtual void setSelected(bool f) override;
    virtual void setVisible(bool f) override;
    virtual void setAutoplace(bool f) override;
    virtual void setColor(const Color& col) override;
    virtual void setZ(int val) override;
    Spanner* nextSpanner(EngravingItem* e, staff_idx_t activeStaff);
    Spanner* prevSpanner(EngravingItem* e, staff_idx_t activeStaff);
    virtual EngravingItem* nextSegmentElement() override;
    virtual EngravingItem* prevSegmentElement() override;

    using EngravingObject::undoChangeProperty;

    void pushUnusedSegment(SpannerSegment* seg);
    SpannerSegment* popUnusedSegment();
    void reuse(SpannerSegment* seg);              // called when segment from unusedSegments
                                                  // is added back to the spanner.
    int reuseSegments(int number);
    void fixupSegments(unsigned int targetNumber, std::function<SpannerSegment* ()> createSegment);

    bool isUserModified() const override;

    virtual bool allowTimeAnchor() const override { return false; }

protected:

    Spanner(const ElementType& type, EngravingItem* parent, ElementFlags = ElementFlag::NOTHING);
    Spanner(const Spanner&);

    virtual void doComputeStartElement();
    virtual void doComputeEndElement();

    virtual bool isInSpannerMap() const { return true; }

private:
    bool canBeCrossStaff() const;

    friend class SpannerSegment;

    EngravingItem* m_startElement = nullptr;
    EngravingItem* m_endElement = nullptr;

    bool m_playSpanner = true;

    Fraction m_tick = Fraction(-1, 1);
    Fraction m_ticks = Fraction(0, 1);
    track_idx_t m_track2 = muse::nidx;

    std::vector<SpannerSegment*> m_segments;
    std::deque<SpannerSegment*> m_unusedSegments;   // Currently unused segments which can be reused later.
                                                    // We cannot just delete them as they can be referenced
                                                    // in undo stack or other places already.
};
} // namespace mu::engraving
