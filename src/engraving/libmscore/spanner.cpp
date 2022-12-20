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
#include "spanner.h"

#include "rw/xml.h"

#include "chord.h"
#include "chordrest.h"
#include "connector.h"
#include "lyrics.h"
#include "measure.h"
#include "note.h"
#include "repeatlist.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "system.h"

#include "translation.h"
#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//-----------------------------------------------------------------------------
//   @@ SpannerWriter
///   Helper class for writing Spanners
//-----------------------------------------------------------------------------

class SpannerWriter : public ConnectorInfoWriter
{
    OBJECT_ALLOCATOR(engraving, SpannerWriter)
protected:
    const char* tagName() const override { return "Spanner"; }
public:
    SpannerWriter(XmlWriter& xml, const EngravingItem* current, const Spanner* spanner, int track, Fraction frac, bool start);

    static void fillSpannerPosition(Location& l, const MeasureBase* endpoint, const Fraction& tick, bool clipboardmode);
};

//---------------------------------------------------------
//   SpannerSegment
//---------------------------------------------------------

SpannerSegment::SpannerSegment(const ElementType& type, Spanner* sp, System* parent, ElementFlags f)
    : EngravingItem(type, parent, f)
{
    _spanner = sp;
    setSpannerSegmentType(SpannerSegmentType::SINGLE);
}

SpannerSegment::SpannerSegment(const ElementType& type, System* parent, ElementFlags f)
    : EngravingItem(type, parent, f)
{
    setSpannerSegmentType(SpannerSegmentType::SINGLE);
    _spanner = 0;
}

SpannerSegment::SpannerSegment(const SpannerSegment& s)
    : EngravingItem(s)
{
    _spanner            = s._spanner;
    _spannerSegmentType = s._spannerSegmentType;
    _p2                 = s._p2;
    _offset2            = s._offset2;
}

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

double SpannerSegment::mag() const
{
    if (spanner()->systemFlag()) {
        return 1.0;
    }
    return staff() ? staff()->staffMag(spanner()->tick()) : 1.0;
}

Fraction SpannerSegment::tick() const
{
    return _spanner ? _spanner->tick() : Fraction(0, 1);
}

//---------------------------------------------------------
//   setSystem
//---------------------------------------------------------

void SpannerSegment::setSystem(System* s)
{
    if (system() != s) {
        if (system()) {
            system()->remove(this);
        }
        if (s) {
            s->add(this);
        } else {
            resetExplicitParent();
        }
    }
}

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void SpannerSegment::spatiumChanged(double ov, double nv)
{
    EngravingItem::spatiumChanged(ov, nv);
    if (offsetIsSpatiumDependent()) {
        _offset2 *= (nv / ov);
    }
}

//---------------------------------------------------------
//   mimeData
//---------------------------------------------------------

ByteArray SpannerSegment::mimeData(const PointF& dragOffset) const
{
    if (dragOffset.isNull()) { // where is dragOffset used?
        return spanner()->mimeData(dragOffset);
    }
    return EngravingItem::mimeData(dragOffset);
}

//---------------------------------------------------------
//   propertyDelegate
//---------------------------------------------------------

