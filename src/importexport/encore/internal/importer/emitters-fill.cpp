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

// Adjust measure length: pickup shortening, trailing-gap fill and over/undershoot correction.

#include "emitters-internal.h"

#include <algorithm>

#include "engraving/dom/chord.h"
#include "engraving/dom/durationtype.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/sig.h"
#include "engraving/dom/tuplet.h"

namespace mu::iex::enc {
// Resize the measure and shift every following measure and every pending element that caches an
// absolute tick; ticks inside the kept content never move.
static void resizeMeasureAndShift(BuildCtx& ctx, Measure* measure, Fraction newLen)
{
    // Reduce to lowest terms so an irregular measure reads as a sensible time signature (99/96 -> 33/32);
    // reduced() does not change the duration, so the shift arithmetic below is unaffected.
    newLen = newLen.reduced();
    const Fraction oldLen = measure->ticks();
    const Fraction delta = newLen - oldLen;
    if (delta == Fraction(0, 1)) {
        return;
    }
    const Fraction measTick = measure->tick();
    // Shrinking removes the [newEnd, oldEnd) span, so a tick exactly at the new end stays put
    // (strict >); extending inserts space at the old end, so a tick there is the next downbeat
    // and moves (>=).
    const bool shrinking = delta < Fraction(0, 1);
    const Fraction boundary = measTick + std::min(oldLen, newLen);

    measure->setTicks(newLen);
    for (Measure* m = measure->nextMeasure(); m; m = m->nextMeasure()) {
        m->setTick(m->tick() + delta);
    }

    auto shift = [&](Fraction& t) {
        const bool past = shrinking ? (t > boundary) : (t >= boundary);
        if (past) {
            t += delta;
        }
    };

    for (PendingHairpin& p : ctx.pendingHairpins) {
        shift(p.startTick);
        shift(p.maxEndTick);
    }
    for (PendingSlur& p : ctx.pendingSlurs) {
        shift(p.startTick);
    }
    for (PendingArpeggio& p : ctx.pendingArpeggios) {
        shift(p.tick);
    }
    for (PendingOrnTremolo& p : ctx.pendingOrnTremolos) {
        shift(p.tick);
        shift(p.measTick);
    }
    for (PendingTrill& p : ctx.pendingTrills) {
        shift(p.tick);
    }
    for (auto& [tr, ends] : ctx.pendingTrillEnds) {
        for (Fraction& e : ends) {
            shift(e);
        }
    }
    for (PendingStaccato& p : ctx.pendingStaccatos) {
        shift(p.tick);
    }
    for (PendingFermata& p : ctx.pendingFermatas) {
        shift(p.tick);
    }
    for (PendingBreath& p : ctx.pendingBreaths) {
        shift(p.tick);
    }
    for (PendingMeasureRepeat& p : ctx.pendingMeasureRepeats) {
        shift(p.measTick);
    }
    for (PendingBowing& p : ctx.pendingBowings) {
        shift(p.tick);
    }
    for (PendingOrnFingering& p : ctx.pendingOrnFingerings) {
        shift(p.tick);
    }
    for (PendingOttava& p : ctx.pendingOttavas) {
        shift(p.startTick);
    }
    for (PendingMarker& p : ctx.pendingMarkers) {
        shift(p.tick);
    }
}

// Exact-value rests split per the time signature, not one whole-measure rest, which renders as a
// centered whole rest whatever its duration. No-op if the first segment is occupied.
static void addGapRests(Measure* measure, const Fraction& fillTick, const Fraction& len,
                        track_idx_t track, bool makeGap)
{
    if (len <= Fraction(0, 1)) {
        return;
    }
    const Fraction rtick = fillTick - measure->tick();
    Fraction pos = fillTick;
    for (const TDuration& d : toRhythmicDurationList(len, true /*isRest*/, rtick,
                                                     measure->timesig(), measure, 0 /*maxDots*/)) {
        Segment* seg = measure->getSegment(SegmentType::ChordRest, pos);
        if (seg->element(track)) {
            break;
        }
        Rest* r = Factory::createRest(seg, d);
        r->setTicks(d.isMeasure() ? measure->ticks() : d.fraction());
        r->setTrack(track);
        r->setGap(makeGap);
        seg->add(r);
        pos += r->actualTicks();
    }
}

// Pickup adjustment: if measure 0 has the same timesig as the full nominal length but the note
// loop placed less content, shorten it to the actual cumTick (and shift following measures).
static void adjustPickupMeasure(BuildCtx& ctx, Measure* measure, int measIdx)
{
    if (!ctx.opts.firstMeasureIsPickup) {
        return;
    }
    if (measIdx != 0 || measure->timesig() != measure->ticks()) {
        return;
    }
    Fraction maxCumTick { 0, 1 };
    for (auto& [key, ct] : ctx.scratch.cumTick) {
        if (ct > maxCumTick) {
            maxCumTick = ct;
        }
    }
    if (maxCumTick <= Fraction(0, 1) || maxCumTick >= measure->ticks()) {
        return;
    }
    resizeMeasureAndShift(ctx, measure, maxCumTick);
}

// Pre-fill trailing silence so checkMeasure does not add its own; the option decides visible rests,
// invisible ones, or an irregular bar. Only for voices that have content.
static void fillTrailingGaps(BuildCtx& ctx, Measure* measure, Fraction measTick)
{
    const bool makeGap = (ctx.opts.underfillMeasureStrategy != UnderfillStrategy::VisibleRests
                          && ctx.opts.underfillMeasureStrategy != UnderfillStrategy::IrregularMeasure);
    const bool irregular = (ctx.opts.underfillMeasureStrategy == UnderfillStrategy::IrregularMeasure);

    for (int si = 0; si < ctx.totalStaves; ++si) {
        for (voice_idx_t v = 0; v < VOICES; ++v) {
            const auto key = std::make_pair(si, static_cast<int>(v));
            if (!ctx.scratch.cumTick.count(key)) {
                continue;
            }
            const Fraction voicePos = ctx.scratch.cumTick.at(key);
            if (voicePos <= Fraction(0, 1)) {
                continue;
            }
            const Fraction remaining = measure->ticks() - voicePos;
            if (remaining <= Fraction(0, 1)) {
                continue;
            }
            if (irregular) {
                continue;
            }
            const track_idx_t tr = static_cast<track_idx_t>(si * VOICES + v);
            const Fraction fillTick = measTick + voicePos;
            addGapRests(measure, fillTick, remaining, tr, makeGap);
        }
    }

    if (irregular) {
        // Shrink to the longest staff's content, but ONLY when every staff is genuinely short.
        // A staff with no content is a whole-bar rest = the full nominal length, so the bar is
        // not short and must stay nominal (short staves are filled to it by checkMeasure).
        Fraction maxPos { 0, 1 };
        bool anyStaffSilent = false;
        for (int si = 0; si < ctx.totalStaves && !anyStaffSilent; ++si) {
            Fraction staffLen { 0, 1 };
            for (voice_idx_t v = 0; v < VOICES; ++v) {
                const auto k = std::make_pair(si, static_cast<int>(v));
                if (ctx.scratch.cumTick.count(k) && ctx.scratch.cumTick.at(k) > staffLen) {
                    staffLen = ctx.scratch.cumTick.at(k);
                }
            }
            if (staffLen <= Fraction(0, 1)) {
                anyStaffSilent = true;   // whole-bar rest: this staff fills the nominal length
            } else if (staffLen > maxPos) {
                maxPos = staffLen;
            }
        }
        if (!anyStaffSilent && maxPos > Fraction(0, 1) && maxPos < measure->ticks()) {
            resizeMeasureAndShift(ctx, measure, maxPos);
        }
    }
}

// Maximum measure-length correction: 1/24 of a whole note (about one 32nd-note triplet).
// Corrections larger than this indicate genuine notation errors, not rounding noise.
static const Fraction kFillMaxDelta(1, 24);

// Fix over/undershoots up to kFillMaxDelta: overshoot removes smallest gap rests, undershoot
// adds exact-valued gap rests.
static void correctMeasureLength(BuildCtx& ctx, Measure* measure)
{
    const bool makeGap = (ctx.opts.underfillMeasureStrategy != UnderfillStrategy::VisibleRests);
    const Fraction mLen = measure->ticks();
    const Fraction maxDelta = kFillMaxDelta;
    for (int si = 0; si < ctx.totalStaves; ++si) {
        for (voice_idx_t v = 0; v < VOICES; ++v) {
            track_idx_t tr = static_cast<track_idx_t>(si * VOICES + v);
            std::vector<ChordRest*> crs;
            Fraction voiceSum = collectVoice(measure, tr, crs);
            if (crs.empty()) {
                continue;   // no content in this voice
            }
            std::vector<Rest*> gapRests;
            for (ChordRest* cr : crs) {
                if (cr->isRest() && toRest(cr)->isGap()) {
                    gapRests.push_back(toRest(cr));
                }
            }
            // Overshoot: remove gap rests smallest-first.
            // Skip for IrregularMeasure overfill, capMeasureLength will extend instead.
            if (voiceSum > mLen && (voiceSum - mLen) <= maxDelta
                && ctx.opts.overfillMeasureStrategy != OverfillStrategy::IrregularMeasure) {
                std::stable_sort(gapRests.begin(), gapRests.end(),
                                 [](Rest* a, Rest* b) {
                    return a->actualTicks() < b->actualTicks();
                });
                for (Rest* gr : gapRests) {
                    if (voiceSum <= mLen) {
                        break;
                    }
                    Fraction at = gr->actualTicks();
                    voiceSum -= at;
                    Segment* gseg = gr->segment();
                    gseg->remove(gr);
                    delete gr;
                }
            }
            // Undershoot: fill the residual with exact-valued rests.
            const Fraction deficit = mLen - voiceSum;
            if (deficit > Fraction(0, 1) && deficit <= maxDelta) {
                addGapRests(measure, measure->tick() + voiceSum, deficit, tr, makeGap);
            }
        }
    }
}

// The one place that owns "every voice on every staff sums to the measure length". The order is
// load-bearing: checkMeasure runs between the underfull fill and the overfull fixes.
// See ENCORE_IMPORTER.md 4.1.
void reconcileMeasureLength(BuildCtx& ctx, Measure* measure, Fraction measTick, int measIdx)
{
    adjustPickupMeasure(ctx, measure, measIdx);
    fillTrailingGaps(ctx, measure, measTick);
    for (int si = 0; si < ctx.totalStaves; ++si) {
        measure->checkMeasure(static_cast<staff_idx_t>(si));
    }
    correctMeasureLength(ctx, measure);
    fitOverfullMeasure(ctx, measure);
}

// Runs before the marks are resolved: taking an element away is only safe while nothing is anchored
// to it. See ENCORE_IMPORTER.md §4.1.
void guaranteeAllMeasures(BuildCtx& ctx)
{
    if (ctx.opts.overfillMeasureStrategy == OverfillStrategy::IrregularMeasure) {
        return;
    }
    for (mu::engraving::Measure* m = ctx.score->firstMeasure(); m; m = m->nextMeasure()) {
        guaranteeMeasureLength(ctx, m);
    }
}

// Extend the measure to its longest voice, shifting later measures and pending hairpins. Used by the
// IrregularMeasure strategy and as the Stretch fallback.
void extendMeasureIrregular(BuildCtx& ctx, Measure* measure)
{
    const Fraction mLen = measure->ticks();
    const Fraction measTick = measure->tick();

    std::vector<ChordRest*> crs;
    Fraction maxVoiceSum { 0, 1 };
    for (int si = 0; si < ctx.totalStaves; ++si) {
        for (voice_idx_t v = 0; v < VOICES; ++v) {
            const Fraction voiceSum = collectVoice(measure, static_cast<track_idx_t>(si * VOICES + v), crs);
            if (voiceSum > maxVoiceSum) {
                maxVoiceSum = voiceSum;
            }
        }
    }
    if (maxVoiceSum > mLen) {
        resizeMeasureAndShift(ctx, measure, maxVoiceSum);
        // Fill all voices that fall short of the extended measure length.
        // Staves whose content stopped at the original measure length now sit
        // inside a longer measure; a visible rest covers the added time.
        for (int si = 0; si < ctx.totalStaves; ++si) {
            for (voice_idx_t v = 0; v < VOICES; ++v) {
                const track_idx_t tr = static_cast<track_idx_t>(si * VOICES + v);
                const Fraction voiceSum = collectVoice(measure, tr, crs);
                if (voiceSum <= Fraction(0, 1) || voiceSum >= maxVoiceSum) {
                    continue;
                }
                addGapRests(measure, measTick + voiceSum, maxVoiceSum - voiceSum, tr, false);
            }
        }
    }
}

// Hard cap: drop trailing elements from any voice that still overshoots, then fill the deficit, so no
// measure ends with the wrong total. IrregularMeasure extends the bar instead.
static bool isWritableLength(const Fraction& f)
{
    const Fraction r = f.reduced();
    return r.numerator() >= 0 && (r.denominator() & (r.denominator() - 1)) == 0;
}

// Last word on the length of a bar: trim past the barline, walk the boundary back to a writable
// length, fill. See ENCORE_IMPORTER.md §4.1.
void guaranteeMeasureLength(BuildCtx& ctx, Measure* measure)
{
    const bool makeGap = (ctx.opts.underfillMeasureStrategy != UnderfillStrategy::VisibleRests);
    const Fraction mLen = measure->ticks();
    const Fraction measTick = measure->tick();

    const int nStaves = static_cast<int>(ctx.score->nstaves());
    for (int si = 0; si < nStaves; ++si) {
        for (voice_idx_t v = 0; v < VOICES; ++v) {
            const track_idx_t tr = static_cast<track_idx_t>(si * VOICES + v);
            std::vector<ChordRest*> crs;
            Fraction sum = collectVoice(measure, tr, crs);
            if (sum == mLen || (crs.empty() && v != 0)) {
                continue;
            }
            // A bracket member is removed, never the bracket dissolved: dissolving lengthens the
            // members in place and the sum stops being the room they occupy.
            auto dropLast = [&]() {
                if (crs.empty()) {
                    return false;
                }
                ChordRest* last = crs.back();
                const Fraction was = last->actualTicks();
                detachSpannersAt(last);
                if (last->tuplet()) {
                    last->tuplet()->remove(last);
                    last->setTuplet(nullptr);
                }
                crs.pop_back();
                Segment* lseg = last->segment();
                lseg->remove(last);
                delete last;
                sum -= was;
                return true;
            };
            int guard = 256;
            while (sum > mLen && guard-- > 0 && dropLast()) {
            }
            guard = 256;
            while (sum < mLen && !isWritableLength(mLen - sum) && guard-- > 0 && dropLast()) {
            }
            if (sum < mLen && isWritableLength(mLen - sum)) {
                addGapRests(measure, measTick + sum, mLen - sum, tr, makeGap);
            }
        }
    }
}

void capMeasureLength(BuildCtx& ctx, Measure* measure)
{
    const bool makeGap = (ctx.opts.underfillMeasureStrategy != UnderfillStrategy::VisibleRests);
    const Fraction mLen = measure->ticks();

    if (ctx.opts.overfillMeasureStrategy == OverfillStrategy::IrregularMeasure) {
        extendMeasureIrregular(ctx, measure);
        return;
    }

    for (int si = 0; si < ctx.totalStaves; ++si) {
        for (voice_idx_t v = 0; v < VOICES; ++v) {
            const track_idx_t tr = static_cast<track_idx_t>(si * VOICES + v);
            std::vector<ChordRest*> crs;
            Fraction voiceSum = collectVoice(measure, tr, crs);
            if (voiceSum <= mLen || crs.empty()) {
                continue;
            }
            while (voiceSum > mLen && !crs.empty()) {
                ChordRest* last = crs.back();
                // A tuplet is atomic: dissolve it whole (members revert to plain face value)
                // rather than removing one member and leaving an invalid partial tuplet, then
                // re-collect and keep trimming the now-plain notes.
                if (last->tuplet()) {
                    dissolveTuplet(last->tuplet());
                    voiceSum = collectVoice(measure, tr, crs);
                    continue;
                }
                crs.pop_back();
                voiceSum -= last->actualTicks();
                Segment* lseg = last->segment();
                lseg->remove(last);
                delete last;
            }
            const Fraction deficit = mLen - voiceSum;
            if (deficit > Fraction(0, 1)) {
                addGapRests(measure, measure->tick() + voiceSum, deficit, tr, makeGap);
            }
        }
    }
}

void handleDanglingGraces(BuildCtx& ctx)
{
    // Grace chords with no principal chord become audible cue notes in the spare cue voice of their own
    // bar, flush to the barline, so the figure and its timing survive.
    for (auto& [key, vec] : ctx.scratch.pendingGraces) {
        const int staffIdx = key.first;
        const track_idx_t track = static_cast<track_idx_t>(staffIdx * static_cast<int>(VOICES) + kCueVoice);

        std::map<Measure*, std::vector<PendingGrace*> > byMeasure;
        for (PendingGrace& g : vec) {
            byMeasure[g.measure].push_back(&g);
        }
        for (auto& [measure, graces] : byMeasure) {
            if (!measure) {
                for (PendingGrace* g : graces) {
                    delete g->gc;
                }
                continue;
            }
            Fraction total(0, 1);
            for (PendingGrace* g : graces) {
                total += g->gc->ticks();
            }
            const Fraction mLen = measure->ticks();
            const Fraction measTick = measure->tick();
            Fraction pos = (total < mLen) ? (mLen - total) : Fraction(0, 1);
            if (pos > Fraction(0, 1)) {
                addGapRests(measure, measTick, pos, track, true /*gap*/);
            }
            for (PendingGrace* g : graces) {
                Chord* src = g->gc;
                const Fraction d = src->ticks();
                Segment* seg = measure->getSegment(SegmentType::ChordRest, measTick + pos);
                Chord* c = Factory::createChord(seg);
                c->setTrack(track);
                c->setDurationType(src->durationType());
                c->setTicks(d);
                c->setDots(0);
                c->setSmall(true);   // cue chord: whole chord small (head + stem), not just the note
                for (Note* sn : src->notes()) {
                    Note* n = Factory::createNote(c);
                    n->setPitch(sn->pitch());
                    n->setTpc1(sn->tpc1());
                    n->setTpc2(sn->tpc2());
                    n->setPlay(!g->en->isMuted());
                    c->add(n);
                }
                seg->add(c);
                pos += d;
                delete src;
            }
            if (pos < mLen) {
                addGapRests(measure, measTick + pos, mLen - pos, track, true /*gap*/);
            }
        }
    }
    ctx.scratch.pendingGraces.clear();
}
} // namespace mu::iex::enc
