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

// Emit notes: duration/tuplet resolution, ties, MIDI-artifact filtering and fingerings.

#include "emitters-internal.h"
#include "mappers.h"
#include "../parser/ticks.h"
#include "durations.h"
#include "engraving/dom/arpeggio.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/fermata.h"
#include "engraving/dom/fingering.h"
#include "engraving/dom/ornament.h"
#include "engraving/dom/tremolosinglechord.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/note.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/drumset.h"
#include "engraving/dom/instrument.h"
#include "engraving/dom/tuplet.h"
#include "engraving/dom/tie.h"
#include "log.h"
#include <set>
#include <tuple>

namespace mu::iex::enc {
using namespace mu::engraving;

// Tuplet ratio and resolved duration for one note, threaded through resolveNoteDuration,
// attachChordToTuplet and advanceCumulativeTick. dt/dots are in/out (attach may shrink them
// for an isolated-explicit fill); dtFace is the pre-cap duration for that fill check.
struct TupletDecision {
    int actualN { 0 };
    int normalN { 0 };
    DurationType dt { DurationType::V_INVALID };
    int dots { 0 };
    DurationType dtFace { DurationType::V_INVALID };
};

// A short realDuration (< 15) is usually a MIDI tie-continuation artifact, not a real note.
static bool isMidiArtifact(const EncNote* en,
                           const NoteElemCtx& ec,
                           const MeasEmitCtx& mc,
                           std::set<std::tuple<int, int, int> >& filteredSenders,
                           int savedPrevMidiTick,
                           bool isChordExt)
{
    if (en->realDuration == 0 || en->realDuration >= 15) {
        return false;
    }
    const quint8 safeFv = fvLow(en->faceValue);
    int fvBase = faceValue2ticks(safeFv);
    if (fvBase <= 15) {
        bool bypass = mc.isTieStartAt(ec.staffIdx, ec.voice, (int)ec.e->tick)
                      || isChordExt;
        if (!bypass) {
            if ((en->grace1 & 0x0F) == 1) {
                filteredSenders.insert({ ec.staffIdx, ec.voice, (int)en->semiTonePitch });
            }
            return true;
        }
    } else {
        if (en->realDuration > CHORD_CLUSTER_THRESHOLD
            && !mc.validTupletGroupMember.count(ec.e)
            && !isChordExt
            && savedPrevMidiTick >= 0) {
            return true;
        }
    }
    return false;
}

// The tie-receiver of an already-filtered artifact (grace1 low nibble == 2) must be filtered too.
static bool isCascadeFilteredTieReceiver(const EncNote* en,
                                         const NoteElemCtx& ec,
                                         std::set<std::tuple<int, int, int> >& filteredSenders)
{
    if ((en->grace1 & 0x0F) != 2) {
        return false;
    }
    auto cascKey = std::make_tuple(ec.staffIdx, ec.voice, (int)en->semiTonePitch);
    if (filteredSenders.count(cascKey)) {
        filteredSenders.erase(cascKey);
        return true;
    }
    return false;
}

// Returns V_INVALID if rdur does not match the beat-relative pattern.
static DurationType resolveBeatRelativeFaceValue(
    const EncNote* en,
    const EncMeasure* encMeas,
    int preACheck,
    int preNCheck)
{
    if (en->realDuration == 0 || preACheck <= 0 || preNCheck <= 0
        || !encMeas || encMeas->beatTicks == 0) {
        return DurationType::V_INVALID;
    }
    const int bt = static_cast<int>(encMeas->beatTicks);
    const int expectedBeatAdv = (bt * preNCheck + preACheck / 2) / preACheck;
    if (static_cast<int>(en->realDuration) != expectedBeatAdv) {
        return DurationType::V_INVALID;
    }
    // Beat-relative fv (e.g. 8/8 where fv=Q means one beat): derive written note from rdur x ratio.
    const int faceTicks = (static_cast<int>(en->realDuration) * preACheck
                           + preNCheck / 2) / preNCheck;
    // Choose fv to avoid the "realDur < faceValue2ticks(fv)" fallback in realDuration2DurationType.
    quint8 computedFv = en->faceValue;
    for (int f = 1; f <= 8; ++f) {
        if (faceValue2ticks(static_cast<quint8>(f)) <= faceTicks) {
            computedFv = static_cast<quint8>(f);
            break;
        }
    }
    return realDuration2DurationType(static_cast<qint16>(faceTicks), computedFv);
}

static void attachChordToTuplet(
    BuildCtx& ctx,
    MeasEmitCtx& mc,
    const NoteElemCtx& ec,
    const EncNote* en,
    Chord* chord,
    TupletDecision& dec)
{
    DurationType& dt = dec.dt;
    int& dots = dec.dots;
    const DurationType dtFace = dec.dtFace;
    const int preACheck = dec.actualN;
    const int preNCheck = dec.normalN;
    const bool isInnerMember = mc.innerGroupMembers.count(ec.e) > 0;
    const bool isInnerFirst  = mc.nestedByInnerFirst.count(ec.e) > 0;
    const bool isInnerLast   = mc.nestedByInnerLast.count(ec.e) > 0;
    Measure* measure = mc.measure;
    const std::set<const EncMeasureElem*>& validTupletGroupMember = mc.validTupletGroupMember;
    const std::set<const EncMeasureElem*>& partialEndGroup = mc.partialEndGroup;
    const EncMeasureElem* e = ec.e;
    const auto& trackKey = ec.trackKey;
    track_idx_t track = ec.track;
    Fraction elemTick = ec.elemTick;
    bool isStandardExplicit = isStandardExplicitTuplet(preACheck, preNCheck);
    auto closeTupletWithFill = [&](TupletTracker& tt, std::pair<int, int> key) {
        mc.closeTupletWithFill(ctx, tt, key);
    };

    auto& tt = ctx.scratch.tuplets[trackKey];
    int actualN = isStandardExplicit ? preACheck : 0;
    int normalN = isStandardExplicit ? preNCheck : 0;
    // Implied tuplet (pre-validated: isImpliedTupletMember set by parser for v0xC2 only).
    if (actualN == 0 && (fvLow(en->faceValue)) >= 4 && validTupletGroupMember.count(e)) {
        actualN = detectImpliedTuplet(en->realDuration, en->faceValue, normalN);
    }
    // Sandwich orphan (tup=0 surrounded by tup=N:M notes): use active ratio to stay in bracket.
    if (actualN == 0 && tt.inTuplet() && !tt.groupFull() && validTupletGroupMember.count(e)) {
        actualN = tt.actualN;
        normalN = tt.normalN;
    }

    if (actualN > 0 && normalN > 0) {
        // Close full group before starting a new one.
        if (tt.groupFull()) {
            closeTupletWithFill(tt, trackKey);
        }
        if (!tt.inTuplet()) {
            if (isStandardExplicit && !validTupletGroupMember.count(e)) {
                // Isolated explicit note: start partial tuplet only when it exactly fills remaining space.
                Fraction tupAdv = TDuration(dtFace).fraction()
                                  * Fraction(normalN, actualN);
                Fraction remaining = measure->ticks() - ctx.scratch.cumTick[trackKey];
                if (tupAdv == remaining) {
                    dt   = dtFace;
                    dots = 0;
                    TDuration faceD(dtFace);
                    chord->setDurationType(faceD);
                    chord->setTicks(faceD.fraction());
                    chord->setDots(0);
                    tt.startTuplet(measure, elemTick, actualN, normalN, dt, track);
                } else {
                    actualN = 0;
                    normalN = 0;           // treat as plain note
                }
            } else {
                // Partial measure-end groups: derive baseLen from remaining/normalN (e.g. rem=1/8, normalN=2 -> baseLen=1/16).
                DurationType baseLenDt = dt;
                if (partialEndGroup.count(e)) {
                    Fraction rem3 = measure->ticks() - ctx.scratch.cumTick[trackKey];
                    Fraction fullAdv = TDuration(dt).fraction() * Fraction(normalN, 1);
                    if (fullAdv > rem3 && rem3 > Fraction(0, 1)) {
                        Fraction baseFrac = Fraction(rem3.numerator(),
                                                     rem3.denominator() * normalN).reduced();
                        TDuration baseLenDur(baseFrac, true /*truncate*/);
                        if (baseLenDur.isValid() && baseLenDur.fraction() == baseFrac) {
                            baseLenDt = baseLenDur.type();
                        }
                    }
                }
                tt.startTuplet(measure, elemTick, actualN, normalN, baseLenDt, track);
            }
        }
    }
    if (actualN > 0 && normalN > 0) {
        // Nested-tuplet: inner notes go into innerTt; outer advances via cumTick (doubly-nested block below).
        auto& innerTt = ctx.scratch.innerTuplets[trackKey];
        if (isInnerMember) {
            if (isInnerFirst) {
                const NestedTupletInfo& ni = *mc.nestedByInnerFirst.at(e);
                innerTt.closeTuplet();
                innerTt.startTuplet(measure, elemTick, ni.innerActualN, ni.innerNormalN, dt, track);
                if (tt.inTuplet() && tt.currentTuplet && innerTt.currentTuplet) {
                    innerTt.currentTuplet->setTuplet(tt.currentTuplet);
                    tt.currentTuplet->add(innerTt.currentTuplet);
                }
            }
            if (innerTt.inTuplet()) {
                chord->setTuplet(innerTt.currentTuplet);
                innerTt.currentTuplet->add(chord);
                innerTt.faceTicks += TDuration(dt).fraction();
            }
            if (isInnerLast && innerTt.inTuplet()) {
                innerTt.closeTuplet();
                // Credit outer TupletTracker with one outer-slot face value.
                if (tt.inTuplet() && tt.currentTuplet) {
                    tt.faceTicks += TDuration(tt.currentTuplet->baseLen()).fraction();
                }
            }
        } else {
            chord->setTuplet(tt.currentTuplet);
            tt.currentTuplet->add(chord);

            // No-downdate: only lower fullFaceSum when smaller fv arrives and current tally still fits the new threshold.
            // {Q,E}/3:2: before E, faceTicks=Q≤3E → update; {Q,Q,8,8}/3:2: faceTicks=2Q>3E → skip.
            if (tt.actualN > 0 && tt.fullFaceSum > Fraction(0, 1)) {
                const Fraction thisFace = TDuration(dt).fraction();
                const Fraction currentBaseLen = tt.fullFaceSum / tt.actualN;
                if (thisFace > Fraction(0, 1) && thisFace < currentBaseLen) {
                    const Fraction newThreshold = thisFace * tt.actualN;
                    if (tt.faceTicks <= newThreshold) {
                        tt.fullFaceSum = newThreshold;
                    }
                }
            }
            tt.faceTicks += TDuration(dt).fraction();
        }
    } else {
        auto& innerTt2 = ctx.scratch.innerTuplets[trackKey];
        if (innerTt2.inTuplet()) {
            innerTt2.closeTuplet();
        }
        if (tt.groupFull()) {
            closeTupletWithFill(tt, trackKey);
        }
        if (tt.inTuplet()) {
            closeTupletWithFill(tt, trackKey); // non-tuplet note exits group
        }
    }
}

// Returns false if chord was discarded (zero-tick residual).
static bool advanceCumulativeTick(
    BuildCtx& ctx,
    const NoteElemCtx& ec,
    const MeasEmitCtx& mc,
    Chord*& chord,
    TupletDecision& dec)
{
    DurationType& dt = dec.dt;
    int& dots = dec.dots;
    const int preACheck = dec.actualN;
    const int preNCheck = dec.normalN;
    const bool isInnerMember = mc.innerGroupMembers.count(ec.e) > 0;
    const auto& trackKey = ec.trackKey;

    auto& tt = ctx.scratch.tuplets[trackKey];
    auto& innerTtAdv = ctx.scratch.innerTuplets[trackKey];

    // Doubly-nested advance: apply both inner and outer ratios so cumTick over the inner group
    // equals one outer slot (3 inner 16ths would otherwise sum to 1/8 > the 1/12 outer slot).
    Fraction advance;
    if (isInnerMember) {
        // Apply inner ratio AND outer ratio. Use saved NI ratios when innerLast has already closed the group.
        const NestedTupletInfo* niAdv = nullptr;
        if (mc.nestedByInnerFirst.count(ec.e)) {
            niAdv = mc.nestedByInnerFirst.at(ec.e);
        } else if (mc.nestedByInnerLast.count(ec.e)) {
            niAdv = mc.nestedByInnerLast.at(ec.e);
        } else if (!mc.nestedInfos.empty()) {
            niAdv = &mc.nestedInfos.front();  // middle inner note; only one nested group per measure in known files
        }
        const int innerAN = niAdv ? niAdv->innerActualN : (innerTtAdv.inTuplet() ? innerTtAdv.actualN : preACheck);
        const int innerNN = niAdv ? niAdv->innerNormalN : (innerTtAdv.inTuplet() ? innerTtAdv.normalN : preNCheck);
        Fraction innerAdv = TDuration(dt).fraction()
                            * Fraction(innerNN, innerAN);
        if (tt.inTuplet()) {
            advance = innerAdv * Fraction(tt.normalN, tt.actualN);
        } else {
            advance = innerAdv;
        }
    } else if (tt.inTuplet()) {
        advance = TDuration(dt).fraction() * Fraction(tt.normalN, tt.actualN);
    } else {
        advance = dottedAdvance(dt, dots);
    }

    // Tuplet-remaining cap: fv > baseLen (e.g. 8th inside 3:2 with baseLen=16th) would produce non-TDuration-aligned Tuplet.ticks and crash layout.
    if (tt.inTuplet() && chord) {
        const Fraction tupExpected = TDuration(tt.currentTuplet->baseLen()).fraction()
                                     * tt.normalN;
        const Fraction tupRemaining = tupExpected - tt.placedTicks;
        if (tupRemaining > Fraction(0, 1) && advance > tupRemaining) {
            const Fraction neededFace = tupRemaining * Fraction(tt.actualN, tt.normalN);
            TDuration cappedFace(neededFace, true /*truncate*/);
            if (cappedFace.isValid() && cappedFace.fraction().numerator() > 0) {
                advance = cappedFace.fraction() * Fraction(tt.normalN, tt.actualN);
                chord->setDurationType(cappedFace);
                chord->setTicks(cappedFace.fraction());
                chord->setDots(0);
                // Re-sync faceTicks: the original dt may have been larger.
                tt.faceTicks -= TDuration(dt).fraction();
                tt.faceTicks += cappedFace.fraction();
            }
        }
    }

    // A plain note may overrun the barline here on purpose; the overfull post-pass
    // (fitOverfullMeasure) recuts it into a tied chain. A tuplet is atomic, so its members are
    // likewise never cut here.
    ctx.scratch.cumTick[trackKey] += advance;
    if (tt.inTuplet()) {
        tt.placedTicks += advance;
    }
    // Inner-group notes: advance innerTt.placedTicks by the singly-nested advance so closeTuplet() sees the correct inner span.
    auto& innerTtFin = ctx.scratch.innerTuplets[trackKey];
    if (isInnerMember && innerTtFin.inTuplet()) {
        const Fraction innerOnlyAdv = TDuration(dt).fraction()
                                      * Fraction(innerTtFin.normalN, innerTtFin.actualN);
        innerTtFin.placedTicks += innerOnlyAdv;
    }
    return true;
}

// Returns false if the note should be skipped (implied-tuplet or zero-tick residual).
// Otherwise sets dt, dots, and dtFace (dt before capping, for isolated-explicit fill check).
static bool resolveNoteDuration(
    BuildCtx& ctx,
    const NoteElemCtx& ec,
    const MeasEmitCtx& mc,
    const EncNote* en,
    bool isStandardExplicit,
    TupletDecision& dec)
{
    const int preACheck = dec.actualN;
    const int preNCheck = dec.normalN;
    const bool isChordExt = ec.isChordExt;
    const int savedPrevMidiTick = ec.savedPrevMidiTick;
    DurationType& dt = dec.dt;
    int& dots = dec.dots;
    DurationType& dtFace = dec.dtFace;
    Measure* measure = mc.measure;
    const std::set<const EncMeasureElem*>& validTupletGroupMember = mc.validTupletGroupMember;
    const std::set<const EncMeasureElem*>& partialEndGroup = mc.partialEndGroup;
    const EncMeasureElem* e = ec.e;
    const auto& trackKey = ec.trackKey;

    // Skip this note (MIDI artifact or zero-tick residual), rolling back prevMidiTick.
    auto bailOut = [&]() -> bool {
        if (savedPrevMidiTick >= 0) {
            ctx.scratch.prevMidiTick[trackKey] = savedPrevMidiTick;
        } else {
            ctx.scratch.prevMidiTick.erase(trackKey);
        }
        return false;
    };

    if (isStandardExplicit) {
        // In files where the face-value byte encodes "beats" rather than absolute note
        // values (e.g. 8/8 where fv=Q means one eighth beat), rdur equals exactly
        // beatTicks x (normalN/actualN). Use rdur in that case; otherwise trust fv.
        // This distinguishes beat-relative face values (rdur=beatTicks x ratio) from
        // truncated rdur (last note in a measure, rdur shortened by a following rest).
        dt = faceValue2DurationType(en->faceValue);
        {
            const DurationType dtBeat = resolveBeatRelativeFaceValue(en, mc.encMeas, preACheck, preNCheck);
            if (dtBeat != DurationType::V_INVALID) {
                dt = dtBeat;
            }
        }
        dots = 0;
        // Partial measure-end groups: reduce dt when the tuplet advance overshoots remaining space.
        if (partialEndGroup.count(e)) {
            const auto& ttX = ctx.scratch.tuplets[trackKey];
            if (ttX.inTuplet() && dt != DurationType::V_INVALID) {
                Fraction adv = TDuration(dt).fraction()
                               * Fraction(ttX.normalN, ttX.actualN);
                Fraction rem = measure->ticks() - ctx.scratch.cumTick[trackKey];
                while (adv > rem && rem > Fraction(0, 1)
                       && dt < DurationType::V_128TH) {
                    dt  = static_cast<DurationType>(static_cast<int>(dt) + 1);
                    adv = TDuration(dt).fraction()
                          * Fraction(ttX.normalN, ttX.actualN);
                }
            }
        }
    } else {
        dt   = realDuration2DurationType(en->realDuration, en->faceValue);
        if (en->forceDotted) {
            // Explicitly marked dotted by the parser (v0xC2 dotted-eighth tick-pattern fix).
            // Bypass computeDotCount to avoid the bit-0 fallback, which may fire on raw
            // binary dotControl values that coincidentally have bit 0 set.
            dots = 1;
        } else if (en->dotControl > 0) {
            // dotControl bit 0 = dotted flag; computeDotCount tries tick-value interpretation first, falls back to bit 0 on MIDI drift.
            dots = computeDotCount(en->dotControl, en->realDuration, en->faceValue,
                                   true /*useBit0Fallback*/);
        } else {
            dots = calcDotsSnap(en->realDuration, en->faceValue);
        }
    }
    dtFace = dt;  // before capping; used for isolated-explicit fill check

    {
        const auto& ttPre = ctx.scratch.tuplets[trackKey];
        int preA = isStandardExplicit ? preACheck : 0;
        int preN = isStandardExplicit ? preNCheck : 0;
        if (!isStandardExplicit) {
            if ((fvLow(en->faceValue)) >= 4 && validTupletGroupMember.count(e)) {
                preA = detectImpliedTuplet(en->realDuration, en->faceValue, preN);
            }
        }

        // Implied-tuplet guard: skip if full group advance doesn't fit (partial triplet leaves 1/3072 residual).
        if (!isStandardExplicit && !ttPre.inTuplet()
            && !isChordExt && preA > 0 && preN > 0) {
            Fraction singleAdv = TDuration(faceValue2DurationType(fvLow(en->faceValue))).fraction()
                                 * Fraction(preN, preA);
            Fraction fullGroupAdv = singleAdv * Fraction(preA, 1);
            Fraction mRemaining = measure->ticks() - ctx.scratch.cumTick[trackKey];
            if (fullGroupAdv > mRemaining) {
                return bailOut();
            }
        }

        bool willBeExplicit = isStandardExplicit && validTupletGroupMember.count(e);
        bool willBeTuplet = (preA > 0 && preN > 0 && (willBeExplicit || !isStandardExplicit))
                            || (ttPre.inTuplet() && !ttPre.groupFull());
        if (!willBeTuplet) {
            Fraction remaining = measure->ticks() - ctx.scratch.cumTick[trackKey];
            TDuration fullDur(dt);  // must include dots; TDuration(dt) alone misses the dotted extension
            fullDur.setDots(dots);
            if (remaining > Fraction(0, 1) && fullDur.fraction() > remaining
                && ctx.opts.overfillMeasureStrategy != OverfillStrategy::IrregularMeasure) {
                TDuration capped(remaining, true);
                // 1/3072-type residual: no representable duration fits (the note begins a hair
                // before the barline). A zero-tick chord breaks sanityCheck, so drop the note.
                if (capped.fraction().numerator() == 0) {
                    return bailOut();
                }
                // Otherwise keep the note's full value and let it overrun; fitOverfullMeasure
                // recuts it into a tied chain (collapsing here would strand the remainder as a rest).
            }
        }
    }
    return true;
}

// Mark the note fixed so layoutDrumset() cannot override its headGroup: a later note on the same
// pitch updates the shared drumset entry, which would otherwise overwrite this note's headGroup.
static void fixNoteHeadImmune(Note* note, const EncNote* en, Drumset* ds)
{
    const int drumLine = (ds && ds->isValid(note->pitch()))
                         ? ds->line(note->pitch())
                         : std::max(-4, 10 - static_cast<int>(en->position));
    note->setFixed(true);
    note->setFixedLine(drumLine);
}

static void configureNoteHeadForDrumset(Note* note, const EncNote* en)
{
    Drumset* ds = note->part()->instrument()->drumset();
    const int nibble = fvHigh(en->faceValue) & 0xF;

    // faceValue high nibble=7: slash notehead in Encore's rhythm-staff notation.
    if (nibble == 7) {
        note->setHeadGroup(NoteHeadGroup::HEAD_SLASH);
        if (ds) {
            if (!ds->isValid(note->pitch())) {
                DrumInstrument di;
                di.name       = String::number(note->pitch());
                di.line       = std::max(-4, 10 - static_cast<int>(en->position));
                di.stemDirection = DirectionV::UP;
                ds->setDrum(note->pitch(), di);
            }
            ds->drum(note->pitch()).notehead = NoteHeadGroup::HEAD_SLASH;
        }
        fixNoteHeadImmune(note, en, ds);
        return;
    }
    // faceValue high nibble=3: square notehead (Encore bass drum notation).
    if (nibble == 3) {
        note->setHeadGroup(NoteHeadGroup::HEAD_CUSTOM);
        if (ds) {
            if (!ds->isValid(note->pitch())) {
                DrumInstrument di;
                di.name = u"drum";
                di.notehead = NoteHeadGroup::HEAD_CUSTOM;
                // Half/whole durations use the open (hollow) square;
                // quarter and shorter use the filled square.
                di.noteheads[int(NoteHeadType::HEAD_WHOLE)]   = SymId::noteheadSquareWhite;
                di.noteheads[int(NoteHeadType::HEAD_HALF)]    = SymId::noteheadSquareWhite;
                di.noteheads[int(NoteHeadType::HEAD_QUARTER)] = SymId::noteheadSquareBlack;
                di.noteheads[int(NoteHeadType::HEAD_BREVIS)]  = SymId::noteheadSquareBlack;
                di.line = 7;
                di.stemDirection = DirectionV::DOWN;
                ds->setDrum(note->pitch(), di);
            } else {
                ds->drum(note->pitch()).notehead = NoteHeadGroup::HEAD_CUSTOM;
            }
        }
        fixNoteHeadImmune(note, en, ds);
        return;
    }
    {
        // 5-line PERC staff: line derived from Encore position byte.
        // faceValue high nibble encodes the notehead type (all 10 values confirmed):
        //   0=normal, 1=diamond, 2=triangle-up, 4=cross, 5=xcircle,
        //   6=plus, 8=large-diamond(soft), 9=invisible(no head)
        static const NoteHeadGroup nibble2head[] = {
            NoteHeadGroup::HEAD_NORMAL,        // 0
            NoteHeadGroup::HEAD_DIAMOND,       // 1 rombo
            NoteHeadGroup::HEAD_TRIANGLE_UP,   // 2 triangulo
            NoteHeadGroup::HEAD_NORMAL,        // 3 (square, handled above)
            NoteHeadGroup::HEAD_CROSS,         // 4 equis
            NoteHeadGroup::HEAD_XCIRCLE,       // 5 equis con circulo
            NoteHeadGroup::HEAD_PLUS,          // 6 mas (+)
            NoteHeadGroup::HEAD_SLASH,         // 7 slash (handled above)
            NoteHeadGroup::HEAD_LARGE_DIAMOND, // 8 rombo blando
            NoteHeadGroup::HEAD_NORMAL,        // 9 sin_cabeza (note made invisible below)
        };
        const NoteHeadGroup nhg = (nibble < 10) ? nibble2head[nibble] : NoteHeadGroup::HEAD_NORMAL;
        if (ds) {
            if (!ds->isValid(note->pitch())) {
                DrumInstrument di;
                di.name = String::number(note->pitch());
                di.line = std::max(-4, 10 - static_cast<int>(en->position));
                di.stemDirection = DirectionV::UP;
                ds->setDrum(note->pitch(), di);
            }
            // Set the drumset default notehead (used when the note is not fixed).
            ds->drum(note->pitch()).notehead = nhg;
            note->setHeadGroup(nhg);
        }
        if (nibble != 0) {
            fixNoteHeadImmune(note, en, ds);
        }
        // nibble 9 = sin_cabeza (no notehead): make the note invisible.
        if (nibble == 9) {
            note->setVisible(false);
        }
    }
}

void handleNote(BuildCtx& ctx, MeasEmitCtx& mc, NoteElemCtx& ec)
{
    Measure* measure = mc.measure;
    std::set<std::tuple<int, int, int> >& filteredTieSenderPitches = mc.filteredTieSenderPitches;
    const EncMeasureElem* e = ec.e;
    int staffIdx = ec.staffIdx;
    track_idx_t track = ec.track;
    auto trackKey = ec.trackKey;
    bool isChordExt = ec.isChordExt;
    Fraction elemTick = ec.elemTick;
    int savedPrevMidiTick = ec.savedPrevMidiTick;

    const EncNote* en = static_cast<const EncNote*>(e);

    if (!isValidFaceValue(en->faceValue)) {
        return;
    }

    // Pass pre-computed isChordExt: after the prevMidiTick update delta==0 would falsely bypass.
    if (isMidiArtifact(en, ec, mc, filteredTieSenderPitches, savedPrevMidiTick, isChordExt)) {
        return;
    }

    if (isCascadeFilteredTieReceiver(en, ec, filteredTieSenderPitches)) {
        return;
    }

    TupletDecision dec;
    dec.actualN = en->actualNotes();
    dec.normalN = en->normalNotes();
    {
        auto orit = mc.overrideGroupRatios.find(e);
        if (orit != mc.overrideGroupRatios.end()) {
            dec.actualN = orit->second.first;
            dec.normalN = orit->second.second;
        }
    }
    bool isStandardExplicit = isStandardExplicitTuplet(dec.actualN, dec.normalN);

    if (!resolveNoteDuration(ctx, ec, mc, en, isStandardExplicit, dec)) {
        return;
    }

    Segment* seg = measure->getSegment(SegmentType::ChordRest, elemTick);
    Chord* chord = nullptr;
    if (seg->element(track) && seg->element(track)->isChord()) {
        chord = toChord(seg->element(track));
    } else {
        chord = Factory::createChord(seg);
        chord->setTrack(track);
        TDuration dur(dec.dt);
        dur.setDots(dec.dots);
        chord->setDurationType(dur);
        chord->setTicks(dur.fraction());
        chord->setDots(dec.dots);
        seg->add(chord);

        attachChordToTuplet(ctx, mc, ec, en, chord, dec);

        if (!advanceCumulativeTick(ctx, ec, mc, chord, dec)) {
            return;
        }
    }

    const int concertPitch = en->semiTonePitch + ctx.staffPitchOffset[staffIdx];
    if (chord->findNote(concertPitch)) {
        // Some files encode the same pitch twice (duplicate NOTE or chord-extension copy); drop it.
        return;
    }
    Note* note = Factory::createNote(chord);
    applyConcertPitch(note, concertPitch);
    chord->add(note);

    // A small note reaching this normal path is a cue note (full value, drawn small); graces never
    // reach here. See ENCORE_IMPORTER.md §Grace and cue notes.
    if (en->isSmall()) {
        note->setSmall(true);
    }
    // Encore per-note mute flag: applies to any note (normal, cue, or grace), independent of size.
    if (en->isMuted()) {
        note->setPlay(false);
    }

    configureNoteHeadForDrumset(note, en);
}
} // namespace mu::iex::enc
