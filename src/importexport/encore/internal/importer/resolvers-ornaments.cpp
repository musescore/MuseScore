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

// Post-pass: place pending ornaments, fermatas, tremolos, trills, arpeggios and breaths.

#include "resolvers.h"
#include "../parser/elem.h"
#include "engraving/dom/arpeggio.h"
#include "engraving/dom/tremolosinglechord.h"
#include "engraving/dom/ornament.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/note.h"
#include "engraving/dom/tie.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/marker.h"
#include "engraving/dom/articulation.h"
#include "engraving/dom/fermata.h"
#include "engraving/dom/breath.h"
#include "engraving/dom/measurerepeat.h"
#include "engraving/dom/trill.h"
#include "engraving/editing/editmeasurerepeat.h"
#include "engraving/editing/transaction/transaction.h"

using namespace mu::engraving;

namespace mu::iex::enc {
static void resolveArpeggios(MasterScore* score,
                             const std::vector<PendingArpeggio>& pendingArpeggios)
{
    // ORN precedes chord in MEAS order, so deferred to resolve phase.
    for (const PendingArpeggio& pa : pendingArpeggios) {
        Chord* c = findChordAt(score, pa.tick, pa.track);
        if (!c || c->arpeggio()) {
            continue;
        }
        Arpeggio* arp = Factory::createArpeggio(c);
        arp->setTrack(pa.track);
        arp->setArpeggioType(ArpeggioType::NORMAL);
        c->add(arp);
    }
}

// Locate the Chord a single-chord tremolo ORN belongs to. The ORN's stored tick/voice are
// unreliable (Encore may place it at durTicks or in voice 0 regardless of the note's real voice),
// so the tick->measure->voice fallbacks try, in order: the exact (tick, track) segment; the last
// chord on that track in the source measure; then any voice on that staff. Returns nullptr when no
// chord is found. See ENCORE_FORMAT.md §Ornament element.
static Chord* findChordForTremolo(MasterScore* score, const PendingOrnTremolo& pt)
{
    // staffIdx/msVoice come from the file; reject an out-of-range staff before deriving tracks.
    if (!validTrack(score, static_cast<track_idx_t>(pt.staffIdx) * VOICES)) {
        return nullptr;
    }
    const track_idx_t trTrack = static_cast<track_idx_t>(pt.staffIdx * VOICES + pt.msVoice);
    Measure* m = score->tick2measure(pt.tick);
    if (!m) {
        m = score->tick2measure(pt.measTick);
    }
    if (!m) {
        return nullptr;
    }
    Segment* seg = m->findSegment(SegmentType::ChordRest, pt.tick);
    if (!seg || !seg->element(trTrack) || !seg->element(trTrack)->isChord()) {
        Measure* srcMeas = score->tick2measure(pt.measTick);
        if (!srcMeas) {
            srcMeas = m;
        }
        seg = nullptr;
        for (Segment* s = srcMeas->first(SegmentType::ChordRest); s;
             s = s->next(SegmentType::ChordRest)) {
            if (s->element(trTrack) && s->element(trTrack)->isChord()) {
                seg = s;
            }
        }
    }
    track_idx_t resolvedTrack = trTrack;
    if (!seg || !seg->element(resolvedTrack) || !seg->element(resolvedTrack)->isChord()) {
        Measure* srcMeas = score->tick2measure(pt.measTick);
        if (!srcMeas) {
            srcMeas = m;
        }
        if (srcMeas) {
            for (int v = 0; v < static_cast<int>(VOICES) && !seg; ++v) {
                const track_idx_t altTrack = static_cast<track_idx_t>(pt.staffIdx * VOICES + v);
                for (Segment* s = srcMeas->first(SegmentType::ChordRest); s;
                     s = s->next(SegmentType::ChordRest)) {
                    if (s->element(altTrack) && s->element(altTrack)->isChord()) {
                        seg = s;
                        resolvedTrack = altTrack;
                    }
                }
            }
        }
    }
    if (!seg || !seg->element(resolvedTrack)) {
        return nullptr;
    }
    EngravingItem* el = seg->element(resolvedTrack);
    if (!el || !el->isChord()) {
        return nullptr;
    }
    return toChord(el);
}

static void resolveSingleChordTremolos(MasterScore* score,
                                       const std::vector<PendingOrnTremolo>& pendingOrnTremolos)
{
    for (const PendingOrnTremolo& pt : pendingOrnTremolos) {
        Chord* c = findChordForTremolo(score, pt);
        if (!c) {
            continue;
        }
        // Encore places the tremolo ORN after the tied-from note; walk back to tie start.
        if (!c->notes().empty() && c->notes().front()->tieBack()) {
            Chord* prev = c->notes().front()->tieBack()->startNote()->chord();
            if (prev) {
                c = prev;
            }
        }
        if (c->tremoloSingleChord()) {
            continue;
        }
        TremoloSingleChord* trem = Factory::createTremoloSingleChord(c);
        trem->setTremoloType(pt.tremType);
        c->add(trem);
    }
}

static void resolveMarkers(MasterScore* score,
                           const std::vector<PendingMarker>& pendingMarkers)
{
    for (const PendingMarker& pm : pendingMarkers) {
        Measure* m = score->tick2measure(pm.tick);
        if (!m) {
            continue;
        }
        Marker* mk = Factory::createMarker(m);
        mk->setMarkerType(pm.type);
        mk->setTrack(0);
        m->add(mk);
    }
}

// True when seg already carries a Fermata with this symId (a per-note artic byte may have
// produced the same glyph; dedup against it).
static bool segmentHasFermata(const Segment* seg, SymId symId)
{
    for (EngravingItem* e : seg->annotations()) {
        if (e->isFermata() && toFermata(e)->symId() == symId) {
            return true;
        }
    }
    return false;
}

// True when chord c already carries an Articulation with this symId (dedup against a
// per-note artic byte that produced the same glyph).
static bool chordHasArticulation(const Chord* c, SymId symId)
{
    for (Articulation* a : c->articulations()) {
        if (a->symId() == symId) {
            return true;
        }
    }
    return false;
}

static void resolveFermatas(MasterScore* score,
                            const std::vector<PendingFermata>& pendingFermatas)
{
    for (const PendingFermata& pf : pendingFermatas) {
        Chord* c = findChordAt(score, pf.tick, pf.track);
        if (!c) {
            continue;
        }
        Segment* seg = c->segment();
        if (segmentHasFermata(seg, pf.symId)) {
            continue;
        }
        const bool isBelow = (pf.symId == SymId::fermataBelow
                              || pf.symId == SymId::fermataShortBelow
                              || pf.symId == SymId::fermataLongBelow);
        Fermata* fermata = Factory::createFermata(seg);
        fermata->setTrack(pf.track);
        fermata->setSymId(pf.symId);
        fermata->setPlacement(isBelow ? PlacementV::BELOW : PlacementV::ABOVE);
        fermata->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);
        seg->add(fermata);
    }
}

