/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

// Overfull-measure resolution, in a post-pass so a tuplet is always handled atomically rather than
// left partial. The three strategies are described in ENCORE_IMPORTER.md §4.5.

#include "emitters-internal.h"

#include "engraving/dom/chord.h"
#include "engraving/dom/durationtype.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/score.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/spanner.h"
#include "engraving/dom/tie.h"
#include "engraving/dom/tuplet.h"

#include <vector>

using namespace mu::engraving;

namespace mu::iex::enc {
static void addFillRest(Measure* measure, track_idx_t tr, const Fraction& startTick, const Fraction& totalLen);

// Largest representable note value (with up to maxDots dots) whose fraction is <= f.
// Returns 0/1 if nothing fits.
static Fraction largestDottedLE(const Fraction& f, int maxDots)
{
    if (f.numerator() <= 0) {
        return Fraction(0, 1);
    }
    TDuration d(f, true /*truncate*/, maxDots);
    if (!d.isValid()) {
        return Fraction(0, 1);
    }
    return d.fraction();
}

// Dissolve a tuplet: detach every member so it reverts to its plain face value, then
// remove the (now empty) tuplet from its parent and delete it. A tuplet is atomic, so it
// is dissolved whole rather than leaving a partial tuplet behind.
void dissolveTuplet(Tuplet* t)
{
    if (!t) {
        return;
    }
    std::vector<DurationElement*> members(t->elements().begin(), t->elements().end());
    for (DurationElement* de : members) {
        if (de->isTuplet()) {
            dissolveTuplet(toTuplet(de));
        } else {
            de->setTuplet(nullptr);
        }
    }
    EngravingObject* parent = t->parent();
    if (parent && parent->isEngravingItem()) {
        toEngravingItem(parent)->remove(t);
    }
    delete t;
}

// Segment::remove() would undo the spanner's endpoints into a dangling state that crashes layout, so
// release them first. Ties live on notes and are unaffected.
void detachSpannersAt(ChordRest* cr)
{
    Score* score = cr->score();
    std::vector<Spanner*> toRemove;
    for (const auto& [tick, s] : score->spanner()) {
        if (s->startElement() == cr || s->endElement() == cr) {
            toRemove.push_back(s);
        }
    }
    for (Spanner* s : toRemove) {
        score->removeSpanner(s);
        delete s;
    }
    // Ties are not in the spanner map: they hang off the notes. A tie left pointing at a note that
    // is about to be freed is followed later by the pitch spelling pass, which walks tied notes and
    // dies on it, so both directions go with the chord.
    if (!cr->isChord()) {
        return;
    }
    for (Note* n : toChord(cr)->notes()) {
        if (Tie* t = n->tieFor()) {
            if (Note* end = t->endNote()) {
                end->setTieBack(nullptr);
            }
            n->setTieFor(nullptr);
            delete t;
        }
        if (Tie* t = n->tieBack()) {
            if (Note* start = t->startNote()) {
                start->setTieFor(nullptr);
            }
            n->setTieBack(nullptr);
            delete t;
        }
    }
}

// Collect the ordered ChordRests of one track in a measure and their total actual ticks.
Fraction collectVoice(Measure* measure, track_idx_t tr, std::vector<ChordRest*>& out)
{
    out.clear();
    Fraction sum(0, 1);
    for (Segment* seg = measure->first(SegmentType::ChordRest); seg; seg = seg->next(SegmentType::ChordRest)) {
        EngravingItem* el = seg->element(tr);
        if (!el || !el->isChordRest()) {
            continue;
        }
        ChordRest* cr = toChordRest(el);
        out.push_back(cr);
        sum += cr->actualTicks();
    }
    return sum;
}

// Recut a trailing element to end at the barline, keeping its notes: the room is written as a chain
// of tied figures, not one smaller figure, so the full sounding value survives.
static void recutCrossingCR(Measure* measure, track_idx_t tr, ChordRest* cr, const Fraction& room)
{
    const Fraction measTick = measure->tick();
    const Fraction startOff = cr->tick() - measTick;
    const bool isChord = cr->isChord();

    // Snapshot the chord's pitches/spellings so the tied chain reproduces them.
    struct NoteSpec {
        int pitch;
        int tpc1;
        int tpc2;
    };
    std::vector<NoteSpec> notes;
    if (isChord) {
        for (Note* n : toChord(cr)->notes()) {
            notes.push_back({ n->pitch(), n->tpc1(), n->tpc2() });
        }
    }
    detachSpannersAt(cr);
    cr->segment()->remove(cr);
    delete cr;

    Fraction pos = startOff;
    Chord* prevChord = nullptr;
    for (const TDuration& d : toDurationList(room, true /*useDots*/, 3 /*maxDots*/, false)) {
        if (!d.isValid()) {
            break;
        }
        Segment* seg = measure->getSegment(SegmentType::ChordRest, measTick + pos);
        if (isChord) {
            Chord* c = Factory::createChord(seg);
            c->setTrack(tr);
            c->setDurationType(d);
            c->setTicks(d.fraction());
            c->setDots(d.dots());
            seg->add(c);
            for (size_t i = 0; i < notes.size(); ++i) {
                Note* n = Factory::createNote(c);
                n->setTrack(tr);
                n->setPitch(notes[i].pitch, notes[i].tpc1, notes[i].tpc2);
                c->add(n);
                if (prevChord && i < prevChord->notes().size()) {
                    Note* prev = prevChord->notes()[i];
                    Tie* tie = Factory::createTie(prev);
                    tie->setStartNote(prev);
                    tie->setEndNote(n);
                    tie->setTrack(tr);
                    prev->add(tie);
                }
            }
            prevChord = c;
        } else {
            Rest* r = Factory::createRest(seg, d);
            r->setTicks(d.fraction());
            r->setTrack(tr);
            r->setGap(false);
            seg->add(r);
        }
        pos += d.fraction();
    }
}

// Truncate: dissolve a trailing tuplet, halve and then drop from the right until the content fits,
// dot the survivor, fill exactly. See ENCORE_IMPORTER.md §4.5.
static void removeExtraNotes(Measure* measure, track_idx_t tr)
{
    const Fraction mLen = measure->ticks();
    const Fraction measTick = measure->tick();

    std::vector<ChordRest*> crs;

    // 1. Resolve trailing plain overflow from the right until the voice fits or a tuplet surfaces:
    //    a note crossing the barline is recut to it, a note starting at/after it is removed.
    while (true) {
        Fraction sum = collectVoice(measure, tr, crs);
        if (crs.empty() || sum <= mLen || crs.back()->tuplet()) {
            break;
        }
        ChordRest* last = crs.back();
        const Fraction lastStart = last->tick() - measTick;
        if (lastStart < mLen) {
            recutCrossingCR(measure, tr, last, mLen - lastStart);
            break;
        }
        detachSpannersAt(last);
        last->segment()->remove(last);
        delete last;
    }

    ChordRest* lastSurvivor = nullptr;
    Fraction contentEnd(0, 1);   // offset from measTick where surviving content ends

    Fraction sum = collectVoice(measure, tr, crs);
    if (!crs.empty() && sum > mLen && crs.back()->tuplet()) {
        // 2. Trailing tuplet: dissolve, then shrink its members from the right.
        Tuplet* t = crs.back()->tuplet();
        std::vector<ChordRest*> members;
        Fraction preContent(0, 1);
        for (ChordRest* cr : crs) {
            if (cr->tuplet() == t) {
                members.push_back(cr);
            } else if (members.empty()) {
                preContent += cr->actualTicks();
            }
        }
        dissolveTuplet(t);   // members now plain at face value (segment positions are stale)

        const Fraction available = mLen - preContent;
        std::vector<Fraction> durs;
        std::vector<int> halvings(members.size(), 0);
        for (ChordRest* m : members) {
            durs.push_back(m->actualTicks());
        }
        auto durSum = [&]() {
            Fraction s(0, 1);
            for (const Fraction& d : durs) {
                s += d;
            }
            return s;
        };
        // Halve each trailing note (max twice -> down to 1/4 of its value); if even a quarter
        // of it still overflows, remove it and move left. Stop as soon as the content fits.
        int idx = static_cast<int>(members.size()) - 1;
        while (durSum() > available && idx >= 0) {
            if (durs[idx] <= Fraction(0, 1)) {
                --idx;
                continue;
            }
            const Fraction halved = durs[idx] * Fraction(1, 2);
            if (halvings[idx] < 2 && fitsTDuration(halved)) {
                durs[idx] = halved;
                ++halvings[idx];
            } else {
                durs[idx] = Fraction(0, 1);
                --idx;
            }
        }

        // Detach every member from its stale segment, then re-place the survivors
        // sequentially from preContent (deleting the removed ones).
        for (ChordRest* m : members) {
            detachSpannersAt(m);
            m->segment()->remove(m);
        }
        Fraction pos = preContent;
        for (size_t j = 0; j < members.size(); ++j) {
            if (durs[j] <= Fraction(0, 1)) {
                delete members[j];
                continue;
            }
            const TDuration td(durs[j]);
            members[j]->setDurationType(td);
            members[j]->setTicks(td.fraction());
            members[j]->setDots(td.dots());
            members[j]->setTrack(tr);
            Segment* seg = measure->getSegment(SegmentType::ChordRest, measTick + pos);
            seg->add(members[j]);
            pos += durs[j];
            lastSurvivor = members[j];
        }
        contentEnd = pos;
    } else {
        contentEnd = sum;
        lastSurvivor = (!crs.empty() && crs.back()->isChord() && !crs.back()->tuplet()) ? crs.back() : nullptr;
    }

    // 3. Dot the last surviving note (up to 3 dots) to absorb the remaining gap, then fill the
    //    residual with an exact rest.
    Fraction gap = mLen - contentEnd;
    if (gap <= Fraction(0, 1)) {
        return;
    }
    if (lastSurvivor && lastSurvivor->isChord()) {
        const Fraction cur = lastSurvivor->actualTicks();
        const Fraction target = largestDottedLE(cur + gap, 3 /*maxDots*/);
        if (target > cur) {
            const TDuration td(target);
            lastSurvivor->setDurationType(td);
            lastSurvivor->setTicks(td.fraction());
            lastSurvivor->setDots(td.dots());
            contentEnd += (target - cur);
            gap = mLen - contentEnd;
        }
    }
    addFillRest(measure, tr, measTick + contentEnd, gap);
}

// Fill `totalLen` from `startTick` with visible rests, split into individually notatable
// figures (up to 3 dots) via toDurationList so layout keeps them as-is. A single rest with an
// odd total (e.g. a dotted 16th) is fine; larger residues split into a tied-rest sequence.
static void addFillRest(Measure* measure, track_idx_t tr, const Fraction& startTick, const Fraction& totalLen)
{
    if (totalLen <= Fraction(0, 1)) {
        return;
    }
    Fraction pos = startTick;
    for (const TDuration& d : toDurationList(totalLen, true /*useDots*/, 3 /*maxDots*/, false)) {
        if (!d.isValid()) {
            break;
        }
        Segment* fillSeg = measure->getSegment(SegmentType::ChordRest, pos);
        if (!fillSeg->element(tr)) {
            Rest* r = Factory::createRest(fillSeg, d);
            r->setTicks(d.fraction());
            r->setTrack(tr);
            r->setGap(false);
            fillSeg->add(r);
        }
        pos += d.fraction();
    }
}

// Stretch tier 1: reclaim space from the latest preceding rests, never from notes. Declines when
// there is not enough rest, or when a tuplet is present, which tier 2 handles.
static bool robRestsToFit(Measure* measure, track_idx_t tr)
{
    const Fraction mLen = measure->ticks();
    const Fraction measTick = measure->tick();

    std::vector<ChordRest*> crs;
    const Fraction voiceSum = collectVoice(measure, tr, crs);
    const Fraction overflow = voiceSum - mLen;
    if (overflow <= Fraction(0, 1) || crs.empty()) {
        return true;   // already fits
    }
    // Sum the reclaimable rest, and decline on any tuplet member (handled by bracket compression).
    Fraction reclaimable(0, 1);
    for (ChordRest* cr : crs) {
        if (cr->tuplet()) {
            return false;
        }
        if (cr->isRest()) {
            reclaimable += cr->actualTicks();
        }
    }
    if (reclaimable < overflow) {
        return false;   // not enough rest to absorb the overflow; caller falls back
    }

    // Decide each element's kept duration: chords unchanged; reclaim `overflow` from the latest
    // rests first (a rest reclaimed in full is dropped, a partially reclaimed one is re-split).
    std::vector<Fraction> keepDur(crs.size());
    Fraction need = overflow;
    for (size_t i = crs.size(); i-- > 0;) {
        if (crs[i]->isRest()) {
            const Fraction cur = crs[i]->actualTicks();
            const Fraction take = (cur < need) ? cur : need;
            keepDur[i] = cur - take;
            need -= take;
        } else {
            keepDur[i] = crs[i]->actualTicks();
        }
    }

    // Detach everything from its (now stale) segment, then re-place sequentially from measTick.
    // Chords are moved intact (notes/ties preserved); rests are re-created at their reduced length
    // via addFillRest so an odd remainder splits into exact figures; fully-reclaimed rests vanish.
    for (ChordRest* cr : crs) {
        detachSpannersAt(cr);
        cr->segment()->remove(cr);
    }
    Fraction pos(0, 1);
    for (size_t i = 0; i < crs.size(); ++i) {
        if (crs[i]->isRest()) {
            delete crs[i];
            if (keepDur[i] > Fraction(0, 1)) {
                addFillRest(measure, tr, measTick + pos, keepDur[i]);
                pos += keepDur[i];
            }
        } else {
            Segment* seg = measure->getSegment(SegmentType::ChordRest, measTick + pos);
            crs[i]->setTrack(tr);
            seg->add(crs[i]);
            pos += crs[i]->actualTicks();
        }
    }
    return true;
}

// Stretch: rests first, then the trailing tuplet's bracket, then a lone note recut to the barline.
// Declines when the result would be too small to be musical, and the caller extends the measure.
static bool stretchOverfullVoice(Measure* measure, track_idx_t tr)
{
    const Fraction mLen = measure->ticks();
    const Fraction measTick = measure->tick();

    std::vector<ChordRest*> crs;
    const Fraction voiceSum = collectVoice(measure, tr, crs);
    if (crs.empty() || voiceSum <= mLen) {
        return true;
    }

    // Tier 1: reclaim space from preceding rests so every note keeps its full value at the
    // nominal measure length. Only when there is enough rest (and no tuplet to preserve).
    if (robRestsToFit(measure, tr)) {
        return true;
    }

    ChordRest* last = crs.back();
    if (last->tuplet()) {
        Tuplet* t = last->tuplet();
        // Position where the tuplet starts = total actual ticks of the notes before its first member.
        Fraction preContent(0, 1);
        std::vector<ChordRest*> members;
        for (ChordRest* cr : crs) {
            if (cr->tuplet() == t) {
                members.push_back(cr);
            } else if (members.empty()) {
                preContent += cr->actualTicks();
            }
        }
        const Fraction available = mLen - preContent;
        if (available <= Fraction(0, 1) || members.empty()) {
            return false;
        }
        const int aN = t->ratio().numerator();
        const int nN = t->ratio().denominator();
        if (aN <= 0 || nN <= 0) {
            return false;
        }
        const Fraction naturalBracket = TDuration(t->baseLen()).fraction() * nN;
        // Compress the bracket to the largest base that fits (up to 3 dots); the exact
        // remainder is filled with a rest below.
        const Fraction newBaseLen = largestDottedLE(available / nN, 3 /*maxDots*/);
        if (newBaseLen <= Fraction(0, 1)) {
            return false;
        }
        const Fraction newBracket = newBaseLen * nN;
        // Too small to be musical: the largest bracket that fits is < half the natural span.
        if (newBracket * Fraction(2, 1) < naturalBracket) {
            return false;
        }
        // Resize and reposition each member within the compressed bracket.
        const TDuration baseTd(newBaseLen);
        const Fraction memberActual = newBaseLen * Fraction(nN, aN);
        Fraction pos = preContent;
        for (ChordRest* m : members) {
            m->setDurationType(baseTd);
            m->setTicks(baseTd.fraction());
            m->setDots(baseTd.dots());
            const Fraction newTick = measTick + pos;
            if (m->segment()->tick() != newTick) {
                detachSpannersAt(m);
                Segment* oldSeg = m->segment();
                oldSeg->remove(m);
                Segment* ns = measure->getSegment(SegmentType::ChordRest, newTick);
                ns->add(m);
            }
            pos += memberActual;
        }
        t->setBaseLen(baseTd);
        t->setTicks(newBracket);
        t->setTick(measTick + preContent);
        addFillRest(measure, tr, measTick + preContent + newBracket, mLen - (preContent + newBracket));
        return true;
    }

    // Lone trailing note (no tuplet): recut it to end exactly at the barline, keeping its full
    // value up to that point as a tied chain (e.g. a dotted half in 5/8 -> half tied to eighth)
    // rather than collapsing it to the largest single figure and padding with a rest.
    const Fraction preContent = voiceSum - last->actualTicks();
    const Fraction available = mLen - preContent;
    if (available <= Fraction(0, 1)) {
        return false;
    }
    recutCrossingCR(measure, tr, last, available);
    return true;
}

// Post-pass entry point: resolve any overfull voice according to the overfill strategy.
void fitOverfullMeasure(BuildCtx& ctx, Measure* measure)
{
    // IrregularMeasure extends the measure to hold all content.
    if (ctx.opts.overfillMeasureStrategy == OverfillStrategy::IrregularMeasure) {
        capMeasureLength(ctx, measure);
        return;
    }

    const Fraction mLen = measure->ticks();
    std::vector<ChordRest*> crs;
    bool needIrregularFallback = false;
    for (int si = 0; si < ctx.totalStaves; ++si) {
        for (voice_idx_t v = 0; v < VOICES; ++v) {
            const track_idx_t tr = static_cast<track_idx_t>(si * VOICES + v);
            const Fraction sum = collectVoice(measure, tr, crs);
            if (crs.empty() || sum <= mLen) {
                continue;
            }
            switch (ctx.opts.overfillMeasureStrategy) {
            case OverfillStrategy::Truncate:
                removeExtraNotes(measure, tr);
                break;
            case OverfillStrategy::StretchLastNote:
                // Declining here degrades to an irregular bar rather than a standard-length one.
                if (!stretchOverfullVoice(measure, tr)) {
                    needIrregularFallback = true;
                }
                break;
            default:
                break;
            }
        }
    }
    if (needIrregularFallback) {
        extendMeasureIrregular(ctx, measure);
    }
}
} // namespace mu::iex::enc
