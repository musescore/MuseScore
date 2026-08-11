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

#include "chordbracketlayout.h"

#include <algorithm>
#include <vector>

#include "dom/chord.h"
#include "dom/chordbracket.h"
#include "dom/engravingitem.h"
#include "dom/note.h"
#include "dom/part.h"
#include "dom/segment.h"

#include "chordlayout.h"
#include "horizontalspacing.h"
#include "tlayout.h"

using namespace muse;
using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

//---------------------------------------------------------
//   chordBracketsInSegment
//---------------------------------------------------------

std::vector<ChordBracket*> ChordBracketLayout::chordBracketsInSegment(const Segment* segment)
{
    std::vector<ChordBracket*> result;

    if (!segment) {
        return result;
    }

    for (EngravingItem* item : segment->elist()) {
        if (!item || !item->isChord()) {
            continue;
        }

        for (EngravingItem* child : toChord(item)->el()) {
            if (child && child->isChordBracket()) {
                result.push_back(toChordBracket(child));
            }
        }
    }

    return result;
}

//---------------------------------------------------------
//   bracketVisualSpan
//---------------------------------------------------------

ChordBracketLayout::BracketVisualSpan ChordBracketLayout::bracketVisualSpan(const ChordBracket* bracket)
{
    const Chord* startChord = bracket->chord();
    EngravingItem* endItem = startChord->segment()->element(bracket->endTrack());
    const Chord* endChord = endItem && endItem->isChord() ? toChord(endItem) : startChord;

    const auto topY = [](const Chord* chord) {
        const Note* note = chord->upNote();
        return chord->pos().y() + note->pos().y() + note->ldata()->bbox().top();
    };

    const auto bottomY = [](const Chord* chord) {
        const Note* note = chord->downNote();
        return chord->pos().y() + note->pos().y() + note->ldata()->bbox().bottom();
    };

    const staff_idx_t startStaff = startChord->vStaffIdx();
    const staff_idx_t endStaff = endChord->vStaffIdx();

    const double spatium = bracket->spatium();
    const double lineWidth = bracket->style().styleAbsolute(Sid::chordBracketLineWidth);

    // Match TLayout::layoutChordBracket() padding.
    const double topPadding = bracket->userLen1() + 0.25 * spatium + 0.5 * lineWidth;
    const double bottomPadding = bracket->userLen2() + 0.5 * spatium + 0.5 * lineWidth;

    BracketVisualSpan result;
    result.topStaff = std::min(startStaff, endStaff);
    result.bottomStaff = std::max(startStaff, endStaff);

    if (startStaff == endStaff) {
        result.topY = std::min(topY(startChord), topY(endChord)) - topPadding;
        result.bottomY = std::max(bottomY(startChord), bottomY(endChord)) + bottomPadding;

        // Enforce the minimum length using endpoint coordinates.
        const double minLen = 2 * spatium;
        const double diff = minLen - std::abs(result.bottomY - result.topY);

        if (diff > 0.0) {
            if (bracket->hookPos() == DirectionV::DOWN) {
                result.topY -= diff;
            } else if (bracket->hookPos() == DirectionV::AUTO) {
                result.topY -= 0.5 * diff;
                result.bottomY += 0.5 * diff;
            } else {
                result.bottomY += diff;
            }
        }
    } else {
        const Chord* upperAnchor = startStaff < endStaff ? startChord : endChord;
        const Chord* lowerAnchor = startStaff < endStaff ? endChord : startChord;

        result.topY = topY(upperAnchor) - topPadding;
        result.bottomY = bottomY(lowerAnchor) + bottomPadding;
    }

    result.topY += bracket->offset().y();
    result.bottomY += bracket->offset().y();

    return result;
}

//---------------------------------------------------------
//   bracketCollisionShape
//---------------------------------------------------------

Shape ChordBracketLayout::bracketCollisionShape(const ChordBracket* bracket, const BracketVisualSpan& span, staff_idx_t staffIdx,
                                                double x)
{
    if (staffIdx < span.topStaff || staffIdx > span.bottomStaff) {
        return Shape();
    }

    double top = span.topY;
    double bottom = span.bottomY;

    // Final staff distances are not known during horizontal spacing.
    // Approximate the bracket per staff: extend it toward the other endpoint on the endpoint staves
    // and across the full height of intermediate staves.
    static constexpr double CROSS_STAFF_COLLISION_EXTENT = 100000.0;

    if (span.topStaff != span.bottomStaff) {
        top = staffIdx == span.topStaff ? span.topY : -CROSS_STAFF_COLLISION_EXTENT;
        bottom = staffIdx == span.bottomStaff ? span.bottomY : CROSS_STAFF_COLLISION_EXTENT;
    }

    if (bottom < top) {
        std::swap(top, bottom);
    }

    return Shape(RectF(x, top, bracket->width(), bottom - top), bracket);
}

