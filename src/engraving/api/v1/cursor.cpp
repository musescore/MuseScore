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

#include "cursor.h"
#include "elements.h"
#include "score.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/chordrest.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/note.h"
#include "engraving/dom/stafftext.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/page.h"
#include "engraving/dom/system.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/timesig.h"
#include "engraving/dom/tuplet.h"

using namespace mu::engraving::apiv1;

Cursor::Cursor(mu::engraving::Score* s)
    : QObject(0), m_filter(mu::engraving::SegmentType::ChordRest)
{
    setScore(s);
}

//---------------------------------------------------------
//   score
//---------------------------------------------------------

Score* Cursor::score() const
{
    return wrap<Score>(m_score, Ownership::SCORE);
}

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Cursor::setScore(mu::engraving::Score* s)
{
    if (m_score == s) {
        return;
    }

    m_score = s;

    switch (m_inputStateMode) {
    case INPUT_STATE_INDEPENDENT:
        is.reset(new InputState);
        break;
    case INPUT_STATE_SYNC_WITH_SCORE:
        break;
    }
}

void Cursor::setScore(Score* s)
{
    setScore(s ? s->score() : nullptr);
}

mu::engraving::InputState& Cursor::inputState()
{
    return is ? *is.get() : m_score->inputState();
}

//---------------------------------------------------------
//   setInputStateMode
//---------------------------------------------------------

void Cursor::setInputStateMode(InputStateMode val)
{
    if (val == INPUT_STATE_SYNC_WITH_SCORE) {
        is.reset();
    } else {
        is.reset(new InputState);
    }

    m_inputStateMode = val;
}

//---------------------------------------------------------
//   rewind
///   Rewind cursor to a certain position.
///   \param mode Determines the position where to move
///   this cursor. See Cursor::RewindMode to see the list of
///   available rewind modes.
///   \note In MuseScore 2.X, this function took an integer
///   value (0, 1 or 2) as its parameter. For compatibility
///   reasons, the old values are still working, but it is
///   recommended to use RewindMode enumerators instead.
//---------------------------------------------------------

void Cursor::rewind(RewindMode mode)
{
    //
    // rewind to start of score
    //
    if (mode == SCORE_START) {
        mu::engraving::Measure* m = m_score->firstMeasure();
        if (m) {
            setSegment(m->first(m_filter));
            nextInTrack();
        } else {
            setSegment(nullptr);
        }
    }
    //
    // rewind to start of selection
    //
    else if (mode == SELECTION_START) {
        if (!m_score->selection().isRange()) {
            return;
        }
        setSegment(m_score->selection().startSegment());
        setTrack(static_cast<int>(m_score->selection().staffStart() * VOICES));
        nextInTrack();
    }
    //
    // rewind to end of selection
    //
    else if (mode == SELECTION_END) {
        if (!m_score->selection().isRange()) {
            return;
        }
        setSegment(m_score->selection().endSegment());
        setTrack(static_cast<int>((m_score->selection().staffEnd() * VOICES) - 1));      // be sure m_track exists
    }
}

//---------------------------------------------------------
//   rewindToTick
///   Rewind cursor to a position defined by tick.
///   \param tick Determines the position where to move
///   this cursor.
///   \see \ref mu::plugins::api::Segment::tick "Segment.tick"
///   \since MuseScore 3.5
//---------------------------------------------------------

void Cursor::rewindToTick(int tick)
{
    // integer ticks may contain numeric errors so it is
    // better to search not precisely if possible
    mu::engraving::Fraction fTick = mu::engraving::Fraction::fromTicks(tick + 1);
    mu::engraving::Segment* seg = m_score->tick2leftSegment(fTick);
    if (!(seg->segmentType() & m_filter)) {
        // we need another segment type, search by known tick
        seg = m_score->tick2segment(seg->tick(), /* first */ true, m_filter);
    }

    setSegment(seg);
    nextInTrack();
}

//---------------------------------------------------------
//   prev
///   Move the cursor to the previous segment.
///   \return \p false if the beginning of the score is
///   reached, \p true otherwise.
///   \since MuseScore 3.3.4
//---------------------------------------------------------

bool Cursor::prev()
{
    if (!segment()) {
        return false;
    }
    prevInTrack();
    return segment();
}

