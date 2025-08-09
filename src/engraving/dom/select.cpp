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

/**
 \file
 Implementation of class Selection plus other selection related functions.
*/

#include "global/containers.h"
#include "global/io/buffer.h"

#include "../types/types.h"

#include "rw/rwregister.h"

#include "accidental.h"
#include "arpeggio.h"
#include "articulation.h"
#include "beam.h"
#include "breath.h"
#include "chord.h"
#include "dynamic.h"
#include "engravingitem.h"
#include "expression.h"
#include "figuredbass.h"
#include "fingering.h"
#include "fret.h"
#include "guitarbend.h"
#include "harmony.h"
#include "harppedaldiagram.h"
#include "hook.h"
#include "laissezvib.h"
#include "linkedobjects.h"
#include "lyrics.h"
#include "measure.h"
#include "mscore.h"
#include "note.h"
#include "notedot.h"
#include "part.h"
#include "partialtie.h"
#include "rest.h"
#include "score.h"
#include "segment.h"
#include "select.h"
#include "staff.h"
#include "stafftextbase.h"
#include "stem.h"
#include "stemslash.h"
#include "sticking.h"
#include "tie.h"
#include "tremolosinglechord.h"
#include "tremolotwochord.h"
#include "tuplet.h"

#include "utils.h"

#include "log.h"

using namespace mu;
using namespace muse::io;
using namespace mu::engraving;

// ====================================================
// Selection
// ====================================================

Selection::Selection(Score* s)
{
    m_score         = s;
    m_state         = SelState::NONE;
    m_startSegment  = 0;
    m_endSegment    = 0;
    m_activeSegment = 0;
    m_staffStart    = 0;
    m_staffEnd      = 0;
    m_activeTrack   = 0;
    m_currentTick   = Fraction(-1, 1);
    m_currentTrack  = 0;
}

Fraction Selection::tickStart() const
{
    switch (m_state) {
    case SelState::RANGE:
        return m_startSegment ? m_startSegment->tick() : Fraction(-1, 1);
    case SelState::LIST: {
        ChordRest* cr = firstChordRest();
        return (cr) ? cr->tick() : Fraction(-1, 1);
    }
    default:
        return Fraction(-1, 1);
    }
}

Fraction Selection::tickEnd() const
{
    switch (m_state) {
    case SelState::RANGE: {
        if (m_endSegment) {
            return m_endSegment->tick();
        } else {         // endsegment == 0 if end of score
            Measure* m = m_score->lastMeasure();
            return m->endTick();
        }
        break;
    }
    case SelState::LIST: {
        ChordRest* cr = lastChordRest();
        return (cr) ? cr->segment()->tick() : Fraction(-1, 1);
        break;
    }
    default:
        return Fraction(-1, 1);
    }
}

bool Selection::isStartActive() const
{
    return activeSegment() && activeSegment()->tick() == tickStart();
}

bool Selection::isEndActive() const
{
    return activeSegment() && activeSegment()->tick() == tickEnd();
}

EngravingItem* Selection::element() const
{
    return ((state() != SelState::RANGE) && (m_el.size() == 1)) ? m_el[0] : 0;
}

ChordRest* Selection::cr() const
{
    EngravingItem* e = element();
    if (!e) {
        return 0;
    }
    if (e->isNote()) {
        e = e->parentItem();
    }
    if (e->isChordRest()) {
        return toChordRest(e);
    }
    return 0;
}

ChordRest* Selection::currentCR() const
{
    // no selection yet - start at very beginning, not first cr
    if (m_currentTick == Fraction(-1, 1)) {
        return nullptr;
    }
    Segment* s = score()->tick2rightSegment(m_currentTick, true);
    if (!s) {
        return nullptr;
    }
    track_idx_t track = m_currentTrack;
    // staff may have been removed - start at top
    if (track >= score()->ntracks()) {
        track = 0;
    }
    EngravingItem* e = s->element(track);
    if (e && e->isChordRest()) {
        return toChordRest(e);
    } else {
        return nullptr;
    }
}

ChordRest* Selection::activeCR() const
{
    if (m_state != SelState::RANGE || !m_activeSegment) {
        return nullptr;
    }
    if (m_activeSegment == m_startSegment) {
        return firstChordRest(m_activeTrack);
    } else {
        return lastChordRest(m_activeTrack);
    }
}

ChordRest* Selection::firstChordRestInRange(track_idx_t preferredTrack) const
{
    IF_ASSERT_FAILED(isRange()) {
        return nullptr;
    }

    Segment* firstCRSegment = nullptr;

    for (Segment* currSeg = m_startSegment; currSeg && (currSeg != m_endSegment); currSeg = currSeg->next1MM()) {
        if (!currSeg->enabled() || !currSeg->isChordRestType()) {
            continue;
        }
        if (preferredTrack == muse::nidx) {
            // no track specified, use the first CR segment we find...
            firstCRSegment = currSeg;
            break;
        }
        EngravingItem* e = currSeg->element(preferredTrack);
        if (e && e->isChordRest()) {
            return toChordRest(e);
        }
    }

    if (!firstCRSegment) {
        return nullptr;
    }

    for (track_idx_t track = staff2track(staffStart()); track < staff2track(staffEnd()); ++track) {
        EngravingItem* e = firstCRSegment->element(track);
        if (e && e->isChordRest()) {
            return toChordRest(e);
        }
    }
    return nullptr;
}

ChordRest* Selection::lastChordRestInRange(track_idx_t preferredTrack) const
{
    IF_ASSERT_FAILED(isRange()) {
        return nullptr;
    }

    Segment* lastCRSegment = nullptr;

    Segment* lastSegInScore = m_score->lastMeasureMM() ? m_score->lastMeasureMM()->last() : nullptr;
    Segment* currSeg = m_endSegment ? m_endSegment->prev1MM() : lastSegInScore;
    while (currSeg) {
        if (!currSeg->enabled() || !currSeg->isChordRestType()) {
            currSeg = currSeg->prev1MM();
            continue;
        }
        if (preferredTrack == muse::nidx) {
            // no track specified, use the first CR segment we find...
            lastCRSegment = currSeg;
            break;
        }
        EngravingItem* e = currSeg->element(preferredTrack);
        if (e && e->isChordRest()) {
            return toChordRest(e);
        }
        currSeg = currSeg->prev1MM();
        if (currSeg == m_startSegment->prev1MM()) {
            break;
        }
    }

    if (!lastCRSegment) {
        return nullptr;
    }

    for (track_idx_t track = staff2track(staffStart()); track < staff2track(staffEnd()); ++track) {
        EngravingItem* e = lastCRSegment->element(track);
        if (e && e->isChordRest()) {
            return toChordRest(e);
        }
    }

    return nullptr;
}

