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

#ifndef __SPANNER_H__
#define __SPANNER_H__

#include <deque>

#include "draw/types/color.h"
#include "engravingitem.h"

namespace mu::engraving {
class Spanner;

//---------------------------------------------------------
//   SpannerSegmentType
//---------------------------------------------------------

enum class SpannerSegmentType {
    SINGLE, BEGIN, MIDDLE, END
};

//---------------------------------------------------------
//   @@ SpannerSegment
//!    parent: System
//---------------------------------------------------------

class SpannerSegment : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, SpannerSegment)

    Spanner* _spanner;
    SpannerSegmentType _spannerSegmentType;

protected:
    mu::PointF _p2;
    mu::PointF _offset2;

    SpannerSegment(const ElementType& type, Spanner*, System* parent, ElementFlags f = ElementFlag::ON_STAFF | ElementFlag::MOVABLE);
    SpannerSegment(const ElementType& type, System* parent, ElementFlags f = ElementFlag::ON_STAFF | ElementFlag::MOVABLE);
    SpannerSegment(const SpannerSegment&);

public:

    // Score Tree functions
    virtual EngravingObject* scanParent() const override;

    virtual double mag() const override;
    virtual Fraction tick() const override;

    Spanner* spanner() const { return _spanner; }
    Spanner* setSpanner(Spanner* val) { return _spanner = val; }

    void setSpannerSegmentType(SpannerSegmentType s) { _spannerSegmentType = s; }
    SpannerSegmentType spannerSegmentType() const { return _spannerSegmentType; }
    bool isSingleType() const { return spannerSegmentType() == SpannerSegmentType::SINGLE; }
    bool isBeginType() const { return spannerSegmentType() == SpannerSegmentType::BEGIN; }
    bool isSingleBeginType() const { return isSingleType() || isBeginType(); }
    bool isSingleEndType() const { return isSingleType() || isEndType(); }
    bool isMiddleType() const { return spannerSegmentType() == SpannerSegmentType::MIDDLE; }
    bool isEndType() const { return spannerSegmentType() == SpannerSegmentType::END; }

    void setSystem(System* s);
    System* system() const { return toSystem(explicitParent()); }

    const mu::PointF& userOff2() const { return _offset2; }
    void setUserOff2(const mu::PointF& o) { _offset2 = o; }
    void setUserXoffset2(double x) { _offset2.setX(x); }
    double& rUserXoffset2() { return _offset2.rx(); }
    double& rUserYoffset2() { return _offset2.ry(); }

    void setPos2(const mu::PointF& p) { _p2 = p; }
    //TODO: rename to spanSegPosWithUserOffset()
    mu::PointF pos2() const { return _p2 + _offset2; }
    //TODO: rename to spanSegPos()
    const mu::PointF& ipos2() const { return _p2; }
    mu::PointF& rpos2() { return _p2; }
    double& rxpos2() { return _p2.rx(); }
    double& rypos2() { return _p2.ry(); }

    bool isEditable() const override { return true; }

    mu::ByteArray mimeData(const mu::PointF& dragOffset) const override;

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
    void setColor(const mu::draw::Color& col) override;

    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;

    EngravingItem* nextSegmentElement() override;
    EngravingItem* prevSegmentElement() override;
    String accessibleInfo() const override;
    void triggerLayout() const override;
    void autoplaceSpannerSegment();

private:
    String formatBarsAndBeats() const override;
    String formatStartBarsAndBeats(const Segment* segment) const;
    String formatEndBarsAndBeats(const Segment* segment) const;
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
private:

    EngravingItem* _startElement = nullptr;
    EngravingItem* _endElement = nullptr;

    Anchor _anchor         { Anchor::SEGMENT };
    Fraction _tick         { Fraction(-1, 1) };
    Fraction _ticks        { Fraction(0, 1) };
    track_idx_t _track2 = mu::nidx;
    bool _broken           { false };

    int _startUniqueTicks = 0;
    int _endUniqueTicks = 0;

    std::vector<SpannerSegment*> segments;
    std::deque<SpannerSegment*> unusedSegments;   // Currently unused segments which can be reused later.
                                                  // We cannot just delete them as they can be referenced
                                                  // in undo stack or other places already.
