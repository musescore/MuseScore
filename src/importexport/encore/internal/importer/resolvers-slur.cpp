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

// Post-pass: resolve slur spanner endpoints (Encore stores no explicit slur end).

#include "resolvers.h"
#include "coords.h"
#include "../parser/elem.h"
#include <optional>
#include <set>
#include <map>
#include <cstdlib>
#include "engraving/dom/chord.h"
#include "engraving/dom/slur.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/note.h"
#include "log.h"

using namespace mu::engraving;

namespace mu::iex::enc {
// ps.track is captured before the note is emitted, so it may be stale after stream-overflow
// voice reassignment; find the voice that actually carries a chord at tick.
static track_idx_t resolveChordTrack(MasterScore* score, Fraction tick, int staffIdx, track_idx_t fallback)
{
    Segment* seg = score->tick2segment(tick, false, SegmentType::ChordRest);
    track_idx_t t = fallback;
    return firstChordVoiceAt(score, seg, staffIdx, t) ? t : fallback;
}

static int getLineSlot(const EncMeasureElem* em, const std::array<int, 256>& lineSlotByRawByte)
{
    const quint8 raw = em->rawStaffByte();
    const int slot = lineSlotByRawByte[static_cast<unsigned char>(raw)];
    return (slot >= 0) ? slot : static_cast<int>(em->staffIdx);
}

static void removeOrphanSlurs(MasterScore* score, const std::set<const Spanner*>& explicitSlurs)
{
    std::vector<Spanner*> toRemove;
    for (auto& [tick, spanner] : score->spannerMap().map()) {
        if (spanner->isSlur()) {
            // Slurs anchored to explicit chord elements (reliable measure-count path): keep as-is,
            // recompute would null them out at bar boundaries.
            if (explicitSlurs.count(spanner)) {
                continue;
            }
            // computeStartElement() would resolve to the regular chord, not the explicitly set
            // grace sub-chord (tick2segment cannot see it), so skip the recompute for grace starts.
            const bool graceStart = spanner->startElement()
                                    && spanner->startElement()->isChord()
                                    && toChord(spanner->startElement())->isGrace();
            if (!graceStart) {
                spanner->computeStartElement();
            }
            // Grace-to-main (tick == tick2): computeEndElement() would fail (no segment at the
            // same tick via the spanner lookup), so skip it. Grace-to-later resolves normally.
            const bool graceToMain = graceStart
                                     && (spanner->tick() == spanner->tick2());
            if (!graceToMain) {
                spanner->computeEndElement();
            }
            // An overfull measure can make a slur's start and end grips resolve to the same
            // chord (zero-length arc); its layout takes atan of a zero span and asserts on a
            // NaN Bezier control point, so drop it along with the orphans.
            if (!spanner->startElement() || !spanner->endElement()
                || spanner->startElement() == spanner->endElement()) {
                toRemove.push_back(spanner);
            }
        }
    }
    for (Spanner* sp : toRemove) {
        score->removeElement(sp);
    }
}

static void createGraceToMainSlur(const PendingSlur& ps, MasterScore* score, Fraction startTick)
{
    // Zero span (SLURSTART tick == parent chord tick): a grace note slurs to its own main note.
    Segment* gSeg = score->tick2segment(startTick, true, SegmentType::ChordRest);
    if (gSeg) {
        const track_idx_t graceTrack = resolveChordTrack(score, startTick, ps.staffIdx, ps.track);
        EngravingItem* el = gSeg->element(graceTrack);
        if (el && el->isChord()) {
            const std::vector<Chord*> graces = toChord(el)->graceNotesBefore();
            if (!graces.empty()) {
                Slur* gSlur = Factory::createSlur(score->dummy());
                gSlur->setTrack(graceTrack);
                gSlur->setTrack2(graceTrack);
                gSlur->setTick(startTick);
                // tick2 == tick signals graceToMain=true to the post-pass, preventing computeEndElement()
                // from replacing the explicit end element with an end-of-measure chord.
                gSlur->setTick2(startTick);
                gSlur->setStartElement(graces.front());
                gSlur->setEndElement(el);
                // false: keep the explicit grace endpoints instead of recomputing them.
                score->addSpanner(gSlur, false);
            }
        }
    }
}

static void createNormalSlur(const PendingSlur& ps, track_idx_t startTrack, track_idx_t endTrack,
                             Fraction endTick, MasterScore* score)
{
    Slur* slur = Factory::createSlur(score->dummy());
    slur->setTrack(startTrack);
    slur->setTrack2(endTrack);
    slur->setTick(ps.startTick);
    slur->setTick2(endTick);
    // If the chord at/after startTick has grace notes, the SLURSTART belongs to the grace; anchor
    // there. tick2rightSegment absorbs grace-note tick stealing (startTick can be just before it).
    {
        Segment* rSeg = score->tick2rightSegment(ps.startTick, false, SegmentType::ChordRest);
        if (rSeg) {
            EngravingItem* rEl = rSeg->element(startTrack);
            if (rEl && rEl->isChord()) {
                const std::vector<Chord*> graces = toChord(rEl)->graceNotesBefore();
                if (!graces.empty()) {
                    slur->setStartElement(graces.front());
                }
            }
        }
    }
    score->addElement(slur);
}

// Fallback 1: xoffset2 directly comparable within target measure (xoffsets reset at barlines).
// Returns the best candidate endTick, or nullopt if no note found.
static std::optional<Fraction> resolveCrossMeasureXoffset(
    const PendingSlur& ps, const EncMeasure& endEncMeas,
    Measure* endMeas, const std::array<int, 256>& lineSlotByRawByte)
{
    const int wholeTicks = encWholeNoteTicks(endEncMeas);
    int bestDist = std::numeric_limits<int>::max();
    int bestEncTick = -1;
    forEachStaffNoteXoff(endEncMeas, ps.staffIdx, /*includeRests*/ false, &lineSlotByRawByte,
                         [&](const EncMeasureElem* em, int xoff) {
        const int dist = std::abs(xoff - ps.slurXoffset2);
        if (dist <= bestDist) {
            bestDist = dist;
            bestEncTick = static_cast<int>(em->tick);
        }
        return true;
    });
    if (bestEncTick >= 0 && wholeTicks > 0) {
        const Fraction candidate = endMeas->tick() + Fraction(bestEncTick, wholeTicks).reduced();
        if (candidate > ps.startTick) {
            return candidate;
        }
    }
    return std::nullopt;
}

// Fallback 2: last chord/rest in the target measure on any voice of the same staff.
// Returns the tick or nullopt if the measure has no chords on that staff.
static std::optional<Fraction> resolveLastChordInMeasure(const PendingSlur& ps, Measure* endMeas)
{
    Segment* lastSeg = nullptr;
    for (Segment* s = endMeas->first(SegmentType::ChordRest); s;
         s = s->next(SegmentType::ChordRest)) {
        for (int v = 0; v < static_cast<int>(VOICES); ++v) {
            track_idx_t t = static_cast<track_idx_t>(ps.staffIdx * VOICES + v);
            if (s->element(t) && s->element(t)->isChord()) {
                lastSeg = s;
                break;
            }
        }
    }
    if (!lastSeg) {
        return std::nullopt;
    }
    return lastSeg->tick();
}

// First chord at or after `from` on any voice of the staff within the measure.
// Sets outTrack to the voice that carries it. Returns nullptr if none.
// Iterates ChordRest segments directly because tick2segment is unreliable at bar boundaries.
static Chord* firstChordOnStaffFrom(const Score* score, Measure* m, int staffIdx,
                                    const Fraction& from, track_idx_t& outTrack)
{
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        if (s->tick() < from) {
            continue;
        }
        if (Chord* c = firstChordVoiceAt(score, s, staffIdx, outTrack)) {
            return c;
        }
    }
    return nullptr;
}