ChordRest* Selection::firstChordRest(track_idx_t track) const
{
    if (isRange()) {
        return firstChordRestInRange(track);
    }
    if (m_el.size() == 1) {
        EngravingItem* el = m_el[0];
        if (el->isNote()) {
            return toChordRest(el->explicitParent());
        } else if (el->isChordRest()) {
            return toChordRest(el);
        }
        return nullptr;
    }
    ChordRest* cr = nullptr;
    for (EngravingItem* el : m_el) {
        if (el->isNote()) {
            el = el->parentItem();
        }
        if (el->isChordRest()) {
            if (track != muse::nidx && el->track() != track) {
                continue;
            }
            if (cr) {
                if (toChordRest(el)->tick() < cr->tick()) {
                    cr = toChordRest(el);
                }
            } else {
                cr = toChordRest(el);
            }
        }
    }
    return cr;
}

ChordRest* Selection::lastChordRest(track_idx_t track) const
{
    if (isRange()) {
        return lastChordRestInRange(track);
    }
    if (m_el.size() == 1) {
        EngravingItem* el = m_el[0];
        if (el) {
            if (el->isNote()) {
                return toChordRest(el->explicitParent());
            } else if (el->isChordRest()) {
                return toChordRest(el);
            }
        }
        return nullptr;
    }
    ChordRest* cr = nullptr;
    for (EngravingItem* el : m_el) {
        if (el->isNote()) {
            el = toNote(el)->chord();
        }
        if (el->isChordRest() && toChordRest(el)->segment()->isChordRestType()) {
            if (track != muse::nidx && el->track() != track) {
                continue;
            }
            if (cr) {
                if (toChordRest(el)->tick() >= cr->tick()) {
                    cr = toChordRest(el);
                }
            } else {
                cr = toChordRest(el);
            }
        }
    }
    return cr;
}

Measure* Selection::findMeasure() const
{
    Measure* m = 0;
    if (m_el.size() > 0) {
        EngravingItem* el = m_el[0];
        m = toMeasure(el->findMeasure());
    }
    return m;
}

MeasureBase* Selection::startMeasureBase() const
{
    EngravingItem* selectionElement = element();
    if (selectionElement) {
        if (selectionElement->isHBox()) {
            return toMeasureBase(selectionElement);
        }
        MeasureBase* mb = selectionElement->findMeasureBase();
        if (mb) {
            return mb;
        }
    }

    if (tickStart().negative()) { // Tick is not set
        return nullptr;
    }

    bool mmrests = m_score->style().styleB(Sid::createMultiMeasureRests);
    Fraction refTick = tickStart();

    return mmrests ? m_score->tick2measureMM(refTick) : m_score->tick2measure(refTick);
}

MeasureBase* Selection::endMeasureBase() const
{
    EngravingItem* selectionElement = element();
    if (selectionElement) {
        if (selectionElement->isHBox()) {
            return toMeasureBase(selectionElement);
        }
        MeasureBase* mb = selectionElement->findMeasureBase();
        if (mb) {
            return mb;
        }
    }

    if (tickEnd().negative()) { // Tick is not set
        return nullptr;
    }

    bool mmrests = m_score->style().styleB(Sid::createMultiMeasureRests);
    Fraction refTick = tickEnd() - Fraction::eps();

    return mmrests ? m_score->tick2measureMM(refTick) : m_score->tick2measure(refTick);
}

std::vector<System*> Selection::selectedSystems() const
{
    EngravingItem* el = element();
    if (el && (el->isSystemLockIndicator() /*TODO: || el->isStaffVisibilityIndicator*/)) {
        return { const_cast<System*>(toIndicatorIcon(el)->system()) };
    }

    const MeasureBase* startMB = startMeasureBase();
    const MeasureBase* endMB = endMeasureBase();
    if (!startMB || !endMB) {
        return {};
    }

    bool mmrests = score()->style().styleB(Sid::createMultiMeasureRests);
    std::vector<System*> systems;
    for (const MeasureBase* mb = startMB; mb && mb->isBeforeOrEqual(endMB); mb = mmrests ? mb->nextMM() : mb->next()) {
        System* sys = mb->system();
        if ((mb->isMeasure() || mb->isHBox()) && (systems.empty() || sys != systems.back())) {
            systems.push_back(sys);
        }
    }

    return systems;
}

void Selection::deselectAll()
{
    if (m_state == SelState::RANGE) {
        m_score->setUpdateAll();
    }
    clear();
    updateState();
}

static RectF changeSelection(EngravingItem* e, bool b)
{
    RectF r = e->canvasBoundingRect();
    e->setSelected(b);
    r.unite(e->canvasBoundingRect());
    return r;
}

void Selection::clear()
{
    IF_ASSERT_FAILED(!isLocked()) {
        LOGE() << "selection locked, reason: " << lockReason();
        return;
    }

    for (EngravingItem* e : m_el) {
        if (e->isSpanner()) {       // TODO: only visible elements should be selectable?
            Spanner* sp = toSpanner(e);
            for (auto s : sp->spannerSegments()) {
                e->score()->addRefresh(changeSelection(s, false));
            }
        } else {
            e->score()->addRefresh(changeSelection(e, false));
        }
    }
    m_el.clear();
    m_startSegment  = 0;
    m_endSegment    = 0;
    m_activeSegment = 0;
    m_staffStart    = 0;
    m_staffEnd      = 0;
    m_activeTrack   = 0;
    setState(SelState::NONE);
}

void Selection::remove(EngravingItem* el)
{
    const bool removed = muse::remove(m_el, el);
    el->setSelected(false);
    if (removed) {
        updateState();
    }
}

void Selection::add(EngravingItem* el)
{
    IF_ASSERT_FAILED(!isLocked()) {
        LOGE() << "selection locked, reason: " << lockReason();
        return;
    }
    m_el.push_back(el);
    update();
}

void Selection::appendFiltered(EngravingItem* e)
{
    IF_ASSERT_FAILED(!isLocked()) {
        LOGE() << "selection locked, reason: " << lockReason();
        return;
    }
    if (canSelectVoice(e->voice()) && canSelect(e)) {
        m_el.push_back(e);
    }
}