//---------------------------------------------------------
//   next
///   Move the cursor to the next segment.
///   \return \p false if the end of the score is reached,
///   \p true otherwise.
//---------------------------------------------------------

bool Cursor::next()
{
    if (!segment()) {
        return false;
    }
    setSegment(segment()->next1(m_filter));
    nextInTrack();
    return segment();
}

//---------------------------------------------------------
//   nextMeasure
///   Move the cursor to the first segment of the next
///   measure.
///   \return \p false if the end of the score is reached,
///   \p true otherwise.
//---------------------------------------------------------

bool Cursor::nextMeasure()
{
    if (!segment()) {
        return false;
    }
    mu::engraving::Measure* m = segment()->measure()->nextMeasure();
    if (!m) {
        setSegment(nullptr);
        return false;
    }
    setSegment(m->first(m_filter));
    nextInTrack();
    return segment();
}

//---------------------------------------------------------
//   add
///   Adds the given element to a score at this cursor's
///   position.
//---------------------------------------------------------

void Cursor::add(EngravingItem* wrapped)
{
    mu::engraving::EngravingItem* s = wrapped ? wrapped->element() : nullptr;
    if (!segment() || !s) {
        return;
    }

    // Ensure that the object has the expected ownership
    if (wrapped->ownership() == Ownership::SCORE) {
        LOGW("Cursor::add: Cannot add this element. The element is already part of the score.");
        return;            // Don't allow operation.
    }

    const int m_track = track();
    mu::engraving::Segment* _segment = segment();

    wrapped->setOwnership(Ownership::SCORE);
    s->setScore(m_score);
    s->setTrack(m_track);
    s->setParent(_segment);

    if (s->isChordRest()) {
        s->score()->undoAddCR(toChordRest(s), _segment->measure(), _segment->tick());
    } else if (s->type() == ElementType::KEYSIG) {
        mu::engraving::Segment* ns = _segment->measure()->undoGetSegment(SegmentType::KeySig, _segment->tick());
        s->setParent(ns);
        m_score->undoAddElement(s);
    } else if (s->type() == ElementType::TIMESIG) {
        mu::engraving::Measure* m = _segment->measure();
        Fraction tick = m->tick();
        m_score->cmdAddTimeSig(m, m_track, toTimeSig(s), false);
        m = m_score->tick2measure(tick);
        _segment = m->first(m_filter);
        nextInTrack();
    } else {
        switch (s->type()) {
        // To be added at measure level
        case ElementType::MEASURE_NUMBER:
        case ElementType::SPACER:
        case ElementType::JUMP:
        case ElementType::MARKER:
        case ElementType::HBOX:
        case ElementType::STAFFTYPE_CHANGE:
        case ElementType::LAYOUT_BREAK: {
            mu::engraving::Measure* m = _segment->measure();
            s->setParent(m);
            m_score->undoAddElement(s);
            break;
        }

        // To be added at chord level
        case ElementType::NOTE:
        case ElementType::ARPEGGIO:
        case ElementType::TREMOLO_SINGLECHORD:
        case ElementType::TREMOLO_TWOCHORD:
        case ElementType::CHORDLINE:
        case ElementType::ORNAMENT:
        case ElementType::ARTICULATION: {
            mu::engraving::EngravingItem* curElement = currentElement();
            if (curElement->isChord()) {
                // call Chord::addInternal() (i.e. do the same as a call to Chord.add())
                Chord::addInternal(toChord(curElement), s);
            }
            break;
            break;
        }

        // To be added at chord/rest level
        case ElementType::LYRICS: {
            mu::engraving::EngravingItem* curElement = currentElement();
            if (curElement->isChordRest()) {
                s->setParent(curElement);
                m_score->undoAddElement(s);
            }
            break;
        }

        // To be added to a note (and in case of SYMBOL also to a rest)
        case ElementType::SYMBOL: {
            mu::engraving::EngravingItem* curElement = currentElement();
            if (curElement->isRest()) {
                s->setParent(curElement);
                m_score->undoAddElement(s);
            }
        } // FALLTHROUGH
        case ElementType::FINGERING:
        case ElementType::BEND:
        case ElementType::NOTEHEAD: {
            mu::engraving::EngravingItem* curElement = currentElement();
            if (curElement->isChord()) {
                mu::engraving::Chord* chord = toChord(curElement);
                if (!chord->notes().empty()) {
                    // Get first note from chord to add element
                    mu::engraving::Note* note = chord->notes().front();
                    Note::addInternal(note, s);
                }
            }
            break;
        }

        // To be added to a segment (clef subtype)
        case ElementType::CLEF:
        case ElementType::AMBITUS: {
            mu::engraving::EngravingItem* parent = nullptr;
            // Find backwards first measure containing a clef
            for (mu::engraving::Measure* m = _segment->measure(); m; m = m->prevMeasure()) {
                mu::engraving::Segment* seg = m->findSegment(SegmentType::Clef | SegmentType::HeaderClef, m->tick());
                if (!seg) {
                    continue;
                }
                parent = m->undoGetSegmentR(s->isAmbitus() ? SegmentType::Ambitus : seg->segmentType(), Fraction(0, 1));
                break;
            }
            if (parent && parent->isSegment()) {
                if (s->isClef()) {
                    mu::engraving::Clef* clef = toClef(s);
                    if (clef->clefType() == mu::engraving::ClefType::INVALID) {
                        clef->setClefType(mu::engraving::ClefType::G);
                    }
                }
                s->setParent(parent);
                s->setTrack(m_track);
                m_score->undoAddElement(s);
            }
            break;
        }

        default:           // All others will be added to the current segment
            m_score->undoAddElement(s);
            break;
        }
    }
}