protected:

    Spanner(const ElementType& type, EngravingItem* parent, ElementFlags = ElementFlag::NOTHING);
    Spanner(const Spanner&);

    void pushUnusedSegment(SpannerSegment* seg);
    SpannerSegment* popUnusedSegment();
    void reuse(SpannerSegment* seg);              // called when segment from unusedSegments
                                                  // is added back to the spanner.
    int reuseSegments(int number);
    SpannerSegment* getNextLayoutSystemSegment(System* system, std::function<SpannerSegment* (System*)> createSegment);
    void fixupSegments(unsigned int targetNumber, std::function<SpannerSegment* (System*)> createSegment);

    const std::vector<SpannerSegment*> spannerSegments() const { return segments; }

public:
    // Score Tree functions
    virtual EngravingObject* scanParent() const override;
    virtual EngravingObjectList scanChildren() const override;

    virtual double mag() const override;

    virtual void setScore(Score* s) override;

    bool readProperties(XmlReader&) override;
    void writeProperties(XmlWriter&) const override;

    void writeSpannerStart(XmlWriter& xml, const EngravingItem* current, track_idx_t track, Fraction frac = { -1, 1 }) const;
    void writeSpannerEnd(XmlWriter& xml,   const EngravingItem* current, track_idx_t track, Fraction frac = { -1, 1 }) const;
    static void readSpanner(XmlReader& e, EngravingItem* current, track_idx_t track);
    static void readSpanner(XmlReader& e, Score* current, track_idx_t track);

    virtual Fraction tick() const override { return _tick; }
    Fraction tick2() const { return _tick + _ticks; }
    Fraction ticks() const { return _ticks; }

    void setTick(const Fraction&);
    void setTick2(const Fraction&);
    void setTicks(const Fraction&);

    int startUniqueTicks() const;
    int endUniqueTicks() const;

    track_idx_t track2() const { return _track2; }
    void setTrack2(track_idx_t v) { _track2 = v; }
    track_idx_t effectiveTrack2() const { return _track2 == mu::nidx ? track() : _track2; }

    bool broken() const { return _broken; }
    void setBroken(bool v) { _broken = v; }

    Anchor anchor() const { return _anchor; }
    void setAnchor(Anchor a) { _anchor = a; }

    const std::vector<SpannerSegment*>& spannerSegments() { return segments; }
    SpannerSegment* frontSegment() { return segments.front(); }
    const SpannerSegment* frontSegment() const { return segments.front(); }
    SpannerSegment* backSegment() { return segments.back(); }
    const SpannerSegment* backSegment() const { return segments.back(); }
    SpannerSegment* segmentAt(int n) { return segments[n]; }
    const SpannerSegment* segmentAt(int n) const { return segments[n]; }
    size_t nsegments() const { return segments.size(); }
    bool segmentsEmpty() const { return segments.empty(); }
    void eraseSpannerSegments();
    bool eitherEndVisible() const;

    virtual SpannerSegment* layoutSystem(System*);
    virtual void layoutSystemsDone();

    virtual void triggerLayout() const override;
    virtual void triggerLayoutAll() const override;
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

    void computeStartElement();
    void computeEndElement();
    static Note* endElementFromSpanner(Spanner* sp, EngravingItem* newStart);
    static Note* startElementFromSpanner(Spanner* sp, EngravingItem* newEnd);
    void setNoteSpan(Note* startNote, Note* endNote);

    EngravingItem* startElement() const { return _startElement; }
    EngravingItem* endElement() const { return _endElement; }

    Measure* startMeasure() const;
    Measure* endMeasure() const;

    void setStartElement(EngravingItem* e);
    void setEndElement(EngravingItem* e);

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

    virtual void setSelected(bool f) override;
    virtual void setVisible(bool f) override;
    virtual void setAutoplace(bool f) override;
    virtual void setColor(const mu::draw::Color& col) override;
    Spanner* nextSpanner(EngravingItem* e, staff_idx_t activeStaff);
    Spanner* prevSpanner(EngravingItem* e, staff_idx_t activeStaff);
    virtual EngravingItem* nextSegmentElement() override;
    virtual EngravingItem* prevSegmentElement() override;

    friend class SpannerSegment;
    using EngravingObject::undoChangeProperty;
};
} // namespace mu::engraving
#endif