void Selection::appendFiltered(const std::unordered_set<EngravingItem*>& elems)
{
    IF_ASSERT_FAILED(!isLocked()) {
        LOGE() << "selection locked, reason: " << lockReason();
        return;
    }

    for (EngravingItem* elem : elems) {
        IF_ASSERT_FAILED(!elem->isSpannerSegment()) {
            LOGE() << "Append whole spanners instead of spanner segments";
            continue;
        }

        // Special handling for bends...
        if (elem->isGuitarBend()) {
            GuitarBend* bend = toGuitarBend(elem);
            appendGuitarBend(bend);
            continue;
        }

        // Special handling for spanners...
        if (elem->isSpanner() && !elem->isPartialTie() && !elem->isLaissezVib()) {
            const Spanner* spanner = toSpanner(elem);
            if (!noteAnchoredSpannerIsInRange(spanner, tickStart(), tickEnd())) {
                continue;
            }
            for (SpannerSegment* spannerSeg : spanner->spannerSegments()) {
                appendFiltered(spannerSeg);
            }
            continue;
        }

        // Special handling for grace notes...
        if (elem->isChord() && toChord(elem)->isGrace()) {
            appendChordRest(toChordRest(elem));
        }

        appendFiltered(elem);
    }
}

void Selection::appendChordRest(ChordRest* cr)
{
    IF_ASSERT_FAILED(!isLocked()) {
        LOGE() << "selection locked, reason: " << lockReason();
        return;
    }

    if (!canSelectVoice(cr->voice())) {
        return;
    }

    const std::unordered_set<EngravingItem*> crAnchored = collectElementsAnchoredToChordRest(cr);
    appendFiltered(crAnchored);

    if (cr->isRestFamily()) {
        appendFiltered(cr);
        Rest* r = toRest(cr);
        for (int i = 0; i < r->dots(); ++i) {
            appendFiltered(r->dot(i));
        }
        return;
    }

    IF_ASSERT_FAILED(cr->isChord()) {
        return;
    }

    Chord* chord = toChord(cr);

    const size_t totalNotesInChord = chord->notes().size();
    const bool isSingleNote = totalNotesInChord == 1;

    size_t totalAppendedNotes = 0;
    for (size_t noteIdx = 0; noteIdx < totalNotesInChord; ++noteIdx) {
        Note* note = chord->notes().at(noteIdx);

        const std::unordered_set<EngravingItem*> noteAnchored = collectElementsAnchoredToNote(note, true, false);
        appendFiltered(noteAnchored);

        if (chord->isGrace() && !canSelect(chord)) {
            continue;
        }

        //! Hack Explainer: Due to the fact that this method is called while we're still in the process of "building" our
        //! selection, we can't know for certain whether the selection as a whole will contain multi-note Chords (and thus
        //! whether the "includeSingleNotes" flag should apply - see includeSingleNotes in select.h). For this reason, this
        //! method ALWAYS appends single note chords. If single note chords should be ommitted from a selection, we simply
        //! don't call this method (see usage of appendChordRest in Selection::updateSelectedElements).
        if (/*hack*/ !isSingleNote && !canSelectNoteIdx(noteIdx, totalNotesInChord, /*hack*/ true)) {
            continue;
        }

        m_el.push_back(note);
        ++totalAppendedNotes;

        if (note->accidental()) {
            m_el.push_back(note->accidental());
        }
        for (EngravingItem* el : note->el()) {
            if (el->isFingering()) {
                // Slight hack (already handled, see collectElementsAnchoredToNote)...
                continue;
            }
            m_el.push_back(el);
        }
        for (NoteDot* dot : note->dots()) {
            m_el.push_back(dot);
        }
    }

    //! NOTE: Beams, stems, etc should only be added if all notes in chord are selected...
    if (totalAppendedNotes < totalNotesInChord) {
        return;
    }
    if (chord->beam() && !muse::contains(m_el, static_cast<EngravingItem*>(chord->beam()))) {
        m_el.push_back(chord->beam());
    }
    if (chord->stem()) {
        m_el.push_back(chord->stem());
    }
    if (chord->hook()) {
        m_el.push_back(chord->hook());
    }
    if (chord->stemSlash()) {
        m_el.push_back(chord->stemSlash());
    }
}

void Selection::appendTupletHierarchy(Tuplet* innermostTuplet)
{
    if (!canSelectVoice(innermostTuplet->voice()) || muse::contains(m_el, static_cast<EngravingItem*>(innermostTuplet))) {
        return;
    }

    //! NOTE: Not hugely efficient since canSelectTuplet will recursively check all contained tuplets...
    if (!selectionFilter().canSelectTuplet(innermostTuplet, tickStart(), tickEnd(),
                                           rangeContainsMultiNoteChords())) {
        return;
    }

    m_el.push_back(innermostTuplet);

    // Recursively append upwards/outwards
    Tuplet* outerTuplet = innermostTuplet->tuplet();
    if (outerTuplet) {
        appendTupletHierarchy(outerTuplet);
    }
}

void Selection::appendGuitarBend(GuitarBend* guitarBend)
{
    if (!guitarBend || !canSelectVoice(guitarBend->voice())) {
        return;
    }

    m_el.push_back(guitarBend);

    if (GuitarBendHold* hold = guitarBend->holdLine()) {
        if (hold->tick2() < tickEnd()) {
            m_el.push_back(hold);
        }
    }

    if (GuitarBendSegment* bendSeg = toGuitarBendSegment(guitarBend->frontSegment())) {
        if (GuitarBendText* bendText = bendSeg->bendText()) {
            m_el.push_back(bendText);
        }
    }
}