static void resolveStaccatos(MasterScore* score,
                             const std::vector<PendingStaccato>& pendingStaccatos)
{
    // Dedup: artic byte 0x1D produces the same glyph.
    for (const PendingStaccato& ps : pendingStaccatos) {
        Chord* c = findChordAt(score, ps.tick, ps.track);
        if (!c) {
            continue;
        }
        if (chordHasArticulation(c, SymId::articStaccatoAbove)
            || chordHasArticulation(c, SymId::articStaccatoBelow)) {
            continue;
        }
        Articulation* art = Factory::createArticulation(c);
        art->setTrack(ps.track);
        art->setSymId(SymId::articStaccatoAbove);
        c->add(art);
    }
}

// True when a non-alt trill on the same track starts before pt, so pt's alt glyph sits within
// that earlier trill's span (Ornament glyph only, no new spanner).
static bool hasEarlierTrillStart(const std::vector<PendingTrill>& pendingTrills, const PendingTrill& pt)
{
    for (const PendingTrill& other : pendingTrills) {
        if (!other.isAlt && other.track == pt.track && other.tick < pt.tick) {
            return true;
        }
    }
    return false;
}

static void resolveTrillsWithSpans(MasterScore* score,
                                   const std::vector<PendingTrill>& pendingTrills,
                                   std::map<track_idx_t, std::vector<Fraction> >& pendingTrillEnds,
                                   const std::vector<Measure*>& measuresByIdx)
{
    // (A) TRILL_ALT within a prior TRILL_START span: Ornament glyph only.
    // (B) TRILL_ALT standalone: spanner on note duration.
    // (C) TRILL_START: spanner when endpoint found; glyph otherwise.
    for (const PendingTrill& pt : pendingTrills) {
        Fraction trillTick = pt.tick;
        Chord* trillChord = findChordAt(score, trillTick, pt.track);
        if (!trillChord) {
            // TRILL_SIMPLE may land on a rest tick; snap forward to next chord in measure.
            if (pt.isAlt) {
                Measure* m = score->tick2measure(pt.tick);
                if (m) {
                    for (Segment* s = m->first(SegmentType::ChordRest); s;
                         s = s->next(SegmentType::ChordRest)) {
                        if (s->tick() < pt.tick) {
                            continue;
                        }
                        EngravingItem* el = s->element(pt.track);
                        if (el && el->isChord()) {
                            trillChord = toChord(el);
                            trillTick = s->tick();
                            break;
                        }
                    }
                }
            }
            if (!trillChord) {
                continue;
            }
        }

        const bool altWithinSpan = pt.isAlt && hasEarlierTrillStart(pendingTrills, pt);
        const bool standaloneAlt = pt.isAlt && !altWithinSpan;

        Fraction endTick;
        bool hasSpan = !altWithinSpan;

        if (hasSpan) {
            hasSpan = false;
            auto it = pendingTrillEnds.find(pt.track);
            if (it != pendingTrillEnds.end()) {
                auto& endVec = it->second;
                for (auto eit = endVec.begin(); eit != endVec.end(); ++eit) {
                    if (*eit > trillTick) {
                        endTick = *eit;
                        hasSpan = true;
                        endVec.erase(eit);
                        break;
                    }
                }
            }
            // Cross-measure span via alMezuro field.
            if (!hasSpan && pt.alMezuro > 0) {
                const size_t endMeasIdx = pt.measIdx + static_cast<size_t>(pt.alMezuro);
                if (endMeasIdx < measuresByIdx.size()) {
                    Measure* endMeas = measuresByIdx[endMeasIdx];
                    if (endMeas) {
                        endTick = endMeas->endTick();
                        hasSpan = true;
                    }
                }
            }
        }

        if (pt.isSimple) {
            hasSpan = false;
        } else if (standaloneAlt && (!hasSpan || endTick <= trillTick)) {
            const Fraction noteDuration = trillChord->actualTicks();
            if (!noteDuration.isZero()) {
                endTick = trillChord->tick() + noteDuration;
                hasSpan = true;
            }
        }

        if (hasSpan && endTick > trillTick) {
            Trill* trill = Factory::createTrill(score->dummy());
            trill->setTrack(pt.track);
            trill->setTrack2(pt.track);
            trill->setTick(trillTick);
            trill->setTick2(endTick);
            trill->setTrillType(TrillType::TRILL_LINE);
            score->addElement(trill);
        } else {
            const SymId sid = pt.isSimple ? pt.simpleSymId : SymId::ornamentTrill;
            bool alreadyHas = false;
            for (Articulation* a : trillChord->articulations()) {
                if (a && a->isOrnament() && toOrnament(a)->symId() == sid) {
                    alreadyHas = true;
                    break;
                }
            }
            if (!alreadyHas) {
                Ornament* orn = Factory::createOrnament(trillChord);
                orn->setTrack(pt.track);
                orn->setSymId(sid);
                trillChord->add(orn);
            }
        }
    }
}

