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

// Emit driver: walk each measure, route every element to its (staff, voice, tick), dispatch to the
// type handler, and run the per-measure tuplet-fill, key/clef/volta and overfull passes.

#include "ctx.h"
#include "import.h"
#include "emitters-internal.h"
#include "../parser/elem.h"
#include "../parser/readers.h"
#include "mappers.h"
#include "../parser/ticks.h"
#include "emitters-tuplets.h"
#include <algorithm>
#include <memory>
#include <map>
#include <set>
#include <vector>
#include <QDataStream>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include "engraving/dom/arpeggio.h"
#include "engraving/dom/box.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/dynamic.h"
#include "engraving/dom/fermata.h"
#include "engraving/dom/fingering.h"
#include "engraving/dom/ornament.h"
#include "engraving/dom/tremolosinglechord.h"
#include "engraving/dom/clef.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/harmony.h"
#include "engraving/dom/jump.h"
#include "engraving/dom/key.h"
#include "engraving/dom/keysig.h"
#include "engraving/editing/transpose.h"
#include "engraving/dom/lyrics.h"
#include "engraving/dom/marker.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/drumset.h"
#include "engraving/dom/note.h"
#include "engraving/dom/instrtemplate.h"
#include "engraving/dom/instrument.h"
#include "engraving/dom/part.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/slur.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafftext.h"
#include "engraving/dom/tempotext.h"
#include "engraving/dom/text.h"
#include "engraving/dom/tie.h"
#include "engraving/dom/timesig.h"
#include "engraving/dom/tuplet.h"
#include "engraving/dom/volta.h"
#include "engraving/engravingerrors.h"
#include "log.h"