void Selection::updateSelectedElements()
{
    IF_ASSERT_FAILED(!isLocked()) {
        LOGE() << "selection locked, reason: " << lockReason();
        return;
    }
    if (m_state != SelState::RANGE) {
        m_rangeContainsMultiNoteChords = false;
        update();
        return;
    }
    if (m_state == SelState::RANGE && m_plannedTick1 != Fraction(-1, 1) && m_plannedTick2 != Fraction(-1, 1)) {
        const staff_idx_t staffStart = m_staffStart;
        const staff_idx_t staffEnd = m_staffEnd;

        deselectAll();

        Segment* s1 = m_score->tick2segmentMM(m_plannedTick1);
        Segment* s2 = m_score->tick2segmentMM(m_plannedTick2, /* first */ true /* HACK */);
        if (s2 && s2->measure()->isMMRest()) {
            s2 = s2->prev1MM(); // HACK
        }
        // These hacks are needed to prevent https://musescore.org/node/173381.
        // This should exclude any segments belonging to MM-rest range from the selection.
        if (s1 && s2 && s1->tick() + s1->ticks() > s2->tick()) {
            // can happen with MM rests as tick2measure returns only the first segment for them.

            m_plannedTick1 = Fraction(-1, 1);
            m_plannedTick2 = Fraction(-1, 1);
            return;
        }

        if (s2 && s2 == s2->measure()->first()) {
            // we want the last segment of the previous measure (unless it's part of a MMrest)
            Measure* prevMeasure = s2->measure()->prevMeasure();
            if (!(prevMeasure && prevMeasure != prevMeasure->coveringMMRestOrThis())) {
                s2 = s2->prev1();
            }
        }

        setRange(s1, s2, staffStart, staffEnd);

        m_plannedTick1 = Fraction(-1, 1);
        m_plannedTick2 = Fraction(-1, 1);
    }

    for (EngravingItem* e : m_el) {
        e->setSelected(false);
    }
    m_el.clear();

    // assert:
    size_t staves = m_score->nstaves();
    if (m_staffStart == muse::nidx || m_staffStart >= staves || m_staffEnd == muse::nidx || m_staffEnd > staves
        || m_staffStart >= m_staffEnd) {
        LOGD("updateSelectedElements: bad staff selection %zu - %zu, staves %zu", m_staffStart, m_staffEnd, staves);
        m_staffStart = 0;
        m_staffEnd   = 0;
    }
    track_idx_t startTrack = m_staffStart * VOICES;
    track_idx_t endTrack   = m_staffEnd * VOICES;

    //! NOTE: See appendChord/appendChordRest - we should include single notes if the selection consists solely of
    //! single notes, even if the "include single notes" filter flag is false...
    std::vector<Chord*> singleNoteChords;
    size_t totalChordsFound = 0;

    //! NOTE: We also need to delay the appending of tuplets. We should only display tuplets as selected
    //! if all of their contained elements are selected...
    std::unordered_set<Tuplet*> innerTuplets;

    for (track_idx_t st = startTrack; st < endTrack; ++st) {
        for (Segment* s = m_startSegment; s && (s != m_endSegment); s = s->next1MM()) {
            if (!s->enabled() || s->isEndBarLineType()) { // do not select end bar line
                continue;
            }
            for (EngravingItem* e : s->annotations()) {
                if (e->track() != st) {
                    continue;
                }
                if (e->isFretDiagram()) {
                    FretDiagram* fd = toFretDiagram(e);
                    if (Harmony* harm = fd->harmony()) {
                        appendFiltered(harm);
                    }
                }
                appendFiltered(e);
            }
            EngravingItem* e = s->element(st);
            if (!e || e->generated() || e->isTimeSig() || e->isKeySig()) {
                continue;
            }

            if (!e->isChordRest()) {
                appendFiltered(e);
                continue;
            }

            ChordRest* cr = toChordRest(e);
            if (Tuplet* tuplet = cr->tuplet()) {
                innerTuplets.emplace(tuplet);
            }

            if (!cr->isChord()) {
                appendChordRest(cr);
                continue;
            }

            Chord* chord = toChord(cr);
            const std::vector<Note*> notes = chord->notes();
            if (notes.size() == 1) {
                singleNoteChords.emplace_back(chord);
            } else {
                appendChordRest(chord);
            }
            ++totalChordsFound;
        }
    }

    m_rangeContainsMultiNoteChords = totalChordsFound > singleNoteChords.size();

    for (Chord* singleNoteChord : singleNoteChords) {
        if (!m_rangeContainsMultiNoteChords || selectionFilter().includeSingleNotes()) {
            appendChordRest(singleNoteChord);
        }
        // Include elements anchored to the note even if the note itself isn't included...
        const Note* note = singleNoteChord->notes().front();
        const std::unordered_set<EngravingItem*> noteAnchored = collectElementsAnchoredToNote(note, true, false);
        appendFiltered(noteAnchored);
        const std::unordered_set<EngravingItem*> crAnchored = collectElementsAnchoredToChordRest(singleNoteChord);
        appendFiltered(crAnchored);
    }

    for (Tuplet* tuplet : innerTuplets) {
        appendTupletHierarchy(tuplet);
    }

    const Fraction rangeStart = tickStart();
    const Fraction rangeEnd = tickEnd();

    //! NOTE: Ties are NOT handled in here - these are note anchored and handled in appendChordRest...
    for (auto i = m_score->spanner().begin(); i != m_score->spanner().end(); ++i) {
        Spanner* sp = (*i).second;
        // ignore spanners belonging to other tracks
        if (sp->track() < startTrack || sp->track() >= endTrack) {
            continue;
        }

        const bool startAndEndCalculated = sp->startElement() && sp->endElement();
        if (!startAndEndCalculated || !canSelectVoice(sp->track()) || sp->isVolta()) {
            continue;
        }

        //! NOTE: isZeroLengthAtStart (for lack of a better name) covers an edge case that may occur if sp spans from
        //! a grace note to its parent note or vice versa. In this scenario sp->tick == sp->tick2 == rangeStart and
        //! we should include the spanner in our selection...
        const bool overlapsSelectionRange = rangeStart < sp->tick2() && rangeEnd > sp->tick();
        const bool isZeroLengthAtStart = sp->tick() == sp->tick2() && sp->tick() == rangeStart;
        if (!overlapsSelectionRange && !isZeroLengthAtStart) {
            continue;
        }

        for (auto seg : sp->spannerSegments()) {
            appendFiltered(seg);
        }
    }
    update();
    m_score->setSelectionChanged(true);
}

bool Selection::rangeContainsMultiNoteChords() const
{
    IF_ASSERT_FAILED(m_state == SelState::RANGE) {
        return false;
    }
    return m_rangeContainsMultiNoteChords;
}

void Selection::setRange(Segment* startSegment, Segment* endSegment, staff_idx_t staffStart, staff_idx_t staffEnd)
{
    assert(staffEnd > staffStart && staffEnd <= m_score->nstaves());
    assert(!(endSegment && !startSegment));

    m_startSegment  = startSegment;
    m_endSegment = endSegment;
    m_activeSegment = endSegment;
    m_staffStart = staffStart;
    m_staffEnd = staffEnd;
    m_activeTrack = staff2track(staffStart);

    if (m_state == SelState::RANGE) {
        m_score->setSelectionChanged(true);
    } else {
        setState(SelState::RANGE);
    }
}