// The xoffset of the slur's start note: scan the start measure for the NOTE at startEncTick on
// this staff (any voice, since the slur ORN's encVoice is the arc position, not the note voice)
// and return its xoffset. A grace note at that tick wins over a regular one (v0xC4 serializes the
// regular note first, but the grace xoffset is the true arc-start reference). Returns -1 when no
// start note is found. See ENCORE_FORMAT.md §Slur.
static int findSlurStartNoteXoffset(const EncMeasure& startEncMeas, int staffIdx, int startEncTick,
                                    const std::array<int, 256>& lineSlotByRawByte)
{
    int firstNoteXoff = -1;
    int graceXoff = -1;
    forEachStaffNoteXoff(startEncMeas, staffIdx, /*includeRests*/ false, &lineSlotByRawByte,
                         [&](const EncMeasureElem* em, int xoff) {
        if (static_cast<int>(em->tick) != startEncTick) {
            return true;
        }
        const EncNote* en = static_cast<const EncNote*>(em);
        if (en->graceType() != EncGraceType::NORMAL) {
            graceXoff = xoff;   // grace wins; stop searching
            return false;
        }
        if (firstNoteXoff < 0) {
            firstNoteXoff = xoff;   // regular: tentative, keep searching
        }
        return true;
    });
    return (graceXoff >= 0) ? graceXoff : firstNoteXoff;
}