EngravingItem* SpannerSegment::propertyDelegate(Pid pid)
{
    if (pid == Pid::COLOR || pid == Pid::VISIBLE || pid == Pid::PLACEMENT) {
        return spanner();
    }
    return 0;
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

engraving::PropertyValue SpannerSegment::getProperty(Pid pid) const
{
    if (EngravingItem* e = const_cast<SpannerSegment*>(this)->propertyDelegate(pid)) {
        return e->getProperty(pid);
    }
    switch (pid) {
    case Pid::OFFSET2:
        return _offset2;
    default:
        return EngravingItem::getProperty(pid);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool SpannerSegment::setProperty(Pid pid, const PropertyValue& v)
{
    if (EngravingItem* e = propertyDelegate(pid)) {
        return e->setProperty(pid, v);
    }
    switch (pid) {
    case Pid::OFFSET2:
        _offset2 = v.value<PointF>();
        triggerLayoutAll();
        break;
    default:
        return EngravingItem::setProperty(pid, v);
    }
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue SpannerSegment::propertyDefault(Pid pid) const
{
    if (EngravingItem* e = const_cast<SpannerSegment*>(this)->propertyDelegate(pid)) {
        return e->propertyDefault(pid);
    }
    switch (pid) {
    case Pid::OFFSET2:
        return PropertyValue();
    default:
        return EngravingItem::propertyDefault(pid);
    }
}

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

Sid SpannerSegment::getPropertyStyle(Pid pid) const
{
    if (EngravingItem* e = const_cast<SpannerSegment*>(this)->propertyDelegate(pid)) {
        return e->getPropertyStyle(pid);
    }
    return EngravingItem::getPropertyStyle(pid);
}

//---------------------------------------------------------
//   propertyFlags
//---------------------------------------------------------

PropertyFlags SpannerSegment::propertyFlags(Pid pid) const
{
    if (EngravingItem* e = const_cast<SpannerSegment*>(this)->propertyDelegate(pid)) {
        return e->propertyFlags(pid);
    }
    return EngravingItem::propertyFlags(pid);
}

//---------------------------------------------------------
//   resetProperty
//---------------------------------------------------------

void SpannerSegment::resetProperty(Pid pid)
{
    if (EngravingItem* e = propertyDelegate(pid)) {
        return e->resetProperty(pid);
    }
    return EngravingItem::resetProperty(pid);
}

//---------------------------------------------------------
//   styleChanged
//---------------------------------------------------------

void SpannerSegment::styleChanged()
{
    spanner()->styleChanged();
}

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void SpannerSegment::reset()
{
    undoChangeProperty(Pid::OFFSET2, PointF());
    EngravingItem::reset();
    spanner()->reset();
}

//---------------------------------------------------------
//   undoChangeProperty
//---------------------------------------------------------

void SpannerSegment::undoChangeProperty(Pid pid, const PropertyValue& val, PropertyFlags ps)
{
    if (pid == Pid::AUTOPLACE && (val.toBool() == true && !autoplace())) {
        // Switching autoplacement on. Save user-defined
        // placement properties to undo stack.
        undoPushProperty(Pid::OFFSET2);
        // other will be saved in EngravingItem::undoChangeProperty
    }
    EngravingItem::undoChangeProperty(pid, val, ps);
}

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void SpannerSegment::setSelected(bool f)
{
    for (SpannerSegment* ss : _spanner->spannerSegments()) {
        ss->EngravingItem::setSelected(f);
    }
    _spanner->setSelected(f);
}

//---------------------------------------------------------
//   setVisible
//---------------------------------------------------------

void SpannerSegment::setVisible(bool f)
{
    if (_spanner) {
        for (SpannerSegment* ss : _spanner->spannerSegments()) {
            ss->EngravingItem::setVisible(f);
        }
        _spanner->setVisible(f);
    } else {
        EngravingItem::setVisible(f);
    }
}

//---------------------------------------------------------
//   setColor
//---------------------------------------------------------

void SpannerSegment::setColor(const mu::draw::Color& col)
{
    if (_spanner) {
        for (SpannerSegment* ss : _spanner->spannerSegments()) {
            ss->_color = col;
        }
        _spanner->_color = col;
    } else {
        _color = col;
    }
}

//---------------------------------------------------------
//   nextSegmentElement
//---------------------------------------------------------

EngravingItem* SpannerSegment::nextSegmentElement()
{
    return spanner()->nextSegmentElement();
}

//---------------------------------------------------------
//   prevSegmentElement
//---------------------------------------------------------

EngravingItem* SpannerSegment::prevSegmentElement()
{
    return spanner()->prevSegmentElement();
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String SpannerSegment::accessibleInfo() const
{
    return spanner()->accessibleInfo();
}

//---------------------------------------------------------
//   triggerLayout
//---------------------------------------------------------

void SpannerSegment::triggerLayout() const
{
    if (_spanner) {
        _spanner->triggerLayout();
    }
}

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void SpannerSegment::scanElements(void* data, void (* func)(void*, EngravingItem*), bool all)
{
    if (all || spanner()->eitherEndVisible() || systemFlag()) {
        func(data, this);
    }
}

//---------------------------------------------------------
//   Spanner
//---------------------------------------------------------

Spanner::Spanner(const ElementType& type, EngravingItem* parent, ElementFlags f)
    : EngravingItem(type, parent, f)
{
}

Spanner::Spanner(const Spanner& s)
    : EngravingItem(s)
{
    _anchor       = s._anchor;
    _startElement = s._startElement;
    _endElement   = s._endElement;
    _tick         = s._tick;
    _ticks        = s._ticks;
    _track2       = s._track2;
    if (!s.startElement() && !spannerSegments().size()) {
        for (auto* segment : s.spannerSegments()) {
            add(segment->clone());
        }
    }
}

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

double Spanner::mag() const
{
    if (systemFlag()) {
        return 1.0;
    }
    return staff() ? staff()->staffMag(tick()) : 1.0;
}

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Spanner::add(EngravingItem* e)
{
    SpannerSegment* ls = toSpannerSegment(e);
    ls->setSpanner(this);
    ls->setSelected(selected());
    ls->setTrack(track());
//      ls->setAutoplace(autoplace());
    segments.push_back(ls);
    e->added();
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Spanner::remove(EngravingItem* e)
{
    SpannerSegment* ss = toSpannerSegment(e);
    if (ss->system()) {
        ss->system()->remove(ss);
    }
    segments.erase(std::remove(segments.begin(), segments.end(), ss), segments.end());
}

//---------------------------------------------------------
//   removeUnmanaged
//
//    Remove the Spanner and its segments from objects which may know about them
//
//    This method and the following are used for spanners which are contained within compound elements
//    which manage their parts themselves without using the standard management supplied by Score;
//    Example can be the LyricsLine within a Lyrics element or the FiguredBassLine within a FiguredBass
//    (not implemented yet).
//---------------------------------------------------------

void Spanner::removeUnmanaged()
{
    for (SpannerSegment* ss : spannerSegments()) {
        if (ss->system()) {
//                  ss->system()->remove(ss);
            ss->setSystem(nullptr);
        }
    }
    score()->removeUnmanagedSpanner(this);
}

//---------------------------------------------------------
//   insertTimeUnmanaged
//---------------------------------------------------------

void Spanner::insertTimeUnmanaged(const Fraction& fromTick, const Fraction& len)
{
    Fraction newTick1 = tick();
    Fraction newTick2 = tick2();

    // check spanner start and end point
    if (len > Fraction(0, 1)) {            // adding time
        if (tick() > fromTick) {          // start after insertion point: shift start to right
            newTick1 += len;
        }
        if (tick2() > fromTick) {         // end after insertion point: shift end to right
            newTick2 += len;
        }
    }
    if (len < Fraction(0, 1)) {            // removing time
        Fraction toTick = fromTick - len;
        if (tick() > fromTick) {          // start after beginning of removed time
            if (tick() < toTick) {        // start within removed time: bring start at removing point
                if (explicitParent()) {
                    parentItem()->remove(this);
                    return;
                } else {
                    newTick1 = fromTick;
                }
            } else {                      // start after removed time: shift start to left
                newTick1 += len;
            }
        }
        if (tick2() > fromTick) {         // end after start of removed time
            if (tick2() < toTick) {       // end within removed time: bring end at removing point
                newTick2 = fromTick;
            } else {                      // end after removed time: shift end to left
                newTick2 += len;
            }
        }
    }

    // update properties as required
    if (newTick2 <= newTick1) {                 // if no longer any span: remove it
        if (explicitParent()) {
            parentItem()->remove(this);
        }
    } else {                                    // if either TICKS or TICK did change, update property
        if (newTick2 - newTick1 != tick2() - tick()) {
            setProperty(Pid::SPANNER_TICKS, newTick2 - newTick1);
        }
        if (newTick1 != tick()) {
            setProperty(Pid::SPANNER_TICK, newTick1);
        }
    }
}

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Spanner::scanElements(void* data, void (* func)(void*, EngravingItem*), bool all)
{
    for (EngravingObject* child : scanChildren()) {
        if (scanParent() && child->isSpannerSegment()) {
            continue; // spanner segments are scanned by the system
                      // except in the palette (in which case scanParent() == nullptr)
        }
        child->scanElements(data, func, all);
    }
}

//---------------------------------------------------------
//   isVisibleCR
//---------------------------------------------------------

static bool isVisibleCR(EngravingItem* e)
{
    if (!e || !e->isChordRest()) {
        return true;  // assume visible
    }
    ChordRest* cr = toChordRest(e);
    return cr->measure()->visible(cr->staffIdx());
}

//---------------------------------------------------------
//   eitherEndVisible
//---------------------------------------------------------

bool Spanner::eitherEndVisible() const
{
    if (isVolta()) {
        return true;
    }
    if (!score()->staff(staffIdx())->show()) {
        return false;
    }
    return isVisibleCR(startElement()) || isVisibleCR(endElement());
}

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Spanner::setScore(Score* s)
{
    EngravingItem::setScore(s);
    for (SpannerSegment* seg : segments) {
        seg->setScore(s);
    }
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Spanner::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::SPANNER_TICK:
        return _tick;
    case Pid::SPANNER_TICKS:
        return _ticks;
    case Pid::SPANNER_TRACK2:
        return track2();
    case Pid::ANCHOR:
        return int(anchor());
    case Pid::LOCATION_STAVES:
        return (track2() / VOICES) - (track() / VOICES);
    case Pid::LOCATION_VOICES:
        return (track2() % VOICES) - (track() / VOICES);
    case Pid::LOCATION_FRACTIONS:
        return _ticks;
    case Pid::LOCATION_MEASURES:
    case Pid::LOCATION_GRACE:
    case Pid::LOCATION_NOTE:
        return Location::getLocationProperty(propertyId, startElement(), endElement());
    default:
        break;
    }
    return EngravingItem::getProperty(propertyId);
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Spanner::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::SPANNER_TICK:
        triggerLayout();           // spanner may have moved to another system
        setTick(v.value<Fraction>());
        setStartElement(0);               // invalidate
        setEndElement(0);                 //
        if (score() && score()->spannerMap().removeSpanner(this)) {
            score()->addSpanner(this);
        }
        break;
    case Pid::SPANNER_TICKS:
        triggerLayout();           // spanner may now span for a smaller number of systems
        setTicks(v.value<Fraction>());
        setEndElement(0);                 // invalidate
        break;
    case Pid::TRACK:
        setTrack(v.value<track_idx_t>());
        setStartElement(0);               // invalidate
        break;
    case Pid::SPANNER_TRACK2:
        setTrack2(v.toInt());
        setEndElement(0);                 // invalidate
        break;
    case Pid::ANCHOR:
        setAnchor(Anchor(v.toInt()));
        break;
    default:
        return EngravingItem::setProperty(propertyId, v);
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue Spanner::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::ANCHOR:
        return int(Anchor::SEGMENT);
    default:
        break;
    }
    return EngravingItem::propertyDefault(propertyId);
}

//---------------------------------------------------------
//   computeStartElement
//---------------------------------------------------------

void Spanner::computeStartElement()
{
    switch (_anchor) {
    case Anchor::SEGMENT: {
        if (systemFlag()) {
            _startElement = startSegment();
        } else {
            Segment* seg = score()->tick2segmentMM(tick(), false, SegmentType::ChordRest);
            track_idx_t strack = (track() / VOICES) * VOICES;
            track_idx_t etrack = strack + VOICES;
            _startElement = 0;
            if (seg) {
                for (track_idx_t t = strack; t < etrack; ++t) {
                    if (seg->element(t)) {
                        _startElement = seg->element(t);
                        break;
                    }
                }
            }
        }
    }
    break;

    case Anchor::MEASURE:
        _startElement = score()->tick2measure(tick());
        break;

    case Anchor::CHORD:
    case Anchor::NOTE:
        return;
    }
}

//---------------------------------------------------------
//   computeEndElement
//---------------------------------------------------------

void Spanner::computeEndElement()
{
    if (score()->isPaletteScore()) {
        // return immediately to prevent lots of
        // "no element found" messages from appearing
        _endElement = nullptr;
        return;
    }

    switch (_anchor) {
    case Anchor::SEGMENT: {
        if (track2() == mu::nidx) {
            setTrack2(track());
        }
        if (ticks().isZero() && isTextLine() && explicitParent()) {           // special case palette
            setTicks(score()->lastSegment()->tick() - _tick);
        }
        if (systemFlag()) {
            _endElement = endSegment();
        } else if (isLyricsLine() && toLyricsLine(this)->isEndMelisma()) {
            // lyrics endTick should already indicate the segment we want
            // except for TEMP_MELISMA_TICKS case
            Lyrics* l = toLyricsLine(this)->lyrics();
            Fraction tick = (l->ticks().ticks() == Lyrics::TEMP_MELISMA_TICKS) ? l->tick() : l->endTick();
            Segment* s = score()->tick2segment(tick, true, SegmentType::ChordRest);
            if (!s) {
                LOGD("%s no end segment for tick %d", typeName(), tick.ticks());
                return;
            }
            voice_idx_t t = trackZeroVoice(track2());
            // take the first chordrest we can find;
            // linePos will substitute one in current voice if available
            for (voice_idx_t v = 0; v < VOICES; ++v) {
                _endElement = s->element(t + v);
                if (_endElement) {
                    break;
                }
            }
        } else {
            // find last cr on this staff that ends before tick2
            _endElement = score()->findCRinStaff(tick2(), track2() / VOICES);
        }
        if (!_endElement) {
            LOGD("%s no end element for tick %d", typeName(), tick2().ticks());
            return;
        }

        if (endCR() && !endCR()->measure()->isMMRest() && !systemFlag()) {
            ChordRest* cr = endCR();
            Fraction nticks = cr->tick() + cr->actualTicks() - _tick;
            if ((_ticks - nticks).isNotZero()) {
                LOGD("%s ticks changed, %d -> %d", typeName(), _ticks.ticks(), nticks.ticks());
                setTicks(nticks);
                if (isOttava()) {
                    staff()->updateOttava();
                }
            }
        }
    }
    break;

    case Anchor::MEASURE:
        _endElement = score()->tick2measure(tick2() - Fraction(1, 1920));
        if (!_endElement) {
            LOGD("Spanner::computeEndElement(), measure not found for tick %d\n", tick2().ticks() - 1);
            _endElement = score()->lastMeasure();
        }
        break;
    case Anchor::NOTE:
        if (!_endElement) {
            ChordRest* cr = score()->findCR(tick2(), track2());
            if (cr && cr->isChord()) {
                _endElement = toChord(cr)->upNote();
            }
        }
    case Anchor::CHORD:
        break;
    }
}

//---------------------------------------------------------
//   startElementFromSpanner
//
//    Given a Spanner and an end element, determines a start element suitable for the end
//    element of a new Spanner, so that it is 'parallel' to the old one.
//    Can be used while cloning a linked Spanner, to update the cloned spanner start and end elements
//    (Spanner(const Spanner&) copies start and end elements from the original to the copy).
//    NOTES:      Only spanners with Anchor::NOTE are currently supported.
//                Going back from end to start ensures the 'other' anchor of this is already set up
//                      (for instance, while cloning staves)
//---------------------------------------------------------

Note* Spanner::startElementFromSpanner(Spanner* sp, EngravingItem* newEnd)
{
    if (sp->anchor() != Anchor::NOTE) {
        return nullptr;
    }

    Note* oldStart   = toNote(sp->startElement());
    Note* oldEnd     = toNote(sp->endElement());
    if (oldStart == nullptr || oldEnd == nullptr) {
        return nullptr;
    }
    Note* newStart   = nullptr;
    Score* score      = newEnd->score();
    // determine the track where to expect the 'parallel' start element
    track_idx_t newTrack    = (newEnd->track() - oldEnd->track()) + oldStart->track();
    // look in notes linked to oldStart for a note with the
    // same score as new score and appropriate track
    for (EngravingObject* newEl : oldStart->linkList()) {
        if (toNote(newEl)->score() == score && toNote(newEl)->track() == newTrack) {
            newStart = toNote(newEl);
            break;
        }
    }
    return newStart;
}

//---------------------------------------------------------
//   endElementFromSpanner
//
//    Given a Spanner and a start element, determines an end element suitable for the start
//    element of a new Spanner, so that it is 'parallel' to the old one.
//    Can be used while cloning a linked Spanner, to update the cloned spanner start and end elements
//    (Spanner(const Spanner&) copies start and end elements from the original to the copy).
//    NOTES:      Only spanners with Anchor::NOTE are currently supported.
//---------------------------------------------------------

Note* Spanner::endElementFromSpanner(Spanner* sp, EngravingItem* newStart)
{
    if (sp->anchor() != Anchor::NOTE) {
        return nullptr;
    }

    Note* oldStart   = toNote(sp->startElement());
    Note* oldEnd     = toNote(sp->endElement());
    if (oldStart == nullptr || oldEnd == nullptr) {
        return nullptr;
    }
    Note* newEnd     = nullptr;
    Score* score      = newStart->score();
    // determine the track where to expect the 'parallel' start element
    track_idx_t newTrack    = newStart->track() + (oldEnd->track() - oldStart->track());
    // look in notes linked to oldEnd for a note with the
    // same score as new score and appropriate track
    for (EngravingObject* newEl : oldEnd->linkList()) {
        if (toNote(newEl)->score() == score && toNote(newEl)->track() == newTrack) {
            newEnd = toNote(newEl);
            break;
        }
    }
    return newEnd;
}

//---------------------------------------------------------
//   setNoteSpan
//
//    Sets up all the variables congruent with given start and end note anchors.
//---------------------------------------------------------

void Spanner::setNoteSpan(Note* startNote, Note* endNote)
{
    if (_anchor != Anchor::NOTE) {
        return;
    }

    setScore(startNote->score());
    setParent(startNote);
    setStartElement(startNote);
    setEndElement(endNote);
    setTick(startNote->chord()->tick());
    setTick2(endNote->chord()->tick());
    setTrack(startNote->track());
    setTrack2(endNote->track());
}

//---------------------------------------------------------
//   startChord
//---------------------------------------------------------

Chord* Spanner::startChord()
{
    assert(_anchor == Anchor::CHORD);
    if (!_startElement) {
        _startElement = findStartChord();
    }

    if (_startElement && _startElement->isChord()) {
        return toChord(_startElement);
    }

    return nullptr;
}

//---------------------------------------------------------
//   endChord
//---------------------------------------------------------

Chord* Spanner::endChord()
{
    assert(_anchor == Anchor::CHORD);
    if (!_endElement && type() == ElementType::SLUR) {
        _endElement = findEndChord();
    }

    if (_endElement && _endElement->isChord()) {
        return toChord(_endElement);
    }

    return nullptr;
}

//---------------------------------------------------------
//   startCR
//---------------------------------------------------------

ChordRest* Spanner::startCR()
{
    assert(_anchor == Anchor::SEGMENT || _anchor == Anchor::CHORD);
    if (!_startElement || _startElement->score() != score()) {
        _startElement = findStartCR();
    }
    return (_startElement && _startElement->isChordRest()) ? toChordRest(_startElement) : nullptr;
}

//---------------------------------------------------------
//   endCR
//---------------------------------------------------------

ChordRest* Spanner::endCR()
{
    assert(_anchor == Anchor::SEGMENT || _anchor == Anchor::CHORD);
    if ((!_endElement || _endElement->score() != score())) {
        _endElement = findEndCR();
    }
    return (_endElement && _endElement->isChordRest()) ? toChordRest(_endElement) : nullptr;
}

//---------------------------------------------------------
//   findStartChord
//---------------------------------------------------------

Chord* Spanner::findStartChord() const
{
    assert(_anchor == Anchor::CHORD);
    ChordRest* cr = score()->findCR(tick(), track());
    return cr->isChord() ? toChord(cr) : nullptr;
}

//---------------------------------------------------------
//   findEndChord
//---------------------------------------------------------

Chord* Spanner::findEndChord() const
{
    assert(_anchor == Anchor::CHORD);
    Segment* s = score()->tick2segmentMM(tick2(), false, SegmentType::ChordRest);
    ChordRest* endCR = s ? toChordRest(s->element(track2())) : nullptr;
    if (endCR && !endCR->isChord()) {
        endCR = nullptr;
    }
    return toChord(endCR);
}

//---------------------------------------------------------
//   findStartCR
//---------------------------------------------------------

ChordRest* Spanner::findStartCR() const
{
    assert(_anchor == Anchor::SEGMENT || _anchor == Anchor::CHORD);
    return score()->findCR(tick(), track());
}

//---------------------------------------------------------
//   findEndCR
//---------------------------------------------------------

ChordRest* Spanner::findEndCR() const
{
    assert(_anchor == Anchor::SEGMENT || _anchor == Anchor::CHORD);
    Segment* s = score()->tick2segmentMM(tick2(), false, SegmentType::ChordRest);
    const track_idx_t tr2 = effectiveTrack2();
    ChordRest* endCR = s ? toChordRest(s->element(tr2)) : nullptr;
    return endCR;
}

//---------------------------------------------------------
//   startSegment
//---------------------------------------------------------

Segment* Spanner::startSegment() const
{
    assert(score() != NULL);
    return score()->tick2rightSegment(tick());
}

//---------------------------------------------------------
//   endSegment
//---------------------------------------------------------

Segment* Spanner::endSegment() const
{
    return score()->tick2leftSegment(tick2());
}

//---------------------------------------------------------
//   startMeasure
//---------------------------------------------------------

Measure* Spanner::startMeasure() const
{
    return toMeasure(_startElement);
}

//---------------------------------------------------------
//   endMeasure
//---------------------------------------------------------

Measure* Spanner::endMeasure() const
{
    return toMeasure(_endElement);
}

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void Spanner::setSelected(bool f)
{
    for (SpannerSegment* ss : spannerSegments()) {
        ss->EngravingItem::setSelected(f);
    }
    EngravingItem::setSelected(f);
}

//---------------------------------------------------------
//   setVisible
//---------------------------------------------------------

void Spanner::setVisible(bool f)
{
    for (SpannerSegment* ss : spannerSegments()) {
        ss->EngravingItem::setVisible(f);
    }
    EngravingItem::setVisible(f);
}

//---------------------------------------------------------
//   setAutoplace
//---------------------------------------------------------

void Spanner::setAutoplace(bool f)
{
    for (SpannerSegment* ss : spannerSegments()) {
        ss->EngravingItem::setAutoplace(f);
    }
    EngravingItem::setAutoplace(f);
}

//---------------------------------------------------------
//   setColor
//---------------------------------------------------------

void Spanner::setColor(const mu::draw::Color& col)
{
    for (SpannerSegment* ss : spannerSegments()) {
        ss->setColor(col);
    }
    _color = col;
}

//---------------------------------------------------------
//   setStartElement
//---------------------------------------------------------

void Spanner::setStartElement(EngravingItem* e)
{
#ifndef NDEBUG
    if (_anchor == Anchor::NOTE) {
        assert(!e || e->type() == ElementType::NOTE);
    }
#endif
    _startElement = e;
}

//---------------------------------------------------------
//   setEndElement
//---------------------------------------------------------

void Spanner::setEndElement(EngravingItem* e)
{
#ifndef NDEBUG
    if (_anchor == Anchor::NOTE) {
        assert(!e || e->type() == ElementType::NOTE);
    }
#endif
    _endElement = e;
    if (e && ticks() == Fraction() && _tick >= Fraction()) {
        setTicks(e->tick() - _tick);
    }
}

//---------------------------------------------------------
//   nextSpanner
//---------------------------------------------------------

Spanner* Spanner::nextSpanner(EngravingItem* e, staff_idx_t activeStaff)
{
    std::multimap<int, Spanner*> mmap = score()->spanner();
    auto range = mmap.equal_range(tick().ticks());
    if (range.first != range.second) {       // range not empty
        for (auto i = range.first; i != range.second; ++i) {
            if (i->second == e) {
                while (i != range.second) {
                    ++i;
                    if (i == range.second) {
                        return nullptr;
                    }
                    Spanner* s =  i->second;
                    EngravingItem* st = s->startElement();
                    if (!st) {
                        continue;
                    }
                    if (s->startSegment() == toSpanner(e)->startSegment()) {
                        if (st->staffIdx() == activeStaff) {
                            return s;
                        } else if (st->isMeasure() && activeStaff == 0) {
                            return s;
                        }
                    }
                }
                break;
            }
        }
    }
    return nullptr;
}

//---------------------------------------------------------
//   prevSpanner
//---------------------------------------------------------

Spanner* Spanner::prevSpanner(EngravingItem* e, staff_idx_t activeStaff)
{
    std::multimap<int, Spanner*> mmap = score()->spanner();
    auto range = mmap.equal_range(tick().ticks());
    if (range.first != range.second) {   // range not empty
        for (auto i = range.first; i != range.second; ++i) {
            if (i->second == e) {
                if (i == range.first) {
                    return nullptr;
                }
                while (i != range.first) {
                    --i;
                    Spanner* s =  i->second;
                    EngravingItem* st = s->startElement();
                    if (s->startSegment() == toSpanner(e)->startSegment()) {
                        if (st->staffIdx() == activeStaff) {
                            return s;
                        } else if (st->isMeasure() && activeStaff == 0) {
                            return s;
                        }
                    }
                }
                break;
            }
        }
    }
    return nullptr;
}

//---------------------------------------------------------
//   nextSegmentElement
//---------------------------------------------------------

EngravingItem* Spanner::nextSegmentElement()
{
    Segment* s = startSegment();
    if (s) {
        return s->firstElement(staffIdx());
    }
    return score()->lastElement();
}

//---------------------------------------------------------
//   prevSegmentElement
//---------------------------------------------------------

EngravingItem* Spanner::prevSegmentElement()
{
    Segment* s = endSegment();
    if (s) {
        return s->lastElement(staffIdx());
    }
    return score()->firstElement();
}

//---------------------------------------------------------
//   setTick
//---------------------------------------------------------

void Spanner::setTick(const Fraction& v)
{
    if (_tick == v) {
        return;
    }

    _tick = v;

    Score* score = this->score();

    if (score) {
        score->spannerMap().setDirty();
    }

    _startUniqueTicks = score ? score->repeatList().tick2utick(tick().ticks()) : 0;
}

//---------------------------------------------------------
//   setTick2
//---------------------------------------------------------

void Spanner::setTick2(const Fraction& f)
{
    setTicks(f - _tick);
}

//---------------------------------------------------------
//   setTicks
//---------------------------------------------------------

void Spanner::setTicks(const Fraction& f)
{
    if (_ticks == f) {
        return;
    }

    _ticks = f;

    Score* score = this->score();

    if (score) {
        score->spannerMap().setDirty();
    }

    _endUniqueTicks = score ? score->repeatList().tick2utick(tick2().ticks()) : 0;
}

int Spanner::startUniqueTicks() const
{
    Score* score = this->score();

    if (!score) {
        return 0;
    }

    return score->repeatList().tick2utick(tick().ticks());
}

int Spanner::endUniqueTicks() const
{
    Score* score = this->score();

    if (!score) {
        return 0;
    }

    return score->repeatList().tick2utick(tick2().ticks());
}

//---------------------------------------------------------
//   triggerLayout
//---------------------------------------------------------

void Spanner::triggerLayout() const
{
    // Spanners do not have parent even when added to a score, so can't check parent here
    const track_idx_t tr2 = effectiveTrack2();
    score()->setLayout(_tick, _tick + _ticks, staffIdx(), track2staff(tr2), this);
}

void Spanner::triggerLayoutAll() const
{
    // Spanners do not have parent even when added to a score, so can't check parent here
    score()->setLayoutAll(staffIdx(), this);

    const track_idx_t tr2 = track2();
    if (tr2 != mu::nidx && tr2 != track()) {
        score()->setLayoutAll(track2staff(tr2), this);
    }
}

//---------------------------------------------------------
//   pushUnusedSegment
//---------------------------------------------------------

void Spanner::pushUnusedSegment(SpannerSegment* seg)
{
    if (!seg) {
        return;
    }
    seg->setSystem(nullptr);
    unusedSegments.push_back(seg);
}

//---------------------------------------------------------
//   popUnusedSegment
//    Take the next unused segment for reusing it.
//    If there is no unused segments left returns nullptr.
//---------------------------------------------------------

SpannerSegment* Spanner::popUnusedSegment()
{
    if (unusedSegments.empty()) {
        return nullptr;
    }
    SpannerSegment* seg = unusedSegments.front();
    unusedSegments.pop_front();
    return seg;
}

//---------------------------------------------------------
//   reuse
//    called when segment from unusedSegments is added
//    back to the spanner.
//---------------------------------------------------------

void Spanner::reuse(SpannerSegment* seg)
{
    add(seg);
}

//---------------------------------------------------------
//   reuseSegments
//    Adds \p number segments from unusedSegments to this
//    spanner via reuse() call. Returns number of new
//    segments that still need to be created, that is,
//    returns (number - nMovedSegments).
//---------------------------------------------------------

int Spanner::reuseSegments(int number)
{
    while (number > 0) {
        SpannerSegment* seg = popUnusedSegment();
        if (!seg) {
            break;
        }
        reuse(seg);
        --number;
    }
    return number;
}

//---------------------------------------------------------
//   fixupSegments
//    Makes number of segments match targetNumber.
//    Tries to reuse unused segments. If there are no
//    unused segments left, uses \p createSegment to create
//    the needed segments.
//    Previously unused segments are added via reuse() call
//---------------------------------------------------------

void Spanner::fixupSegments(unsigned int targetNumber, std::function<SpannerSegment* (System* parent)> createSegment)
{
    const int diff = targetNumber - int(nsegments());
    if (diff == 0) {
        return;
    }
    if (diff > 0) {
        const int ncreate = reuseSegments(diff);
        for (int i = 0; i < ncreate; ++i) {
            add(createSegment(score()->dummy()->system()));
        }
    } else { // diff < 0
        const int nremove = -diff;
        for (int i = 0; i < nremove; ++i) {
            SpannerSegment* seg = segments.back();
            segments.pop_back();
            pushUnusedSegment(seg);
        }
    }
}

//---------------------------------------------------------
//   eraseSpannerSegments
//    Completely erase all spanner segments, both used and
//    unused.
//---------------------------------------------------------

void Spanner::eraseSpannerSegments()
{
    DeleteAll(segments);
    DeleteAll(unusedSegments);
    segments.clear();
    unusedSegments.clear();
}

//---------------------------------------------------------
//   layoutSystem
//---------------------------------------------------------

SpannerSegment* Spanner::layoutSystem(System*)
{
    LOGD(" %s", typeName());
    return 0;
}

//---------------------------------------------------------
//   getNextLayoutSystemSegment
//---------------------------------------------------------

SpannerSegment* Spanner::getNextLayoutSystemSegment(System* system, std::function<SpannerSegment* (System* parent)> createSegment)
{
    SpannerSegment* seg = nullptr;
    for (SpannerSegment* ss : spannerSegments()) {
        if (!ss->system()) {
            seg = ss;
            break;
        }
    }
    if (!seg) {
        if ((seg = popUnusedSegment())) {
            reuse(seg);
        } else {
            seg = createSegment(system);
            assert(seg);
            add(seg);
        }
    }
    seg->setSystem(system);
    seg->setSpanner(this);
    seg->setTrack(track());
    seg->setVisible(visible());
    return seg;
}

//---------------------------------------------------------
//   layoutSystemsDone
//    Called after layout of all systems is done so precise
//    number of systems for this spanner becomes available.
//---------------------------------------------------------

void Spanner::layoutSystemsDone()
{
    std::vector<SpannerSegment*> validSegments;
    for (SpannerSegment* seg : segments) {
        if (seg->system()) {
            validSegments.push_back(seg);
        } else { // TODO: score()->selection().remove(ss); needed?
            pushUnusedSegment(seg);
        }
    }
    segments = std::move(validSegments);
}

//--------------------------------------------------
//   fraction
//---------------------------------------------------------

static Fraction fraction(const XmlWriter& xml, const EngravingItem* current, const Fraction& t)
{
    Fraction tick(t);
    if (!xml.context()->clipboardmode()) {
        const Measure* m = toMeasure(current->findMeasure());
        if (m) {
            tick -= m->tick();
        }
    }
    return tick;
}

//---------------------------------------------------------
//   Spanner::readProperties
//---------------------------------------------------------

bool Spanner::readProperties(XmlReader& e)
{
    const AsciiStringView tag(e.name());
    if (e.context()->pasteMode()) {
        if (tag == "ticks_f") {
            setTicks(e.readFraction());
            return true;
        }
    }
    return EngravingItem::readProperties(e);
}

//---------------------------------------------------------
//   Spanner::writeProperties
//---------------------------------------------------------

void Spanner::writeProperties(XmlWriter& xml) const
{
    if (xml.context()->clipboardmode()) {
        xml.tagFraction("ticks_f", ticks());
    }
    EngravingItem::writeProperties(xml);
}

//--------------------------------------------------
//   Spanner::writeSpannerStart
//---------------------------------------------------------

void Spanner::writeSpannerStart(XmlWriter& xml, const EngravingItem* current, track_idx_t track, Fraction tick) const
{
    Fraction frac = fraction(xml, current, tick);
    SpannerWriter w(xml, current, this, static_cast<int>(track), frac, true);
    w.write();
}

//--------------------------------------------------
//   Spanner::writeSpannerEnd
//---------------------------------------------------------

void Spanner::writeSpannerEnd(XmlWriter& xml, const EngravingItem* current, track_idx_t track, Fraction tick) const
{
    Fraction frac = fraction(xml, current, tick);
    SpannerWriter w(xml, current, this, static_cast<int>(track), frac, false);
    w.write();
}

//--------------------------------------------------
//   Spanner::readSpanner
//---------------------------------------------------------

void Spanner::readSpanner(XmlReader& e, EngravingItem* current, track_idx_t track)
{
    std::unique_ptr<ConnectorInfoReader> info(new ConnectorInfoReader(e, current, static_cast<int>(track)));
    ConnectorInfoReader::readConnector(std::move(info), e);
}

//--------------------------------------------------
//   Spanner::readSpanner
//---------------------------------------------------------

void Spanner::readSpanner(XmlReader& e, Score* current, track_idx_t track)
{
    std::unique_ptr<ConnectorInfoReader> info(new ConnectorInfoReader(e, current, static_cast<int>(track)));
    ConnectorInfoReader::readConnector(std::move(info), e);
}

//---------------------------------------------------------
//   SpannerWriter::fillSpannerPosition
//---------------------------------------------------------

void SpannerWriter::fillSpannerPosition(Location& l, const MeasureBase* m, const Fraction& tick, bool clipboardmode)
{
    if (clipboardmode) {
        l.setMeasure(0);
        l.setFrac(tick);
    } else {
        if (!m) {
            LOGW("fillSpannerPosition: couldn't find spanner's endpoint's measure");
            l.setMeasure(0);
            l.setFrac(tick);
            return;
        }
        l.setMeasure(m->measureIndex());
        l.setFrac(tick - m->tick());
    }
}

//---------------------------------------------------------
//   SpannerWriter::SpannerWriter
//---------------------------------------------------------

SpannerWriter::SpannerWriter(XmlWriter& xml, const EngravingItem* current, const Spanner* sp, int track, Fraction frac, bool start)
    : ConnectorInfoWriter(xml, current, sp, track, frac)
{
    const bool clipboardmode = xml.context()->clipboardmode();
    if (!sp->startElement() || !sp->endElement()) {
        LOGW("SpannerWriter: spanner (%s) doesn't have an endpoint!", sp->typeName());
        return;
    }
    if (current->isMeasure() || current->isSegment() || (sp->startElement()->type() != current->type())) {
        // (The latter is the hairpins' case, for example, though they are
        // covered by the other checks too.)
        // We cannot determine position of the spanner from its start/end
        // elements and will try to obtain this info from the spanner itself.
        if (!start) {
            _prevLoc.setTrack(static_cast<int>(sp->track()));
            Measure* m = sp->score()->tick2measure(sp->tick());
            fillSpannerPosition(_prevLoc, m, sp->tick(), clipboardmode);
        } else {
            const track_idx_t track2 = (sp->track2() != mu::nidx) ? sp->track2() : sp->track();
            _nextLoc.setTrack(static_cast<int>(track2));
            Measure* m = sp->score()->tick2measure(sp->tick2());
            fillSpannerPosition(_nextLoc, m, sp->tick2(), clipboardmode);
        }
    } else {
        // We can obtain the spanner position info from its start/end
        // elements and will prefer this source of information.
        // Reason: some spanners contain no or wrong information (e.g. Ties).
        if (!start) {
            updateLocation(sp->startElement(), _prevLoc, clipboardmode);
        } else {
            updateLocation(sp->endElement(), _nextLoc, clipboardmode);
        }
    }
}

//---------------------------------------------------------
//   autoplaceSpannerSegment
//---------------------------------------------------------

void SpannerSegment::autoplaceSpannerSegment()
{
    if (!explicitParent()) {
        setOffset(PointF());
        return;
    }
    if (isStyled(Pid::OFFSET)) {
        setOffset(spanner()->propertyDefault(Pid::OFFSET).value<PointF>());
    }

    if (spanner()->anchor() == Spanner::Anchor::NOTE) {
        return;
    }

    // rebase vertical offset on drag
    double rebase = 0.0;
    if (offsetChanged() != OffsetChange::NONE) {
        rebase = rebaseOffset();
    }

    if (autoplace()) {
        double sp = score()->spatium();
        if (!systemFlag() && !spanner()->systemFlag()) {
            sp *= staff()->staffMag(spanner()->tick());
        }
        double md = minDistance().val() * sp;
        bool above = spanner()->placeAbove();
        SkylineLine sl(!above);
        Shape sh = shape();
        sl.add(sh.translated(pos()));
        double yd = 0.0;
        staff_idx_t stfIdx = systemFlag() ? staffIdxOrNextVisible() : staffIdx();
        if (stfIdx == mu::nidx) {
            _skipDraw = true;
            return;
        }
        if (above) {
            double d  = system()->topDistance(stfIdx, sl);
            if (d > -md) {
                yd = -(d + md);
            }
        } else {
            double d  = system()->bottomDistance(stfIdx, sl);
            if (d > -md) {
                yd = d + md;
            }
        }
        if (yd != 0.0) {
            if (offsetChanged() != OffsetChange::NONE) {
                // user moved element within the skyline
                // we may need to adjust minDistance, yd, and/or offset
                double adj = pos().y() + rebase;
                bool inStaff = above ? sh.bottom() + adj > 0.0 : sh.top() + adj < staff()->height();
                rebaseMinDistance(md, yd, sp, rebase, above, inStaff);
            }
            movePosY(yd);
        }
    }
    setOffsetChanged(false);
}

String SpannerSegment::formatBarsAndBeats() const
{
    const Spanner* spanner = this->spanner();

    if (!spanner) {
        return EngravingItem::formatBarsAndBeats();
    }

    const Segment* endSegment = spanner->endSegment();

    if (!endSegment) {
        endSegment = score()->lastSegment()->prev1MM(SegmentType::ChordRest);
    }

    if (endSegment->tick() != score()->lastSegment()->prev1MM(SegmentType::ChordRest)->tick()
        && spanner->type() != ElementType::SLUR
        && spanner->type() != ElementType::TIE) {
        endSegment = endSegment->prev1MM(SegmentType::ChordRest);
    }

    return formatStartBarsAndBeats(spanner->startSegment()) + u' ' + formatEndBarsAndBeats(endSegment);
}

String SpannerSegment::formatStartBarsAndBeats(const Segment* segment) const
{
    std::pair<int, float> barbeat = segment->barbeat();
    return mtrc("engraving", "Start measure: %1; Start beat: %2")
           .arg(String::number(barbeat.first), String::number(barbeat.second));
}

String SpannerSegment::formatEndBarsAndBeats(const Segment* segment) const
{
    std::pair<int, float> barbeat = segment->barbeat();
    return mtrc("engraving", "End measure: %1; End beat: %2")
           .arg(String::number(barbeat.first), String::number(barbeat.second));
}

//---------------------------------------------------------
//   undoChangeProperty
//---------------------------------------------------------

void Spanner::undoChangeProperty(Pid id, const PropertyValue& v, PropertyFlags ps)
{
    if (id == Pid::PLACEMENT) {
        EngravingObject::undoChangeProperty(id, v, ps);
        // change offset of all segments if styled

        for (SpannerSegment* s : segments) {
            if (s->isStyled(Pid::OFFSET)) {
                s->setOffset(s->propertyDefault(Pid::OFFSET).value<PointF>());
                s->triggerLayout();
            }
        }
        return;
    }
    EngravingItem::undoChangeProperty(id, v, ps);
}
}