//---------------------------------------------------------
//   setRangeTicks
//    sets the range to be selected on next
//    updateSelectedElements() call. Can be used if some
//    segment structure changes are expected (e.g. if
//    creating MM rests is pending).
//---------------------------------------------------------

void Selection::setRangeTicks(const Fraction& tick1, const Fraction& tick2, staff_idx_t staffStart, staff_idx_t staffEnd)
{
    assert(staffEnd > staffStart && staffEnd <= m_score->nstaves());

    m_plannedTick1 = tick1;
    m_plannedTick2 = tick2;
    m_startSegment = m_endSegment = m_activeSegment = nullptr;
    m_staffStart = staffStart;
    m_staffEnd = staffEnd;
    m_activeTrack = staff2track(staffStart);

    if (m_state == SelState::RANGE) {
        m_score->setSelectionChanged(true);
    } else {
        setState(SelState::RANGE);
    }
}

//---------------------------------------------------------
//   update
///   Set select flag for all Elements in select list.
//---------------------------------------------------------

void Selection::update()
{
    for (EngravingItem* e : m_el) {
        e->setSelected(true); // also tells accessibility that e has focus
    }

    ChordRest* cr = activeCR();
    if (!cr) {
        updateState();
        return;
    }

    // Only one element can have focus at a time, and currently the final element in m_el has
    // focus. That's ok for a LIST selection (because it corresponds to the last element the
    // user clicked on) but for range selections, we should focus something in activeCR...
    EngravingItem* toSelectAgain = nullptr;
    if (cr->isChord()) {
        // Use the top selected note in the chord...
        const std::vector<Note*> notes = toChord(cr)->notes();
        const size_t noteCount = notes.size();
        for (size_t noteIdx = noteCount - 1; noteIdx < noteCount; --noteIdx) {
            Note* note = notes.at(noteIdx);
            if (selectionFilter().canSelectNoteIdx(noteIdx, notes.size(), rangeContainsMultiNoteChords())) {
                toSelectAgain = note;
                break;
            }
        }
    } else {
        toSelectAgain = cr;
    }

    if (!toSelectAgain || !toSelectAgain->selected()) {
        // Means we have a range with no selected elements...
        updateState();
        return;
    }

    toSelectAgain->setSelected(true); // HACK: select it again so accessibility thinks it has focus

    updateState();
}

void Selection::dump()
{
    LOGD("Selection dump: ");
    switch (m_state) {
    case SelState::NONE:   LOGD("NONE");
        return;
    case SelState::RANGE:  LOGD("RANGE");
        break;
    case SelState::LIST:   LOGD("LIST");
        break;
    }
    for (const EngravingItem* e : m_el) {
        LOGD("  %p %s", e, e->typeName());
    }
}

//---------------------------------------------------------
//   updateState
///   update selection and input state
//---------------------------------------------------------

void Selection::updateState()
{
    const size_t totalStaves = m_score->nstaves();

    //! NOTE: m_startSegment being non-null and m_endSegment being null is a valid case. It means we've selected the
    //! last segment of the final measure...
    const bool rangeSegsInvalid = !m_startSegment || (m_endSegment && m_endSegment->tick() <= m_startSegment->tick());
    const bool rangeStavesInvalid = m_staffStart == muse::nidx || m_staffStart >= totalStaves
                                    || m_staffEnd == muse::nidx || m_staffEnd > totalStaves
                                    || m_staffStart >= m_staffEnd;
    const bool rangeIsValid = isRange() && !rangeSegsInvalid && !rangeStavesInvalid;

    //! NOTE: Range can be valid even if no elements are selectable in that range...
    if (m_el.size() == 0 && !rangeIsValid) {
        setState(SelState::NONE);
    } else if (m_state == SelState::NONE) {
        setState(SelState::LIST);
    }

    const EngravingItem* e = element();
    if (!e) {
        return;
    }

    if (e->isSpannerSegment()) {
        m_currentTick = toSpannerSegment(e)->spanner()->tick();
    } else {
        m_currentTick = e->tick();
    }
    // ignore system elements (e.g., frames)
    if (e->track() != muse::nidx) {
        m_currentTrack = e->track();
    }
}

void Selection::setState(SelState s)
{
    if (m_state == s) {
        return;
    }

    m_state = s;
    m_score->setSelectionChanged(true);
}

String Selection::mimeType() const
{
    switch (m_state) {
    case SelState::LIST:
        return isSingle() ? String::fromAscii(mimeSymbolFormat) : String::fromAscii(mimeSymbolListFormat);
    case SelState::RANGE:
        return String::fromAscii(mimeStaffListFormat);
    case SelState::NONE:
    default:
        break;
    }

    return String();
}

muse::ByteArray Selection::mimeData() const
{
    muse::ByteArray a;
    switch (m_state) {
    case SelState::LIST:
        if (isSingle()) {
            a = element()->mimeData();
        } else {
            a = symbolListMimeData();
        }
        break;
    case SelState::NONE:
        break;
    case SelState::RANGE:
        a = staffMimeData();
        break;
    }
    return a;
}

static bool hasElementInTrack(Segment* startSeg, Segment* endSeg, track_idx_t track)
{
    for (Segment* seg = startSeg; seg != endSeg; seg = seg->next1MM()) {
        if (!seg->enabled()) {
            continue;
        }
        if (seg->element(track)) {
            return true;
        }
    }
    return false;
}

static Fraction firstElementInTrack(Segment* startSeg, Segment* endSeg, track_idx_t track)
{
    for (Segment* seg = startSeg; seg != endSeg; seg = seg->next1MM()) {
        if (!seg->enabled()) {
            continue;
        }
        if (seg->element(track)) {
            return seg->tick();
        }
    }
    return Fraction(-1, 1);
}

