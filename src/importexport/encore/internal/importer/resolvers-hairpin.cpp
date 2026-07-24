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

// Post-pass: resolve hairpin (crescendo/diminuendo) spanner endpoints.

#include "resolvers.h"
#include "coords.h"
#include "../parser/elem.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/dynamic.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/segment.h"

using namespace mu::engraving;

namespace mu::iex::enc {
// Snap the hairpin end to the note/rest nearest xoffset2, else return currentEndTick unchanged.
// targetMeasTick is the MuseScore start tick of enc measure ph.endMeasIdx.
static Fraction resolveHairpinEndByXoffset(
    const PendingHairpin& ph,
    const EncRoot& enc,
    Fraction currentEndTick,
    Fraction targetMeasTick)
{
    if (ph.endMeasIdx < 0
        || ph.endMeasIdx >= static_cast<int>(enc.measures.size())) {
        return currentEndTick;
    }
    const EncMeasure& endEncMeas = enc.measures[static_cast<size_t>(ph.endMeasIdx)];
    const int wholeTicks = encWholeNoteTicks(endEncMeas);
    const int xoff2 = ph.hairpinXoffset2;
    int bestEncTick     = -1;
    int bestXoff        = -1;
    int anyPositiveXoff = -1;   // sentinel: any note/rest with xoff > 0 exists
    forEachStaffNoteXoff(endEncMeas, ph.staffIdx, /*includeRests*/ true, /*lineSlotByRawByte*/ nullptr,
                         [&](const EncMeasureElem* em, int xoff) {
        if (xoff <= 0) {
            return true;
        }
        anyPositiveXoff = xoff;
        if (xoff <= xoff2 && xoff > bestXoff) {
            bestXoff    = xoff;
            bestEncTick = static_cast<int>(em->tick);
        }
        return true;
    });
    if (bestEncTick >= 0) {
        // Snap end to the note/rest whose xoffset best matches xoff2.
        Fraction snapEnd = targetMeasTick + Fraction(bestEncTick, wholeTicks).reduced();
        return std::min(currentEndTick, snapEnd);
    } else if (anyPositiveXoff >= 0) {
        // xoff2 precedes all notes with positive xoffsets: end at the barline.
        return std::min(currentEndTick, targetMeasTick);
    }
    // else: no notes with positive xoffsets (synthetic/empty data); keep maxEndTick.
    return currentEndTick;
}

void resolveHairpins(BuildCtx& ctx)
{
    MasterScore* score = ctx.score;
    const EncRoot& enc = ctx.enc;

    // Pre-pass: detect same-measure CRESC+DIM swell pairs on the same track and split the measure
    // at its midpoint so each hairpin covers a half. xoffset2 pixels do not map linearly to ticks
    // when the measure has empty beats, so a midpoint split better matches Encore's visual.
    const size_t n = ctx.pendingHairpins.size();
    std::vector<Fraction> startOverride(n, Fraction(-1, 1)); // -1 = no override
    std::vector<Fraction> endOverride(n, Fraction(-1, 1));

    for (size_t i = 0; i < n; ++i) {
        const PendingHairpin& ph1 = ctx.pendingHairpins[i];
        if (ph1.type != HairpinType::CRESC_HAIRPIN) {
            continue;
        }
        const Measure* m1 = score->tick2measure(ph1.startTick);
        if (!m1) {
            continue;
        }
        for (size_t j = i + 1; j < n; ++j) {
            const PendingHairpin& ph2 = ctx.pendingHairpins[j];
            if (ph2.track != ph1.track) {
                continue;
            }
            if (ph2.type != HairpinType::DIM_HAIRPIN) {
                continue;
            }
            const Measure* m2 = score->tick2measure(ph2.startTick);
            if (m2 != m1) {
                continue;  // different measures: not a same-measure swell pair
            }
            // Both CRESC and DIM start in the same measure: split at midpoint.
            const Fraction midTick = m1->tick() + m1->ticks() / 2;
            endOverride[i]   = midTick;                         // CRESC ends at mid
            startOverride[j] = midTick;                         // DIM starts at mid
            endOverride[j]   = m1->tick() + m1->ticks();        // DIM ends at barline
            break;
        }
    }

    // Main pass: resolve each hairpin's endTick then create the Hairpin element.
    for (size_t i = 0; i < n; ++i) {
        const PendingHairpin& ph = ctx.pendingHairpins[i];

        Fraction startTick = (startOverride[i].numerator() >= 0) ? startOverride[i] : ph.startTick;
        Fraction endTick   = ph.maxEndTick;

        if (endOverride[i].numerator() >= 0) {
            // Swell-pair override: use the pre-computed midpoint or barline.
            endTick = endOverride[i];
        } else {
            // (1) Next Dynamic on track takes priority; handles mf<f>mf chains.
            bool foundNextDynamic = false;
            for (Segment* s = score->firstSegment(SegmentType::ChordRest); s;
                 s = s->next1(SegmentType::ChordRest)) {
                if (s->tick() <= ph.startTick) {
                    continue;
                }
                if (s->tick() > ph.maxEndTick) {
                    break;
                }
                bool stopHere = false;
                for (EngravingItem* ann : s->annotations()) {
                    if (ann && ann->isDynamic() && ann->track() == ph.track) {
                        stopHere = true;
                        break;
                    }
                }
                if (stopHere) {
                    endTick = std::min(endTick, s->tick());
                    foundNextDynamic = true;
                    break;
                }
            }

            // (2) xoffset2 snap: find the last note/rest in the target measure with xoffset <= xoff2.
            if (!foundNextDynamic
                && ph.hairpinXoffset2 > 0
                && ph.endMeasIdx >= 0
                && ph.endMeasIdx < static_cast<int>(enc.measures.size())
                && ph.endMeasIdx < static_cast<int>(ctx.measuresByIdx.size())) {
                Fraction targetMeasTick
                    = ctx.measuresByIdx[static_cast<size_t>(ph.endMeasIdx)]->tick();
                endTick = resolveHairpinEndByXoffset(ph, enc, endTick, targetMeasTick);
            }
        }

        if (endTick <= startTick) {
            continue;
        }
        // ph.track is derived from a raw grand-staff voice byte; skip a hairpin whose track is out
        // of range rather than hand an out-of-bounds track to the engraving DOM (matches ottava).
        if (!validTrack(score, ph.track)) {
            continue;
        }
        Hairpin* hp = Factory::createHairpin(score->dummy()->segment());
        hp->setTrack(ph.track);
        hp->setTrack2(ph.track);
        hp->setTick(startTick);
        hp->setTick2(endTick);
        hp->setHairpinType(ph.type);
        score->addElement(hp);
    }
}
} // namespace mu::iex::enc
