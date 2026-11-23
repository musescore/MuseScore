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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "editposition.h"

#include "../dom/engravingitem.h"
#include "../dom/hairpin.h"
#include "../dom/score.h"
#include "../dom/slurtie.h"
#include "../dom/spanner.h"

#include "../rendering/iscorerenderer.h"

using namespace mu::engraving;

// The slur/tie grips whose user offsets are stored as properties
struct SlurTieGripProperty {
    Grip grip;
    Pid pid;
};

constexpr std::array<SlurTieGripProperty, 4> SLUR_TIE_GRIP_PROPERTIES { {
    { Grip::START, Pid::BEZIER_OFFSET1 },
    { Grip::BEZIER1, Pid::BEZIER_OFFSET2 },
    { Grip::BEZIER2, Pid::BEZIER_OFFSET3 },
    { Grip::END, Pid::BEZIER_OFFSET4 },
} };

using SlurTieGripPoints = std::array<PointF, SLUR_TIE_GRIP_PROPERTIES.size ()>;

SlurTieGripPoints slurTieGripPositions(const SlurTieSegment* segment)
{
    SlurTieGripPoints positions;
    for (size_t i = 0; i < SLUR_TIE_GRIP_PROPERTIES.size(); ++i) {
        positions.at(i) = segment->ups(SLUR_TIE_GRIP_PROPERTIES.at(i).grip).pos();
    }
    return positions;
}

SlurTieGripPoints slurTieGripOffsets(const SlurTieSegment* segment)
{
    SlurTieGripPoints offsets;
    for (size_t i = 0; i < SLUR_TIE_GRIP_PROPERTIES.size(); ++i) {
        offsets.at(i) = segment->ups(SLUR_TIE_GRIP_PROPERTIES.at(i).grip).off;
    }
    return offsets;
}