namespace mu::iex::enc {
bool MeasEmitCtx::isTieStartAt(int si, int v, int tick, int notePosition) const
{
    auto checkAt = [&](int t) {
        auto range = tieStartSet.equal_range({ si, v, t });
        for (auto it = range.first; it != range.second; ++it) {
            // notePosition<0 = match any; sourcePosition<0 = any note in chord
            if (notePosition < 0 || it->second < 0 || it->second == static_cast<int8_t>(notePosition)) {
                return true;
            }
        }
        return false;
    };
    for (int dt = 0; dt < CHORD_CLUSTER_THRESHOLD; ++dt) {
        if (checkAt(tick - dt)) {
            return true;
        }
        if (dt > 0 && checkAt(tick + dt)) {
            return true;
        }
    }
    return false;
}

void MeasEmitCtx::closeTupletWithFill(BuildCtx& ctx, TupletTracker& tt,
                                      std::pair<int, int> trackKey)
{
    if (!tt.inTuplet() || tt.placedTicks <= Fraction(0, 1)) {
        tt.closeTuplet();
        return;
    }
    const Fraction expectedTup = TDuration(tt.currentTuplet->baseLen()).fraction()
                                 * tt.currentTuplet->ratio().denominator();
    TDuration snap(tt.placedTicks, true /*truncate*/);
    const bool fitsTD = snap.isValid() && snap.fraction() == tt.placedTicks;
    if (tt.placedTicks < expectedTup && !fitsTD) {
        const bool countShort = (static_cast<int>(tt.currentTuplet->elements().size()) < tt.actualN);
        // Mixed-duration group: element count reaches actualN but the face-value sum is short.
        const bool faceShort = (tt.fullFaceSum > Fraction(0, 1) && tt.faceTicks < tt.fullFaceSum);
        if (countShort || faceShort) {
            track_idx_t trk = static_cast<track_idx_t>(trackKey.first) * VOICES
                              + trackKey.second;
            DurationType baseLen = tt.currentTuplet->baseLen().type();
            Fraction perNote = TDuration(baseLen).fraction()
                               * Fraction(tt.normalN, tt.actualN);
            DurationType fillDurType = baseLen;
            // Mixed-duration fill: derive from remaining face value rather than baseLen.
            if (faceShort && !countShort) {
                const Fraction remFace = tt.fullFaceSum - tt.faceTicks;
                TDuration remDur(remFace, true /*truncate*/);
                if (remDur.isValid() && remDur.fraction() == remFace) {
                    fillDurType = remDur.type();
                    perNote = remFace * Fraction(tt.normalN, tt.actualN);
                }
            }
            int safety = tt.actualN + 1;
            while (tt.placedTicks < expectedTup && safety-- > 0
                   && (static_cast<int>(tt.currentTuplet->elements().size()) < tt.actualN
                       || (faceShort && tt.faceTicks < tt.fullFaceSum))
                   && ctx.scratch.cumTick[trackKey] + perNote <= measure->ticks()) {
                Fraction restTick = measure->tick() + ctx.scratch.cumTick[trackKey];
                Segment* seg = measure->getSegment(SegmentType::ChordRest, restTick);
                if (!seg) {
                    break;
                }
                if (seg->element(trk)) {
                    break;
                }
                TDuration dur(fillDurType);
                Rest* rest = Factory::createRest(seg, dur);
                rest->setTrack(trk);
                rest->setTicks(dur.fraction());
                rest->setVisible(false);
                rest->setTuplet(tt.currentTuplet);
                tt.currentTuplet->add(rest);
                seg->add(rest);
                tt.placedTicks += perNote;
                ctx.scratch.cumTick[trackKey] += perNote;
            }
        }
    }
    tt.closeTuplet();
}

struct DeferredKeySig {
    Key writtenKey;
    Key concertKey;
    int staffIdx { 0 };
};

static bool hasPitchedNotes(const EncMeasure& m)
{
    for (const auto& elem : m.elements) {
        if (static_cast<EncElemType>(elem->type) == EncElemType::NOTE) {
            return true;
        }
    }
    return false;
}

static bool hasMultiRest(const EncMeasure& m)
{
    if (m.elements.empty()) {
        return false;
    }
    for (const auto& ep : m.elements) {
        if (static_cast<EncElemType>(ep->type) != EncElemType::REST) {
            return false;
        }
    }
    return static_cast<const EncRest*>(m.elements[0].get())->mrestCount > 1;
}

static int measDisplayCount(const EncMeasure& m, const EncMeasure* prev)
{
    if (m.elements.empty()) {
        return 1;
    }
    for (const auto& ep : m.elements) {
        if (static_cast<EncElemType>(ep->type) != EncElemType::REST) {
            return 1;
        }
    }
    const int cnt = static_cast<int>(static_cast<const EncRest*>(m.elements[0].get())->mrestCount);
    if (cnt <= 1) {
        return 1;
    }
    if (prev && hasMultiRest(*prev)) {
        return 1;
    }
    return cnt;
}

static void sortMeasureElements(const EncMeasure& encMeas, MeasureElemRefVec& sortedElems)
{
    sortedElems.reserve(encMeas.elements.size());
    for (const auto& elem : encMeas.elements) {
        sortedElems.push_back(elem.get());
    }
    std::stable_sort(sortedElems.begin(), sortedElems.end(),
                     [](const EncMeasureElem* a, const EncMeasureElem* b) {
        if (a->tick != b->tick) {
            return a->tick < b->tick;
        }
        bool aIsNote = (static_cast<EncElemType>(a->type) == EncElemType::NOTE
                        || static_cast<EncElemType>(a->type) == EncElemType::REST);
        bool bIsNote = (static_cast<EncElemType>(b->type) == EncElemType::NOTE
                        || static_cast<EncElemType>(b->type) == EncElemType::REST);
        if (aIsNote != bIsNote) {
            return !aIsNote;  // non-notes before notes
        }
        if (!aIsNote) {
            return false;     // both non-notes: preserve stable order
        }
        // Among notes at same tick: tuplet notes before non-tuplet notes
        bool aTuplet = a->tupletByte() != 0;
        bool bTuplet = b->tupletByte() != 0;
        if (aTuplet != bTuplet) {
            return aTuplet;   // tuplet note first
        }
        // Shortest faceValue first: chord root drives cumTick by minimum step.
        const quint8 aFv = fvLow(a->faceValueByte());
        const quint8 bFv = fvLow(b->faceValueByte());
        if (aFv != bFv) {
            return aFv > bFv;  // higher number = shorter duration = comes first
        }
        return false;         // stable for equal keys
    });
}

static void collectTieStartPositions(const MeasureElemRefVec& sortedElems,
                                     const std::array<int, 256>& lineSlotByRawByte,
                                     int totalStaves, MeasEmitCtx& mc)
{
    for (const EncMeasureElem* e : sortedElems) {
        if (static_cast<EncElemType>(e->type) == EncElemType::TIE) {
            const EncTie* et = static_cast<const EncTie*>(e);
            if (et->isTieStart) {
                int si = static_cast<int>(e->staffIdx);
                int v  = static_cast<int>(e->voice);
                const quint8 tiRawByte = e->rawStaffByte();
                const int tiOrigSW = static_cast<int>(e->staffWithin);
                bool tiResolved = false;
                if (lineSlotByRawByte[tiRawByte] >= 0) {
                    si = lineSlotByRawByte[tiRawByte];
                    tiResolved = true;
                    if (tiOrigSW > 0 && v < static_cast<int>(VOICES)) {
                        const int vBase = tiOrigSW * (static_cast<int>(VOICES) / 2);
                        if (v >= vBase) {
                            v -= vBase;
                        }
                    }
                }
                if (v >= static_cast<int>(VOICES)) {
                    v = 0;
                } else if (!tiResolved && tiOrigSW > 0) {
                    const int sw = tiOrigSW;
                    const int vBase = sw * (static_cast<int>(VOICES) / 2);
                    if (v >= vBase && si + sw < totalStaves) {
                        si += sw;
                        v  -= vBase;
                    }
                }
                mc.tieStartSet.insert({ { si, v, (int)e->tick }, et->sourcePosition });
            }
        }
    }
}

static void scanMeasureMetadata(const MeasureElemRefVec& sortedElems, MeasEmitCtx& mc)
{
    for (const EncMeasureElem* em : sortedElems) {
        const EncElemType et2 = static_cast<EncElemType>(em->type);
        if (et2 == EncElemType::NOTE) {
            mc.noteTicks.insert(static_cast<int>(em->tick));
            mc.noteStaffVoiceTicks.insert({ static_cast<int>(em->staffIdx),
                                            static_cast<int>(em->voice), static_cast<int>(em->tick) });
            if (em->voice >= static_cast<int>(VOICES)) {
                mc.voice4NoteTicks.insert(static_cast<int>(em->tick));
            } else {
                mc.v0NoteCountAtTick[static_cast<int>(em->tick)]++;
                mc.maxVoice0Tick = std::max(mc.maxVoice0Tick, static_cast<int>(em->tick));
                mc.stavesWithRealNote.insert(static_cast<int>(em->staffIdx));
            }
            // Detect scale string number anchors (au in 0x39..0x40)
            const EncNote* enPre = static_cast<const EncNote*>(em);
            if (encArticByteToScaleStringNumber(enPre->articulationUp) > 0
                || encArticByteToScaleStringNumber(enPre->articulationDown) > 0) {
                mc.hasScaleStringAnchors = true;
            }
        } else if (et2 == EncElemType::ORNAMENT) {
            const EncOrnament* eo2 = static_cast<const EncOrnament*>(em);
            const EncOrnamentType ot2 = eo2->ornType();
            if (ot2 >= EncOrnamentType::FINGER_1 && ot2 <= EncOrnamentType::FINGER_5) {
                mc.ornFingCountAtTick[static_cast<int>(em->tick)]++;
            }
        }
    }
}

// Returns false if the element should be skipped entirely (tick out of range).
static bool shouldIncludeElement(const EncMeasureElem* e, const EncMeasure& encMeas)
{
    const EncElemType et = static_cast<EncElemType>(e->type);
    // Notes/rests at or beyond measure end are dropped; ORNs at durTicks are end-of-measure markers
    // (e.g. hairpin on barline) and must not be dropped.
    if ((et == EncElemType::NOTE || et == EncElemType::REST)
        && e->tick >= encMeas.durTicks) {
        return false;
    }
    if (et == EncElemType::ORNAMENT && e->tick > encMeas.durTicks) {
        // Past-durTicks dynamics/STAFFTEXT: let through and clamp to the last segment.
        const EncOrnament* eoFilt = static_cast<const EncOrnament*>(e);
        const EncOrnamentType ot = eoFilt->ornType();
        const bool isDyn = (ot >= EncOrnamentType::DYN_PPP
                            && ot <= EncOrnamentType::DYN_FP)
                           || ot == EncOrnamentType::DYN_FZ
                           || ot == EncOrnamentType::DYN_SF;
        const bool isText = (ot == EncOrnamentType::STAFFTEXT);
        if (!isDyn && !isText) {
            return false;
        }
    }
    return true;
}

// Returns false if the element should be skipped (staffIdx out of range or voice invalid).
// On success, fills staffIdx, voice, msVoice, track, trackKey, encVoiceKey.
std::optional<RoutedTrack> routeElementStaffVoice(
    const EncMeasureElem* e,
    bool isNoteOrRest,
    const std::array<int, 256>& lineSlotByRawByte,
    const MeasEmitCtx& mc,
    const BuildCtx& ctx)
{
    const EncRoot& enc = ctx.enc;
    const int nLineStaves = mc.nLineStaves;
    const std::vector<int>& lineStaffInstrIdx = *mc.lineStaffInstrIdx;
    const std::vector<int>& lineStaffWithin   = *mc.lineStaffWithin;

    int staffIdx = static_cast<int>(e->staffIdx);
    int voice    = static_cast<int>(e->voice);

    // Translate rawStaff byte (staffWithin<<6)|instrIdx to LINE slot; apply case-B voice remap when origStaffWithin > 0.
    const quint8 rawNoteStaff = e->rawStaffByte();
    const int origStaffWithin  = static_cast<int>(e->staffWithin);
    bool rawByteResolved = false;
    if (lineSlotByRawByte[rawNoteStaff] >= 0) {
        staffIdx = lineSlotByRawByte[rawNoteStaff];
        rawByteResolved = true;
        if (isNoteOrRest && origStaffWithin > 0 && voice < static_cast<int>(VOICES)) {
            const int vBase = origStaffWithin * (static_cast<int>(VOICES) / 2);
            if (voice >= vBase) {
                voice -= vBase;
            }
        }
    }

    if (staffIdx >= ctx.totalStaves) {
        // The element references a staff the score does not have. This is almost always orphan
        // data from a staff deleted in Encore (its index is not reused, and Encore does not show
        // it). Count it; emitMeasures reports the total once instead of one line per element.
        ++ctx.scratch.droppedByMissingStaff[staffIdx];
        return std::nullopt;
    }
    // Multi-staff routing:
    // (A) voice == VOICES (the canonical staff-2 / silent-voice marker): route to staffIdx+1, voice=0.
    // (A') voice > VOICES (5..7): a genuine extra voice on the SAME staff, mapped into 0..3.
    // (B) staffWithin > 0: route to staffIdx+sw, remap voice down by sw*(VOICES/2).
    if (voice == static_cast<int>(VOICES)) {
        bool routedToSecondStaff = false;
        bool singleStaffInstr = false;
        if (staffIdx < nLineStaves) {
            const int instrIdx = lineStaffInstrIdx[staffIdx];
            if (instrIdx >= 0 && instrIdx < static_cast<int>(enc.instruments.size())) {
                singleStaffInstr = (enc.instruments[instrIdx].nstaves <= 1);
                if (enc.instruments[instrIdx].nstaves > 1
                    && lineStaffWithin[staffIdx] + 1 < enc.instruments[instrIdx].nstaves
                    && staffIdx + 1 < ctx.totalStaves) {
                    staffIdx += 1;
                    routedToSecondStaff = true;
                }
            }
        }
        // Grand-staff: voice 4 is the second staff's silent voice -> voice 0 there. On a single-staff
        // instrument whose voice 0 already has a line, voice 4 is a genuine second melodic voice ->
        // MuseScore voice 1 (kept separate; mergeNonOverlappingVoices may fold it back). If voice 0
        // is empty (an SATB line stored only as voice 4), keep it on voice 0.
        const bool hasOwnVoice0 = mc.stavesWithRealNote.count(static_cast<int>(e->staffIdx)) > 0;
        voice = (singleStaffInstr && !routedToSecondStaff && hasOwnVoice0) ? 1 : 0;
    } else if (voice > static_cast<int>(VOICES)) {
        // Voices 5..7 are extra voices on this SAME staff, not a staff-2 marker: collapse to voice 0
        // without changing the staff.
        voice = 0;
    } else if (!rawByteResolved && origStaffWithin > 0) {
        // Standalone ORNs always have voice=0 and route by staffWithin alone.
        const int sw    = origStaffWithin;
        const int vBase = sw * (static_cast<int>(VOICES) / 2);
        const bool voiceInRange = !isNoteOrRest || voice >= vBase;
        if (voiceInRange && staffIdx < nLineStaves) {
            const int instrIdx = lineStaffInstrIdx[staffIdx];
            if (instrIdx >= 0
                && instrIdx < static_cast<int>(enc.instruments.size())
                && enc.instruments[instrIdx].nstaves > sw
                && staffIdx + sw < ctx.totalStaves) {
                staffIdx += sw;
                if (isNoteOrRest) {
                    voice -= vBase;
                }
            }
        }
    }

    const int msVoice = voice;
    if (msVoice >= static_cast<int>(VOICES)) {
        ++ctx.scratch.droppedByBadVoice;
        return std::nullopt;  // voice out of range
    }
    RoutedTrack r;
    r.staffIdx    = staffIdx;
    r.voice       = voice;
    r.msVoice     = msVoice;
    r.track       = static_cast<track_idx_t>(staffIdx * VOICES + msVoice);
    r.trackKey    = std::make_pair(staffIdx, msVoice);
    r.encVoiceKey = std::make_pair(staffIdx, voice);
    return r;
}

// Returns the MuseScore tick where this element should be placed.
// Has side effects on ctx: updates prevMidiTick, lastChordPos, prevRestTick, noteXoffByMeasStaff.
static Fraction computeElementTick(
    const EncMeasureElem* e,
    bool isNoteOrRest,
    bool isChordExt,
    int voice,
    int staffIdx,
    std::pair<int, int> trackKey,
    const Measure* measure,
    Fraction measTick,
    BuildCtx& ctx,
    const MeasEmitCtx& mc)
{
    constexpr int CHORD_MIDI_THRESHOLD = 2 * CHORD_CLUSTER_THRESHOLD;  // = 8
    const EncElemType et = static_cast<EncElemType>(e->type);
    if (isChordExt) {
        return ctx.scratch.lastChordPos.count(trackKey) ? ctx.scratch.lastChordPos.at(trackKey) : measTick;
    }

    // Gap-snap: when the binary tick is on the face grid and gap > CHORD_MIDI_THRESHOLD, advance
    // cumTick to reveal implicit rests; sub-grid live-recorded ticks never trigger it, preserving
    // multi-stream placement. See ENCORE_IMPORTER.md §Implicit-silence gap snap.
    if (isNoteOrRest) {
        quint8 elemFv = 0;
        if (et == EncElemType::NOTE) {
            elemFv = static_cast<const EncNote*>(e)->faceValue;
        } else if (et == EncElemType::REST) {
            elemFv = static_cast<const EncRest*>(e)->faceValue;
        }
        const int faceTicks = faceValue2ticks(elemFv);
        const bool onFaceGrid = faceTicks > 0
                                && ((int)e->tick % faceTicks) == 0;
        // Suppress gap-snap: (a) grace pending (v0xA6 ticks on face grid but no real time),
        // (b) inside active tuplet (apparent gap is tuplet-internal timing artifact),
        // (c) gap equals stolen grace ticks (grace-displaced note must not fire spurious rest).
        const bool gracePending = !ctx.scratch.pendingGraces[trackKey].empty();
        const bool inActiveTuplet = ctx.scratch.tuplets.count(trackKey)
                                    && ctx.scratch.tuplets.at(trackKey).inTuplet();
        const int stolenTicks = ctx.scratch.graceStolenTicks.count(trackKey)
                                ? ctx.scratch.graceStolenTicks.at(trackKey) : 0;
        if (onFaceGrid && !gracePending && !inActiveTuplet) {
            // Use kEncWholeTicks: the beatTicks*timeSigDen formula breaks for
            // non-standard beatTicks (e.g. 2/2 with beatTicks=240 gives 480).
            const Fraction encTickFrac((int)e->tick, kEncWholeTicks);
            if (encTickFrac > ctx.scratch.cumTick[trackKey]) {
                const Fraction gap = encTickFrac - ctx.scratch.cumTick[trackKey];
                const int gapEncTicks
                    = (gap.numerator() * kEncWholeTicks)
                      / std::max(1, gap.denominator());
                const bool gapIsGraceArtifact
                    = (stolenTicks > 0 && gapEncTicks <= stolenTicks);
                if (gapEncTicks > CHORD_MIDI_THRESHOLD && !gapIsGraceArtifact
                    && encTickFrac < measure->ticks()) {
                    ctx.scratch.cumTick[trackKey] = encTickFrac;
                }
            }
        }
    }

    const Fraction elemTick = measTick + ctx.scratch.cumTick[trackKey];
    if (isNoteOrRest) {
        ctx.scratch.lastChordPos[trackKey] = elemTick;
    }
    // Rests don't set prevMidiTick: a note after a rest is a fresh cluster.
    if (et == EncElemType::NOTE) {
        ctx.scratch.prevMidiTick[trackKey] = e->tick;
        ctx.scratch.prevEncVoice[trackKey] = voice;
        ctx.scratch.prevXoffset[trackKey] = static_cast<int>(e->xoffset);
        // Record note xoffset for bowing-mark cluster resolution.
        const auto* en = static_cast<const EncNote*>(e);
        auto& vec = ctx.noteXoffByMeasStaff[{ mc.measIdx, staffIdx }];
        const int encTick = static_cast<int>(e->tick);
        const int xoff = static_cast<int>(en->xoffset);
        bool already = false;
        for (const auto& p : vec) {
            if (p.first == encTick) {
                already = true;
                break;
            }
        }
        if (!already) {
            vec.push_back({ encTick, xoff });
        }
    } else if (et == EncElemType::REST) {
        ctx.scratch.prevRestTick[trackKey] = static_cast<int>(e->tick);
    }
    return elemTick;
}

static void initLineStaffMappings(
    const EncRoot& enc,
    int nLineStaves,
    std::vector<int>& lineStaffInstrIdx,
    std::vector<int>& lineStaffWithin,
    std::array<int, 256>& lineSlotByRawByte)
{
    lineStaffInstrIdx.assign(nLineStaves, -1);
    lineStaffWithin.assign(nLineStaves, 0);
    for (int s = 0; s < nLineStaves; ++s) {
        lineStaffInstrIdx[s] = static_cast<int>(enc.lines[0].staffData[s].instrumentIndex());
        lineStaffWithin[s]   = static_cast<int>(enc.lines[0].staffData[s].staffIndex());
    }

    buildLineSlotByRawByte(enc, lineSlotByRawByte);
}

static void fillExpandedMrestMeasure(Measure* vm, int totalStaves)
{
    const Fraction vmTick = vm->tick();
    const Fraction vmLen  = vm->ticks();
    for (int si = 0; si < totalStaves; ++si) {
        const track_idx_t tr = static_cast<track_idx_t>(si) * VOICES;
        Segment* seg = vm->getSegment(SegmentType::ChordRest, vmTick);
        Rest* r = Factory::createRest(seg, TDuration(DurationType::V_MEASURE));
        r->setTicks(vmLen);
        r->setTrack(tr);
        seg->add(r);
    }
}

static void coalesceVolta(BuildCtx& ctx, Measure* measure,
                          const EncMeasure& encMeas, Fraction measTick)
{
    // A volta is closed (down-turning end hook) only when its last measure has a repeat-end
    // barline; a terminal ending has none and must be drawn open. See ENCORE_IMPORTER.md
    // §Volta coalescing and numbered text.
    const bool voltaClosed = (encMeas.endBarline() == EncBarlineType::REPEATEND);
    if (encMeas.repeatAlternative != 0) {
        if (ctx.activeVolta && ctx.activeVoltaBits == encMeas.repeatAlternative) {
            ctx.activeVolta->setTick2(measTick + measure->ticks());
            // The volta now ends at this later measure; its hook follows that measure's barline.
            ctx.activeVolta->setVoltaType(voltaClosed ? Volta::Type::CLOSED : Volta::Type::OPEN);
        } else {
            // Accumulate the bits of the bracket being closed so the next bracket can filter out
            // already-labelled endings (e.g. after "1.-3.", raw bits {2,4} show only "4.").
            if (ctx.activeVolta) {
                ctx.usedVoltaBits |= ctx.activeVoltaBits;
            }
            // Visible endings = new bits not already covered by earlier brackets.
            const quint8 rawBits = encMeas.repeatAlternative;
            const quint8 newBits = rawBits & ~ctx.usedVoltaBits;
            const quint8 displayBits = (newBits != 0) ? newBits : rawBits;

            std::vector<int> endings;
            for (int b = 0; b < 8; ++b) {
                if (displayBits & (1 << b)) {
                    endings.push_back(b + 1);
                }
            }
            Volta* volta = Factory::createVolta(ctx.score->dummy());
            volta->setVoltaType(voltaClosed ? Volta::Type::CLOSED : Volta::Type::OPEN);
            volta->setTrack(0);
            volta->setTrack2(0);
            volta->setTick(measTick);
            volta->setTick2(measTick + measure->ticks());
            volta->setEndings(endings);
            // setText required: setEndings alone leaves the bracket blank.
            String voltaText;
            for (int number : endings) {
                if (!voltaText.empty()) {
                    voltaText += u", ";
                }
                voltaText += String::number(number);
            }
            voltaText += u".";
            volta->setText(voltaText);
            ctx.score->addElement(volta);
            ctx.activeVolta = volta;
            ctx.activeVoltaBits = rawBits;
        }
    } else {
        ctx.activeVolta = nullptr;
        ctx.activeVoltaBits = 0;
        ctx.usedVoltaBits = 0;  // Reset for next repeat block
    }
}

static void resetPerMeasureState(BuildCtx& ctx)
{
    for (auto& [key, tt] : ctx.scratch.tuplets) {
        if (tt.inTuplet()) {
            tt.closeTuplet();
        }
    }
    ctx.scratch.tuplets.clear();
    for (auto& [key, tt] : ctx.scratch.innerTuplets) {
        if (tt.inTuplet()) {
            tt.closeTuplet();
        }
    }
    ctx.scratch.innerTuplets.clear();
    ctx.scratch.cumTick.clear();
    ctx.scratch.prevMidiTick.clear();
    ctx.scratch.prevEncVoice.clear();
    ctx.scratch.lastChordPos.clear();
    ctx.scratch.prevRestTick.clear();
    ctx.scratch.graceStolenTicks.clear();
    ctx.scratch.lastGraceChord.clear();
    ctx.scratch.lastGraceTick.clear();

    // pendingGraces are deliberately NOT cleared: end-of-measure graces ornament the first note of
    // the next measure, so they carry across the barline. Any left over become cue notes via
    // handleDanglingGraces() at end of score.
}

static void buildNestedTupletMaps(MeasEmitCtx& mc,
                                  const MeasureElemRefVec& sortedElems)
{
    mc.nestedByInnerFirst.clear();
    mc.nestedByInnerLast.clear();
    mc.innerGroupMembers.clear();
    for (const NestedTupletInfo& ni : mc.nestedInfos) {
        if (ni.innerFirst) {
            mc.nestedByInnerFirst[ni.innerFirst] = &ni;
        }
        if (ni.innerLast) {
            mc.nestedByInnerLast[ni.innerLast] = &ni;
        }
        // Collect all sorted elements between innerFirst and innerLast (inclusive).
        if (ni.innerFirst && ni.innerLast) {
            bool inInner = false;
            for (const EncMeasureElem* em2 : sortedElems) {
                if (em2 == ni.innerFirst) {
                    inInner = true;
                }
                if (inInner && em2->staffIdx == ni.innerFirst->staffIdx) {
                    mc.innerGroupMembers.insert(em2);
                }
                if (em2 == ni.innerLast) {
                    break;
                }
            }
        }
    }
}

static void handleClefChange(BuildCtx& ctx, const MeasEmitCtx& mc,
                             const NoteElemCtx& ec, const EncMeasureElem* e)
{
    const EncClefChange* ecc = static_cast<const EncClefChange*>(e);
    if (ec.staffIdx < 0 || ec.staffIdx >= ctx.totalStaves) {
        return;
    }
    const ClefType ct = encClef2MuseScore(ecc->clefType);
    const track_idx_t track = static_cast<track_idx_t>(ec.staffIdx) * VOICES;

    // A clef change applies before the note it physically precedes, not at its own stored tick, so
    // anchor it to the first note/rest on the same staff after this clef element. A trailing clef
    // (no following note/rest) is cautionary and belongs on the next measure's downbeat.
    const EncMeasureElem* nextCr = nullptr;
    if (mc.encMeas) {
        bool seenSelf = false;
        for (const auto& up : mc.encMeas->elements) {
            const EncMeasureElem* el = up.get();
            if (el == e) {
                seenSelf = true;
                continue;
            }
            if (!seenSelf) {
                continue;
            }
            const EncElemType t = static_cast<EncElemType>(el->type);
            if ((t == EncElemType::NOTE || t == EncElemType::REST)
                && el->staffIdx == e->staffIdx && el->staffWithin == e->staffWithin) {
                nextCr = el;
                break;
            }
        }
    }

    Measure* target = mc.measure;
    Fraction segTick;
    if (nextCr) {
        segTick = mc.measure->tick() + Fraction(static_cast<int>(nextCr->tick), kEncWholeTicks);
    } else if (Measure* next = mc.measure->nextMeasure()) {
        target = next;
        segTick = next->tick();
    } else {
        segTick = mc.measure->tick() + mc.measure->ticks();
    }

    Segment* seg = target->getSegment(SegmentType::Clef, segTick);
    Clef* clef = Factory::createClef(seg);
    clef->setTrack(track);
    clef->setClefType(ct);
    seg->add(clef);
}

// Set the staff key and add a KeySig segment element. Shared by the immediate key change
// and the deferred flush (a key change in a rest-only measure is held until the next measure
// with notes, so it does not break MMRest condensation).
static void placeKeySig(MasterScore* score, Measure* measure, Fraction tick,
                        int staffIdx, track_idx_t track, Key concertKey, Key writtenKey)
{
    Staff* staff = score->staff(staffIdx);
    if (!staff) {
        return;
    }
    KeySigEvent ke;
    ke.setConcertKey(concertKey);
    ke.setKey(writtenKey);
    staff->setKey(tick, ke);
    Segment* seg = measure->getSegment(SegmentType::KeySig, tick);
    KeySig* ks = Factory::createKeySig(seg);
    ks->setTrack(track);
    ks->setKey(concertKey, writtenKey);
    seg->add(ks);
}

static void handleKeyChange(BuildCtx& ctx, const MeasEmitCtx& mc,
                            const NoteElemCtx& ec, const EncMeasureElem* e,
                            std::vector<DeferredKeySig>& pendingKeySigs)
{
    MasterScore* score = ctx.score;
    const EncKeyChange* ekc = static_cast<const EncKeyChange*>(e);
    Staff* staff = score->staff(ec.staffIdx);
    if (!staff) {
        return;
    }
    Key writtenKey = Key(encKeyToFifths(ekc->tipo));
    Interval v = Interval(ctx.staffPitchOffset[ec.staffIdx]);
    Key concertKey = v.isZero() ? writtenKey : Transpose::transposeKey(writtenKey, v);

    if (!hasPitchedNotes(*mc.encMeas)) {
        // Placing a KeySig in a rest-only measure breaks MuseScore's MMRest
        // condensation.  Defer to the next measure that contains notes.
        pendingKeySigs.push_back({ writtenKey, concertKey, ec.staffIdx });
        return;
    }

    placeKeySig(score, mc.measure, ec.elemTick, ec.staffIdx, ec.track, concertKey, writtenKey);
}

// Runs at the end of each MEAS block after all elements are placed.
// Closes open tuplets, attaches lyrics, adjusts pickup, fills gaps, validates,
// and advances the measure-skip counter for multi-measure rest expansion.
static void finalizeMeasureAfterNoteLoop(BuildCtx& ctx, MeasEmitCtx& mc,
                                         Measure* measure, const EncMeasure& encMeas,
                                         const Fraction& measTick, int measIdx,
                                         int& measSkip, size_t& msIdxCounter,
                                         const EncRoot& enc)
{
    for (auto& [key, tt] : ctx.scratch.tuplets) {
        mc.closeTupletWithFill(ctx, tt, key);
    }
    adjustPickupMeasure(ctx, measure, measIdx);
    fillTrailingGaps(ctx, measure, measTick);
    for (int si = 0; si < ctx.totalStaves; ++si) {
        measure->checkMeasure(static_cast<staff_idx_t>(si));
    }
    correctMeasureLength(ctx, measure);
    fitOverfullMeasure(ctx, measure);
    const EncMeasure* prevMeas = (measIdx > 0) ? &enc.measures[measIdx - 1] : nullptr;
    measSkip = measDisplayCount(encMeas, prevMeas) - 1;
    ++msIdxCounter;
}

// Flush key changes that were deferred from rest-only measures onto the first later measure
// that has notes (placing a KeySig in an MMRest-eligible measure breaks condensation).
static void flushPendingKeySigs(MasterScore* score, Measure* measure, Fraction measTick,
                                const EncMeasure& encMeas,
                                std::vector<DeferredKeySig>& pendingKeySigs)
{
    if (pendingKeySigs.empty() || !hasPitchedNotes(encMeas)) {
        return;
    }
    for (const DeferredKeySig& dks : pendingKeySigs) {
        placeKeySig(score, measure, measTick, dks.staffIdx,
                    dks.staffIdx * VOICES, dks.concertKey, dks.writtenKey);
    }
    pendingKeySigs.clear();
}

// Sort the measure's elements and compute the per-measure tuplet/tie/metadata state the
// element emit loop reads from mc.
static void prepareMeasureContext(BuildCtx& ctx, MeasEmitCtx& mc, const EncMeasure& encMeas,
                                  MeasureElemRefVec& sortedElems)
{
    // Sort: tick asc, ORNs before notes, tuplet notes before non-tuplet (ensures tup note sets duration at shared tick).
    sortMeasureElements(encMeas, sortedElems);
    // Collect TIE-START positions using routed (staffIdx, voice) so bit6-encoded second-staff notes resolve correctly.
    collectTieStartPositions(sortedElems, *mc.lineSlotByRawByte, ctx.totalStaves, mc);
    mc.overrideGroupRatios.clear();
    mc.validTupletGroupMember
        = computeImpliedTupletMembers(sortedElems, encMeas, ctx.totalStaves,
                                      &mc.partialEndGroup, &mc.nestedInfos,
                                      &mc.overrideGroupRatios);
    buildNestedTupletMaps(mc, sortedElems);
    scanMeasureMetadata(sortedElems, mc);
}

// Route one measure element to its (staff, voice, tick) and dispatch it to the type handler.
static void emitMeasureElement(BuildCtx& ctx, MeasEmitCtx& mc, const EncMeasureElem* e,
                               std::vector<DeferredKeySig>& pendingKeySigs)
{
    Measure* measure = mc.measure;
    const Fraction measTick = mc.measTick;
    const EncMeasure& encMeas = *mc.encMeas;
    const std::array<int, 256>& lineSlotByRawByte = *mc.lineSlotByRawByte;

    const EncElemType et = static_cast<EncElemType>(e->type);
    const bool isNoteOrRest = (et == EncElemType::NOTE || et == EncElemType::REST);

    // Let notes/rests past durTicks through for every overfill strategy so an
    // overshooting tuplet's later members still arrive; non-tuplet overflow is
    // re-dropped below (the "voice full" guard), and the post-pass resolves the
    // rest. (Only note/rest are let through; other element gating is unchanged.)
    if (!shouldIncludeElement(e, encMeas) && !isNoteOrRest) {
        return;
    }

    std::optional<RoutedTrack> routed = routeElementStaffVoice(e, isNoteOrRest, lineSlotByRawByte, mc, ctx);
    if (!routed) {
        return;
    }
    const int staffIdx = routed->staffIdx;
    const int voice = routed->voice;
    const int msVoice = routed->msVoice;
    const track_idx_t track = routed->track;
    const std::pair<int, int> trackKey = routed->trackKey;
    const std::pair<int, int> encVoiceKey = routed->encVoiceKey;

    // Encore's "voice 4" is a silent-voice placeholder that routing folds into voice 0. A voice-4
    // rest on a staff that already carries a real note is redundant: merged into voice 0 it collides
    // with the notes and pushes content past the barline. Drop it here.
    if (et == EncElemType::REST && e->voice >= static_cast<int>(VOICES)
        && mc.stavesWithRealNote.count(staffIdx)) {
        return;
    }

    // Near-simultaneous notes (< CHORD_MIDI_THRESHOLD) extend the chord; same Encore voice required.
    constexpr int CHORD_MIDI_THRESHOLD = 2 * CHORD_CLUSTER_THRESHOLD;  // = 8
    // Two notes close in time but in different notated columns (xoffset) are sequential events, not
    // one chord: a chord's members share a column (a few pixels of notehead offset at most), so only
    // a gap >= COLUMN_SEPARATION_MIN marks a genuine column change. Only for formats that store the
    // column (see EncFormatReader::clustersChordsByXoffset).
    constexpr int COLUMN_SEPARATION_MIN = 8;
    const bool columnAware = ctx.enc.fmt && ctx.enc.fmt->clustersChordsByXoffset();
    const bool differentColumn = columnAware && e->xoffset != 0
                                 && ctx.scratch.prevXoffset.count(trackKey)
                                 && ctx.scratch.prevXoffset.at(trackKey) != 0
                                 && std::abs(ctx.scratch.prevXoffset.at(trackKey) - static_cast<int>(e->xoffset))
                                 >= COLUMN_SEPARATION_MIN;
    bool isChordExt = isNoteOrRest && !differentColumn
                      && ctx.scratch.prevMidiTick.count(trackKey)
                      && ctx.scratch.prevEncVoice.count(trackKey)
                      && ctx.scratch.prevEncVoice.at(trackKey) == voice
                      && (int)e->tick - (int)ctx.scratch.prevMidiTick.at(trackKey) >= 0
                      && (int)e->tick - (int)ctx.scratch.prevMidiTick.at(trackKey)
                      < CHORD_MIDI_THRESHOLD;
    // REST-REST dedup: two Encore voices routing to the same MuseScore voice at the same tick; the
    // second REST would double-advance cumTick.
    if (!isChordExt && et == EncElemType::REST
        && ctx.scratch.prevRestTick.count(trackKey)
        && ctx.scratch.prevRestTick.at(trackKey) == static_cast<int>(e->tick)) {
        return;
    }
    // A REST before its voice's already-filled position is redundant (a chord or earlier rest covers
    // that beat); placing it would push later content forward and inflate the bar. Drop it.
    if (!isChordExt && et == EncElemType::REST
        && ctx.scratch.cumTick.count(trackKey)
        && Fraction(static_cast<int>(e->tick), kEncWholeTicks) < ctx.scratch.cumTick.at(trackKey)) {
        return;
    }
    // Same when the coincident rest is serialized before its voice's note (cumTick not yet advanced):
    // a plain REST sharing (staff, voice, tick) with a NOTE is a redundant placeholder; drop it so
    // the note keeps the beat. Tuplet rests are exempt (a same-tick tuplet rest + note are sequential).
    if (et == EncElemType::REST && e->tupletByte() == 0
        && mc.noteStaffVoiceTicks.count({ static_cast<int>(e->staffIdx),
                                          static_cast<int>(e->voice), static_cast<int>(e->tick) })) {
        return;
    }

    // Drop overflow notes when the voice is full (MIDI artifacts must not spill to the next voice),
    // but only under Truncate: the other strategies keep them for the post-pass to resolve. An open
    // tuplet is never dropped here so the whole tuplet lands intact. See ENCORE_IMPORTER.md
    // §Overfull measures.
    const bool inOpenTuplet = ctx.scratch.tuplets.count(trackKey) && ctx.scratch.tuplets.at(trackKey).inTuplet();
    if (isNoteOrRest && !isChordExt && ctx.scratch.cumTick[trackKey] >= measure->ticks()
        && ctx.opts.overfillMeasureStrategy == OverfillStrategy::Truncate
        && !inOpenTuplet) {
        return;
    }

    const int savedPrevMidiTick = ctx.scratch.prevMidiTick.count(trackKey)
                                  ? ctx.scratch.prevMidiTick.at(trackKey) : -1;
    const bool hadLastChordPos = ctx.scratch.lastChordPos.count(trackKey);
    const Fraction savedLastChordPos = hadLastChordPos
                                       ? ctx.scratch.lastChordPos.at(trackKey) : Fraction(-1, 1);

    const Fraction elemTick = computeElementTick(e, isNoteOrRest, isChordExt, voice,
                                                 staffIdx, trackKey, measure, measTick,
                                                 ctx, mc);

    NoteElemCtx ec;
    ec.e = e;
    ec.et = et;
    ec.staffIdx = staffIdx;
    ec.voice = voice;
    ec.msVoice = msVoice;
    ec.track = track;
    ec.trackKey = trackKey;
    ec.encVoiceKey = encVoiceKey;
    ec.isChordExt = isChordExt;
    ec.isNoteOrRest = isNoteOrRest;
    ec.elemTick = elemTick;
    ec.savedPrevMidiTick = savedPrevMidiTick;
    ec.hadLastChordPos = hadLastChordPos;
    ec.savedLastChordPos = savedLastChordPos;

    switch (et) {
    case EncElemType::NOTE:      handleNote(ctx, mc, ec);
        break;
    case EncElemType::REST:      handleRest(ctx, mc, ec);
        break;
    default: break;
    }
}

// Walk every parsed measure and emit its elements. The resolver post-pass (spanners, etc.) runs
// afterwards from buildScore.
void emitMeasures(BuildCtx& ctx)
{
    MasterScore* score = ctx.score;
    const EncRoot& enc = ctx.enc;

    const int nLineStaves = (!enc.lines.empty())
                            ? static_cast<int>(enc.lines[0].staffData.size()) : 0;
    std::vector<int> lineStaffInstrIdx;
    std::vector<int> lineStaffWithin;
    std::array<int, 256> lineSlotByRawByte;
    initLineStaffMappings(enc, nLineStaves, lineStaffInstrIdx, lineStaffWithin, lineSlotByRawByte);

    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (mb->isMeasure()) {
            ctx.measuresByIdx.push_back(toMeasure(mb));
        }
    }

