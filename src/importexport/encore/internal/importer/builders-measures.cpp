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

// Build MuseScore measures from EncMeasure data: time signatures, barlines, multi-measure-rest expansion, and initial clefs/keys.

#include "builders.h"
#include "ctx.h"
#include "import.h"
#include "../parser/elem.h"
#include "mappers.h"
#include <algorithm>
#include <map>
#include <set>
#include <vector>
#include <QDataStream>
#include "engraving/dom/clef.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/key.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/part.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/timesig.h"
#include "engraving/engravingerrors.h"
#include "log.h"

using namespace mu::engraving;

namespace mu::iex::enc {
// True if every element in the measure is a REST with mrestCount > 1.
// Multi-staff files emit one REST per staff, so there can be N > 1 elements.
static bool encMeasHasMultiRest(const EncMeasure& m)
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

// Returns the number of MuseScore measures to create for a single EncMeasure.
// Encore stores N consecutive empty measures as one MEAS block with mrestCount==N
// (byte +15 of REST element data). Expansion is suppressed when the predecessor is
// already an mrest block, preventing cascades from consecutive mrest blocks.
static int encMeasDisplayCount(const EncMeasure& m, const EncMeasure* prev)
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
    if (prev && encMeasHasMultiRest(*prev)) {
        return 1;
    }
    return cnt;
}

// Number of times a repeat-end barline plays. Encore stores no explicit count; the pass count is
// the highest volta ending number belonging to this repeat (bitmask on the end-repeat measure plus
// the ending measures that follow it). A plain repeat with no endings falls back to 2.
static int encRepeatPlayCount(const std::vector<EncMeasure>& measures, size_t endIdx)
{
    quint8 bits = measures[endIdx].repeatAlternative;
    for (size_t j = endIdx + 1; j < measures.size(); ++j) {
        if (measures[j].startBarline() == EncBarlineType::REPEATSTART
            || measures[j].repeatAlternative == 0) {
            break;
        }
        bits |= measures[j].repeatAlternative;
    }
    int highest = 0;
    for (int b = 0; b < 8; ++b) {
        if (bits & (1 << b)) {
            highest = b + 1;
        }
    }
    return highest > 2 ? highest : 2;
}

// Time signature of a measure, with a 4/4 fallback when the stored num/den are 0.
static Fraction encMeasTimeSig(const EncMeasure& m)
{
    const int num = m.timeSigNum > 0 ? m.timeSigNum : 4;
    const int den = m.timeSigDen > 0 ? m.timeSigDen : 4;
    return Fraction(num, den);
}

