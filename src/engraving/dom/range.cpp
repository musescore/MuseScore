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

#include "range.h"

#include "barline.h"
#include "chord.h"
#include "excerpt.h"
#include "factory.h"
#include "linkedobjects.h"
#include "measure.h"
#include "measurerepeat.h"
#include "note.h"
#include "rest.h"
#include "score.h"
#include "segment.h"
#include "slur.h"
#include "staff.h"
#include "tie.h"
#include "spacer.h"
#include "marker.h"
#include "volta.h"

#include "tremolosinglechord.h"
#include "tremolotwochord.h"
#include "tuplet.h"
#include "utils.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   cleanupTuplet
//---------------------------------------------------------

static void cleanupTuplet(Tuplet* t)
{
    const auto elements(t->elements());
    t->clear();
    for (DurationElement* e : elements) {
        e->setTuplet(nullptr);     // prevent deleting the top tuplet by its children
        if (e->isTuplet()) {
            cleanupTuplet(toTuplet(e));
        }
        delete e;
    }
}

//   clonedNote
//---------------------------------------------------------
static Note* clonedNote(const Note* srcNote, TrackList::ClonedChordMap& cChordMap)
{
    Note* cNote = nullptr;
    Chord* srcChord = srcNote->chord();
    Chord* cChord = cChordMap[srcChord];

    if (srcChord && cChord) {
        int noteSrcPosition = 0;
        int noteClonedPosition = 0;
        bool noteFound = false;
        // We presume notes where created in the same order in the srcChord and clonedChord
        // Note position in sourceChord
        for (Note* n : srcChord->notes()) {
            ++noteSrcPosition;
            if (srcNote == n) {
                noteFound = true;
                break;
            }
        }
        if (noteFound) {
            // Note in clonedChord
            for (Note* n : cChord->notes()) {
                ++noteClonedPosition;
                if (noteClonedPosition == noteSrcPosition) {
                    cNote = n;
                    break;
                }
            }
        }
    }
    return cNote;
}

//---------------------------------------------------------
//   cloneAndRebuildSpanners
//---------------------------------------------------------
static void cloneAndRebuildSpanners(TrackList::ClonedChordMap& cChordMap)
{
    Chord* srcChord = nullptr;
    Note* cNote = nullptr;
    auto iter = cChordMap.begin();

    // Clone Spanners from Chords
    while (iter != cChordMap.end()) {
        srcChord = iter->first;

        //
        // Add For and Back Note's spanners to cloned Note
        //
        for (Note* srcNote : srcChord->notes()) {
            cNote = clonedNote(srcNote, cChordMap);

            if (cNote) {
                for (Spanner* sp : srcNote->spannerFor()) {
                    if (sp->isGuitarBend() || sp->isGlissando()) {
                        Spanner* csp = toSpanner(sp->clone());

                        Note* endClonedNote = clonedNote(toNote(sp->endElement()), cChordMap);
                        if (endClonedNote) {
                            csp->setNoteSpan(cNote, endClonedNote);
                        }
                        cNote->addSpannerFor(csp);
                    }
                }
                for (Spanner* sp : srcNote->spannerBack()) {
                    if (sp->isGuitarBend() || sp->isGlissando()) {
                        Spanner* csp = toSpanner(sp->clone());
                        Note* startClonedNote = clonedNote(toNote(sp->startElement()), cChordMap);

                        if (startClonedNote) {
                            csp->setNoteSpan(startClonedNote, cNote);
                        }
                        cNote->addSpannerBack(csp);
                    }
                }
            }
        }
        ++iter;
    }
}

//---------------------------------------------------------
//   TrackList
//---------------------------------------------------------

TrackList::~TrackList()
{
    m_clonedChord.clear();
    for (EngravingItem* e : *this) {
        if (e->isTuplet()) {
            Tuplet* t = toTuplet(e);
            cleanupTuplet(t);
        } else {
            delete e;
        }
    }
}

//---------------------------------------------------------
//   appendTuplet
//---------------------------------------------------------

void TrackList::appendTuplet(Tuplet* srcTuplet, Tuplet* dstTuplet)
{
    for (DurationElement* de : srcTuplet->elements()) {
        DurationElement* e = toDurationElement(de->clone());
        dstTuplet->add(e);
        if (de->isTuplet()) {
            Tuplet* st = toTuplet(de);
            Tuplet* dt = toTuplet(e);
            appendTuplet(st, dt);
        } else if (de->explicitParent() && de->explicitParent()->isSegment()) {
            Segment* seg = toSegment(de->explicitParent());
            for (EngravingItem* ee : seg->annotations()) {
                bool addSysObject = ee->systemFlag() && !ee->isLinked() && ee->track() == 0 && e->track() == 0;
                if (addSysObject || (!ee->systemFlag() && ee->track() == e->track())) {
                    m_range->m_annotations.push_back({ e->tick(), ee->clone() });
                }
            }
        }
    }
}

//---------------------------------------------------------
//   combineTuplet
//---------------------------------------------------------

void TrackList::combineTuplet(Tuplet* dst, Tuplet* src)
{
    dst->setTicks(dst->ticks() * 2);
    dst->setBaseLen(dst->baseLen().shift(-1));

    // try to combine tie'd notes
    unsigned idx = 0;
    if (dst->elements().back()->isChord() && src->elements().front()->isChord()) {
        Chord* chord = toChord(src->elements().front());
        bool akkumulateChord = true;
        for (Note* n : chord->notes()) {
            if (!n->tieBackNonPartial() || !n->tieBack()->generated()) {
                akkumulateChord = false;
                break;
            }
        }
        if (akkumulateChord) {
            Chord* bc  = toChord(dst->elements().back());
            bc->setTicks(bc->ticks() + chord->ticks());

            // forward ties
            int i = 0;
            for (Note* n : bc->notes()) {
                n->setTieFor(chord->notes()[i]->tieFor());
                ++i;
            }
            idx = 1;          // skip first src element
        }
    }

    for (; idx < src->elements().size(); ++idx) {
        DurationElement* de = src->elements()[idx];
        DurationElement* e = toDurationElement(de->clone());
        dst->add(e);
        if (de->isTuplet()) {
            Tuplet* st = toTuplet(de);
            Tuplet* dt = toTuplet(e);
            appendTuplet(st, dt);
        }
    }
}