// Cross-measure endpoint search: when alMezuro is unreliable and the arc clearly runs past the
// start measure, scan the next one or two measures for the note whose xoffset best matches
// targetEndXoff. bestDist is in/out: only a strictly closer note updates it and yields a result.
// Returns the endpoint tick when a closer note is found, else nullopt (caller keeps its endpoint).
static std::optional<Fraction> extendSlurToLaterMeasures(
    BuildCtx& ctx, const PendingSlur& ps, const std::array<int, 256>& lineSlotByRawByte,
    int targetEndXoff, int& bestDist)
{
    const EncRoot& enc = ctx.enc;
    std::optional<Fraction> result;
    for (int nextMIdx = ps.startMeasIdx + 1;
         nextMIdx <= ps.startMeasIdx + 2
         && nextMIdx < static_cast<int>(enc.measures.size())
         && nextMIdx < static_cast<int>(ctx.measuresByIdx.size());
         ++nextMIdx) {
        const EncMeasure& nextEncMeas = enc.measures[nextMIdx];
        const int nextWt = encWholeNoteTicks(nextEncMeas);
        Measure* nextMs = ctx.measuresByIdx[nextMIdx];
        forEachStaffNoteXoff(nextEncMeas, ps.staffIdx, /*includeRests*/ false, &lineSlotByRawByte,
                             [&](const EncMeasureElem* em, int xoff) {
            const int dist = std::abs(xoff - targetEndXoff);
            if (dist < bestDist) {
                bestDist = dist;
                const Fraction endRel(static_cast<int>(em->tick), nextWt);
                result = nextMs->tick() + endRel.reduced();
            }
            return true;
        });
        if (bestDist == 0) {
            break;
        }
    }
    return result;
}

