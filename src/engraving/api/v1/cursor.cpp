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
    : QObject(0), _filter(mu::engraving::SegmentType::ChordRest)
{
    setScore(s);
}

//---------------------------------------------------------
//   score
//---------------------------------------------------------

Score* Cursor::score() const
{
    return wrap<Score>(_score, Ownership::SCORE);
}

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Cursor::setScore(mu::engraving::Score* s)
{
    if (_score == s) {
        return;
    }

    _score = s;

    switch (_inputStateMode) {
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
    return is ? *is.get() : _score->inputState();
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

    _inputStateMode = val;
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
        mu::engraving::Measure* m = _score->firstMeasure();
        if (m) {
            setSegment(m->first(_filter));
            nextInTrack();
        } else {
            setSegment(nullptr);
        }
    }
    //
    // rewind to start of selection
    //
    else if (mode == SELECTION_START) {
        if (!_score->selection().isRange()) {
            return;
        }
        setSegment(_score->selection().startSegment());
        setTrack(static_cast<int>(_score->selection().staffStart() * VOICES));
        nextInTrack();
    }
    //
    // rewind to end of selection
    //
    else if (mode == SELECTION_END) {
        if (!_score->selection().isRange()) {
            return;
        }
        setSegment(_score->selection().endSegment());
        setTrack(static_cast<int>((_score->selection().staffEnd() * VOICES) - 1));      // be sure _track exists
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
    mu::engraving::Segment* seg = _score->tick2leftSegment(fTick);
    if (!(seg->segmentType() & _filter)) {
        // we need another segment type, search by known tick
        seg = _score->tick2segment(seg->tick(), /* first */ true, _filter);
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
    setSegment(segment()->next1(_filter));
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
    if (m == 0) {
        setSegment(nullptr);
        return false;
    }
    setSegment(m->first(_filter));
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

    const int _track = track();
    mu::engraving::Segment* _segment = segment();

    wrapped->setOwnership(Ownership::SCORE);
    s->setScore(_score);
    s->setTrack(_track);
    s->setParent(_segment);

    if (s->isChordRest()) {
        s->score()->undoAddCR(toChordRest(s), _segment->measure(), _segment->tick());
    } else if (s->type() == ElementType::KEYSIG) {
        mu::engraving::Segment* ns = _segment->measure()->undoGetSegment(SegmentType::KeySig, _segment->tick());
        s->setParent(ns);
        _score->undoAddElement(s);
    } else if (s->type() == ElementType::TIMESIG) {
        mu::engraving::Measure* m = _segment->measure();
        Fraction tick = m->tick();
        _score->cmdAddTimeSig(m, _track, toTimeSig(s), false);
        m = _score->tick2measure(tick);
        _segment = m->first(_filter);
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
            _score->undoAddElement(s);
            break;
        }

        // To be added at chord level
        case ElementType::NOTE:
        case ElementType::ARPEGGIO:
        case ElementType::TREMOLO_SINGLECHORD:
        case ElementType::TREMOLO_TWOCHORD:
        case ElementType::CHORDLINE:
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
                _score->undoAddElement(s);
            }
            break;
        }

        // To be added to a note (and in case of SYMBOL also to a rest)
        case ElementType::SYMBOL: {
            mu::engraving::EngravingItem* curElement = currentElement();
            if (curElement->isRest()) {
                s->setParent(curElement);
                _score->undoAddElement(s);
            }
        } // FALLTHROUGH
        case ElementType::FINGERING:
        case ElementType::BEND:
        case ElementType::NOTEHEAD: {
            mu::engraving::EngravingItem* curElement = currentElement();
            if (curElement->isChord()) {
                mu::engraving::Chord* chord = toChord(curElement);
                mu::engraving::Note* note = nullptr;
                if (chord->notes().size() > 0) {
                    // Get first note from chord to add element
                    note = chord->notes().front();
                }
                if (note) {
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
            for (mu::engraving::Measure* m = _segment->measure(); m != 0; m = m->prevMeasure()) {
                mu::engraving::Segment* seg = m->findSegment(SegmentType::Clef | SegmentType::HeaderClef, m->tick());
                if (seg != 0) {
                    parent = m->undoGetSegmentR(s->isAmbitus() ? SegmentType::Ambitus : seg->segmentType(), Fraction(0,
                                                                                                                     1));
                    break;
                }
            }
            if (parent && parent->isSegment()) {
                if (s->isClef()) {
                    mu::engraving::Clef* clef = toClef(s);
                    if (clef->clefType() == mu::engraving::ClefType::INVALID) {
                        clef->setClefType(mu::engraving::ClefType::G);
                    }
                }
                s->setParent(parent);
                s->setTrack(_track);
                _score->undoAddElement(s);
            }
            break;
        }

        default:           // All others will be added to the current segment
            _score->undoAddElement(s);
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
    _score->addPitch(nval, addToChord, is.get());
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
    _score->enterRest(inputState().duration(), is.get());
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

    _score->expandVoice(inputState().segment(), inputState().track());
    mu::engraving::ChordRest* cr = inputState().cr();
    if (!cr) { // shouldn't happen?
        return;
    }

    _score->changeCRlen(cr, fDuration);

    mu::engraving::Tuplet* tuplet = new mu::engraving::Tuplet(tupletMeasure);
    tuplet->setParent(tupletMeasure);
    tuplet->setTrack(track());
    tuplet->setTick(tupletTick);
    tuplet->setRatio(fRatio);
    tuplet->setTicks(fDuration);
    tuplet->setBaseLen(baseLen);

    _score->cmdCreateTuplet(cr, tuplet);

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
    return _score->utick2utime(tick()) * 1000;
}

//---------------------------------------------------------
//   tempo
//---------------------------------------------------------

qreal Cursor::tempo()
{
    return _score->originalTempo(Fraction::fromTicks(tick())).val;
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

void Cursor::setTrack(int _track)
{
    int tracks = static_cast<int>(_score->nstaves() * VOICES);
    if (_track < 0) {
        _track = 0;
    } else if (_track >= tracks) {
        _track = tracks - 1;
    }
    inputState().setTrack(_track);
}

//---------------------------------------------------------
//   setStaffIdx
//---------------------------------------------------------

void Cursor::setStaffIdx(int v)
{
    track_idx_t _track = v * VOICES + track() % VOICES;
    size_t tracks = _score->nstaves() * VOICES;
    if (_track == muse::nidx) {
        _track = 0;
    } else if (_track >= tracks) {
        _track = tracks - 1;
    }
    inputState().setTrack(_track);
}

//---------------------------------------------------------
//   setVoice
//---------------------------------------------------------

void Cursor::setVoice(int v)
{
    track_idx_t _track = (track() / VOICES) * VOICES + v;
    size_t tracks = _score->nstaves() * VOICES;
    if (_track == muse::nidx) {
        _track = 0;
    } else if (_track >= tracks) {
        _track = tracks - 1;
    }
    inputState().setTrack(_track);
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
//   staffIdx
//---------------------------------------------------------

int Cursor::staffIdx() const
{
    return track() / VOICES;
}

//---------------------------------------------------------
//   voice
//---------------------------------------------------------

int Cursor::voice() const
{
    return track() % VOICES;
}

//---------------------------------------------------------
//   prevInTrack
//    go to first segment before _segment which has notes / rests in _track
//---------------------------------------------------------

void Cursor::prevInTrack()
{
    const int t = track();
    mu::engraving::Segment* seg = segment();
    if (seg) {
        seg = seg->prev1(_filter);
    }
    while (seg && !seg->element(t)) {
        seg = seg->prev1(_filter);
    }
    setSegment(seg);
}

//---------------------------------------------------------
//   nextInTrack
//    go to first segment at or after _segment which has notes / rests in _track
//---------------------------------------------------------

void Cursor::nextInTrack()
{
    const int t = track();
    mu::engraving::Segment* seg = segment();
    while (seg && seg->element(t) == 0) {
        seg = seg->next1(_filter);
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
    mu::engraving::Staff* staff = _score->staves()[staffIdx()];
    return static_cast<int>(staff->key(Fraction::fromTicks(tick())));
}

//---------------------------------------------------------
//   inputStateString
//---------------------------------------------------------

int Cursor::inputStateString() const
{
    const InputState& istate = inputState();
    return _score->staff(staffIdx())->staffType(istate.tick())->visualStringToPhys(istate.string());
}

void Cursor::setInputStateString(int string)
{
    InputState& istate = inputState();
    const int visString = _score->staff(staffIdx())->staffType(istate.tick())->visualStringToPhys(string);
    istate.setString(visString);
}