void buildMeasures(BuildCtx& ctx)
{
    MasterScore* score = ctx.score;
    const EncRoot& enc = ctx.enc;

    // Pickup measure: measure 0 holds the short duration, measure 1 the real sig. The "nominal"
    // sig is measure 1's when present (it survives a pickup in measure 0), else measure 0's.
    const Fraction sig0 = enc.measures.empty() ? Fraction(4, 4) : encMeasTimeSig(enc.measures[0]);
    ctx.nominalTimeSig = (enc.measures.size() >= 2) ? encMeasTimeSig(enc.measures[1]) : sig0;

    // Nominal time-sig type: use measure 1's glyph when its sig differs from measure 0 (pickup),
    // otherwise measure 0's.
    const size_t nomIdx = (enc.measures.size() >= 2 && ctx.nominalTimeSig != sig0) ? 1 : 0;
    if (nomIdx < enc.measures.size()) {
        ctx.nominalTimeSigType = encTimeSigGlyph2Type(enc.measures[nomIdx].timeSigGlyph,
                                                      ctx.nominalTimeSig);
    }

    int currentTick = 0;
    bool firstMeasure = true;
    size_t msIdxCounter = 0;
    ctx.encToMsIdx.reserve(enc.measures.size());
    for (size_t mi = 0; mi < enc.measures.size(); ++mi) {
        const EncMeasure& encMeas = enc.measures[mi];
        int num = encMeas.timeSigNum > 0 ? encMeas.timeSigNum : 4;
        int den = encMeas.timeSigDen > 0 ? encMeas.timeSigDen : 4;
        Fraction ts(num, den);
        ctx.measTickToTimeSigType[currentTick] = encTimeSigGlyph2Type(encMeas.timeSigGlyph, ts);

        const EncMeasure* prev = (mi > 0) ? &enc.measures[mi - 1] : nullptr;
        const int displayCount = encMeasDisplayCount(encMeas, prev);

        ctx.encToMsIdx.push_back(msIdxCounter);

        for (int di = 0; di < displayCount; ++di) {
            Measure* measure = Factory::createMeasure(score->dummy()->system());
            measure->setTick(Fraction::fromTicks(currentTick));

            // Case A: timeSig[0] != timeSig[1], pickup with explicit shorter sig; shorten now.
            // Case B (same sig, partial content): detected post-emitters via actual cumTick.
            // When firstMeasureIsPickup=false, bypass pickup detection and use the nominal sig.
            const bool pickupEnabled = ctx.opts.firstMeasureIsPickup;
            const bool isPickupA = pickupEnabled && firstMeasure && di == 0
                                   && ts != ctx.nominalTimeSig;
            if (!pickupEnabled && firstMeasure && di == 0) {
                measure->setTimesig(ctx.nominalTimeSig);
                measure->setTicks(ctx.nominalTimeSig);
            } else {
                measure->setTimesig(isPickupA ? ctx.nominalTimeSig : ts);
                measure->setTicks(ts);
            }

            if (di == 0) {
                if (encMeas.startBarline() == EncBarlineType::REPEATSTART) {
                    measure->setRepeatStart(true);
                }
                // A non-repeat special barline drawn at a measure's START (e.g. a double bar
                // before this measure) belongs, in MuseScore's model, to the end of the
                // previous measure. Encore stores it as this measure's startBarline; map it
                // onto the preceding measure's end barline so the divider is not dropped.
                if (encMeas.startBarline() == EncBarlineType::DOUBLEL
                    || encMeas.startBarline() == EncBarlineType::DOUBLER
                    || encMeas.startBarline() == EncBarlineType::DOTTED) {
                    if (Measure* prevMeas = score->lastMeasure()) {
                        if (prevMeas->endBarLineType() == BarLineType::NORMAL) {
                            const BarLineType t = (encMeas.startBarline() == EncBarlineType::DOTTED)
                                                  ? BarLineType::DOTTED : BarLineType::DOUBLE;
                            for (int s = 0; s < ctx.totalStaves; ++s) {
                                prevMeas->setEndBarLineType(t, static_cast<track_idx_t>(s) * VOICES);
                            }
                        }
                    }
                }
                if (encMeas.endBarline() == EncBarlineType::REPEATEND) {
                    measure->setRepeatEnd(true);
                    const int playCount = encRepeatPlayCount(enc.measures, mi);
                    if (playCount > 2) {
                        measure->setRepeatCount(playCount);
                    }
                } else if (encMeas.endBarline() == EncBarlineType::FINAL
                           || encMeas.endBarline() == EncBarlineType::DOUBLEL
                           || encMeas.endBarline() == EncBarlineType::DOUBLER
                           || encMeas.endBarline() == EncBarlineType::DOTTED) {
                    BarLineType type = BarLineType::DOUBLE;
                    if (encMeas.endBarline() == EncBarlineType::FINAL) {
                        type = BarLineType::END;
                    } else if (encMeas.endBarline() == EncBarlineType::DOTTED) {
                        type = BarLineType::DOTTED;
                    }
                    for (int s = 0; s < ctx.totalStaves; ++s) {
                        measure->setEndBarLineType(type, static_cast<track_idx_t>(s) * VOICES);
                    }
                }
            }

            score->measures()->append(measure);
            // Must match setTicks() above: both use nominalTimeSig in the no-pickup first-measure
            // branch so subsequent measure positions are consistent with that measure's duration.
            const bool usedNominal = !pickupEnabled && firstMeasure && di == 0;
            currentTick += usedNominal ? ctx.nominalTimeSig.ticks() : ts.ticks();
        }
        firstMeasure = false;
        msIdxCounter += static_cast<size_t>(displayCount);
    }
}