muse::ByteArray Selection::staffMimeData() const
{
    Buffer buffer;
    buffer.open(IODevice::WriteOnly);
    XmlWriter xml(&buffer);

    xml.startDocument();

    SelectionFilter filter = selectionFilter();
    Fraction curTick;

    Fraction ticks  = tickEnd() - tickStart();
    int staves = static_cast<int>(staffEnd() - staffStart());

    xml.startElement("StaffList", { { "version", (MScore::testMode ? "2.00" : Constants::MSC_VERSION_STR) },
                         { "tick", tickStart().toString() },
                         { "len", ticks.toString() },
                         { "staff", staffStart() },
                         { "staves", staves } });

    Segment* seg1 = m_startSegment;
    Segment* seg2 = m_endSegment;

    for (staff_idx_t staffIdx = staffStart(); staffIdx < staffEnd(); ++staffIdx) {
        track_idx_t startTrack = staffIdx * VOICES;
        track_idx_t endTrack   = startTrack + VOICES;

        xml.startElement("Staff", { { "id", staffIdx } });

        Staff* staff = m_score->staff(staffIdx);
        Part* part = staff->part();
        Interval interval = part->instrument(seg1->tick())->transpose();
        if (interval.chromatic) {
            xml.tag("transposeChromatic", interval.chromatic);
        }
        if (interval.diatonic) {
            xml.tag("transposeDiatonic", interval.diatonic);
        }
        xml.startElement("voiceOffset");
        for (voice_idx_t voice = 0; voice < VOICES; voice++) {
            if (hasElementInTrack(seg1, seg2, startTrack + voice) && filter.canSelectVoice(voice)) {
                Fraction offset = firstElementInTrack(seg1, seg2, startTrack + voice) - tickStart();
                xml.tag("voice", { { "id", voice } }, offset.ticks());
            }
        }
        xml.endElement();     // </voiceOffset>

        rw::RWRegister::writer(m_score->iocContext())->writeSegments(xml, &filter, startTrack, endTrack, seg1, seg2, false, false, curTick);
        xml.endElement();
    }

    xml.endElement();
    return buffer.data();
}

muse::ByteArray Selection::symbolListMimeData() const
{
    struct MapData {
        EngravingItem* e;
        Segment* s;
    };

    Buffer buffer;
    buffer.open(IODevice::WriteOnly);
    XmlWriter xml(&buffer);

    xml.startDocument();

    track_idx_t topTrack    = 1000000;
    track_idx_t bottomTrack = 0;
    Segment* firstSeg    = 0;
    Fraction firstTick   = Fraction(0x7FFFFFFF, 1);
    Segment* seg         = 0;
    std::multimap<int64_t, MapData> map;

    // scan selection element list, inserting relevant elements in a tick-sorted map
    for (EngravingItem* e : m_el) {
        switch (e->type()) {
        case ElementType::ARTICULATION:
        case ElementType::ORNAMENT:
        case ElementType::TAPPING:
        case ElementType::ARPEGGIO:
        case ElementType::TREMOLO_SINGLECHORD: {
            // ignore articulations not attached to chords/rest or segment
            if (!e->explicitParent()->isChordRest()) {
                continue;
            }
            ChordRest* cr = toChordRest(e->explicitParent());
            seg = cr->segment();
        } break;
        case ElementType::FERMATA:
            seg = toFermata(e)->segment();
            break;
        case ElementType::BREATH:
            seg = toBreath(e)->segment();
            break;
        case ElementType::PLAYTECH_ANNOTATION:
        case ElementType::CAPO:
        case ElementType::STRING_TUNINGS:
        case ElementType::STAFF_TEXT:
            seg = toStaffTextBase(e)->segment();
            break;
        case ElementType::EXPRESSION:
            seg = toExpression(e)->segment();
            break;
        case ElementType::STICKING:
            seg = toSticking(e)->segment();
            break;
        case ElementType::FIGURED_BASS:
            seg = toFiguredBass(e)->segment();
            break;
        case ElementType::LYRICS:
            seg = toLyrics(e)->segment();
            break;
        case ElementType::DYNAMIC:
            seg = toDynamic(e)->segment();
            break;
        case ElementType::HARMONY:
        case ElementType::FRET_DIAGRAM:
            // ignore chord symbols or fret diagrams not attached to segment
            if (e->explicitParent()->isSegment()) {
                seg = toSegment(e->explicitParent());
                break;
            }
            continue;
        case ElementType::HARP_DIAGRAM:
            seg = toHarpPedalDiagram(e)->segment();
            break;
        case ElementType::SLUR_SEGMENT:
        case ElementType::HAIRPIN_SEGMENT:
        case ElementType::OTTAVA_SEGMENT:
        case ElementType::TRILL_SEGMENT:
        case ElementType::LET_RING_SEGMENT:
        case ElementType::VIBRATO_SEGMENT:
        case ElementType::PALM_MUTE_SEGMENT:
        case ElementType::WHAMMY_BAR_SEGMENT:
        case ElementType::RASGUEADO_SEGMENT:
        case ElementType::HARMONIC_MARK_SEGMENT:
        case ElementType::PICK_SCRAPE_SEGMENT:
        case ElementType::TEXTLINE_SEGMENT:
        case ElementType::PEDAL_SEGMENT:
            e = toSpannerSegment(e)->spanner();
            [[fallthrough]];
        case ElementType::SLUR:
        case ElementType::HAMMER_ON_PULL_OFF:
        case ElementType::HAIRPIN:
        case ElementType::OTTAVA:
        case ElementType::TRILL:
        case ElementType::LET_RING:
        case ElementType::VIBRATO:
        case ElementType::PALM_MUTE:
        case ElementType::WHAMMY_BAR:
        case ElementType::RASGUEADO:
        case ElementType::HARMONIC_MARK:
        case ElementType::PICK_SCRAPE:
        case ElementType::TEXTLINE:
        case ElementType::PEDAL:
            seg = toSpanner(e)->startSegment();
            break;
        default:
            // Elements of other types are ignored. To allow copying them,
            // add support for them here and in `Score::pasteSymbols`.
            continue;
        }
        track_idx_t track = e->track();
        if (track < topTrack) {
            topTrack = track;
        }
        if (track > bottomTrack) {
            bottomTrack = track;
        }
        if (seg->tick() < firstTick) {
            firstSeg  = seg;
            firstTick = seg->tick();
        }
        MapData mapData = { e, seg };
        map.insert(std::pair<int64_t, MapData>(((int64_t)track << 32) + seg->tick().ticks(), mapData));
    }

    xml.startElement("SymbolList", { { "version", Constants::MSC_VERSION_STR },
                         { "fromtrack", topTrack },
                         { "totrack", bottomTrack } });

    // scan the map, outputting elements each with a relative <track> tag on track change,
    // a relative tick and the number of CR segments to skip
    track_idx_t currTrack = muse::nidx;
    for (auto iter = map.cbegin(); iter != map.cend(); ++iter) {
        int numSegs;
        track_idx_t track = static_cast<track_idx_t>(iter->first >> 32);
        if (currTrack != track) {
            xml.tag("trackOffset", static_cast<int>(track - topTrack));
            currTrack = track;
            seg       = firstSeg;
        }
        xml.tag("tickOffset", static_cast<int>(iter->first & 0xFFFFFFFF) - firstTick.ticks());
        numSegs = 0;
        // with figured bass, we need to look for the proper segment
        // not only according to ChordRest elements, but also annotations
        if (iter->second.e->type() == ElementType::FIGURED_BASS) {
            bool done = false;
            for (; seg; seg = seg->next1()) {
                if (seg->isChordRestType()) {
                    // if no ChordRest in right track, look in annotations
                    if (seg->element(currTrack) == nullptr) {
                        for (EngravingItem* el : seg->annotations()) {
                            // do annotations include our element?
                            if (el == iter->second.e) {
                                done = true;
                                break;
                            }
                            // do annotations include any f.b.?
                            if (el->type() == ElementType::FIGURED_BASS && el->track() == track) {
                                numSegs++;                  //yes: it counts as a step
                                break;
                            }
                        }
                        if (done) {
                            break;
                        }
                        continue;                           // segment is not relevant: no ChordRest nor f.b.
                    } else {
                        if (iter->second.s == seg) {
                            break;
                        }
                    }
                    numSegs++;
                }
            }
        } else {
            while (seg && iter->second.s != seg) {
                seg = seg->nextCR(currTrack);
                numSegs++;
            }
        }
        xml.tag("segDelta", numSegs);
        rw::RWRegister::writer(m_score->iocContext())->writeItem(iter->second.e, xml);
    }

    xml.endElement();
    buffer.close();
    return buffer.data();
}