//---------------------------------------------------------
//   layoutHorizontal
//---------------------------------------------------------
void ChordBracketLayout::layoutHorizontal(ChordBracket* bracket)
{
    Chord* owner = bracket->chord();
    Segment* segment = owner->segment();
    const TrackRange range = bracket->part()->trackRange();
    const BracketVisualSpan span = bracketVisualSpan(bracket);

    double requiredDistance = 0.0;
    const double baseX = owner->pos().x() + bracket->offset().x();

    for (track_idx_t track = range.startTrack; track < range.endTrack; ++track) {
        EngravingItem* item = segment->element(track);
        if (!item || !item->isChord()) {
            continue;
        }

        Chord* chord = toChord(item);
        Shape chordShape = chord->shape();
        chordShape.removeTypes({ ElementType::CHORD_BRACKET });
        chordShape.translate(chord->pos());
        if (chordShape.empty()) {
            continue;
        }

        Shape bracketShape = bracketCollisionShape(bracket, span, chord->vStaffIdx(), baseX);
        if (bracketShape.empty()) {
            continue;
        }

        const double distance = bracket->rightSide()
                                ? HorizontalSpacing::minHorizontalDistance(chordShape, bracketShape, bracket->spatium())
                                : HorizontalSpacing::minHorizontalDistance(bracketShape, chordShape, bracket->spatium());
        requiredDistance = std::max(requiredDistance, distance);
    }

    bracket->mutldata()->setPosX(bracket->rightSide() ? requiredDistance : -requiredDistance);
}

//---------------------------------------------------------
//   layoutSegment
//---------------------------------------------------------

void ChordBracketLayout::layoutSegment(Segment* segment, LayoutContext& ctx)
{
    if (!segment) {
        return;
    }

    for (ChordBracket* bracket : chordBracketsInSegment(segment)) {
        Chord* owner = bracket->chord();
        if (!owner || owner->segment() != segment || !bracket->part() || owner->onTabStaff()) {
            continue;
        }

        // Initialize the bracket's width, vertical geometry, and Shape.
        // Replace the chord-local horizontal position with the segment-level result.
        TLayout::layoutItem(bracket, ctx);
        layoutHorizontal(bracket);

        // Rebuild the owner Shape after moving the bracket.
        ChordLayout::fillShape(owner, owner->mutldata());
    }
}

//---------------------------------------------------------
//   updateHorizontalSpacing
//---------------------------------------------------------

void ChordBracketLayout::updateHorizontalSpacing(const Segment* firstSegment, const Segment* secondSegment,
                                                 staff_idx_t staffIdx, double squeezeFactor, double& minDistance)
{
    if (!firstSegment || !secondSegment) {
        return;
    }

    auto updateForBrackets = [&](const Segment* segment, const Shape& otherShape, bool rightSide) {
        for (ChordBracket* bracket : chordBracketsInSegment(segment)) {
            const Chord* owner = bracket->chord();
            if (!owner || owner->onTabStaff() || !bracket->visible() || !bracket->addToSkyline() || bracket->rightSide() != rightSide) {
                continue;
            }

            const BracketVisualSpan span = bracketVisualSpan(bracket);

            if (span.topStaff == span.bottomStaff) {
                continue;
            }

            const double x = owner->pos().x() + bracket->pos().x();
            const Shape bracketShape = bracketCollisionShape(bracket, span, staffIdx, x);

            if (bracketShape.empty()) {
                continue;
            }

            const double distance = rightSide
                                    ? HorizontalSpacing::minHorizontalDistance(bracketShape, otherShape, bracket->spatium(), squeezeFactor)
                                    : HorizontalSpacing::minHorizontalDistance(otherShape, bracketShape, bracket->spatium(), squeezeFactor);

            minDistance = std::max(minDistance, distance);
        }
    };

    // Check the brackets facing the gap between the two Segments:
    // left-side brackets on the second Segment and right-side brackets on the first.
    updateForBrackets(secondSegment, firstSegment->staffShape(staffIdx), /*rightSide=*/ false);
    updateForBrackets(firstSegment, secondSegment->staffShape(staffIdx), /*rightSide=*/ true);
}

//---------------------------------------------------------
//   updateVerticalGeometry
//---------------------------------------------------------

void ChordBracketLayout::updateVerticalGeometry(ChordBracket* bracket, LayoutContext& ctx)
{
    if (!bracket || !bracket->chord() || !bracket->chord()->segment()) {
        return;
    }

    // Re-layout the final height without changing the horizontal position.
    const double x = bracket->ldata()->pos().x();
    TLayout::layoutItem(bracket, ctx);
    bracket->mutldata()->setPosX(x);

    // Refresh cached shapes after restoring the segment-level position.
    Chord* chord = bracket->chord();
    Segment* segment = chord->segment();
    ChordLayout::fillShape(chord, chord->mutldata());

    const BracketVisualSpan span = bracketVisualSpan(bracket);
    for (staff_idx_t staffIdx = span.topStaff; staffIdx <= span.bottomStaff; ++staffIdx) {
        segment->createShape(staffIdx);
    }
}