void buildInitialSignatures(BuildCtx& ctx)
{
    MasterScore* score = ctx.score;
    const EncRoot& enc = ctx.enc;
    if (!enc.measures.empty()) {
        addInitialTimeSig(score, ctx.totalStaves, ctx.nominalTimeSig, ctx.nominalTimeSigType);
    }
    if (!enc.lines.empty()) {
        const auto& firstLine = enc.lines[0];
        for (int si = 0; si < static_cast<int>(firstLine.staffData.size()) && si < ctx.totalStaves; ++si) {
            const auto& sd = firstLine.staffData[si];
            addInitialKeySig(score, si, sd.key);
            const int keyOffset = si < static_cast<int>(ctx.staffPitchOffset.size())
                                  ? ctx.staffPitchOffset[si] : 0;
            // Drumset instruments always use PERC clef; LINE block clefs must not override it.
            const Staff* st = score->staff(static_cast<staff_idx_t>(si));
            const bool hasDrumset = st && st->part() && st->part()->instrument()
                                    && st->part()->instrument()->drumset();
            const ClefType ct = hasDrumset ? ClefType::PERC
                                : pickStaffClef(sd.clef, keyOffset);
            addInitialClef(score, si, ct);
        }

        // v0xA6: staffData is empty (its header staffPerSystem reads 0 and the staff entry
        // layout differs), so the loop above adds no key signature. The per-staff written
        // key was parsed separately into staffKeys; apply it here. Clefs still come from the
        // instrument template, handled by the !haveLineClefs block below.
        if (firstLine.staffData.empty() && !firstLine.staffKeys.empty()) {
            for (int si = 0; si < ctx.totalStaves; ++si) {
                const size_t ki = std::min(static_cast<size_t>(si), firstLine.staffKeys.size() - 1);
                addInitialKeySig(score, si, firstLine.staffKeys[ki]);
            }
        }
    }

    // Files without per-staff LINE clef data (v0xA6): the initial clef comes from the
    // instrument template, which does not reflect an octave Key. The note pitches are already
    // octave-shifted by the Key, so apply the matching octave-decorated clef to bring the
    // display back to the written octave, mirroring what pickStaffClef does for v0xC4.
    const bool haveLineClefs = !enc.lines.empty() && !enc.lines[0].staffData.empty();
    if (!haveLineClefs) {
        for (int si = 0; si < ctx.totalStaves; ++si) {
            const int keyOffset = si < static_cast<int>(ctx.staffPitchOffset.size())
                                  ? ctx.staffPitchOffset[si] : 0;
            if (keyOffset == 0 || keyOffset % 12 != 0) {
                continue;   // only pure-octave Keys need a compensating clef
            }
            const Staff* st = score->staff(static_cast<staff_idx_t>(si));
            const bool hasDrumset = st && st->part() && st->part()->instrument()
                                    && st->part()->instrument()->drumset();
            if (hasDrumset) {
                continue;   // percussion has no octave clef
            }
            const ClefType base = si < static_cast<int>(ctx.staffTemplateConcertClef.size())
                                  ? ctx.staffTemplateConcertClef[si] : ClefType::INVALID;
            if (base == ClefType::INVALID) {
                continue;
            }
            const ClefType oct = applyOctaveToClef(base, keyOffset);
            if (oct != base) {
                addInitialClef(score, si, oct);
            }
        }
    }

    // Emit TimeSig elements at change points. Use identical() not operator==: Fraction(6,8)
    // == Fraction(3,4) via cross-multiplication, so operator== would miss 6/8 vs 3/4 changes.
    Fraction prevTs = ctx.nominalTimeSig;
    for (const Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
        Fraction mTs = m->timesig();
        if (mTs.identical(prevTs)) {
            continue;
        }
        Fraction mTick = m->tick();
        auto tsTypeIt = ctx.measTickToTimeSigType.find(mTick.ticks());
        TimeSigType tsType = (tsTypeIt != ctx.measTickToTimeSigType.end())
                             ? tsTypeIt->second : TimeSigType::NORMAL;
        for (int si = 0; si < ctx.totalStaves; ++si) {
            Segment* seg = const_cast<Measure*>(m)->getSegment(SegmentType::TimeSig, mTick);
            TimeSig* tsig = Factory::createTimeSig(seg);
            tsig->setTrack(static_cast<track_idx_t>(si) * VOICES);
            tsig->setSig(mTs, tsType);
            seg->add(tsig);
        }
        prevTs = mTs;
    }
}
} // namespace mu::iex::enc
