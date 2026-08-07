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

// Tuplet detection and construction: the per-voice TupletTracker, nested-tuplet annotations, and
// the pass that finds implied (v0xC2) and explicit tuplet group members in a measure.

#ifndef MU_IMPORTEXPORT_ENC_IMPORT_TUPLETS_H
#define MU_IMPORTEXPORT_ENC_IMPORT_TUPLETS_H

#include <set>

#include "engraving/dom/durationtype.h"
#include "engraving/types/fraction.h"
#include "engraving/types/types.h"

#include "../parser/elem.h"

namespace mu::engraving {
class Measure;
class Tuplet;
}

namespace mu::iex::enc {
// Tuplet state per staff+voice. Group closes when face-value sum reaches actualN * baseLen.
// This handles mixed-duration brackets (e.g. 3:2 triplet with 8th+8th+16th+16th).
struct TupletTracker {
    mu::engraving::Tuplet* currentTuplet { nullptr };
    int actualN       { 0 };        // ratio numerator (e.g. 3 for 3:2)
    int normalN       { 0 };        // ratio denominator (e.g. 2 for 3:2)
    mu::engraving::Fraction placedTicks   { 0, 1 };// cumulative cumTick advances while in this group
    mu::engraving::Fraction faceTicks     { 0, 1 };// cumulative FACE-VALUE sum of notes in this group
    mu::engraving::Fraction fullFaceSum   { 0, 1 };// target = baseLen * actualN (group closes when reached)

    bool inTuplet()  const { return currentTuplet != nullptr; }
    bool groupFull() const;

    void closeTuplet();

    mu::engraving::Tuplet* startTuplet(mu::engraving::Measure* measure, mu::engraving::Fraction tick, int aN, int normalN_,
                                       mu::engraving::DurationType baseType, mu::engraving::track_idx_t track_);

    // Duration advance per note within this tuplet group
    mu::engraving::Fraction noteAdvance(mu::engraving::DurationType baseType) const;
};

// Nested-tuplet annotation: an inner group of actualN same-ratio notes whose combined
// face-value equals one outer-group slot. Created when a flat group closes via the
// no-downdate rule and the ending smaller-fv notes form a valid inner group.
struct NestedTupletInfo {
    const EncMeasureElem* innerFirst { nullptr };   // first note of the inner group
    const EncMeasureElem* innerLast  { nullptr };   // last note of the inner group
    int innerActualN { 0 };
    int innerNormalN { 0 };
    // The outer group spans: [outerGroupStart … innerLast, then one or more outer
    // continuation notes]. outerActualN/NormalN = same as the flat group's ratio.
    int outerActualN { 0 };
    int outerNormalN { 0 };
};

// True when a Fraction fits exactly in a TDuration (power-of-two, up to 4 dots).
bool fitsTDuration(const mu::engraving::Fraction& f);

// Find all elements belonging to complete tuplet groups (implied v0xC2 or explicit). Isolated notes with matching rdur are MIDI swing drift.
// partialEndGroup: if non-null, receives measure-end partial groups (rdur fills measure AND face-value would overflow without scaling).
// nestedInfos: if non-null, receives nested-tuplet annotations for detected inner groups.
// overrideRatios: if non-null, receives {actualN, normalN} overrides for notes that triggered
//   uniform-fill detection (e.g. 15 equal notes → [15:8] instead of the tup-byte ratio [9:5]).
std::set<const EncMeasureElem*> computeImpliedTupletMembers(
    const MeasureElemRefVec& sortedElems, const EncMeasure& encMeas, int totalStaves,
    std::set<const EncMeasureElem*>* partialEndGroup = nullptr, std::vector<NestedTupletInfo>* nestedInfos = nullptr,
    std::map<const EncMeasureElem*, std::pair<int, int> >* overrideRatios = nullptr);
} // namespace mu::iex::enc

#endif // MU_IMPORTEXPORT_ENC_IMPORT_TUPLETS_H