void EditPosition::freezeItemsPositions(Transaction&, const std::vector<EngravingItem*>& items)
{
    if (items.empty()) {
        return;
    }
    Score* score = items.front()->score();

    std::unordered_map<EngravingItem*, PointF> frozenItems;
    // Spanner segments also need their end point (pos2, relative to pos) kept in place
    std::unordered_map<SpannerSegment*, PointF> frozenSpannerEnds;
    // Slur and tie segments also need their grips (relative to pos) kept in place
    std::unordered_map<SlurTieSegment*, SlurTieGripPoints> frozenSlurTieGrips;
    for (EngravingItem* item : items) {
        if (item->generated() || !autoplaceAppliesToType(item->type()) || !item->autoplace()) {
            continue;
        }

        frozenItems.emplace(item, item->pos());
        if (item->isSpannerSegment()) {
            SpannerSegment* spannerSegment = toSpannerSegment(item);
            frozenSpannerEnds.emplace(spannerSegment, spannerSegment->pos2());
        }

        if (item->isSlurTieSegment()) {
            SlurTieSegment* slurTieSeg = toSlurTieSegment(item);
            frozenSlurTieGrips.emplace(slurTieSeg, slurTieGripPositions(slurTieSeg));
        }

        if (item->isHairpinSegment()) {
            HairpinSegment* hpSeg = toHairpinSegment(item);
            Hairpin* hairpin = hpSeg->hairpin();
            hairpin->undoChangeProperty(Pid::SNAP_BEFORE, false);
            hairpin->undoChangeProperty(Pid::SNAP_AFTER, false);
        }

        PropertyFlags autoplacePf = item->propertyFlags(Pid::AUTOPLACE);
        if (autoplacePf == PropertyFlags::STYLED) {
            autoplacePf = PropertyFlags::UNSTYLED;
        }

        item->undoChangeProperty(Pid::AUTOPLACE, false, autoplacePf);

        item->triggerLayout();
    }

    if (frozenItems.empty()) {
        return;
    }

    // Some items are laid out relative to the positions (including offsets) of other items
    // Setting one item's offset can move another item's layout position, so lay out and update offsets
    // until they stop changing. A chain of N dependent items needs roughly N + 1 passes.
    // We can't know whether elements affect each other's position without performing another layout
    // This is currently limited to N + 3 passes to protect against (possible) circular layout dependencies
    std::unordered_map<EngravingItem*, PointF> originalOffsets;
    for (const auto& [item, oldPos] : frozenItems) {
        originalOffsets.emplace(item, item->offset());
    }
    std::unordered_map<SpannerSegment*, PointF> originalOffset2s;
    for (const auto& [spannerSegment, oldPos2] : frozenSpannerEnds) {
        originalOffset2s.emplace(spannerSegment, spannerSegment->userOff2());
    }
    std::unordered_map<SlurTieSegment*, SlurTieGripPoints> originalSlurTieGripOffsets;
    for (const auto& [slurTieSeg, oldGripPositions] : frozenSlurTieGrips) {
        originalSlurTieGripOffsets.emplace(slurTieSeg, slurTieGripOffsets(slurTieSeg));
    }

    const size_t maxPasses = frozenItems.size() + 3;
    bool converged = false;
    for (size_t pass = 0; pass < maxPasses && !converged; ++pass) {
        score->update();

        converged = true;
        for (const auto& [item, oldPos] : frozenItems) {
            const PointF newOffset = oldPos - item->ldata()->pos();
            const PointF& currentOffset = item->offset();
            // Compare with an absolute tolerance (see slur grips below)
            if ((newOffset - currentOffset).isNull()) {
                continue;
            }

            converged = false;
            item->setOffset(newOffset);
            item->triggerLayout();
        }

        for (const auto& [spannerSegment, oldPos2] : frozenSpannerEnds) {
            const PointF newOffset2 = oldPos2 - spannerSegment->ipos2();
            if ((newOffset2 - spannerSegment->userOff2()).isNull()) {
                continue;
            }

            converged = false;
            spannerSegment->setUserOff2(newOffset2);
            spannerSegment->triggerLayout();
        }

        // A slur's bezier points are laid out relative to its (offset) end points, so changing
        // the end point offsets requires another pass to recalculate the bezier points
        for (const auto& [slurTieSeg, oldGripPositions] : frozenSlurTieGrips) {
            for (size_t i = 0; i < SLUR_TIE_GRIP_PROPERTIES.size(); ++i) {
                const Grip grip = SLUR_TIE_GRIP_PROPERTIES.at(i).grip;
                const UP& curUP = slurTieSeg->ups(grip);
                // Grip points go through coordinate transforms in layout, so compare with an absolute tolerance.
                // An exact (or relative) comparison of the offsets can oscillate forever on rounding errors near zero
                if ((oldGripPositions.at(i) - curUP.pos()).isNull()) {
                    continue;
                }

                converged = false;
                const PointF newGripOffset = oldGripPositions.at(i) - curUP.p;
                slurTieSeg->setSlurOffset(grip, newGripOffset);
                slurTieSeg->triggerLayout();
            }
        }

        // We know we only need 1 layout when freezing a single item (unless it's a slur or tie)
        if (frozenItems.size() == 1 && frozenSlurTieGrips.empty()) {
            converged = true;
            break;
        }
    }

    IF_ASSERT_FAILED(converged) {
        LOGW() << "Frozen item offsets did not converge after " << maxPasses << " layout passes";
    }

    // Make actual undoable change. Not using undoable functions above avoids having N + 1 commands for affected elements in the transaction
    for (const auto& [item, originalOffset] : originalOffsets) {
        const PointF frozenOffset = item->offset();
        // Restore the original value so undoChangeProperty registers the change for undo
        item->setOffset(originalOffset);

        PropertyFlags offsetPf = item->propertyFlags(Pid::OFFSET);
        if (offsetPf == PropertyFlags::STYLED) {
            offsetPf = PropertyFlags::UNSTYLED;
        }

        item->undoChangeProperty(Pid::OFFSET, frozenOffset, offsetPf);
    }

    for (const auto& [spannerSegment, originalOffset2] : originalOffset2s) {
        const PointF frozenOffset2 = spannerSegment->userOff2();
        spannerSegment->setUserOff2(originalOffset2);
        spannerSegment->undoChangeProperty(Pid::OFFSET2, frozenOffset2);
    }

    for (const auto& [slurTieSeg, originalGripOffsets] : originalSlurTieGripOffsets) {
        for (size_t i = 0; i < SLUR_TIE_GRIP_PROPERTIES.size(); ++i) {
            const Grip grip = SLUR_TIE_GRIP_PROPERTIES.at(i).grip;
            const Pid pid = SLUR_TIE_GRIP_PROPERTIES.at(i).pid;
            const PointF frozenGripOffset = slurTieSeg->ups(grip).off;
            slurTieSeg->setSlurOffset(grip, originalGripOffsets.at(i));
            slurTieSeg->undoChangeProperty(pid, frozenGripOffset, slurTieSeg->propertyFlags(pid));
        }
    }
}
