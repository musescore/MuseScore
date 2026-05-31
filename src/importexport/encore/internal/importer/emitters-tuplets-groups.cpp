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

// Detect implied and explicit tuplet groupings across a measure's note runs.

#include "emitters-tuplets.h"

#include <map>
#include <set>
#include <vector>

#include "../parser/ticks.h"
#include "durations.h"
#include "engraving/dom/mscore.h"

using namespace mu::engraving;

namespace mu::iex::enc {
// Explicit tuplet ratio from the tup byte (high nibble = actual, low nibble = normal).
static void getExplicit(const std::vector<const EncMeasureElem*>& grp,
                        int& outActual, int& outNormal)
{
    outActual = 0;
    outNormal = 0;
    if (grp.empty()) {
        return;
    }
    const EncMeasureElem* e = grp[0];
    const quint8 tup = e->tupletByte();
    int a = tup >> 4, n = tup & 0x0F;
    if (isStandardExplicitTuplet(a, n)) {
        outActual = a;
        outNormal = n;
    }
}

// Implied tuplet ratio from realDuration vs faceValue. Only fires when the first element was
// pre-marked by the parser (calculateRealDurations Phase 3b), so v0xC4 notes with incidental
// MIDI drift are never misidentified as implied triplets.
static void getImplied(const std::vector<const EncMeasureElem*>& grp,
                       int& outActual, int& outNormal)
{
    outActual = 0;
    outNormal = 0;
    if (grp.empty()) {
        return;
    }
    const EncMeasureElem* e = grp[0];
    if (!e->impliedTupletMember()) {
        return;
    }
    const quint8 fv = fvLow(e->faceValueByte());
    const qint16 rdur = e->realDuration;
    if (fv >= 4) {
        outActual = detectImpliedTuplet(rdur, fv, outNormal);
    }
}

// Face value as Fraction for the first element of a chord group.
static Fraction getFaceValue(const std::vector<const EncMeasureElem*>& grp)
{
    if (grp.empty()) {
        return Fraction(0, 1);
    }
    const quint8 fv = fvLow(grp[0]->faceValueByte());
    return faceValue2DurationType(fv) == DurationType::V_INVALID ? Fraction(0, 1)
           : TDuration(faceValue2DurationType(fv)).fraction();
}

// Actual Encore-tick duration of chord at index k.
// = fv_ticks x (nn/an) for explicit tup, = fv_ticks for plain note.
static int actualDurEnc(int k,
                        const std::vector<std::vector<const EncMeasureElem*> >& chords,
                        int n)
{
    if (k < 0 || k >= n || chords[k].empty()) {
        return 0;
    }
    const EncMeasureElem* ch = chords[k][0];
    const quint8 fv = fvLow(ch->faceValueByte());
    const quint8 tupByte = ch->tupletByte();
    const int fvt = faceValue2ticks(fv);
    const int an2 = tupByte >> 4, nn2 = tupByte & 0x0F;
    if (an2 > 0 && nn2 > 0) {
        return (fvt * nn2 + an2 / 2) / an2;  // tuplet-scaled, rounded
    }
    return fvt;
}

// Segment-override pre-pass: when N notes share a tup byte but N is not a multiple of actualN,
// reinterpret as [N:m] where m = round(available / fv_ticks).
// available = durTicks - leading_dur - trailing_dur.
static void processSegmentOverrides(
    const std::vector<std::vector<const EncMeasureElem*> >& chords,
    int n,
    const EncMeasure& encMeas,
    std::map<const EncMeasureElem*, std::pair<int, int> >* overrideRatios,
    std::set<const EncMeasureElem*>& result)
{
    int j = 0;
    while (j < n) {
        int a0 = 0, n0 = 0;
        getExplicit(chords[j], a0, n0);
        if (a0 <= 0) {
            ++j;
            continue;
        }

        // Find end of same-tup run.
        int segStart = j;
        while (j < n) {
            int aj = 0, nj = 0;
            getExplicit(chords[j], aj, nj);
            if (aj != a0 || nj != n0) {
                break;
            }
            ++j;
        }
        int N = j - segStart;

        // Override only when N exceeds one group and is not a clean multiple.
        if (N <= a0 || (N % a0) == 0) {
            continue;
        }

        // Require uniform face value and NOTE type throughout the segment.
        const Fraction fv0 = getFaceValue(chords[segStart]);
        if (fv0 <= Fraction(0, 1)) {
            continue;
        }
        bool allOk = !chords[segStart].empty()
                     && (static_cast<EncElemType>(chords[segStart][0]->type) == EncElemType::NOTE);
        for (int k = segStart + 1; k < j && allOk; ++k) {
            allOk = !chords[k].empty()
                    && (static_cast<EncElemType>(chords[k][0]->type) == EncElemType::NOTE)
                    && (getFaceValue(chords[k]) == fv0);
        }
        if (!allOk) {
            continue;
        }

        // Leading/trailing durations to compute available space for the segment.
        int leadingDur = 0;
        for (int k = 0; k < segStart; ++k) {
            leadingDur += actualDurEnc(k, chords, n);
        }
        int trailingDur = 0;
        for (int k = j; k < n; ++k) {
            trailingDur += actualDurEnc(k, chords, n);
        }

        const int available = static_cast<int>(encMeas.durTicks) - leadingDur - trailingDur;
        if (available <= 0) {
            continue;
        }

        // fv_enc = fv0 x durTicks (fv0 is whole-note-relative; durTicks spans one measure).
        const int fvEnc = static_cast<int>(
            static_cast<long long>(fv0.numerator()) * encMeas.durTicks / fv0.denominator());
        if (fvEnc <= 0) {
            continue;
        }

        // m = round(available / fvEnc)
        const int m = (available + fvEnc / 2) / fvEnc;
        if (m <= 0) {
            continue;
        }

        // Tolerance: error must be < 10% of available.
        if (std::abs(m * fvEnc - available) * 10 > available) {
            continue;
        }

        const Fraction tupTicks = fv0 * m;
        if (!fitsTDuration(tupTicks) || !isStandardExplicitTuplet(N, m)) {
            continue;
        }

        // Skip override when the non-override interpretation (complete groups + orphan plain notes) fits the measure.
        // Example: 4 notes tup=3:2 + 1 plain Q: non-override total=960 <= 960 -> leave alone.
        {
            const int completeTicks = (N / a0) * (fvEnc * n0);
            const int orphanTicks   = (N % a0) * fvEnc;
            const int noOverrideTotal = leadingDur + completeTicks + orphanTicks + trailingDur;
            if (noOverrideTotal <= static_cast<int>(encMeas.durTicks)) {
                continue;  // non-override interpretation fits: skip override
            }
        }

        // Mark segment and record override ratio.
        for (int k = segStart; k < j; ++k) {
            for (const EncMeasureElem* e2 : chords[k]) {
                result.insert(e2);
                (*overrideRatios)[e2] = { N, m };
            }
        }
    }
}

// Sandwich heuristic: a note whose tup byte is missing/mismatched still belongs to the current
// bracket when the NEXT note matches the ratio, the orphan's face value equals baseLen, and it sits
// at the expected advance tick after the previous member (v0xC4 live recording occasionally drops
// the byte). i must be inside an open group (faceSum > 0) with a valid previous and next chord.
static bool isSandwichOrphan(
    const std::vector<std::vector<const EncMeasureElem*> >& chords, int i, int n,
    int actualN, int normalN, Fraction baseLen, Fraction faceSum)
{
    if (!(faceSum > Fraction(0, 1) && i + 1 < n
          && !chords[i].empty() && !chords[i - 1].empty())) {
        return false;
    }
    int a3 = 0, n3 = 0;
    getExplicit(chords[i + 1], a3, n3);
    const Fraction fvOrphan = getFaceValue(chords[i]);
    if (a3 != actualN || n3 != normalN || fvOrphan != baseLen) {
        return false;
    }
    // Orphan must land at the expected advance tick after the previous member.
    const int advNum = baseLen.numerator() * normalN;
    const int advDen = baseLen.denominator() * actualN;
    const int advTicks = (advNum * kEncWholeTicks + advDen / 2) / advDen;
    const int tol = std::max(4, advTicks / 4);
    const int lastTick   = static_cast<int>(chords[i - 1][0]->tick);
    const int orphanTick = static_cast<int>(chords[i][0]->tick);
    return std::abs(orphanTick - lastTick - advTicks) <= tol;
}

// Implied (v0xC2): require exactly actualN consecutive matching groups.
static void processImpliedTupletGroup(
    int& i, int actualN, int normalN,
    const std::vector<std::vector<const EncMeasureElem*> >& chords,
    int n,
    std::set<const EncMeasureElem*>& result)
{
    if (i + actualN > n) {
        ++i;
        return;
    }
    bool allMatch = true;
    for (int j = 1; j < actualN && allMatch; ++j) {
        int a2 = 0, n2 = 0;
        getImplied(chords[i + j], a2, n2);
        allMatch = (a2 == actualN && n2 == normalN);
    }
    if (allMatch) {
        for (int j = 0; j < actualN; ++j) {
            for (const EncMeasureElem* e : chords[i + j]) {
                result.insert(e);
            }
        }
        i += actualN;
    } else {
        ++i;
    }
}

// Nested-tuplet: the current group closed via a no-downdate reduction, and the next
// (actualN - 1) notes also share innerBaseLen -> record NestedTupletInfo for the emitters
// and pull the peeked-ahead notes into the result set.
static void detectNestedTuplet(
    std::vector<NestedTupletInfo>* nestedInfos,
    int innerGroupStartIdx, Fraction innerBaseLen, Fraction originalBaseLen,
    int actualN, int normalN, int i, int n,
    const std::vector<std::vector<const EncMeasureElem*> >& chords,
    std::set<const EncMeasureElem*>& result)
{
    if (!(nestedInfos && innerGroupStartIdx >= 2
          && innerBaseLen > Fraction(0, 1)
          && innerBaseLen < originalBaseLen)) {
        return;
    }
    int peekAhead = actualN - (i - innerGroupStartIdx);  // remaining notes needed to complete inner group
    bool innerOk = (peekAhead >= 0);
    int innerEndIdx = i - 1;  // last note of inner group so far (= end of current group)
    if (innerOk && peekAhead > 0) {
        innerOk = false;
        if (i + peekAhead <= n) {
            innerOk = true;
            for (int p = 0; p < peekAhead && innerOk; ++p) {
                int a3 = 0, n3 = 0;
                getExplicit(chords[i + p], a3, n3);
                if (a3 != actualN || n3 != normalN) {
                    innerOk = false;
                } else {
                    const Fraction fvp = getFaceValue(chords[i + p]);
                    if (fvp != innerBaseLen) {
                        innerOk = false;
                    }
                }
            }
            if (innerOk) {
                innerEndIdx = i + peekAhead - 1;
            }
        }
    }
    if (innerOk && innerEndIdx >= innerGroupStartIdx) {
        NestedTupletInfo ni;
        ni.outerActualN = actualN;
        ni.outerNormalN = normalN;
        ni.innerActualN = actualN;
        ni.innerNormalN = normalN;
        if (!chords[innerGroupStartIdx].empty()) {
            ni.innerFirst = chords[innerGroupStartIdx][0];
        }
        if (!chords[innerEndIdx].empty()) {
            ni.innerLast = chords[innerEndIdx][0];
        }
        if (ni.innerFirst && ni.innerLast) {
            nestedInfos->push_back(ni);
            // Include peeked-ahead notes in result.
            for (int p = 0; p < peekAhead; ++p) {
                for (const EncMeasureElem* e2 : chords[i + p]) {
                    result.insert(e2);
                }
            }
        }
    }
}

// Partial end group: when realDuration fills to the measure end while the face-value sum would
// overflow without tuplet scaling, mark [groupStart, i) as an incomplete implied tuplet.
static void detectPartialEndGroup(
    int groupStart, int i,
    const std::vector<std::vector<const EncMeasureElem*> >& chords,
    const EncMeasure& encMeas, bool seenCompleteGroup,
    std::set<const EncMeasureElem*>* partialEndGroup,
    std::set<const EncMeasureElem*>& result)
{
    if (i <= groupStart) {
        return;
    }
    int startTick = 0;
    if (!chords[groupStart].empty()) {
        startTick = static_cast<int>(chords[groupStart][0]->tick);
    }
    int rdurSum = 0;
    int faceTickSum = 0;
    for (int j = groupStart; j < i; ++j) {
        if (!chords[j].empty()) {
            const EncMeasureElem* ch = chords[j][0];
            rdurSum += std::max(0, static_cast<int>(ch->realDuration));
            EncElemType cht = static_cast<EncElemType>(ch->type);
            quint8 fv = 0;
            if (cht == EncElemType::NOTE) {
                fv = static_cast<const EncNote*>(ch)->faceValue & 0x0F;
            } else if (cht == EncElemType::REST) {
                fv = static_cast<const EncRest*>(ch)->faceValue & 0x0F;
            }
            faceTickSum += faceValue2ticks(fv);
        }
    }
    const bool rdurFillsMeasure = (rdurSum > 0)
                                  && (startTick + rdurSum == static_cast<int>(encMeas.durTicks));
    const bool faceWouldOverflow = (startTick + faceTickSum
                                    > static_cast<int>(encMeas.durTicks));
    // Guard: skip when a complete group was already found (tail MIDI notes after
    // a full group can accidentally satisfy rdurFillsMeasure).
    if (rdurFillsMeasure && faceWouldOverflow && !seenCompleteGroup) {
        for (int j = groupStart; j < i; ++j) {
            for (const EncMeasureElem* e2 : chords[j]) {
                result.insert(e2);
                if (partialEndGroup) {
                    partialEndGroup->insert(e2);
                }
            }
        }
    }
}

std::set<const EncMeasureElem*> computeImpliedTupletMembers(
    const MeasureElemRefVec& sortedElems,
    const EncMeasure& encMeas,
    int totalStaves,
    std::set<const EncMeasureElem*>* partialEndGroup,
    std::vector<NestedTupletInfo>* nestedInfos,
    std::map<const EncMeasureElem*, std::pair<int, int> >* overrideRatios)
{
    std::set<const EncMeasureElem*> result;

    // Group by (staffIdx, voice): collapse same-tick notes into chord groups.
    std::map<std::pair<int, int>, std::vector<std::vector<const EncMeasureElem*> > > voiceChords;
    for (const EncMeasureElem* e : sortedElems) {
        EncElemType et = static_cast<EncElemType>(e->type);
        if (et != EncElemType::NOTE && et != EncElemType::REST) {
            continue;
        }
        if (e->tick >= encMeas.durTicks) {
            continue;
        }
        int si = static_cast<int>(e->staffIdx);
        int v  = static_cast<int>(e->voice);
        if (si >= totalStaves || v >= static_cast<int>(VOICES)) {
            continue;
        }
        auto key = std::make_pair(si, v);
        auto& chords = voiceChords[key];
        if (!chords.empty() && chords.back()[0]->tick == e->tick) {
            chords.back().push_back(e);  // same chord group
        } else {
            chords.push_back({ e });     // new chord group
        }
    }

    for (auto& [key, chords] : voiceChords) {
        int n = static_cast<int>(chords.size());

        if (overrideRatios && n >= 2 && encMeas.durTicks > 0) {
            processSegmentOverrides(chords, n, encMeas, overrideRatios, result);
        }

        int i = 0;
        while (i < n) {
            // Already handled by segment-override pre-pass.
            if (overrideRatios && !chords[i].empty()
                && overrideRatios->count(chords[i][0])) {
                ++i;
                continue;
            }

            // Try explicit tup byte first, then implied.
            int actualN = 0, normalN = 0;
            bool isExplicit = false;
            getExplicit(chords[i], actualN, normalN);
            if (actualN > 0) {
                isExplicit = true;
            } else {
                getImplied(chords[i], actualN, normalN);
            }
            if (actualN < 2 || normalN < 1) {
                ++i;
                continue;
            }

            if (isExplicit) {
                // Explicit: accumulate faceSum; close when faceSum >= threshold.
                // No-downdate rule: baseLen only shrinks when faceSum still fits the new threshold,
                // allowing mixed-duration brackets like {Q,E}/3:2 or {Q,Q,8,8}/3:2.
                Fraction baseLen = getFaceValue(chords[i]);
                if (baseLen <= Fraction(0, 1)) {
                    ++i;
                    continue;
                }
                Fraction threshold = baseLen * actualN;
                Fraction faceSum(0, 1);
                int groupStart = i;
                Fraction originalBaseLen = baseLen;  // before any no-downdate
                int innerGroupStartIdx = -1;          // index of inner group's first chord
                Fraction innerBaseLen(0, 1);
                bool seenCompleteGroup = false;        // at least one full group closed
                while (i < n) {
                    int a2 = 0, n2 = 0;
                    getExplicit(chords[i], a2, n2);
                    if (a2 != actualN || n2 != normalN) {
                        if (isSandwichOrphan(chords, i, n, actualN, normalN, baseLen, faceSum)) {
                            a2 = actualN;
                            n2 = normalN;
                        } else {
                            break;
                        }
                    }
                    // No-downdate: shrink baseLen only when faceSum still fits the new (smaller) threshold.
                    const Fraction fv_i = getFaceValue(chords[i]);
                    if (fv_i > Fraction(0, 1) && fv_i < baseLen) {
                        const Fraction newThreshold = fv_i * actualN;
                        if (faceSum <= newThreshold) {
                            // Record where the inner group starts (= the downdating note).
                            innerGroupStartIdx = i;
                            innerBaseLen       = fv_i;
                            baseLen    = fv_i;
                            threshold  = newThreshold;
                        }
                    }
                    faceSum += fv_i;
                    ++i;
                    if (faceSum < threshold) {
                        continue;
                    }
                    // Complete group: mark [groupStart, i-1]
                    for (int j = groupStart; j < i; ++j) {
                        for (const EncMeasureElem* e : chords[j]) {
                            result.insert(e);
                        }
                    }

                    detectNestedTuplet(nestedInfos, innerGroupStartIdx, innerBaseLen,
                                       originalBaseLen, actualN, normalN, i, n, chords, result);

                    faceSum   = Fraction(0, 1);
                    groupStart = i;
                    originalBaseLen    = Fraction(0, 1);
                    innerGroupStartIdx = -1;
                    innerBaseLen       = Fraction(0, 1);
                    seenCompleteGroup  = true;
                    // Reset for next group.
                    if (i < n) {
                        baseLen         = getFaceValue(chords[i]);
                        threshold       = baseLen * actualN;
                        originalBaseLen = baseLen;
                    }
                }
                detectPartialEndGroup(groupStart, i, chords, encMeas, seenCompleteGroup,
                                      partialEndGroup, result);
            } else {
                processImpliedTupletGroup(i, actualN, normalN, chords, n, result);
            }
        }
    }
    return result;
}
} // namespace mu::iex::enc