static void resolveUnconsumedTrillEnds(MasterScore* score,
                                       std::map<track_idx_t, std::vector<Fraction> >& pendingTrillEnds)
{
    // TRILL_END markers not consumed by TRILL_START: create a spanner on the note's duration.
    for (auto& [trTrack, endTicks] : pendingTrillEnds) {
        for (const Fraction& eTick : endTicks) {
            Chord* c = findChordAt(score, eTick, trTrack);
            if (!c) {
                continue;
            }
            const Fraction noteDuration = c->actualTicks();
            if (noteDuration.isZero()) {
                continue;
            }
            Trill* trill = Factory::createTrill(score->dummy());
            trill->setTrack(trTrack);
            trill->setTrack2(trTrack);
            trill->setTick(c->tick());
            trill->setTick2(c->tick() + noteDuration);
            trill->setTrillType(TrillType::TRILL_LINE);
            score->addElement(trill);
        }
    }
    pendingTrillEnds.clear();
}

static void resolveBreaths(MasterScore* score,
                           const std::vector<PendingBreath>& pendingBreaths)
{
    // pb.tick is the following note; attach after the preceding chord.
    // If pb.tick is at a measure boundary, that chord is in the prior measure.
    for (const PendingBreath& pb : pendingBreaths) {
        Measure* m = score->tick2measure(pb.tick);
        if (m && m->tick() == pb.tick) {
            MeasureBase* prevBase = m->prev();
            while (prevBase && !prevBase->isMeasure()) {
                prevBase = prevBase->prev();
            }
            if (prevBase) {
                m = toMeasure(prevBase);
            }
        }
        if (!m) {
            continue;
        }
        Chord* prevChord = nullptr;
        for (Segment* s = m->first(SegmentType::ChordRest); s;
             s = s->next(SegmentType::ChordRest)) {
            EngravingItem* el = s->element(pb.track);
            if (el && el->isChord()) {
                Chord* c = toChord(el);
                if (c->tick() + c->actualTicks() <= pb.tick) {
                    prevChord = c;
                }
            }
        }
        const Fraction breathTick = prevChord
                                    ? prevChord->tick() + prevChord->actualTicks()
                                    : pb.tick;
        Measure* breathMeasure = prevChord ? prevChord->measure() : m;
        if (!breathMeasure) {
            continue;
        }
        Segment* seg = breathMeasure->getSegment(SegmentType::Breath, breathTick);
        Breath* breath = Factory::createBreath(seg);
        breath->setTrack(pb.track);
        breath->setSymId(pb.symId);
        breath->setPlacement(PlacementV::ABOVE);
        breath->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);
        seg->add(breath);
    }
}