//---------------------------------------------------------
//   addNote
///   \brief Adds a note to the current cursor position.
///   \details The duration of the added note equals to
///   what has been set by the previous setDuration() call.
///   \param pitch MIDI pitch of the added note.
///   \param addToChord add note to the current chord
///   instead of replacing it. This parameter is available
///   since MuseScore 3.3.4.
//---------------------------------------------------------

void Cursor::addNote(int pitch, bool addToChord)
{
    if (!pitchIsValid(pitch)) {
        LOGW("Cursor::addNote: invalid pitch: %d", pitch);
        return;
    }
    if (!segment()) {
        LOGW("Cursor::addNote: cursor location is undefined, use rewind() to define its location");
        return;
    }
    if (!inputState().duration().isValid()) {
        setDuration(1, 4);
    }
    NoteVal nval(pitch);
    m_score->addPitch(nval, addToChord, is.get());
}

//---------------------------------------------------------
//   addRest
///   \brief Adds a rest to the current cursor position.
///   \details The duration of the added rest equals to
///   what has been set by the previous setDuration() call.
///   \since MuseScore 3.5
//---------------------------------------------------------

void Cursor::addRest()
{
    if (!segment()) {
        LOGW("Cursor::addRest: cursor location is undefined, use rewind() to define its location");
        return;
    }
    if (!inputState().duration().isValid()) {
        setDuration(1, 4);
    }
    m_score->enterRest(inputState().duration(), is.get());
}

//---------------------------------------------------------
//   addTuplet
///   \brief Adds a tuplet to the current cursor position.
///   \details This function provides a possibility to setup
///   the tuplet's ratio to any value (similarly to
///   Add > Tuplets > Other... dialog in MuseScore).
///
///   Examples of most typical usage:
///   \code
///   // add a triplet of three eighth notes
///   cursor.addTuplet(fraction(3, 2), fraction(1, 4));
///
///   // add a quintuplet in place of the current chord/rest
///   var cr = cursor.element;
///   if (cr)
///       cursor.addTuplet(fraction(5, 4), cr.duration);
///   \endcode
///
///   \param ratio tuplet ratio. Numerator represents
///   actual number of notes in this tuplet, denominator is
///   a number of "normal" notes which correspond to the
///   same total duration. For example, a triplet has a
///   ratio of 3/2 as it has 3 notes fitting to the
///   duration which would normally be occupied by 2 notes
///   of the same nominal length.
///   \param duration total duration of the tuplet. To
///   create a tuplet with duration matching to duration of
///   existing chord or rest, use its
///   \ref DurationElement.duration "duration" value as
///   a parameter.
///   \since MuseScore 3.5
///   \see \ref DurationElement.tuplet
//---------------------------------------------------------

