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

#include "translation.h"

#include "chord.h"
#include "chordrest.h"
#include "location.h"
#include "lyrics.h"
#include "measure.h"
#include "note.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "system.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   SpannerSegment
//---------------------------------------------------------

SpannerSegment::SpannerSegment(const ElementType& type, Spanner* sp, System* parent, ElementFlags f)
    : EngravingItem(type, parent, f)
{
    m_spanner = sp;
    setSpannerSegmentType(SpannerSegmentType::SINGLE);
}

SpannerSegment::SpannerSegment(const ElementType& type, System* parent, ElementFlags f)
    : EngravingItem(type, parent, f)
{
    setSpannerSegmentType(SpannerSegmentType::SINGLE);
    m_spanner = 0;
}

SpannerSegment::SpannerSegment(const SpannerSegment& s)
    : EngravingItem(s)
{
    m_spanner            = s.m_spanner;
    m_spannerSegmentType = s.m_spannerSegmentType;
    m_p2                 = s.m_p2;
    m_offset2            = s.m_offset2;
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
    return m_spanner ? m_spanner->tick() : Fraction(0, 1);
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
        m_offset2 *= (nv / ov);
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
    switch (pid) {
    case Pid::COLOR:
    case Pid::VISIBLE:
    case Pid::PLACEMENT:
    case Pid::EXCLUDE_FROM_OTHER_PARTS:
    case Pid::POSITION_LINKED_TO_MASTER:
    case Pid::APPEARANCE_LINKED_TO_MASTER:
        return spanner();
    default: break;
    }

    return nullptr;
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
        return m_offset2;
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
        m_offset2 = v.value<PointF>();
        triggerLayout();
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
    for (SpannerSegment* ss : m_spanner->spannerSegments()) {
        ss->EngravingItem::setSelected(f);
    }
    m_spanner->setSelected(f);
}

//---------------------------------------------------------
//   setVisible
//---------------------------------------------------------

void SpannerSegment::setVisible(bool f)
{
    if (m_spanner) {
        for (SpannerSegment* ss : m_spanner->spannerSegments()) {
            ss->EngravingItem::setVisible(f);
        }
        m_spanner->setVisible(f);
    } else {
        EngravingItem::setVisible(f);
    }
}

//---------------------------------------------------------
//   setColor
//---------------------------------------------------------