std::vector<EngravingItem*> Selection::elements(ElementType type) const
{
    std::vector<EngravingItem*> result;

    for (EngravingItem* element : m_el) {
        if (element->type() == type) {
            result.push_back(element);
        }
    }

    return result;
}

std::vector<Note*> Selection::noteList(track_idx_t selTrack) const
{
    std::vector<Note*> nl;

    if (m_state == SelState::LIST) {
        for (EngravingItem* e : m_el) {
            if (e->isNote()) {
                nl.push_back(toNote(e));
            }
        }
    } else if (m_state == SelState::RANGE) {
        for (staff_idx_t staffIdx = staffStart(); staffIdx < staffEnd(); ++staffIdx) {
            track_idx_t startTrack = staffIdx * VOICES;
            track_idx_t endTrack   = startTrack + VOICES;
            for (Segment* seg = m_startSegment; seg && seg != m_endSegment; seg = seg->next1()) {
                if (!(seg->segmentType() & (SegmentType::ChordRest))) {
                    continue;
                }
                for (track_idx_t track = startTrack; track < endTrack; ++track) {
                    if (!canSelectVoice(track)) {
                        continue;
                    }
                    EngravingItem* e = seg->element(track);
                    if (e == 0 || e->type() != ElementType::CHORD
                        || (selTrack != muse::nidx && selTrack != track)) {
                        continue;
                    }
                    Chord* c = toChord(e);
                    const std::vector<Note*> notes = c->notes();
                    for (size_t noteIdx = 0; noteIdx < notes.size(); ++noteIdx) {
                        Note* note = notes.at(noteIdx);
                        if (selectionFilter().canSelectNoteIdx(noteIdx, notes.size(), rangeContainsMultiNoteChords())) {
                            nl.push_back(note);
                        }
                    }
                    for (Chord* g : c->graceNotes()) {
                        nl.insert(nl.end(), g->notes().begin(), g->notes().end());
                    }
                }
            }
        }
    }
    return nl;
}

//---------------------------------------------------------
//   checkStart
//     return false if element is NOT a tuplet or is start of a tuplet/tremolo
//     return true  if element is part of a tuplet/tremolo, but not the start
//---------------------------------------------------------

static bool checkStart(EngravingItem* e)
{
    if (e == 0 || !e->isChordRest()) {
        return false;
    }
    ChordRest* cr = toChordRest(e);
    bool rv = false;
    if (cr->tuplet()) {
        // check that complete tuplet is selected, all the way up to top level
        Tuplet* tuplet = cr->tuplet();
        while (tuplet) {
            if (tuplet->elements().front() != e) {
                return true;
            }
            e = tuplet;
            tuplet = tuplet->tuplet();
        }
    } else if (cr->type() == ElementType::CHORD) {
        rv = false;
        Chord* chord = toChord(cr);
        if (chord->tremoloTwoChord()) {
            rv = chord->tremoloTwoChord()->chord2() == chord;
        }
    }
    return rv;
}

//---------------------------------------------------------
//   checkEnd
//     return false if element is NOT a tuplet or is end of a tuplet
//     return true  if element is part of a tuplet, but not the end
//---------------------------------------------------------

static bool checkEnd(EngravingItem* e, const Fraction& endTick)
{
    if (e == 0 || !e->isChordRest()) {
        return false;
    }
    ChordRest* cr = toChordRest(e);
    bool rv = false;
    if (cr->tuplet()) {
        // check that complete tuplet is selected, all the way up to top level
        Tuplet* tuplet = cr->tuplet();
        while (tuplet) {
            if (tuplet->elements().back() != e) {
                return true;
            }
            e = tuplet;
            tuplet = tuplet->tuplet();
        }
        // also check that the selection extends to the end of the top-level tuplet
        tuplet = toTuplet(e);
        if (tuplet->elements().front()->tick() + tuplet->actualTicks() > endTick) {
            return true;
        }
    } else if (cr->type() == ElementType::CHORD) {
        rv = false;
        Chord* chord = toChord(cr);
        if (chord->tremoloTwoChord()) {
            rv = chord->tremoloTwoChord()->chord1() == chord;
        }
    }
    return rv;
}

//---------------------------------------------------------
//   canCopy
//    return false if range selection intersects a tuplet
//    or a tremolo, or a local time signature, or only part
//    of a measure repeat group
//---------------------------------------------------------