static void resolveMeasureRepeats(MasterScore* score,
                                  const std::vector<PendingMeasureRepeat>& pendingMeasureRepeats)
{
    // Replace measure content with "%" symbol.
    for (const PendingMeasureRepeat& pmr : pendingMeasureRepeats) {
        Measure* m = score->tick2measure(pmr.measTick);
        if (!m) {
            continue;
        }
        const track_idx_t track = static_cast<track_idx_t>(pmr.staffIdx) * VOICES;
        Segment* firstSeg = m->first(SegmentType::ChordRest);
        if (!firstSeg) {
            continue;
        }
        Staff* st = score->staff(static_cast<staff_idx_t>(pmr.staffIdx));
        if (!st) {
            continue;
        }
        score->makeGap(firstSeg, track, m->stretchedLen(st), nullptr);
        EditMeasureRepeat::addMeasureRepeat(score->transactionManager()->currentOrDummyTransaction(), score, m->tick(), track, 1);
        m->setMeasureRepeatCount(1, static_cast<staff_idx_t>(pmr.staffIdx));
    }
}

void resolveOrnaments(BuildCtx& ctx)
{
    MasterScore* score = ctx.score;
    resolveArpeggios(score, ctx.pendingArpeggios);
    resolveSingleChordTremolos(score, ctx.pendingOrnTremolos);
    resolveMarkers(score, ctx.pendingMarkers);
    resolveFermatas(score, ctx.pendingFermatas);
    resolveStaccatos(score, ctx.pendingStaccatos);
    resolveTrillsWithSpans(score, ctx.pendingTrills, ctx.pendingTrillEnds, ctx.measuresByIdx);
    resolveUnconsumedTrillEnds(score, ctx.pendingTrillEnds);
    resolveBreaths(score, ctx.pendingBreaths);
    resolveMeasureRepeats(score, ctx.pendingMeasureRepeats);
}
} // namespace mu::iex::enc