void SpannerSegment::setColor(const mu::draw::Color& col)
{
    if (m_spanner) {
        for (SpannerSegment* ss : m_spanner->spannerSegments()) {
            ss->m_color = col;
        }
        m_spanner->m_color = col;
    } else {
        m_color = col;
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
    if (m_spanner) {
        m_spanner->triggerLayout();
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

std::list<EngravingObject*> SpannerSegment::linkListForPropertyPropagation() const
{
    std::list<EngravingObject*> result;
    result.push_back(const_cast<SpannerSegment*>(this));

    if (isMiddleType()) {
        return result;
    }

    for (const EngravingObject* linkedSpanner : m_spanner->linkList()) {
        if (linkedSpanner == m_spanner || toSpanner(linkedSpanner)->placement() != m_spanner->placement()) {
            continue;
        }
        const std::vector<SpannerSegment*>& linkedSegments = toSpanner(linkedSpanner)->spannerSegments();
        if (linkedSegments.empty()) {
            continue;
        }
        if (isSingleBeginType()) {
            result.push_back(linkedSegments.front());
        } else if (isSingleEndType()) {
            result.push_back(linkedSegments.back());
        }
    }

    return result;
}

bool SpannerSegment::isPropertyLinkedToMaster(Pid id) const
{
    bool linkedForSpannerSegment = EngravingItem::isPropertyLinkedToMaster(id);
    if (!linkedForSpannerSegment) {
        return false;
    }

    // The property is linked for the spanner segment, but may be unlinked for the spanner, in which case we consider it unlinked
    return spanner()->isPropertyLinkedToMaster(id);
}

bool SpannerSegment::isUserModified() const
{
    bool modified = !autoplace() || !visible()
                    || (propertyFlags(Pid::MIN_DISTANCE) == PropertyFlags::UNSTYLED
                        || getProperty(Pid::MIN_DISTANCE) != propertyDefault(Pid::MIN_DISTANCE))
                    || (!isStyled(Pid::OFFSET) && (!offset().isNull() || !userOff2().isNull()));

    return modified;
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
    m_anchor       = s.m_anchor;
    m_startElement = s.m_startElement;
    m_endElement   = s.m_endElement;
    m_tick         = s.m_tick;
    m_ticks        = s.m_ticks;
    m_track2       = s.m_track2;

    for (auto* segment : s.m_segments) {
        SpannerSegment* newSegment = toSpannerSegment(segment->clone());
        newSegment->setParent(nullptr);
        add(newSegment);
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
    m_segments.push_back(ls);
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
    m_segments.erase(std::remove(m_segments.begin(), m_segments.end(), ss), m_segments.end());
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
    if (score()->isPaletteScore()) {
        EngravingObject::scanElements(data, func, all);
        return;
    }

    for (EngravingObject* child : scanChildren()) {
        if (child->isSpannerSegment()) {
            // spanner segments are scanned by the system
            continue;
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
    for (SpannerSegment* seg : m_segments) {
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
        return m_tick;
    case Pid::SPANNER_TICKS:
        return m_ticks;
    case Pid::SPANNER_TRACK2:
        return track2();
    case Pid::ANCHOR:
        return int(anchor());
    case Pid::LOCATION_STAVES:
        return (track2() / VOICES) - (track() / VOICES);
    case Pid::LOCATION_VOICES:
        return (track2() % VOICES) - (track() / VOICES);
    case Pid::LOCATION_FRACTIONS:
        return m_ticks;
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
    case Pid::POSITION_LINKED_TO_MASTER:
        setPositionLinkedToMaster(v.toBool());
        if (isPositionLinkedToMaster()) {
            for (SpannerSegment* seg : spannerSegments()) {
                seg->relinkPropertiesToMaster(PropertyGroup::POSITION);
            }
            relinkPropertiesToMaster(PropertyGroup::POSITION);
        }
        break;
    case Pid::APPEARANCE_LINKED_TO_MASTER:
        setAppearanceLinkedToMaster(v.toBool());
        if (isAppearanceLinkedToMaster()) {
            for (SpannerSegment* seg : spannerSegments()) {
                seg->relinkPropertiesToMaster(PropertyGroup::APPEARANCE);
            }
            relinkPropertiesToMaster(PropertyGroup::APPEARANCE);
        }
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
    switch (m_anchor) {
    case Anchor::SEGMENT: {
        if (systemFlag()) {
            m_startElement = startSegment();
        } else {
            Segment* seg = score()->tick2segmentMM(tick(), false, SegmentType::ChordRest);
            if (!seg || seg->empty() || !seg->element(track())) {
                seg = score()->tick2segment(tick(), false, SegmentType::ChordRest);
            }
            m_startElement = nullptr;
            if (seg) {
                EngravingItem* e = seg->element(track());
                if (e) {
                    m_startElement = e;
                } else {
                    track_idx_t strack = (track() / VOICES) * VOICES;
                    track_idx_t etrack = strack + VOICES;
                    for (track_idx_t t = strack; t < etrack; ++t) {
                        if (seg->element(t)) {
                            m_startElement = seg->element(t);
                            break;
                        }
                    }
                }
            }
        }
    }
    break;

    case Anchor::MEASURE:
        m_startElement = score()->tick2measure(tick());
        break;

    case Anchor::CHORD:
    case Anchor::NOTE:
        break;
    }

    Chord* startChord = m_startElement && m_startElement->isChord() ? toChord(m_startElement) : nullptr;
    if (startChord) {
        startChord->addStartingSpanner(this);
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
        m_endElement = nullptr;
        return;
    }

    switch (m_anchor) {
    case Anchor::SEGMENT: {
        if (track2() == mu::nidx) {
            setTrack2(track());
        }
        if (ticks().isZero() && isTextLine() && explicitParent()) {           // special case palette
            setTicks(score()->lastSegment()->tick() - m_tick);
        }
        if (systemFlag()) {
            m_endElement = endSegment();
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
                m_endElement = s->element(t + v);
                if (m_endElement) {
                    break;
                }
            }
        } else {
            // find last cr on this staff that ends before tick2
            m_endElement = score()->findCRinStaff(tick2(), track2() / VOICES);
        }
        if (!m_endElement) {
            LOGD("%s no end element for tick %d", typeName(), tick2().ticks());
            return;
        }

        if (endCR() && !endCR()->measure()->isMMRest() && !systemFlag()) {
            ChordRest* cr = endCR();
            Fraction nticks = cr->tick() + cr->actualTicks() - m_tick;
            if ((m_ticks - nticks) > Fraction(0, 1)) {
                LOGD("%s ticks changed, %d -> %d", typeName(), m_ticks.ticks(), nticks.ticks());
                setTicks(nticks);
                if (isOttava()) {
                    staff()->updateOttava();
                }
            }
        }
    }
    break;

    case Anchor::MEASURE:
        m_endElement = score()->tick2measure(tick2() - Fraction(1, 1920));
        if (!m_endElement) {
            LOGD("Spanner::computeEndElement(), measure not found for tick %d\n", tick2().ticks() - 1);
            m_endElement = score()->lastMeasure();
        }
        break;
    case Anchor::NOTE:
        if (!m_endElement) {
            ChordRest* cr = score()->findCR(tick2(), track2());
            if (cr && cr->isChord()) {
                m_endElement = toChord(cr)->upNote();
            }
        }
    case Anchor::CHORD:
        break;
    }

    Chord* endChord = m_endElement && m_endElement->isChord() ? toChord(m_endElement) : nullptr;
    if (endChord) {
        endChord->addEndingSpanner(this);
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
    if (m_anchor != Anchor::NOTE) {
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
    assert(m_anchor == Anchor::CHORD);
    if (!m_startElement) {
        m_startElement = findStartChord();
    }

    if (m_startElement && m_startElement->isChord()) {
        return toChord(m_startElement);
    }

    return nullptr;
}

//---------------------------------------------------------
//   endChord
//---------------------------------------------------------

Chord* Spanner::endChord()
{
    assert(m_anchor == Anchor::CHORD);
    if (!m_endElement && type() == ElementType::SLUR) {
        m_endElement = findEndChord();
    }

    if (m_endElement && m_endElement->isChord()) {
        return toChord(m_endElement);
    }

    return nullptr;
}

//---------------------------------------------------------
//   startCR
//---------------------------------------------------------

ChordRest* Spanner::startCR()
{
    assert(m_anchor == Anchor::SEGMENT || m_anchor == Anchor::CHORD);
    if (!m_startElement || m_startElement->score() != score()) {
        m_startElement = findStartCR();
    }
    return (m_startElement && m_startElement->isChordRest()) ? toChordRest(m_startElement) : nullptr;
}

//---------------------------------------------------------
//   endCR
//---------------------------------------------------------

ChordRest* Spanner::endCR()
{
    assert(m_anchor == Anchor::SEGMENT || m_anchor == Anchor::CHORD);
    if ((!m_endElement || m_endElement->score() != score())) {
        m_endElement = findEndCR();
    }
    return (m_endElement && m_endElement->isChordRest()) ? toChordRest(m_endElement) : nullptr;
}

//---------------------------------------------------------
//   findStartChord
//---------------------------------------------------------

Chord* Spanner::findStartChord() const
{
    assert(m_anchor == Anchor::CHORD);
    ChordRest* cr = score()->findCR(tick(), track());
    return cr && cr->isChord() ? toChord(cr) : nullptr;
}

//---------------------------------------------------------
//   findEndChord
//---------------------------------------------------------

Chord* Spanner::findEndChord() const
{
    assert(m_anchor == Anchor::CHORD);
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
    assert(m_anchor == Anchor::SEGMENT || m_anchor == Anchor::CHORD);
    return score()->findCR(tick(), track());
}

//---------------------------------------------------------
//   findEndCR
//---------------------------------------------------------

ChordRest* Spanner::findEndCR() const
{
    assert(m_anchor == Anchor::SEGMENT || m_anchor == Anchor::CHORD);
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
    Segment* rightSegment = score()->tick2rightSegment(tick(), style().styleB(Sid::createMultiMeasureRests));
    if (rightSegment && rightSegment->tick() < tick2()) {
        return rightSegment;
    }
    return score()->tick2leftSegment(tick(), style().styleB(Sid::createMultiMeasureRests));
}

//---------------------------------------------------------
//   endSegment
//---------------------------------------------------------

Segment* Spanner::endSegment() const
{
    return score()->tick2leftSegment(tick2(), style().styleB(Sid::createMultiMeasureRests), systemFlag());
}

//---------------------------------------------------------
//   startMeasure
//---------------------------------------------------------

Measure* Spanner::startMeasure() const
{
    return toMeasure(m_startElement);
}

//---------------------------------------------------------
//   endMeasure
//---------------------------------------------------------

Measure* Spanner::endMeasure() const
{
    return toMeasure(m_endElement);
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
    m_color = col;
}

//---------------------------------------------------------
//   setStartElement
//---------------------------------------------------------

void Spanner::setStartElement(EngravingItem* e)
{
#ifndef NDEBUG
    if (m_anchor == Anchor::NOTE) {
        assert(!e || e->type() == ElementType::NOTE);
    }
#endif
    m_startElement = e;
}

//---------------------------------------------------------
//   setEndElement
//---------------------------------------------------------

void Spanner::setEndElement(EngravingItem* e)
{
#ifndef NDEBUG
    if (m_anchor == Anchor::NOTE) {
        assert(!e || e->type() == ElementType::NOTE);
    }
#endif
    m_endElement = e;
    if (e && ticks() == Fraction() && m_tick >= Fraction()) {
        setTicks(std::max(e->tick() - m_tick, Fraction()));
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
    if (m_tick == v) {
        return;
    }

    m_tick = v;

    Score* score = this->score();

    if (score) {
        score->spannerMap().setDirty();
    }
}

//---------------------------------------------------------
//   setTick2
//---------------------------------------------------------

void Spanner::setTick2(const Fraction& f)
{
    setTicks(f - m_tick);
}

//---------------------------------------------------------
//   setTicks
//---------------------------------------------------------

void Spanner::setTicks(const Fraction& f)
{
    if (m_ticks == f) {
        return;
    }

    m_ticks = f;

    Score* score = this->score();

    if (score) {
        score->spannerMap().setDirty();
    }
}

bool Spanner::isVoiceSpecific() const
{
    static const std::unordered_set<ElementType> VOICE_SPECIFIC_SPANNERS {
        ElementType::TRILL,
        ElementType::HAIRPIN,
        ElementType::LET_RING,
    };

    return VOICE_SPECIFIC_SPANNERS.find(type()) != VOICE_SPECIFIC_SPANNERS.end();
}

//---------------------------------------------------------
//   triggerLayout
//---------------------------------------------------------

void Spanner::triggerLayout() const
{
    // Spanners do not have parent even when added to a score, so can't check parent here
    const track_idx_t tr2 = effectiveTrack2();
    score()->setLayout(m_tick, m_tick + m_ticks, staffIdx(), track2staff(tr2), this);
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
    m_unusedSegments.push_back(seg);
}

//---------------------------------------------------------
//   popUnusedSegment
//    Take the next unused segment for reusing it.
//    If there is no unused segments left returns nullptr.
//---------------------------------------------------------

SpannerSegment* Spanner::popUnusedSegment()
{
    if (m_unusedSegments.empty()) {
        return nullptr;
    }
    SpannerSegment* seg = m_unusedSegments.front();
    m_unusedSegments.pop_front();
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
            SpannerSegment* seg = m_segments.back();
            m_segments.pop_back();
            pushUnusedSegment(seg);
        }
    }
}

bool Spanner::isUserModified() const
{
    for (SpannerSegment* seg : m_segments) {
        if (seg->isUserModified()) {
            return true;
        }
    }

    return false;
}

//---------------------------------------------------------
//   eraseSpannerSegments
//    Completely erase all spanner segments, both used and
//    unused.
//---------------------------------------------------------

void Spanner::eraseSpannerSegments()
{
    DeleteAll(m_segments);
    DeleteAll(m_unusedSegments);
    m_segments.clear();
    m_unusedSegments.clear();
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

        for (SpannerSegment* s : m_segments) {
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