//---------------------------------------------------------
//   append
//---------------------------------------------------------

void TrackList::append(EngravingItem* e)
{
    if (e->isDurationElement()) {
        m_duration += toDurationElement(e)->ticks();

        bool accumulateRest = e->isRest() && !empty() && back()->isRest();
        Segment* s          = accumulateRest ? toRest(e)->segment() : 0;

        if (s && !s->score()->isSpannerStartEnd(s->tick(), e->track()) && !s->annotations().size()) {
            // akkumulate rests
            Rest* rest  = toRest(back());
            Fraction du = rest->ticks();
            du += toRest(e)->ticks();
            rest->setTicks(du);
        } else {
            EngravingItem* element = 0;
            if (e->isTuplet()) {
                Tuplet* src = toTuplet(e);
                if (src->generated() && !empty() && back()->isTuplet()) {
                    Tuplet* b = toTuplet(back());
                    combineTuplet(b, src);
                } else {
                    element = e->clone();
                    Tuplet* dst = toTuplet(element);
                    appendTuplet(src, dst);
                }
            } else {
                element = e->clone();
                ChordRest* src = toChordRest(e);
                Segment* s1 = src->segment();
                for (EngravingItem* ee : s1->annotations()) {
                    bool addSysObject = ee->systemFlag() && !ee->isLinked() && ee->track() == 0 && e->track() == 0;
                    if (addSysObject || (!ee->systemFlag() && ee->track() == e->track())) {
                        m_range->m_annotations.push_back({ s1->tick(), ee->clone() });
                    }
                }
                if (e->isChord()) {
                    Chord* chord = toChord(e);

                    // Map between Chords and clonned chords
                    m_clonedChord.insert(std::make_pair(chord, toChord(element)));

                    bool akkumulateChord = true;
                    for (Note* n : chord->notes()) {
                        if (!n->tieBackNonPartial() || !n->tieBack()->generated()) {
                            akkumulateChord = false;
                            break;
                        }
                    }
                    if (akkumulateChord && !empty() && back()->isChord()) {
                        Chord* bc   = toChord(back());
                        const Fraction du = bc->ticks() + chord->ticks();
                        bc->setTicks(du);

                        // forward ties
                        int idx = 0;
                        for (Note* n : bc->notes()) {
                            n->setTieFor(chord->notes()[idx]->tieFor());
                            ++idx;
                        }
                        delete element;
                        element = 0;
                    }
                }
            }
            if (element) {
                element->setSelected(false);
                std::vector<EngravingItem*>::push_back(element);
            }
        }
    } else {
        EngravingItem* c = e->clone();
        c->resetExplicitParent();
        std::vector<EngravingItem*>::push_back(c);
    }
}

//---------------------------------------------------------
//   appendGap
//---------------------------------------------------------

void TrackList::appendGap(const Fraction& du, Score* score)
{
    if (du.isZero()) {
        return;
    }
    EngravingItem* e = empty() ? 0 : back();
    if (e && e->isRest()) {
        Rest* rest  = toRest(back());
        Fraction dd = rest->ticks();
        dd          += du;
        m_duration   += du;
        rest->setTicks(dd);
    } else {
        Rest* rest = Factory::createRest(score->dummy()->segment());
        rest->setTicks(du);
        std::vector<EngravingItem*>::push_back(rest);
        m_duration   += du;
    }
}

//---------------------------------------------------------
//   truncate
//    reduce len of last gap by f
//---------------------------------------------------------