void Cursor::addTuplet(FractionWrapper* ratio, FractionWrapper* duration)
{
    if (!segment()) {
        LOGW("Cursor::addTuplet: cursor location is undefined, use rewind() to define its location");
        return;
    }

    const mu::engraving::Fraction fRatio = ratio->fraction();
    const mu::engraving::Fraction fDuration = duration->fraction();

    if (!fRatio.isValid() || fRatio.isZero() || fRatio.negative()
        || !fDuration.isValid() || fDuration.isZero() || fDuration.negative()) {
        LOGW("Cursor::addTuplet: invalid parameter values: %s, %s", qPrintable(fRatio.toString()), qPrintable(fDuration.toString()));
        return;
    }

    mu::engraving::Measure* tupletMeasure = segment()->measure();
    const mu::engraving::Fraction tupletTick = segment()->tick();
    if (tupletTick + fDuration > tupletMeasure->endTick()) {
        LOGW(
            "Cursor::addTuplet: cannot add cross-measure tuplet (measure %d, rel.tick %s, duration %s)",
            tupletMeasure->no() + 1, qPrintable(segment()->rtick().toString()), qPrintable(fDuration.toString()));

        return;
    }

    const mu::engraving::Fraction baseLen = fDuration * Fraction(1, fRatio.denominator());
    if (!TDuration::isValid(baseLen)) {
        LOGW(
            "Cursor::addTuplet: cannot create tuplet for ratio %s and duration %s",
            qPrintable(fRatio.toString()), qPrintable(fDuration.toString()));

        return;
    }

    m_score->expandVoice(inputState().segment(), inputState().track());
    mu::engraving::ChordRest* cr = inputState().cr();
    if (!cr) { // shouldn't happen?
        return;
    }

    m_score->changeCRlen(cr, fDuration);

    mu::engraving::Tuplet* tuplet = new mu::engraving::Tuplet(tupletMeasure);
    tuplet->setParent(tupletMeasure);
    tuplet->setTrack(track());
    tuplet->setTick(tupletTick);
    tuplet->setRatio(fRatio);
    tuplet->setTicks(fDuration);
    tuplet->setBaseLen(baseLen);

    m_score->cmdCreateTuplet(cr, tuplet);

    inputState().setSegment(tupletMeasure->tick2segment(tupletTick));
    inputState().setDuration(baseLen);
}

//---------------------------------------------------------
//   setDuration
///   Set duration of the notes added by the cursor.
///   \param z: numerator
///   \param n: denominator. If n == 0, sets duration to
///   a quarter.
///   \see addNote()
//---------------------------------------------------------

void Cursor::setDuration(int z, int n)
{
    TDuration d(Fraction(z, n));
    if (!d.isValid()) {
        d = TDuration(DurationType::V_QUARTER);
    }
    inputState().setDuration(d);
}

//---------------------------------------------------------
//   tick
//---------------------------------------------------------

int Cursor::tick()
{
    const mu::engraving::Segment* seg = segment();
    return seg ? seg->tick().ticks() : 0;
}

//---------------------------------------------------------
//   time
//---------------------------------------------------------

double Cursor::time()
{
    return m_score->utick2utime(tick()) * 1000;
}

//---------------------------------------------------------
//   tempo
//---------------------------------------------------------

qreal Cursor::tempo()
{
    return m_score->tempo(Fraction::fromTicks(tick())).val;
}

//---------------------------------------------------------
//   currentElement
//---------------------------------------------------------

mu::engraving::EngravingItem* Cursor::currentElement() const
{
    const int t = track();
    mu::engraving::Segment* seg = segment();
    return seg && seg->element(t) ? seg->element(t) : nullptr;
}

//---------------------------------------------------------
//   segment
//---------------------------------------------------------

Segment* Cursor::qmlSegment() const
{
    mu::engraving::Segment* seg = segment();
    return seg ? wrap<Segment>(seg, Ownership::SCORE) : nullptr;
}

//---------------------------------------------------------
//   element
//---------------------------------------------------------

EngravingItem* Cursor::element() const
{
    mu::engraving::EngravingItem* e = currentElement();
    if (!e) {
        return nullptr;
    }
    return wrap(e, Ownership::SCORE);
}