    // Slurs resolved after the pass: .enc has no SLURSTOP; end anchored at last ChordRest in the alMezuro target measure.

    std::vector<DeferredKeySig> pendingKeySigs;

    // measSkip tracks the expansion of multi-measure rests (1 enc MEAS to N MuseScore measures).
    int measSkip = 0;
    size_t msIdxCounter = 0;

    int measIdx = 0;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }

        if (measSkip > 0) {
            --measSkip;
            ++msIdxCounter;
            fillExpandedMrestMeasure(toMeasure(mb), ctx.totalStaves);
            continue;
        }

        if (measIdx >= static_cast<int>(enc.measures.size())) {
            break;
        }
        Measure* measure = toMeasure(mb);
        const EncMeasure& encMeas = enc.measures[measIdx];
        const Fraction measTick = measure->tick();

        MeasEmitCtx mc;
        mc.measure = measure;
        mc.encMeas = &encMeas;
        mc.measTick = measTick;
        mc.measIdx = measIdx;
        mc.nLineStaves = nLineStaves;
        mc.lineStaffInstrIdx = &lineStaffInstrIdx;
        mc.lineStaffWithin = &lineStaffWithin;
        mc.lineSlotByRawByte = &lineSlotByRawByte;

        resetPerMeasureState(ctx);

        MeasureElemRefVec sortedElems;
        prepareMeasureContext(ctx, mc, encMeas, sortedElems);

        for (const EncMeasureElem* e : sortedElems) {
            emitMeasureElement(ctx, mc, e, pendingKeySigs);
        }

        finalizeMeasureAfterNoteLoop(ctx, mc, measure, encMeas, measTick, measIdx,
                                     measSkip, msIdxCounter, enc);
        ++measIdx;
    }

    applyMeasureBpmMarks(ctx);

    // One-line summary of elements that could not be placed (they reference a staff/voice the
    // score does not have), instead of a debug line per dropped element.
    if (!ctx.scratch.droppedByMissingStaff.empty() || ctx.scratch.droppedByBadVoice > 0) {
        int total = ctx.scratch.droppedByBadVoice;
        std::string byStaff;
        for (const auto& [staffIdx, count] : ctx.scratch.droppedByMissingStaff) {
            total += count;
            if (!byStaff.empty()) {
                byStaff += ", ";
            }
            byStaff += "staff " + std::to_string(staffIdx) + ": " + std::to_string(count);
        }
        LOGD() << "Encore import: dropped " << total << " element(s) the score has no place for"
               << " (" << ctx.totalStaves << " staves built"
               << (byStaff.empty() ? std::string() : "; missing-staff refs: " + byStaff)
               << (ctx.scratch.droppedByBadVoice ? "; out-of-range voice: "
            + std::to_string(ctx.scratch.droppedByBadVoice) : std::string())
               << "). These usually come from staves deleted in Encore (not shown there).";
    }
}
} // namespace mu::iex::enc