bool TrackList::truncate(const Fraction& f)
{
    if (empty()) {
        return true;
    }
    EngravingItem* e = back();
    if (!e->isRest()) {
        return false;
    }
    Rest* r = toRest(e);
    if (r->ticks() < f) {
        return false;
    }
    if (r->ticks() == f) {
        this->pop_back();
        delete r;
    } else {
        r->setTicks(r->ticks() - f);
    }
    m_duration -= f;
    return true;
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void TrackList::read(const Segment* fs, const Segment* es)
{
    Fraction tick = fs->tick();
    m_clonedChord.clear();

    const Segment* s;
    for (s = fs; s && (s != es); s = s->next1()) {
        if (!s->enabled()) {
            continue;
        }
        EngravingItem* e = s->element(m_track);
        if (!e || e->generated()) {
            for (EngravingItem* ee : s->annotations()) {
                if (ee->systemFlag() && ee->track() != 0) {
                    // Only process the top system object
                    continue;
                }
                if (ee->track() == m_track) {
                    m_range->m_annotations.push_back({ s->tick(), ee->clone() });
                }
            }
            continue;
        }
        if (e->isMeasureRepeat()) {
            // TODO: copy previous measure contents?
            MeasureRepeat* rm = toMeasureRepeat(e);
            Rest* r = Factory::copyRest(*rm);
            //! TODO Perhaps there is a bug.
            //! Previously, the element changed its type (because there was a virtual method that returned the type).
            //! This code has been added for compatibility reasons to maintain the same behavior.
            r->hack_toRestType();
            r->reset();
            append(r);
            tick += r->ticks();
        } else if (e->isChordRest()) {
            DurationElement* de = toDurationElement(e);
            Fraction gap = s->tick() - tick;
            if (de->tuplet()) {
                Tuplet* t = de->topTuplet();
                s  = skipTuplet(t);            // continue with first chord/rest after tuplet
                de = t;
            }

            if (gap.isNotZero()) {
                appendGap(gap, e->score());
                tick += gap;
            }
            append(de);
            tick += de->ticks();
        } else if (e->isBarLine()) {
            BarLine* bl = toBarLine(e);
            if (bl->barLineType() != BarLineType::NORMAL) {
                append(e);
            }
        } else {
            append(e);
        }
    }
    Fraction gap = es->tick() - tick;
    if (gap.isNotZero()) {
        appendGap(gap, es->score());
    }

    // Clone and rebuild Spanners
    cloneAndRebuildSpanners(m_clonedChord);

    //
    // connect ties
    //
    size_t n = size();
    for (size_t i = 0; i < n; ++i) {
        EngravingItem* e = at(i);
        if (!e->isChord()) {
            continue;
        }
        Chord* chord = toChord(e);
        for (Note* n1 : chord->notes()) {
            Tie* tie = n1->tieForNonPartial();
            if (!tie) {
                continue;
            }
            for (size_t k = i + 1; k < n; ++k) {
                EngravingItem* ee = at(k);
                if (!ee->isChord()) {
                    continue;
                }
                Chord* c2 = toChord(ee);
                bool found = false;
                for (Note* n2 : c2->notes()) {
                    if (n1->pitch() == n2->pitch()) {
                        tie->setEndNote(n2);
                        n2->setTieBack(tie);
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    LOGD("Tied note not found");
                }
                break;
            }
        }
    }
    m_clonedChord.clear();
}

//---------------------------------------------------------
//   checkRest
//---------------------------------------------------------

static bool checkRest(Fraction& rest, Measure*& m, const Fraction& d)
{
    if (rest.isZero()) {
        if (m->nextMeasure()) {
            m  = m->nextMeasure();
            rest = m->ticks();
        } else {
            LOGW("premature end of measure list, rest %d/%d", d.numerator(), d.denominator());
            return false;
        }
    }
    return true;
}

//---------------------------------------------------------
//   writeTuplet
//    measure - current measure
//    rest    - available time in measure
//---------------------------------------------------------

Tuplet* TrackList::writeTuplet(Tuplet* parent, Tuplet* tuplet, Measure*& measure, Fraction& rest) const
{
    Score* score = measure->score();
    Tuplet* dt   = tuplet->clone();
    dt->setParent(measure);
    Fraction du  = tuplet->ticks();
    if (du > rest) {
        // we must split the tuplet
        dt->setTicks(du * Fraction(1, 2));
        dt->setBaseLen(tuplet->baseLen().shift(1));
    }
    if (parent) {
        parent->add(dt);
    }

    for (DurationElement* e : tuplet->elements()) {
        Fraction duration = e->globalTicks();
        Tuplet* tt        = dt;
        Fraction ratio    = Fraction(1, 1);
        while (tt) {
            ratio *= tt->ratio();
            tt = tt->tuplet();
        }

        bool firstpart = true;
        while (duration > Fraction(0, 1)) {
            if (rest.isZero()) {
                if (measure->nextMeasure()) {
                    measure = measure->nextMeasure();
                    rest    = measure->ticks();
                    if (e != tuplet->elements().back()) {
                        // create second part of split tuplet
                        dt = dt->clone();
                        dt->setGenerated(true);
                        dt->setParent(measure);
                        Tuplet* pt = dt;
                        while (parent) {
                            Tuplet* tt1 = parent->clone();
                            tt1->setGenerated(true);
                            tt1->setParent(measure);
                            tt1->add(pt);
                            pt = tt1;
                            parent = parent->tuplet();
                        }
                    }
                } else {
                    ASSERT_X(String(u"premature end of measure list in track %1, rest %2/%3")
                             .arg(m_track).arg(duration.numerator(), duration.denominator()));
                }
            }
            if (e->isChordRest()) {
                Fraction dd = std::min(rest, duration) * ratio;
                std::vector<TDuration> dl = toDurationList(dd, false);
                for (const TDuration& k : dl) {
                    Segment* segment = measure->undoGetSegmentR(SegmentType::ChordRest, measure->ticks() - rest);
                    Fraction gd      = k.fraction() / ratio;
                    ChordRest* cr    = toChordRest(e->clone());
                    if (!firstpart) {
                        cr->removeMarkings(true);
                    }
                    cr->setScore(score);
                    cr->setTrack(m_track);
                    segment->add(cr);
                    cr->setTicks(k.fraction());
                    cr->setDurationType(k);
                    rest     -= gd;
                    duration -= gd;

                    if (cr->isChord()) {
                        for (Note* note : toChord(cr)->notes()) {
                            if (!duration.isZero() && !note->tieFor()) {
                                Tie* tie = Factory::createTie(note);
                                tie->setGenerated(true);
                                note->add(tie);
                            }
                        }
                    }
                    dt->add(cr);
                    firstpart = false;
                }
            } else if (e->isTuplet()) {
                Tuplet* tt1 = toTuplet(e);
                Tuplet* ttt = writeTuplet(dt, tt1, measure, rest);
                dt          = ttt->tuplet();
                parent      = dt->tuplet();
                duration    = Fraction();
            }
            firstpart = false;
        }
    }
    return dt;
}

//---------------------------------------------------------
//   write
//    rewrite notes into measure list measure
//---------------------------------------------------------

bool TrackList::write(Score* score, const Fraction& tick) const
{
    if ((m_track % VOICES) && size() == 1 && at(0)->isRest()) {     // don’t write rests in voice > 0
        return true;
    }
    ClonedChordMap wClonedChord;
    Measure* measure = score->tick2measure(tick);
    Measure* m       = measure;
    Fraction remains = m->endTick() - tick;
    Segment* segment = 0;

    for (EngravingItem* e : *this) {
        if (e->isDurationElement()) {
            Fraction duration = toDurationElement(e)->ticks();
            if (!checkRest(remains, m, duration)) {     // go to next measure, if necessary
                MScore::setError(MsError::CORRUPTED_MEASURE);
                return false;
            }
            if (duration > remains && e->isTuplet()) {
                // experimental: allow tuplet split in the middle
                if (duration != remains * 2) {
                    MScore::setError(MsError::CANNOT_SPLIT_TUPLET);
                    return false;
                }
            }
            bool firstpart = true;
            while (duration > Fraction(0, 1)) {
                if ((e->isRest() || e->isMeasureRepeat()) && (duration >= remains || e == back()) && (remains == m->ticks())) {
                    //
                    // handle full measure rest
                    //
                    Segment* seg = m->getSegmentR(SegmentType::ChordRest, m->ticks() - remains);
                    if ((m_track % VOICES) == 0) {
                        // write only for voice 1
                        Rest* r = Factory::createRest(seg, DurationType::V_MEASURE);
                        // ideally we should be using stretchedLen
                        // but this is not valid during rewrite when adding time signatures
                        // since the time signature has not been added yet
                        //Fraction stretchedLen = m->stretchedLen(staff);
                        //r->setTicks(stretchedLen);
                        r->setTicks(m->ticks());
                        r->setTrack(m_track);
                        seg->add(r);
                    }
                    duration -= m->ticks();
                    remains.set(0, 1);
                } else if (e->isChordRest()) {
                    Fraction du               = std::min(remains, duration);
                    std::vector<TDuration> dl = toDurationList(du, e->isChord());
                    if (dl.empty()) {
                        MScore::setError(MsError::CORRUPTED_MEASURE);
                        return false;
                    }
                    for (const TDuration& k : dl) {
                        segment       = m->undoGetSegmentR(SegmentType::ChordRest, m->ticks() - remains);
                        ChordRest* cr = toChordRest(e->clone());
                        if (!firstpart) {
                            cr->removeMarkings(true);
                        }
                        cr->setTrack(m_track);
                        cr->setScore(score);
                        Fraction gd = k.fraction();
                        cr->setTicks(gd);
                        cr->setDurationType(k);

                        segment->add(cr);
                        duration -= gd;
                        remains  -= gd;

                        if (cr->isChord()) {
                            // Map between Chords and clonned chords
                            wClonedChord.insert(std::make_pair(toChord(e), toChord(cr)));

                            TremoloTwoChord* tremolo = toChord(cr)->tremoloTwoChord();
                            if (!firstpart && tremolo) {               // remove partial two-note tremolo
                                if (toChord(e)->tremoloTwoChord()->chord1() == toChord(e)) {
                                    tremolo->setChords(toChord(cr), nullptr);
                                } else {
                                    tremolo->setChords(nullptr, toChord(cr));
                                }
                                toChord(cr)->setTremoloTwoChord(nullptr);
                                delete tremolo;
                            }
                            for (Note* note : toChord(cr)->notes()) {
                                if (!duration.isZero() && !note->tieFor()) {
                                    Tie* tie = Factory::createTie(note);
                                    tie->setGenerated(true);
                                    note->add(tie);
                                }
                            }
                        }
                    }
                } else if (e->isTuplet()) {
                    writeTuplet(0, toTuplet(e), m, remains);
                    duration = Fraction();
                }
                firstpart = false;
                if (duration > Fraction(0, 1)) {
                    if (!checkRest(remains, m, duration)) {     // go to next measure, if necessary
                        MScore::setError(MsError::CORRUPTED_MEASURE);
                        return false;
                    }
                }
            }
        } else if (e->isBarLine()) {
        } else if (e->isClef()) {
            Segment* seg;
            if (remains == m->ticks() && m->tick() > Fraction(0, 1)) {
                Measure* pm = m->prevMeasure();
                seg = pm->getSegmentR(SegmentType::Clef, pm->ticks());
            } else if (remains != m->ticks()) {
                seg = m->getSegmentR(SegmentType::Clef, m->ticks() - remains);
            } else {
                seg = m->getSegmentR(SegmentType::HeaderClef, Fraction(0, 1));
            }
            EngravingItem* ne = e->clone();
            ne->setScore(score);
            ne->setTrack(m_track);
            seg->add(ne);
        } else {
            if (!m) {
                break;
            }
            // add the element in its own segment;
            // but KeySig has to be at start of (current) measure

            Segment* seg = m->getSegmentR(Segment::segmentType(e->type()), e->isKeySig() ? Fraction() : m->ticks() - remains);
            EngravingItem* ne = e->clone();
            ne->setScore(score);
            ne->setTrack(m_track);
            seg->add(ne);
        }
    }

    //
    // Rebuild Spanner's Start and End Notes
    //
    cloneAndRebuildSpanners(wClonedChord);

    //
    // connect ties from measure->first() to segment
    //

    for (Segment* s = measure->first(); s; s = s->next1()) {
        EngravingItem* e = s->element(m_track);
        if (!e || !e->isChord()) {
            continue;
        }
        Chord* chord = toChord(e);
        for (Note* n : chord->notes()) {
            Tie* tie = n->tieForNonPartial();
            if (!tie) {
                continue;
            }
            Note* nn = searchTieNote(n);
            if (nn) {
                tie->setEndNote(nn);
                nn->setTieBack(tie);
            } else {
                score->doUndoRemoveElement(tie);
            }
        }
        if (s == segment) {
            break;
        }
    }
    return true;
}

//---------------------------------------------------------
//   ScoreRange
//---------------------------------------------------------

ScoreRange::~ScoreRange()
{
    muse::DeleteAll(m_tracks);
    deleteBarLines();
    deleteSpacers();
    deleteJumpsAndMarkers();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void ScoreRange::read(Segment* first, Segment* last, bool readSpanner)
{
    m_first        = first;
    m_last         = last;
    Score* score  = first->score();
    std::vector<track_idx_t> sl = score->uniqueStaves();

    track_idx_t startTrack = 0;
    track_idx_t endTrack   = score->nstaves() * VOICES;

    m_spanner.clear();

    if (readSpanner) {
        Fraction stick = first->tick();
        Fraction etick = last->tick();
        for (auto i : first->score()->spanner()) {
            Spanner* s = i.second;
            if (s->systemFlag() && s->track() != 0) {
                // Only process the top system object
                continue;
            }

            if (s->tick() >= stick && s->tick() < etick && s->track() >= startTrack && s->track() < endTrack) {
                Spanner* ns = toSpanner(s->clone());
                ns->resetExplicitParent();
                ns->setStartElement(0);
                ns->setEndElement(0);
                ns->setTick(ns->tick() - stick);
                m_spanner.push_back(ns);
            }
        }
    }
    for (track_idx_t staffIdx : sl) {
        track_idx_t sTrack = staffIdx * VOICES;
        track_idx_t eTrack = sTrack + VOICES;
        for (track_idx_t track = sTrack; track < eTrack; ++track) {
            TrackList* dl = new TrackList(this);
            dl->setTrack(track);
            dl->read(first, last);
            m_tracks.push_back(dl);
        }
    }

    // Make sure incoming ties are restored
    Measure* m1 = first->measure();
    for (track_idx_t track = startTrack; track < endTrack; track++) {
        Chord* startChord = m1->findChord(first->tick(), track);
        if (!startChord) {
            continue;
        }
        for (Note* note : startChord->notes()) {
            Tie* tie = note->tieBack();
            if (!tie) {
                continue;
            }
            m_startTies.push_back(tie->clone());
            score->doUndoRemoveElement(tie);
        }
    }

    backupBarLines(first, last);
    backupBreaks(first, last);
    backupRepeats(first, last);
    backupSpacers(first, last);
    backupJumpsAndMarkers(first, last);
}

//---------------------------------------------------------
//   restoreVolta
//---------------------------------------------------------

void ScoreRange::restoreVolta(Score* score, const Fraction& tick, Volta* v) const
{
    bool voltaStartOK = false;
    bool voltaEndOK = false;
    Fraction measureSize;

    if (v->voltaType() == Volta::Type::OPEN) {
        voltaEndOK = true;
    }

    // Check if we should keep this volta
    for (Measure* m = score->tick2measure(tick); m; m = m->nextMeasure()) {
        if (m->tick() == (v->tick() + tick)) {
            voltaStartOK = true;
            measureSize = m->endTick() - m->tick();
        }
        if (m->endTick() == (v->tick2() + tick)) {
            voltaEndOK = true;
            // No need to continue looking for
            break;
        }
        // Last Measure
        if (m->sectionBreak() || (m->nextMeasure() && (m->nextMeasure()->first(SegmentType::TimeSig)))) {
            break;
        }
    }
    bool shouldKeepVolta = voltaStartOK && voltaEndOK;

    if (shouldKeepVolta) {
        // Volta start
        v->setTick(v->tick() + tick);

        // Review Volta endings with open Voltas
        if (v->voltaType() == Volta::Type::OPEN) {
            Fraction voltaFormerTicks = v->ticks();
            Fraction voltaNewTicks = voltaFormerTicks;

            if (voltaFormerTicks < measureSize) {
                voltaNewTicks = measureSize;
            } else if (voltaFormerTicks > measureSize) {
                if ((voltaFormerTicks / measureSize).reduced().denominator() != 1) {
                    voltaNewTicks = (std::floor((voltaFormerTicks / measureSize).toDouble()) + 1) * measureSize;
                }
            }
            if (voltaFormerTicks != voltaNewTicks) {
                v->setTicks(voltaNewTicks);
            }
        }
        score->undoAddElement(v);
    }
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

bool ScoreRange::write(Score* score, const Fraction& tick) const
{
    for (TrackList* dl : m_tracks) {
        track_idx_t track = dl->track();
        if (!dl->write(score, tick)) {
            return false;
        }
        if ((track % VOICES) == VOICES - 1) {
            // clone staff if appropriate after all voices have been copied
            staff_idx_t staffIdx = track / VOICES;
            Staff* ostaff = score->staff(staffIdx);
            const LinkedObjects* linkedStaves = ostaff->links();
            if (linkedStaves) {
                for (auto le : *linkedStaves) {
                    Staff* nstaff = toStaff(le);
                    if (nstaff == ostaff) {
                        continue;
                    }
                    Excerpt::cloneStaff2(ostaff, nstaff, tick, (tick + dl->ticks()));
                }
            }
        }
        ++track;
    }
    for (Spanner* s : m_spanner) {
        if (s->isVolta()) {
            restoreVolta(score, tick, toVolta(s));
            continue;
        }
        s->setTick(s->tick() + tick);
        if (s->isSlur()) {
            Slur* slur = toSlur(s);
            if (slur->startCR() && slur->startCR()->isGrace()) {
                Chord* sc = slur->startChord();
                size_t idx = sc->graceIndex();
                Chord* dc = toChord(score->findCR(s->tick(), s->track()));
                s->setStartElement(dc->graceNotes()[idx]);
            }
            if (slur->endCR() && slur->endCR()->isGrace()) {
                Chord* sc = slur->endChord();
                size_t idx = sc->graceIndex();
                Chord* dc = toChord(score->findCR(s->tick2(), s->track2()));
                s->setEndElement(dc->graceNotes()[idx]);
            }
        }
        if (s->anchor() == Spanner::Anchor::MEASURE) {
            Fraction startTick = s->tick();
            Measure* startMeasure = score->tick2measureMM(startTick);
            Fraction startMeasureTick = startMeasure->tick();
            Fraction endTick = s->tick2();
            Measure* endMeasure = score->tick2measureMM(endTick);
            Fraction endMeasureTick = endMeasure->tick();
            if (startMeasureTick != startTick) {
                s->setTick(startMeasureTick);
            }
            if (endMeasureTick != endTick) {
                s->setTick2(endMeasureTick != startMeasureTick ? endMeasureTick : endMeasure->endTick());
            }
        }
        score->undoAddElement(s);
    }
    for (const Annotation& a : m_annotations) {
        Measure* tm = score->tick2measure(a.tick);
        Segment* op = toSegment(a.e->explicitParent());
        Fraction destTick = a.e->isRehearsalMark() ? tm->tick() : a.tick; // Ensure reharsal mark can only go at measure start
        Segment* s = tm->undoGetSegment(op->segmentType(), destTick);
        if (s) {
            a.e->setParent(s);
            score->undoAddElement(a.e);
        }
    }

    // Restore ties to the beginning of the first measure
    for (Tie* tie : m_startTies) {
        Note* endNote = searchTieNote(tie->startNote());
        if (!endNote) {
            delete tie;
            continue;
        }
        tie->setEndNote(endNote);
        score->doUndoAddElement(tie);
    }

    restoreBarLines(score, tick);
    restoreBreaks(score, tick);
    restoreRepeats(score, tick);
    restoreSpacers(score, tick);
    restoreJumpsAndMarkers(score, tick);
    return true;
}

//---------------------------------------------------------
//   fill
//---------------------------------------------------------

void ScoreRange::fill(const Fraction& f)
{
    const Fraction oldDuration = ticks();
    Fraction oldEndTick = m_first->tick() + oldDuration;
    for (auto t : m_tracks) {
        t->appendGap(f, m_first->score());
    }

    Fraction diff = ticks() - oldDuration;
    for (Spanner* sp : m_spanner) {
        if (sp->tick2() >= oldEndTick && sp->tick() < oldEndTick) {
            sp->setTicks(sp->ticks() + diff);
        }
    }
}

//---------------------------------------------------------
//   truncate
//    reduce len of last gap by f
//---------------------------------------------------------

bool ScoreRange::truncate(const Fraction& f)
{
    for (TrackList* dl : m_tracks) {
        if (dl->empty()) {
            continue;
        }
        EngravingItem* e = dl->back();
        if (!e->isRest()) {
            return false;
        }
        Rest* r = toRest(e);
        if (r->ticks() < f) {
            return false;
        }
    }
    for (TrackList* dl : m_tracks) {
        dl->truncate(f);
    }
    return true;
}

//---------------------------------------------------------
//   ticks
//---------------------------------------------------------

Fraction ScoreRange::ticks() const
{
    return m_tracks.empty() ? Fraction() : m_tracks.front()->ticks();
}

//---------------------------------------------------------
//   backupBarLines
//---------------------------------------------------------
void ScoreRange::backupBarLines(Segment* first, Segment* last)
{
    for (Segment* s = first; s && (s->isBefore(last) || s == last); s = s->next1()) {
        if (!s->isType(SegmentType::BarLineType)) {
            continue;
        }
        for (EngravingItem* e : s->elist()) {
            if (e && !e->generated()) {
                BarLinesBackup blBackup;
                blBackup.sPosition = s->tick();
                blBackup.formerMeasureStartOrEnd = s->rtick().isZero() || s->rtick() == s->measure()->ticks();
                blBackup.bl = toBarLine(e)->clone();
                m_barLines.push_back(blBackup);
            }
        }
    }
}

//---------------------------------------------------------
//   insertBarLine
//---------------------------------------------------------
bool ScoreRange::insertBarLine(Measure* m, const BarLinesBackup& barLine) const
{
    //---------------------------------------------------------
    //   addBarLine
    //---------------------------------------------------------
    auto addBarLine = [&](Measure* m, BarLine* bl, SegmentType st, Fraction pos)
    {
        bool middle = (pos != m->tick()) && (pos != m->endTick());
        Segment* seg = m->undoGetSegment(st, pos);
        if (seg) {
            BarLineType blt = bl->barLineType();
            // get existing bar line if it does exist
            BarLine* nbl = toBarLine(seg->element(bl->track()));
            if (!nbl) {
                // no suitable bar line: create a new one
                nbl = Factory::createBarLine(seg);
                nbl->setParent(seg);
                nbl->setTrack(bl->track());
                // A BL in the middle of a Measure does have SpanStaff to false
                nbl->setSpanStaff(middle ? false : bl->spanStaff());
                m->score()->addElement(nbl);
            } else {
                // We change BarLineType if necessary to keep END_START repeats if in the middle of a meassure
                if ((nbl->barLineType() == BarLineType::END_REPEAT) && (bl->barLineType() == BarLineType::START_REPEAT) && middle) {
                    blt = BarLineType::END_START_REPEAT;
                }
            }
            nbl->setGenerated(false);
            nbl->setBarLineType(blt);
            nbl->setVisible(bl->visible());
            nbl->setColor(bl->color());

            // We check if the same BL type is in every Stave
            bool blAcrossStaves = false;
            // if there is only one stave or spanStaff
            if ((m->score()->nstaves() == 1) || bl->spanStaff()) {
                blAcrossStaves = true;
            }
            // If we are in the last stave
            else if (bl->staffIdx() == (m->score()->nstaves() - 1)) {
                bool sameBL = true;
                // We check if and every previous stave has the same type of BL
                for (size_t i = 0; i < (m->score()->nstaves() - 1); ++i) {
                    BarLine* sbl = toBarLine(seg->element(staff2track(i)));
                    if (!sbl || (sbl->barLineType() != blt)) {
                        sameBL = false;
                        break;
                    }
                }
                blAcrossStaves = sameBL;

                if (blAcrossStaves && !middle) {
                    // Set Spanstaff to true if not in the middle and there is the same BL across staves
                    for (size_t i = 0; i < (m->score()->nstaves() - 1); ++i) {
                        BarLine* sbl = toBarLine(seg->element(staff2track(i)));
                        if (sbl) {
                            sbl->setSpanStaff(middle ? false : true);
                        }
                    }
                }
            }

            // Adding Set repeats
            if ((pos == m->tick()) && (bl->barLineType() == BarLineType::START_REPEAT) && blAcrossStaves) {
                m->setRepeatStart(true);
            } else if ((pos == m->endTick()) && (bl->barLineType() == BarLineType::END_REPEAT) && blAcrossStaves) {
                m->setRepeatEnd(true);
            }
        }
    };

    bool processed = false;

    // if END_START_REPEAT AND at the end of a Measure
    if ((barLine.sPosition == m->endTick()) && (barLine.bl->barLineType() == BarLineType::END_START_REPEAT)) {
        // Create END_REPEAT and START_REPEAT (and ignore return value)
        barLine.bl->setBarLineType(BarLineType::END_REPEAT);
        insertBarLine(m, barLine);
        // Start Repeat into the next measure
        if (m->nextMeasure()) {
            barLine.bl->setBarLineType(BarLineType::START_REPEAT);
            insertBarLine(m->nextMeasure(), barLine);
        }

        // Restore initial value just in case
        barLine.bl->setBarLineType(BarLineType::END_START_REPEAT);
        processed = true;
    } else {
        // First position
        if (barLine.sPosition == m->tick()) {
            // Just Start Repeat at the left of the Measure
            if (barLine.bl->barLineType() == BarLineType::START_REPEAT) {
                addBarLine(m, barLine.bl, SegmentType::StartRepeatBarLine, barLine.sPosition);
                processed = true;
            }
        }
        // Last position
        else if (barLine.sPosition == m->endTick()) {
            // Avoid Start Repeat at the end of the Measure
            if (barLine.bl->barLineType() != BarLineType::START_REPEAT) {
                addBarLine(m, barLine.bl, SegmentType::EndBarLine, barLine.sPosition);
                processed = true;
            }
        }
        // Middle
        else {
            // We don't create BL in the Middle
            processed = true;
        }
    }
    return processed;
}

//---------------------------------------------------------
//   restoreBarLines
//---------------------------------------------------------

void ScoreRange::restoreBarLines(Score* score, const Fraction& tick) const
{
    for (const BarLinesBackup& bbl : m_barLines) {
        for (Measure* m = score->tick2measure(tick); m; m = m->nextMeasure()) {
            // if inserted within a suitable measure ... to the next barline
            if (((bbl.sPosition >= m->tick()) && (bbl.sPosition <= m->endTick())) && (insertBarLine(m, bbl))) {
                break;
            }
            if (m->tick() > bbl.sPosition) {
                break;
            }
        }
    }
}

//---------------------------------------------------------
//   backupBreaks
//---------------------------------------------------------
void ScoreRange::backupBreaks(Segment* first, Segment* last)
{
    Measure* fm = first->measure();
    Measure* lm = last->measure();
    for (Measure* m = fm; m && m != lm->nextMeasure(); m = m->nextMeasure()) {
        if (m->lineBreak()) {
            BreaksBackup bBackup;
            bBackup.sPosition = m->endTick();
            bBackup.lBreakType = LayoutBreakType::LINE;
            m_breaks.push_back(bBackup);
        } else if (m->pageBreak()) {
            BreaksBackup bBackup;
            bBackup.sPosition = m->endTick();
            bBackup.lBreakType = LayoutBreakType::PAGE;
            m_breaks.push_back(bBackup);
        }
    }
}

//---------------------------------------------------------
//   restoreBreaks
//---------------------------------------------------------

void ScoreRange::restoreBreaks(Score* score, const Fraction& tick) const
{
    // Break list
    for (const BreaksBackup& bb : m_breaks) {
        // Look for suitable measure
        for (Measure* m = score->tick2measure(tick); m; m = m->nextMeasure()) {
            // We keep them as long as they are in the measure after the start tick
            if ((bb.sPosition > m->tick()) && (bb.sPosition <= m->endTick())) {
                m->undoSetBreak(true, bb.lBreakType);
                break;
            }
            if (m->tick() > bb.sPosition) {
                break;
            }
        }
    }
}

void ScoreRange::backupRepeats(Segment* first, Segment* last)
{
    Measure* fm = first->measure();
    Measure* lm = last->measure();
    for (Measure* m = fm; m && m != lm->nextMeasure(); m = m->nextMeasure()) {
        if (m->repeatStart()) {
            StartEndRepeatBackup repeatBackup;
            repeatBackup.sPosition = m->tick();
            repeatBackup.isStartRepeat = true;
            m_startEndRepeats.push_back(repeatBackup);
        }
        if (m->repeatEnd()) {
            StartEndRepeatBackup repeatBackup;
            repeatBackup.sPosition = m->endTick();
            repeatBackup.isStartRepeat = false;
            m_startEndRepeats.push_back(repeatBackup);
        }
    }
}

void ScoreRange::restoreRepeats(Score* score, const Fraction& tick) const
{
    Fraction refTick = tick.isZero() ? tick : tick - Fraction::eps(); // start checking one measure before
    for (const StartEndRepeatBackup& startEndRepeat : m_startEndRepeats) {
        for (Measure* m = score->tick2measure(refTick); m; m = m->nextMeasure()) {
            Fraction mTick = m->tick();
            if (startEndRepeat.isStartRepeat && startEndRepeat.sPosition == mTick) {
                m->undoChangeProperty(Pid::REPEAT_START, true);
            } else if (!startEndRepeat.isStartRepeat && startEndRepeat.sPosition == m->endTick()) {
                m->undoChangeProperty(Pid::REPEAT_END, true);
            }
            if (mTick > startEndRepeat.sPosition) {
                break;
            }
        }
    }
}

//---------------------------------------------------------
//   deleteBarLines
//---------------------------------------------------------

void ScoreRange::deleteBarLines()
{
    for (const BarLinesBackup& bbl : m_barLines) {
        delete bbl.bl;
    }
    m_barLines.clear();
}

//---------------------------------------------------------
//   backupSpacers
//---------------------------------------------------------

void ScoreRange::backupSpacers(Segment* first, Segment* last)
{
    Measure* fm = first->measure();
    Measure* lm = last->measure();

    for (Measure* m = fm; m && m != lm->nextMeasure(); m = m->nextMeasure()) {
        staff_idx_t nStaves = m->score()->nstaves();

        for (staff_idx_t i = 0; i < nStaves; ++i) {
            if (m->vspacerUp(i)) {
                SpacerBackup sb;
                sb.s = m->vspacerUp(i)->clone();
                sb.sPosition = m->tick() + ((m->endTick() - m->tick()) / 2);
                sb.staffIdx = i;
                m_spacers.push_back(sb);
            }
            if (m->vspacerDown(i)) {
                SpacerBackup sb;
                sb.s = m->vspacerDown(i)->clone();
                sb.sPosition = m->tick() + ((m->endTick() - m->tick()) / 2);
                sb.staffIdx = i;
                m_spacers.push_back(sb);
            }
        }
    }
}

//---------------------------------------------------------
//   restoreSpacers
//---------------------------------------------------------
void ScoreRange::restoreSpacers(Score* score, const Fraction& tick) const
{
    //---------------------------------------------------------
    //   addSpacer
    //---------------------------------------------------------
    auto addSpacer = [&](Measure* m, Spacer* s, staff_idx_t staffIdx)
    {
        // We only add an element if there isn't a previous one of the same Type (UP/DOWN)
        if ((s->spacerType() == SpacerType::UP && !m->vspacerUp(staffIdx))
            || ((s->spacerType() == SpacerType::DOWN || s->spacerType() == SpacerType::FIXED) && !m->vspacerDown(staffIdx))) {
            Spacer* ns = Factory::createSpacer(m);
            ns->setSpacerType(s->spacerType());
            ns->setGap(s->gap());
            ns->setTrack(staffIdx * VOICES);
            m->add(ns);
        }
    };

    // Spacers list
    for (const SpacerBackup& sb : m_spacers) {
        // Look for suitable measure
        for (Measure* m = score->tick2measure(tick); m; m = m->nextMeasure()) {
            if (sb.sPosition >= m->tick() && sb.sPosition <= m->endTick()) {
                addSpacer(m, sb.s, sb.staffIdx);
                break;
            }
            if (m->tick() > sb.sPosition) {
                break;
            }
        }
    }
}

//---------------------------------------------------------
//   deleteSpacers
//---------------------------------------------------------
void ScoreRange::deleteSpacers()
{
    // Spacers list
    for (const SpacerBackup& sb : m_spacers) {
        delete sb.s;
    }
    m_spacers.clear();
}

//---------------------------------------------------------
//   endOfMeasureElement
//---------------------------------------------------------

bool ScoreRange::endOfMeasureElement(EngravingItem* e) const
{
    bool result = false;

    if (e->isMarker()
        && ((muse::contains(Marker::RIGHT_MARKERS, toMarker(e)->markerType()) || toMarker(e)->markerType() == MarkerType::FINE))) {
        result = true;
    } else if (e->isJump()) {
        result = true;
    }
    return result;
}

//---------------------------------------------------------
//   backupJumpsAndMarkers
//---------------------------------------------------------

void ScoreRange::backupJumpsAndMarkers(Segment* first, Segment* last)
{
    Measure* fm = first->measure();
    Measure* lm = last->measure();

    for (Measure* m = fm; m && m != lm->nextMeasure(); m = m->nextMeasure()) {
        // Backup Markers and Jumps (Measures's son)
        for (EngravingItem* e : m->el()) {
            if (e && (e->isMarker() || e->isJump())) {
                JumpsMarkersBackup mJMBackup;
                mJMBackup.sPosition = (endOfMeasureElement(e) ? m->endTick() : m->tick());
                mJMBackup.e = e->clone();
                m_jumpsMarkers.push_back(mJMBackup);
            }
        }
    }
}

//---------------------------------------------------------
//   restoreJumpsAndMarkers
//---------------------------------------------------------
void ScoreRange::restoreJumpsAndMarkers(Score* score, const Fraction& tick) const
{
    //---------------------------------------------------------
    //   addJumpMarker
    //---------------------------------------------------------
    auto addJumpMarker = [&](Measure* m, EngravingItem* e)
    {
        // We add every element as a Measure could have as many elements (even of the same type) as the users decides
        EngravingItem* ce = e->clone();
        ce->setParent(m);
        ce->setTrack(0);
        m->add(ce);
    };

    for (const JumpsMarkersBackup& jmb : m_jumpsMarkers) {
        for (Measure* m = score->tick2measure(tick); m; m = m->nextMeasure()) {
            // Markers: we keep them as long as they are in the measure before the final tick
            // Jumps: we keep them as long as they are in the measure after the start tick
            if ((endOfMeasureElement(jmb.e) && jmb.sPosition > m->tick() && jmb.sPosition <= m->endTick())
                || (!endOfMeasureElement(jmb.e) && jmb.sPosition >= m->tick() && jmb.sPosition < m->endTick())) {
                addJumpMarker(m, jmb.e);
                LOGI() << "tpacebes hago restore";
            }
            if (m->tick() > jmb.sPosition) {
                break;
            }
        }
    }
}

//---------------------------------------------------------
//   deleteJumpsAndMarkers
//---------------------------------------------------------

void ScoreRange::deleteJumpsAndMarkers()
{
    for (const JumpsMarkersBackup& jmb : m_jumpsMarkers) {
        delete jmb.e;
    }
    m_jumpsMarkers.clear();
}

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void TrackList::dump() const
{
    LOGD("elements %zu, duration %d/%d", size(), m_duration.numerator(), m_duration.denominator());
    for (EngravingItem* e : *this) {
        if (e->isDurationElement()) {
            Fraction du = toDurationElement(e)->ticks();
            LOGD("   %s  %d/%d", e->typeName(), du.numerator(), du.denominator());
        } else {
            LOGD("   %s", e->typeName());
        }
    }
}
}
