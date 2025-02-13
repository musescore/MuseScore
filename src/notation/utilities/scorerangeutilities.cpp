/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "scorerangeutilities.h"

#include "engraving/dom/page.h"
#include "engraving/dom/score.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/system.h"

using namespace mu::notation;
using namespace mu::engraving;

std::vector<muse::RectF> ScoreRangeUtilities::boundingArea(const Score* score,
                                                           const Segment* startSegment, const Segment* endSegment,
                                                           staff_idx_t startStaffIndex, staff_idx_t endStaffIndex)
{
    if (!startSegment || !endSegment || startSegment->tick() > endSegment->tick()) {
        return {};
    }

    std::vector<RectF> result;

    const std::vector<RangeSection> sections = splitRangeBySections(startSegment, endSegment);

    for (const RangeSection& section : sections) {
        const staff_idx_t firstStaff = firstVisibleStaffIdx(score, section.system, startStaffIndex);
        const staff_idx_t lastStaff = lastVisibleStaffIdx(score, section.system, endStaffIndex);
        if (firstStaff == muse::nidx || lastStaff == muse::nidx) {
            continue;
        }

        const SysStaff* segmentFirstStaff = section.system->staff(firstStaff);
        const SysStaff* segmentLastStaff = section.system->staff(lastStaff);

        const Staff* scoreFirstStaff = score->staff(firstStaff);
        const Staff* scoreLastStaff = score->staff(lastStaff);

        const double standardStaffHeight = 4 * scoreFirstStaff->spatium(Fraction(0, 1));
        const double firstStaffHeight = scoreFirstStaff->staffHeight();
        const double lastStaffHeight = scoreLastStaff->staffHeight();

        double topY = 0.0;
        if (firstStaffHeight < standardStaffHeight) {
            const double diff = standardStaffHeight - firstStaffHeight;
            topY -= 0.5 * diff;
        }

        double bottomY = lastStaffHeight;
        if (lastStaffHeight < standardStaffHeight) {
            const double diff = standardStaffHeight - lastStaffHeight;
            bottomY += 0.5 * diff;
        }

        double x1 = section.startSegment->pagePos().x();
        const double x2 = section.endSegment->pageBoundingRect().right();
        const int padding = 0.5 * scoreFirstStaff->spatium(startSegment->tick());
        const double y1 = topY + segmentFirstStaff->y() + section.startSegment->pagePos().y() - padding;
        const double y2 = bottomY + segmentLastStaff->y() + section.endSegment->pagePos().y() + padding;

        if (section.startSegment->measure()->firstEnabled() == section.startSegment) {
            x1 = section.startSegment->measure()->pagePos().x();
        }

        const RectF rect = RectF(PointF(x1, y1), PointF(x2, y2)).translated(section.system->page()->pos());
        result.push_back(rect);
    }

    return result;
}

std::vector<ScoreRangeUtilities::RangeSection> ScoreRangeUtilities::splitRangeBySections(
    const Segment* rangeStartSegment,
    const Segment* rangeEndSegment)
{
    std::vector<RangeSection> sections;

    const Segment* startSegment = rangeStartSegment;
    const Fraction rangeEndTick = rangeEndSegment->tick();

    for (const Segment* segment = startSegment; segment && segment != rangeEndSegment && segment->tick() < rangeEndTick;) {
        const System* currentSegmentSystem = segment->measure()->system();

        const Segment* nextSegment = segment->next1MMenabled();
        if (!nextSegment) {
            RangeSection section;
            section.system = currentSegmentSystem;
            section.startSegment = startSegment;
            section.endSegment = segment;

            sections.push_back(section);
            break;
        }

        const System* nextSegmentSystem = nextSegment->measure()->system();
        if (!nextSegmentSystem) {
            const Measure* mmr = nextSegment->measure()->coveringMMRestOrThis();
            if (mmr) {
                nextSegmentSystem = mmr->system();
            }
            if (!nextSegmentSystem) {
                break;
            }
        }

        if (nextSegmentSystem != currentSegmentSystem || nextSegment->tick() >= rangeEndTick) {
            RangeSection section;
            section.system = currentSegmentSystem;
            section.startSegment = startSegment;
            section.endSegment = segment;

            sections.push_back(section);
            startSegment = nextSegment;
        }

        segment = nextSegment;
    }

    return sections;
}

staff_idx_t ScoreRangeUtilities::firstVisibleStaffIdx(const Score* score, const System* system, staff_idx_t startStaffIndex)
{
    for (staff_idx_t i = startStaffIndex; i < score->nstaves(); ++i) {
        if (system->staff(i)->show()) {
            return i;
        }
    }

    return muse::nidx;
}

staff_idx_t ScoreRangeUtilities::lastVisibleStaffIdx(const Score*, const System* system, staff_idx_t endStaffIndex)
{
    for (int i = static_cast<int>(endStaffIndex) - 1; i >= 0; --i) {
        if (system->staff(i)->show()) {
            return static_cast<staff_idx_t>(i);
        }
    }

    return muse::nidx;
}