static std::optional<Fraction> resolveSameMeasureHeuristic(
    BuildCtx& ctx, const PendingSlur& ps, const std::array<int, 256>& lineSlotByRawByte)
{
    MasterScore* score = ctx.score;
    const EncRoot& enc = ctx.enc;
    const bool tryHeuristic = (!ps.alMezuroValid || ps.alMezuro == 0)
                              && ps.startMeasIdx >= 0
                              && ps.startMeasIdx < static_cast<int>(enc.measures.size());
    if (!tryHeuristic) {
        return std::nullopt;
    }
    Fraction endTick;
    bool resolved = false;

    const EncMeasure& startEncMeas = enc.measures[ps.startMeasIdx];
    const Fraction relStartTick = ps.startTick - ctx.measuresByIdx[ps.startMeasIdx]->tick();
    const int wt = encWholeNoteTicks(startEncMeas);
    const int startEncTick = (relStartTick.numerator() * wt)
                             / std::max(1, relStartTick.denominator());
    const int firstNoteXoff = findSlurStartNoteXoffset(startEncMeas, ps.staffIdx, startEncTick,
                                                       lineSlotByRawByte);
    if (firstNoteXoff >= 0) {
        const int pixelSpan = ps.slurXoffset2 - ps.slurXoffset;
        // Tiny pixelSpan (0-2) with note before arc start: firstNoteXoff+pixelSpan near 0 matches a decoy.
        // Use slurXoffset2 directly as the arc-end target instead.
        const bool usedTinyPixelSpan = (pixelSpan >= 0 && pixelSpan <= 2
                                        && firstNoteXoff < ps.slurXoffset);
        // v0xC2 short slur: slurXoffset2 lives in a stale ornament-coordinate origin, so matching
        // it over-extends the arc. pixelSpan is origin-independent but only distinguishes short
        // from long, so a tiny span is treated as a note-to-next-note slur (anchored below).
        const bool v0c2ShortSlur = enc.fmt->slurXoffset2Stale()
                                   && (std::abs(pixelSpan) <= 2);
        const int targetEndXoff = usedTinyPixelSpan
                                  ? ps.slurXoffset2
                                  : firstNoteXoff + pixelSpan;
        // One pass: pick the best later-note endpoint and detect a grace/regular co-location
        // at the start (the grace-to-main shortcut below).
        {
            int bestDist = std::numeric_limits<int>::max();
            int bestEncTick = -1;
            int maxXoffInMeas = -1;
            bool hasGraceAtStart = false;
            int regularXoffAtStart = -1;
            for (const auto& elem : startEncMeas.elements) {
                const EncMeasureElem* em = elem.get();
                if (em->type != static_cast<quint8>(EncElemType::NOTE)) {
                    continue;
                }
                if (getLineSlot(em, lineSlotByRawByte) != ps.staffIdx) {
                    continue;
                }
                const int xoff = static_cast<int>(em->xoffset);
                if (xoff > maxXoffInMeas) {
                    maxXoffInMeas = xoff;
                }
                if (static_cast<int>(em->tick) == startEncTick) {
                    const EncNote* en = static_cast<const EncNote*>(em);
                    if (en->graceType() != EncGraceType::NORMAL) {
                        hasGraceAtStart = true;
                    } else {
                        // Gap notes often have xoffset=0; keep the best-matching regular note.
                        const int thisDist = std::abs(xoff - targetEndXoff);
                        if (regularXoffAtStart < 0
                            || thisDist < std::abs(regularXoffAtStart - targetEndXoff)) {
                            regularXoffAtStart = xoff;
                        }
                    }
                }
                // Only notes strictly after the start can be endpoints.
                if (static_cast<int>(em->tick) <= startEncTick) {
                    continue;
                }
                if (v0c2ShortSlur) {
                    // Next-note rule: pick the earliest note after the start.
                    if (bestEncTick < 0 || static_cast<int>(em->tick) < bestEncTick) {
                        bestEncTick = static_cast<int>(em->tick);
                        bestDist = 0;
                    }
                    continue;
                }
                const int dist = std::abs(xoff - targetEndXoff);
                if (dist < bestDist) {
                    bestDist = dist;
                    bestEncTick = static_cast<int>(em->tick);
                }
            }
            // Grace-to-main: grace + regular share startEncTick and regular is closest match: zero-span.
            // If a later note is closer, resolve as grace-to-later instead.
            // For a v0xC2 short slur the next-note rule sets bestDist=0, so the distance
            // comparison can never pick grace-to-main; a grace at the start is the strong
            // signal that the slur ornaments its own main note, so prefer zero-span there.
            if (hasGraceAtStart && regularXoffAtStart >= 0) {
                const int regularDist = std::abs(regularXoffAtStart - targetEndXoff);
                if (v0c2ShortSlur || regularDist < bestDist) {
                    endTick  = ps.startTick;
                    resolved = true;
                }
            }
            if (!resolved && bestEncTick > startEncTick) {
                const Fraction endRel(bestEncTick, wt);
                const Fraction candidate = ctx.measuresByIdx[ps.startMeasIdx]->tick()
                                           + endRel;
                // Snap to chord segment: grace notes steal time, shifting cumTick earlier than proportional tick.
                Measure* sMeas = ctx.measuresByIdx[ps.startMeasIdx];
                Segment* snappedSeg = score->tick2leftSegment(
                    candidate, false, SegmentType::ChordRest);
                if (snappedSeg && snappedSeg->measure() == sMeas
                    && snappedSeg->tick() >= ps.startTick) {
                    bool hasChord = false;
                    for (int v = 0; v < static_cast<int>(VOICES) && !hasChord; ++v) {
                        track_idx_t t = static_cast<track_idx_t>(
                            ps.staffIdx * VOICES + v);
                        if (snappedSeg->element(t)
                            && snappedSeg->element(t)->isChord()) {
                            hasChord = true;
                        }
                    }
                    endTick = hasChord ? snappedSeg->tick() : candidate;
                } else {
                    endTick = candidate;
                }
                resolved = true;
            }
            // Cross-measure extension when alMezuro is unreliable and the arc endpoint clearly
            // exceeds the start measure. Excluded for tiny-pixelspan slurs (ornament placed after
            // first note) because their targetEndXoff is slurXoffset2, which may be in the next
            // measure's coordinate space and would produce a false positive. Also excluded when
            // the same-measure search already resolved to a zero-span (grace-to-main) endpoint.
            if (!ps.alMezuroValid && !usedTinyPixelSpan && bestDist > 0
                && (targetEndXoff > maxXoffInMeas || bestEncTick < 0)
                && !(resolved && endTick == ps.startTick)) {
                if (auto ext = extendSlurToLaterMeasures(ctx, ps, lineSlotByRawByte,
                                                         targetEndXoff, bestDist)) {
                    endTick = *ext;
                    resolved = true;
                }
            }
        } // integrated endpoint-search block
    }
    return resolved ? std::optional<Fraction>(endTick) : std::nullopt;
}

