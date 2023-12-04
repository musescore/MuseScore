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
#include "tremolo.h"
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

//---------------------------------------------------------
//   TrackList
//---------------------------------------------------------

TrackList::~TrackList()
{
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
            if (!n->tieBack() || !n->tieBack()->generated()) {
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

void TrackList::append(EngravingItem* e, std::unordered_map<EngravingItem*, std::vector<Spanner*> >& notFinishedSpanners)
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
                    bool akkumulateChord = true;
                    for (Note* n : chord->notes()) {
                        if (!n->tieBack() || !n->tieBack()->generated()) {
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

                    cloneChordSpanners(toChord(e), toChord(element), notFinishedSpanners, {});
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
    std::unordered_map<EngravingItem*, std::vector<Spanner*> > notFinishedSpanners;

    const Segment* s;
    for (s = fs; s && (s != es); s = s->next1()) {
        if (!s->enabled()) {
            continue;
        }
        EngravingItem* e = s->element(m_track);
        //! NOTE: some barlines are generated, they are important to us
        if (!e || (e->generated() && !e->isBarLine())) {
            for (EngravingItem* ee : s->annotations()) {
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
            append(r, notFinishedSpanners);
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
            append(de, notFinishedSpanners);
            tick += de->ticks();
        } else if (e->isBarLine()) {
            BarLine* bl = toBarLine(e);
            if (bl->barLineType() != BarLineType::NORMAL) {
                append(e, notFinishedSpanners);
            }
        } else {
            append(e, notFinishedSpanners);
        }
    }
    Fraction gap = es->tick() - tick;
    if (gap.isNotZero()) {
        appendGap(gap, es->score());
    }

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
            Tie* tie = n1->tieFor();
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
    Measure* measure = score->tick2measure(tick);
    Measure* m       = measure;
    Fraction remains = m->endTick() - tick;
    Segment* segment = 0;

    std::unordered_map<EngravingItem*, std::vector<Spanner*> > notFinishedSpanners;

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
                            if (!firstpart && toChord(cr)->tremolo() && toChord(cr)->tremolo()->twoNotes()) {               // remove partial two-note tremolo
                                if (toChord(e)->tremolo()->chord1() == toChord(e)) {
                                    toChord(cr)->tremolo()->setChords(toChord(cr), nullptr);
                                } else {
                                    toChord(cr)->tremolo()->setChords(nullptr, toChord(cr));
                                }
                                Tremolo* tremoloPointer = toChord(cr)->tremolo();
                                toChord(cr)->setTremolo(nullptr);
                                delete tremoloPointer;
                            }

                            std::vector<Note*> ignoredNotes;
                            for (Note* note : toChord(cr)->notes()) {
                                if (!duration.isZero() && !note->tieFor()) {
                                    Tie* tie = Factory::createTie(note);
                                    tie->setGenerated(true);
                                    note->add(tie);

                                    ignoredNotes.emplace_back(note);
                                }
                            }

                            cloneChordSpanners(toChord(e), toChord(cr), notFinishedSpanners, ignoredNotes);
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
            BarLine* bl = toBarLine(e);
            Segment* seg;

            if (bl->barLineType() == BarLineType::START_REPEAT) {
                Measure* next = m->nextMeasure();
                if (next) {
                    next->setProperty(Pid::REPEAT_START, true);
                }
            } else if (bl->barLineType() == BarLineType::END_REPEAT) {
                m->setProperty(Pid::REPEAT_END, true);
            } else if (bl->barLineType() == BarLineType::END_START_REPEAT) {
                m->setProperty(Pid::REPEAT_END, true);
                Measure* next = m->nextMeasure();
                if (next) {
                    next->setProperty(Pid::REPEAT_START, true);
                }
            } else {
                seg = m->getSegmentR(SegmentType::EndBarLine, m->ticks());
                if (seg) {
                    EngravingItem* ne = e->clone();
                    ne->setScore(score);
                    ne->setTrack(m_track);
                    seg->add(ne);
                }
            }
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
    // connect ties from measure->first() to segment
    //

    for (Segment* s = measure->first(); s; s = s->next1()) {
        EngravingItem* e = s->element(m_track);
        if (!e || !e->isChord()) {
            continue;
        }
        Chord* chord = toChord(e);
        for (Note* n : chord->notes()) {
            Tie* tie = n->tieFor();
            if (!tie) {
                continue;
            }
            Note* nn = searchTieNote(n);
            if (nn) {
                tie->setEndNote(nn);
                nn->setTieBack(tie);

                if (!nn->spannerBack().empty()) {
                    for (Spanner* spanner : nn->spannerBack()) {
                        if (!spanner->isGuitarBend()) {
                            continue;
                        }

                        n->chord()->setEndsGlissandoOrGuitarBend(true);
                        spanner->setEndElement(n);
                    }
                }
            }
        }
        if (s == segment) {
            break;
        }
    }
    return true;
}

void TrackList::cloneChordSpanners(Chord* fromChord, Chord* toChord,
                                   std::unordered_map<EngravingItem*, std::vector<Spanner*> >& notFinishedSpanners,
                                   const std::vector<Note*>& ignoredNotes) const
{
    std::vector<Note*> fromNotes;
    std::vector<Note*> toNotes;

    GraceNotesGroup& fromChordGraceNotesBefore = fromChord->graceNotesBefore();
    if (!fromChordGraceNotesBefore.empty()) {
        for (const Chord* graceNotesChord : fromChordGraceNotesBefore) {
            mu::join(fromNotes, graceNotesChord->notes());
        }
    }

    GraceNotesGroup& toChordGraceNotesBefore = toChord->graceNotesBefore();
    if (!toChordGraceNotesBefore.empty()) {
        for (const Chord* graceNotesChord : toChordGraceNotesBefore) {
            mu::join(toNotes, graceNotesChord->notes());
        }
    }

    cloneNotesSpanners(fromNotes, toNotes, notFinishedSpanners, ignoredNotes);

    fromNotes.clear();
    toNotes.clear();

    for (Note* note : fromChord->notes()) {
        fromNotes.emplace_back(note);
    }

    for (Note* note : toChord->notes()) {
        toNotes.emplace_back(note);
    }
    cloneNotesSpanners(fromNotes, toNotes, notFinishedSpanners, ignoredNotes);

    fromNotes.clear();
    toNotes.clear();

    GraceNotesGroup& fromChordGraceNotesAfter = fromChord->graceNotesAfter();
    if (!fromChordGraceNotesAfter.empty()) {
        for (const Chord* graceNotesChord : fromChordGraceNotesAfter) {
            mu::join(fromNotes, graceNotesChord->notes());
        }
    }
    GraceNotesGroup& toChordGraceNotesAfter = toChord->graceNotesAfter();
    if (!toChordGraceNotesAfter.empty()) {
        for (const Chord* graceNotesChord : toChordGraceNotesAfter) {
            mu::join(toNotes, graceNotesChord->notes());
        }
    }

    cloneNotesSpanners(fromNotes, toNotes, notFinishedSpanners, ignoredNotes);
}

void TrackList::cloneNotesSpanners(std::vector<Note*>& fromNotes, const std::vector<Note*>& toNotes,
                                   std::unordered_map<EngravingItem*, std::vector<Spanner*> >& notFinishedSpanners,
                                   const std::vector<Note*>& ignoredNotes) const
{
    if (fromNotes.empty() || toNotes.empty()) {
        return;
    }

    for (size_t i = 0; i < fromNotes.size(); ++i) {
        Note* note = fromNotes[i];
        if (i > toNotes.size() - 1) {
            return;
        }
        Note* newNote = toNotes[i];

        if (contains(ignoredNotes, newNote)) {
            continue;
        }

        std::vector<Spanner*> notFinishedSpannersForNote = value(notFinishedSpanners, note);

        if (!notFinishedSpannersForNote.empty()) {
            for (Spanner* spanner : notFinishedSpannersForNote) {
                spanner->setEndElement(newNote);
                if (spanner->isGlissando() || spanner->isGuitarBend()) {
                    newNote->chord()->setEndsGlissandoOrGuitarBend(true);
                }
                newNote->addSpannerBack(spanner);
            }

            remove(notFinishedSpanners, note);
        }

        const std::vector<Spanner*> spannerFor = note->spannerFor();
        if (!spannerFor.empty()) {
            for (Spanner* spanner : spannerFor) {
                Spanner* newSpanner = toSpanner(spanner->clone());
                newSpanner->setParent(newNote);
                newSpanner->setStartElement(newNote);

                newNote->addSpannerFor(newSpanner);

                if (spanner->endElement() == note) {
                    newSpanner->setEndElement(newNote);
                    if (spanner->isGlissando() || spanner->isGuitarBend()) {
                        newNote->chord()->setEndsGlissandoOrGuitarBend(true);
                    }
                    newNote->addSpannerBack(newSpanner);
                } else {
                    notFinishedSpanners[spanner->endElement()].emplace_back(newSpanner);
                }
            }
        }
    }
}

std::vector<Note*> TrackList::chordNotes(Chord* chord) const
{
    std::vector<Note*> notes;

    GraceNotesGroup& graceNotesBefore = chord->graceNotesBefore();

    if (!graceNotesBefore.empty()) {
        for (const Chord* graceNotesChord : graceNotesBefore) {
            mu::join(notes, graceNotesChord->notes());
        }
    }

    for (Note* note : chord->notes()) {
        if (note->tieFor()) {
            notes.emplace_back(note->tiedNotes().back());
        } else {
            notes.emplace_back(note);
        }
    }

    GraceNotesGroup& graceNotesAfter = chord->graceNotesAfter();
    if (!graceNotesAfter.empty()) {
        for (const Chord* graceNotesChord : graceNotesAfter) {
            mu::join(notes, graceNotesChord->notes());
        }
    }

    return notes;
}

//---------------------------------------------------------
//   ScoreRange
//---------------------------------------------------------

ScoreRange::~ScoreRange()
{
    DeleteAll(m_tracks);
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void ScoreRange::read(Segment* first, Segment* last, bool readSpanner)
{
    m_first        = first;
    m_last         = last;
    Score* score  = first->score();
    std::list<track_idx_t> sl = score->uniqueStaves();

    track_idx_t startTrack = 0;
    track_idx_t endTrack   = score->nstaves() * VOICES;

    m_spanner.clear();

    if (readSpanner) {
        Fraction stick = first->tick();
        Fraction etick = last->tick();
        for (auto i : first->score()->spanner()) {
            Spanner* s = i.second;
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
        s->setTick(s->tick() + tick);
        if (s->isSlur()) {
            Slur* slur = toSlur(s);
            if (slur->startCR() && slur->startCR()->isGrace()) {
                Chord* sc = slur->startChord();
                size_t idx = sc->graceIndex();
                Chord* dc = toChord(score->findCR(s->tick(), s->track()));
                s->setStartElement(dc->graceNotes()[idx]);
            } else {
                s->setStartElement(0);
            }
            if (slur->endCR() && slur->endCR()->isGrace()) {
                Chord* sc = slur->endChord();
                size_t idx = sc->graceIndex();
                Chord* dc = toChord(score->findCR(s->tick2(), s->track2()));
                s->setEndElement(dc->graceNotes()[idx]);
            } else {
                s->setEndElement(0);
            }
        }
        score->undoAddElement(s);
    }
    for (const Annotation& a : m_annotations) {
        Measure* tm = score->tick2measure(a.tick);
        Segment* op = toSegment(a.e->explicitParent());
        Segment* s = tm->undoGetSegment(op->segmentType(), a.tick);
        if (s) {
            a.e->setParent(s);
            score->undoAddElement(a.e);
        }
    }
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