bool Selection::canCopy() const
{
    if (m_state != SelState::RANGE) {
        return true;
    }

    Fraction endTick = m_endSegment ? m_endSegment->tick() : m_score->lastSegment()->tick();

    for (staff_idx_t staffIdx = m_staffStart; staffIdx != m_staffEnd; ++staffIdx) {
        for (voice_idx_t voice = 0; voice < VOICES; ++voice) {
            track_idx_t track = staffIdx * VOICES + voice;
            if (!canSelectVoice(track)) {
                continue;
            }

            // check first cr in track within selection
            ChordRest* check = m_startSegment->nextChordRest(track);
            if (check && check->tick() < endTick && checkStart(check)) {
                return false;
            }

            if (!m_endSegment) {
                continue;
            }

            // find last segment in the selection.
            // Note that _endSegment is the first segment after the selection

            Segment* endSegmentSelection = m_startSegment;
            while (endSegmentSelection->nextCR(track)
                   && (endSegmentSelection->nextCR(track)->tick() < m_endSegment->tick())) {
                endSegmentSelection = endSegmentSelection->nextCR(track);
            }

            if (checkEnd(endSegmentSelection->element(track), endTick)) {
                return false;
            }
        }

        // loop through measures on this staff checking for local time signatures
        for (Measure* m = m_startSegment->measure(); m && m->tick() < endTick; m = m->nextMeasure()) {
            if (m_score->staff(staffIdx)->isLocalTimeSignature(m->tick())) {
                return false;
            }
        }

        // check if selection starts or ends partway through measure repeat group
        const ChordRest* firstCR = firstChordRest();
        const ChordRest* lastCR = lastChordRest();
        if ((firstCR && firstCR->measure()->isMeasureRepeatGroupWithPrevM(staffIdx))
            || (lastCR && lastCR->measure()->isMeasureRepeatGroupWithNextM(staffIdx))) {
            return false;
        }
    }
    return true;
}

//---------------------------------------------------------
//   measureRange
//    return false if no measure range selected
//---------------------------------------------------------

bool Selection::measureRange(Measure** m1, Measure** m2) const
{
    if (!isRange()) {
        return false;
    }
    *m1 = startSegment()->measure();
    Segment* s2 = endSegment();
    *m2 = s2 ? s2->measure() : m_score->lastMeasure();
    if (*m1 == *m2) {
        return true;
    }
    // if selection extends to last segment of a measure,
    // then endSegment() will point to next measure
    // this won't normally happen because end barlines are excluded from range selection
    // but just in case, detect this and back up one measure
    if (*m2 && s2 && (*m2)->tick() == s2->tick()) {
        *m2 = (*m2)->prevMeasure();
    }
    return true;
}

//---------------------------------------------------------
//   uniqueElements
//    Return list of selected elements.
//    If some elements are linked, only one of the linked
//    elements show up in the list.
//---------------------------------------------------------

const std::list<EngravingItem*> Selection::uniqueElements() const
{
    std::list<EngravingItem*> l;

    for (EngravingItem* e : elements()) {
        bool alreadyThere = false;
        for (EngravingItem* ee : l) {
            if ((ee->links() && ee->links()->contains(e)) || e == ee) {
                alreadyThere = true;
                break;
            }
        }
        if (!alreadyThere) {
            l.push_back(e);
        }
    }
    return l;
}

//---------------------------------------------------------
//   uniqueNotes
//    Return list of selected notes.
//    If some notes are linked, only one of the linked
//    elements show up in the list.
//---------------------------------------------------------

std::list<Note*> Selection::uniqueNotes(track_idx_t track) const
{
    std::list<Note*> l;

    for (Note* nn : noteList(track)) {
        for (Note* note : nn->tiedNotes()) {
            bool alreadyThere = false;
            for (Note* n : l) {
                if ((n->links() && n->links()->contains(note)) || n == note) {
                    alreadyThere = true;
                    break;
                }
            }
            if (!alreadyThere) {
                l.push_back(note);
            }
        }
    }
    return l;
}

bool Selection::elementsSelected(const ElementTypeSet& types) const
{
    for (const EngravingItem* element : m_el) {
        if (muse::contains(types, element->type())) {
            return true;
        }
    }

    return false;
}

//---------------------------------------------------------
//   extendRangeSelection
//    Extends the range selection to contain the given
//    chord rest.
//---------------------------------------------------------

void Selection::extendRangeSelection(ChordRest* cr)
{
    extendRangeSelection(cr->segment(),
                         cr->nextSegmentAfterCR(SegmentType::ChordRest
                                                | SegmentType::EndBarLine
                                                | SegmentType::Clef),
                         cr->staffIdx(),
                         cr->tick(),
                         cr->tick());
}

//---------------------------------------------------------
//   extendRangeSelection
//    Extends the range selection to contain the given
//    segment. SegAfter should represent the segment
//    that is after seg. Tick and etick represent
//    the start and end tick of an element. Useful when
//    extending by a chord rest.
//---------------------------------------------------------

void Selection::extendRangeSelection(Segment* seg, Segment* segAfter, staff_idx_t staffIdx, const Fraction& tick, const Fraction& etick)
{
    bool activeSegmentIsStart = false;
    staff_idx_t activeStaff = m_activeTrack / VOICES;

    if (staffIdx < m_staffStart) {
        m_staffStart = staffIdx;
    } else if (staffIdx >= m_staffEnd) {
        m_staffEnd = staffIdx + 1;
    } else if (m_staffEnd - m_staffStart > 1) { // at least 2 staff selected
        if (staffIdx == m_staffStart + 1 && activeStaff == m_staffStart) {   // going down
            m_staffStart = staffIdx;
        } else if (staffIdx == m_staffEnd - 2 && activeStaff == m_staffEnd - 1) { // going up
            m_staffEnd = staffIdx + 1;
        }
    }

    if (tick < tickStart()) {
        m_startSegment = seg;
        activeSegmentIsStart = true;
    } else if (etick > tickEnd()) {
        m_endSegment = segAfter;
    } else {
        if (m_activeSegment == m_startSegment) {
            m_startSegment = seg;
            activeSegmentIsStart = true;
        } else {
            m_endSegment = segAfter;
        }
    }
    m_activeSegment = activeSegmentIsStart ? m_startSegment : m_endSegment;
    m_score->setSelectionChanged(true);
    assert(!(m_endSegment && !m_startSegment));
}

SelectionFilter Selection::selectionFilter() const
{
    return m_score->selectionFilter();
}

bool Selection::canSelectNoteIdx(size_t noteIdx, size_t totalNotesInChord, bool selectionContainsMultiNoteChords) const
{
    return selectionFilter().canSelectNoteIdx(noteIdx, totalNotesInChord, selectionContainsMultiNoteChords);
}