void resolveSlurs(BuildCtx& ctx)
{
    MasterScore* score = ctx.score;
    const EncRoot& enc = ctx.enc;

    // Raw-staff-byte to LINE-slot lookup (shared with the emitter's staff/voice routing).
    std::array<int, 256> lineSlotByRawByte;
    buildLineSlotByRawByte(enc, lineSlotByRawByte);

    // Slurs whose endpoints were set explicitly (reliable measure-count path); excluded from
    // the recompute in removeOrphanSlurs.
    std::set<const Spanner*> explicitSlurs;

    // The v0xC2 slur measure-count (element +16) is unreliable: some files store noise or a
    // per-staff constant there rather than a per-slur forward count. Two tells mark the whole
    // file's field as junk, so every slur then resolves by the xoffset heuristic: (1) any count
    // pointing past the last measure, and (2) the same multi-measure count repeated at different
    // start measures (a real span varies per slur). See ENCORE_FORMAT.md §Slur.
    bool v0c2SlurCountUnreliable = false;
    if (enc.fmt->slurXoffset2Stale()) {
        const int measCount = static_cast<int>(ctx.measuresByIdx.size());
        std::map<int, std::set<int> > startMeasuresByCount;   // alMezuro value -> distinct start measures
        for (const PendingSlur& ps : ctx.pendingSlurs) {
            if (!ps.alMezuroValid || ps.alMezuro <= 0) {
                continue;
            }
            if (ps.startMeasIdx + ps.alMezuro >= measCount) {
                v0c2SlurCountUnreliable = true;
                break;
            }
            // A multi-measure span (>= 3) repeated at two or more different start measures is a
            // constant, not a per-slur count. Small spans (1-2 measures) legitimately recur.
            if (ps.alMezuro >= 3) {
                startMeasuresByCount[ps.alMezuro].insert(ps.startMeasIdx);
                if (startMeasuresByCount[ps.alMezuro].size() >= 2) {
                    v0c2SlurCountUnreliable = true;
                    break;
                }
            }
        }
    }

    // .enc has no SLURSTOP; endpoint derived from alMezuro (target measure) + xoffset heuristic.
    for (PendingSlur ps : ctx.pendingSlurs) {
        // File-level: +16 is noise here, so drop the count and let the heuristic anchor the arc.
        if (v0c2SlurCountUnreliable) {
            ps.alMezuroValid = false;
            ps.alMezuro = 0;
            ps.endMeasIdx = ps.startMeasIdx;
        }
        // When alMezuro is not a reliable measure count, clamp to the start measure
        // so the same-measure xoffset heuristic handles it.
        int clampedEndMeasIdx = ps.endMeasIdx;
        if (!ps.alMezuroValid && ps.startMeasIdx >= 0
            && ps.startMeasIdx < static_cast<int>(ctx.measuresByIdx.size())) {
            clampedEndMeasIdx = ps.startMeasIdx;
        }
        if (clampedEndMeasIdx < 0
            || clampedEndMeasIdx >= static_cast<int>(ctx.measuresByIdx.size())) {
            continue;
        }
        Measure* endMeas = ctx.measuresByIdx[clampedEndMeasIdx];
        Fraction endTick;
        bool resolved = false;

        // v0xC2 reliable forward measure-count (element +16): Encore draws these as
        // note-1-to-note-1 arcs between bar starts; xoffset2 is stale in this format, so anchor
        // explicitly to the downbeat chord of the target measure rather than guessing by
        // coordinate. v0xC4/SCO5 keep the xoffset2 heuristic (reliable there).
        // tick2segment is unreliable at bar boundaries (computeEndElement returns null there),
        // so locate both endpoints by iterating ChordRest segments and set the slur elements
        // explicitly; these are protected from recompute in removeOrphanSlurs.
        if (enc.fmt->slurXoffset2Stale() && ps.alMezuroValid && ps.alMezuro > 0
            && ps.startMeasIdx >= 0
            && ps.startMeasIdx < static_cast<int>(ctx.measuresByIdx.size())) {
            Measure* startMeas = ctx.measuresByIdx[ps.startMeasIdx];
            track_idx_t st = 0, et = 0;
            Chord* sc = firstChordOnStaffFrom(score, startMeas, ps.staffIdx, ps.startTick, st);
            Chord* ec = firstChordOnStaffFrom(score, endMeas, ps.staffIdx, endMeas->tick(), et);
            if (sc && ec && ec->tick() > sc->tick()) {
                Slur* slur = Factory::createSlur(score->dummy());
                slur->setTrack(st);
                slur->setTrack2(et);
                slur->setTick(sc->tick());
                slur->setTick2(ec->tick());
                slur->setStartElement(sc);
                slur->setEndElement(ec);
                score->addSpanner(slur, false);   // explicit endpoints: skip recompute
                explicitSlurs.insert(slur);
                continue;
            }
        }

        if (auto t = resolveSameMeasureHeuristic(ctx, ps, lineSlotByRawByte)) {
            endTick = *t;
            resolved = true;
        }

        const track_idx_t startTrack = resolveChordTrack(score, ps.startTick, ps.staffIdx, ps.track);

        // Fallback 1: cross-measure xoffset2 matching.
        if (!resolved && clampedEndMeasIdx < static_cast<int>(enc.measures.size())) {
            if (auto t = resolveCrossMeasureXoffset(ps, enc.measures[clampedEndMeasIdx],
                                                    endMeas, lineSlotByRawByte)) {
                endTick = *t;
                resolved = true;
            }
        }

        // Fallback 2: last chord in target measure.
        if (!resolved) {
            if (auto t = resolveLastChordInMeasure(ps, endMeas)) {
                endTick = *t;
            } else {
                continue;  // no chord on this staff: skip slur
            }
        }

        if (endTick < ps.startTick) {
            continue;   // negative span: always drop
        }
        if (endTick == ps.startTick) {
            createGraceToMainSlur(ps, score, ps.startTick);
            continue;   // handled as grace-to-main or dropped (no grace notes)
        }
        const track_idx_t endTrack = resolveChordTrack(score, endTick, ps.staffIdx, startTrack);
        createNormalSlur(ps, startTrack, endTrack, endTick, score);
    }

    // Remove slurs with missing start/end note (corrupted files cause NaN in Bezier layout).
    removeOrphanSlurs(score, explicitSlurs);
}
} // namespace mu::iex::enc