//---------------------------------------------------------
//   measure
//---------------------------------------------------------

Measure* Cursor::measure() const
{
    mu::engraving::Segment* seg = segment();
    return seg ? wrap<Measure>(seg->measure(), Ownership::SCORE) : nullptr;
}

//---------------------------------------------------------
//   track
//---------------------------------------------------------

int Cursor::track() const
{
    return static_cast<int>(inputState().track());
}

//---------------------------------------------------------
//   setTrack
//---------------------------------------------------------

void Cursor::setTrack(int track)
{
    if (m_score->ntracks() == 0) {
        return;
    }
    int m_track = std::clamp(track, 0, int(m_score->ntracks() - 1));
    inputState().setTrack(static_cast<track_idx_t>(m_track));
}

//---------------------------------------------------------
//   voice
//---------------------------------------------------------

int Cursor::voice() const
{
    return track() % VOICES;
}

//---------------------------------------------------------
//   setVoice
//---------------------------------------------------------

void Cursor::setVoice(int v)
{
    int m_track = (track() / VOICES) * VOICES + v;
    setTrack(m_track);
}

//---------------------------------------------------------
//   staffIdx
//---------------------------------------------------------

int Cursor::staffIdx() const
{
    return track() / VOICES;
}

//---------------------------------------------------------
//   staff
//---------------------------------------------------------

Staff* Cursor::staff() const
{
    return wrap<Staff>(inputState().staff(), Ownership::SCORE);
}

//---------------------------------------------------------
//   setStaffIdx
//---------------------------------------------------------

void Cursor::setStaffIdx(int v)
{
    int m_track = v * VOICES + track() % VOICES;
    setTrack(m_track);
}

//---------------------------------------------------------
//   setStaff
///   \brief Positions the cursor on a specific \ref Staff.
///   \details As opposed to \ref cursor.setStaffIdx, this
///   method is used to set the position of the cursor to
///   a specific \ref Staff object (as obtained e.g. through
///   \ref element.staff).
///   \since MuseScore 4.6
///   \see \ref Staff
//---------------------------------------------------------

void Cursor::setStaff(Staff* s)
{
    mu::engraving::Staff* staff = s->staff();
    if (!staff || staff == inputState().staff()) {
        return;
    }

    int m_track = int(staff->idx()) + track() % VOICES;
    setTrack(m_track);
}

//---------------------------------------------------------
//   segment
//---------------------------------------------------------

mu::engraving::Segment* Cursor::segment() const
{
    return inputState().segment();
}

//---------------------------------------------------------
//   setSegment
//---------------------------------------------------------

void Cursor::setSegment(mu::engraving::Segment* seg)
{
    inputState().setSegment(seg);
}

//---------------------------------------------------------
//   prevInTrack
//    go to first segment before _segment which has notes / rests in m_track
//---------------------------------------------------------

void Cursor::prevInTrack()
{
    const int t = track();
    mu::engraving::Segment* seg = segment();
    if (seg) {
        seg = seg->prev1(m_filter);
    }
    while (seg && !seg->element(t)) {
        seg = seg->prev1(m_filter);
    }
    setSegment(seg);
}

//---------------------------------------------------------
//   nextInTrack
//    go to first segment at or after _segment which has notes / rests in m_track
//---------------------------------------------------------

void Cursor::nextInTrack()
{
    const int t = track();
    mu::engraving::Segment* seg = segment();
    while (seg && !seg->element(t)) {
        seg = seg->next1(m_filter);
    }
    setSegment(seg);
}

//---------------------------------------------------------
//   qmlKeySignature
//   read access to key signature in current track
//   at current position
//---------------------------------------------------------

int Cursor::qmlKeySignature()
{
    return static_cast<int>(inputState().staff()->key(Fraction::fromTicks(tick())));
}

//---------------------------------------------------------
//   inputStateString
//---------------------------------------------------------

int Cursor::inputStateString() const
{
    const InputState& istate = inputState();
    return istate.staff()->staffType(istate.tick())->visualStringToPhys(istate.string());
}

void Cursor::setInputStateString(int string)
{
    InputState& istate = inputState();
    const int visString = istate.staff()->staffType(istate.tick())->visualStringToPhys(string);
    istate.setString(visString);
}
