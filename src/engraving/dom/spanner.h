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

#ifndef MU_ENGRAVING_SPANNER_H
#define MU_ENGRAVING_SPANNER_H

#include <deque>

#include "draw/types/color.h"

#include "../types/types.h"

#include "engravingitem.h"

namespace mu::engraving {
class Spanner;

//---------------------------------------------------------
//   @@ SpannerSegment
//!    parent: System
//---------------------------------------------------------

class SpannerSegment : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, SpannerSegment)

public:

    // Score Tree functions
    virtual EngravingObject* scanParent() const override;

    virtual double mag() const override;
    virtual Fraction tick() const override;

    Spanner* spanner() const { return m_spanner; }
    Spanner* setSpanner(Spanner* val) { return m_spanner = val; }

    void setSpannerSegmentType(SpannerSegmentType s) { m_spannerSegmentType = s; }
    SpannerSegmentType spannerSegmentType() const { return m_spannerSegmentType; }
    bool isSingleType() const { return spannerSegmentType() == SpannerSegmentType::SINGLE; }
    bool isBeginType() const { return spannerSegmentType() == SpannerSegmentType::BEGIN; }
    bool isSingleBeginType() const { return isSingleType() || isBeginType(); }
    bool isSingleEndType() const { return isSingleType() || isEndType(); }
    bool isMiddleType() const { return spannerSegmentType() == SpannerSegmentType::MIDDLE; }
    bool isEndType() const { return spannerSegmentType() == SpannerSegmentType::END; }

    void setSystem(System* s);
    System* system() const { return toSystem(explicitParent()); }

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
    virtual EngravingItem* propertyDelegate(Pid) override;
    void undoChangeProperty(Pid id, const PropertyValue&, PropertyFlags ps) override;
    using EngravingObject::undoChangeProperty;

    Sid getPropertyStyle(Pid id) const override;
    PropertyFlags propertyFlags(Pid id) const override;
    void resetProperty(Pid id) override;
    void styleChanged() override;
    void reset() override;

    void setSelected(bool f) override;
    void setVisible(bool f) override;
    void setColor(const Color& col) override;

    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;

    EngravingItem* nextSegmentElement() override;
    EngravingItem* prevSegmentElement() override;
    String accessibleInfo() const override;
    void triggerLayout() const override;

    std::list<EngravingObject*> linkListForPropertyPropagation() const override;
    bool isPropertyLinkedToMaster(Pid id) const override;

    bool isUserModified() const override;

    bool allowTimeAnchor() const override;

protected:

    SpannerSegment(const ElementType& type, Spanner*, System* parent, ElementFlags f = ElementFlag::ON_STAFF | ElementFlag::MOVABLE);
    SpannerSegment(const ElementType& type, System* parent, ElementFlags f = ElementFlag::ON_STAFF | ElementFlag::MOVABLE);
    SpannerSegment(const SpannerSegment&);

    PointF m_p2;
    PointF m_offset2;

private:
    String formatBarsAndBeats() const override;
    String formatStartBarsAndBeats(const Segment* segment) const;
    String formatEndBarsAndBeats(const Segment* segment) const;

    Spanner* m_spanner = nullptr;
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
    enum class Anchor {
        SEGMENT, MEASURE, CHORD, NOTE
    };

    // Score Tree functions
    virtual EngravingObject* scanParent() const override;
    virtual EngravingObjectList scanChildren() const override;

    virtual double mag() const override;

    virtual void setScore(Score* s) override;

    virtual Fraction tick() const override { return m_tick; }
    Fraction tick2() const { return m_tick + m_ticks; }
    Fraction ticks() const { return m_ticks; }

    void setTick(const Fraction&);
    void setTick2(const Fraction&);
    void setTicks(const Fraction&);

    bool isVoiceSpecific() const;
    track_idx_t track2() const { return m_track2; }
    void setTrack2(track_idx_t v) { m_track2 = v; }
    track_idx_t effectiveTrack2() const { return m_track2 == muse::nidx ? track() : m_track2; }

    bool broken() const { return m_broken; }
    void setBroken(bool v) { m_broken = v; }

    bool playSpanner() const { return m_playSpanner; }
    void setPlaySpanner(bool p) { m_playSpanner = p; }

    Anchor anchor() const { return m_anchor; }
    void setAnchor(Anchor a) { m_anchor = a; }

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
    virtual void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;
    bool removeSpannerBack();
    virtual void removeUnmanaged();
    virtual void insertTimeUnmanaged(const Fraction& tick, const Fraction& len);

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue& v) override;
    PropertyValue propertyDefault(Pid propertyId) const override;
    virtual void undoChangeProperty(Pid id, const PropertyValue&, PropertyFlags ps) override;

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
    void fixupSegments(unsigned int targetNumber, std::function<SpannerSegment* (System*)> createSegment);

    bool isUserModified() const override;

    virtual bool allowTimeAnchor() const override { return false; }

protected:

    Spanner(const ElementType& type, EngravingItem* parent, ElementFlags = ElementFlag::NOTHING);
    Spanner(const Spanner&);

    virtual void doComputeStartElement();
    virtual void doComputeEndElement();

private:

    friend class SpannerSegment;

    EngravingItem* m_startElement = nullptr;
    EngravingItem* m_endElement = nullptr;

    bool m_playSpanner = true;

    Anchor m_anchor = Anchor::SEGMENT;
    Fraction m_tick = Fraction(-1, 1);
    Fraction m_ticks = Fraction(0, 1);
    track_idx_t m_track2 = muse::nidx;
    bool m_broken = false;

    std::vector<SpannerSegment*> m_segments;
    std::deque<SpannerSegment*> m_unusedSegments;   // Currently unused segments which can be reused later.
                                                    // We cannot just delete them as they can be referenced
                                                    // in undo stack or other places already.
};
} // namespace mu::engraving
#endif
