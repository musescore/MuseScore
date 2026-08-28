/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited and others
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

#include "engraving/dom/box.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/page.h"
#include "engraving/dom/score.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/system.h"

using namespace mu::notation;
using namespace mu::engraving;

const Box* getLeadingTrailingBox(const System* system, bool leading)
{
    IF_ASSERT_FAILED(system) {
        return nullptr;
    }
    const MeasureBase* mb = leading ? system->first() : system->last();
    return mb->isBox() ? toBox(mb) : nullptr;
}

static std::set<const System*> boxOnlySystems(const System* startSystem, const System* endSystem)
{
    std::set<const System*> result;
    IF_ASSERT_FAILED(startSystem) {
        return result;
    }
    const System* currSystem = startSystem;
    while (currSystem) {
        bool isBoxOnlySystem = true;
        const MeasureBase* mb = currSystem->first();
        while (mb && mb->system() == currSystem) {
            if (!mb->isBox()) {
                isBoxOnlySystem = false;
                break;
            }
            mb = mb->nextMM();
        }
        if (isBoxOnlySystem) {
            result.emplace(currSystem);
        }
        if (currSystem == endSystem) {
            break;
        }
        const MeasureBase* firstInNext = currSystem->last() ? currSystem->last()->next() : nullptr;
        currSystem = firstInNext ? firstInNext->system() : nullptr;
    }
    return result;
}

std::vector<muse::RectF> ScoreRangeUtilities::boundingArea(const Score* score,
                                                           const Segment* startSegment, const Segment* endSegment,
                                                           staff_idx_t startStaffIndex, staff_idx_t endStaffIndex,
                                                           const engraving::Box* startBox,
                                                           const engraving::Box* endBox)
{
    if (!score || !startSegment || !endSegment || startSegment->tick() > endSegment->tick()) {
        return {};
    }

    std::vector<RectF> result;

    // Collect and add systems consisting solely of boxes...
    const System* startSystem = startBox ? startBox->system() : startSegment->system();
    const System* endSystem = endBox ? endBox->system() : endSegment->system();
    const std::set<const System*> boxSystems = boxOnlySystems(startSystem, endSystem);
    for (const System* boxSys : boxSystems) {
        RectF startRect = boxSys->first()->canvasBoundingRect();
        const RectF& endRect = boxSys->last()->canvasBoundingRect();
        result.push_back(startRect.unite(endRect));
    }

    // Handle start/EndBoxes that exist on a different system to the start/end segment...
    if (startBox && startBox->system() != startSegment->system() && boxSystems.find(startSegment->system()) != boxSystems.end()) {
        // TODO: This doesn't quite work correctly...
        RectF startRect = startBox->canvasBoundingRect();
        const MeasureBase* mb = startBox->nextMM();
        while (mb && mb->system() == startBox->system() && mb->isBox()) {
            startRect.unite(mb->canvasBoundingRect());
            mb = mb->nextMM();
        }
        result.push_back(startRect);
    }
    if (endBox && endSegment->system() != endSegment->system() && boxSystems.find(endSegment->system()) != boxSystems.end()) {
        // TODO: This doesn't quite work correctly...
        RectF endRect = endBox->canvasBoundingRect();
        const MeasureBase* mb = endBox->prevMM();
        while (mb && mb->system() == startBox->system() && mb->isBox()) {
            endRect.unite(mb->canvasBoundingRect());
            mb = mb->prevMM();
        }
        result.push_back(endRect);
    }

    const std::vector<RangeSection> sections = splitRangeBySections(startSegment, endSegment);

    for (const RangeSection& section : sections) {
        const staff_idx_t firstStaff = firstVisibleStaffIdx(score, section.system, startStaffIndex);
        const staff_idx_t lastStaff = lastVisibleStaffIdx(score, section.system, endStaffIndex);
        if (firstStaff == muse::nidx || lastStaff == muse::nidx) {
            continue;
        }

        // If the range starts before this system, we'll visually include all of the leading boxes for this system - otherwise
        // we'll visually include startBox (if any) and all leading boxes after it...
        const bool rangeStartsBefore = startSegment->system() != section.system && (!startBox || startBox->system() != section.system);
        const Box* sectionStartBox = rangeStartsBefore ? getLeadingTrailingBox(section.system, /*leading*/ true) : startBox;

        // // If the range ends after this system, we'll visually include all of the trailing boxes for this system - otherwise
        // // we'll visually include endBox (if any) and all trailing before after it...
        const bool rangeEndsAfter = endSegment->system() != section.system && (!endBox || endBox->system() != section.system);
        const Box* sectionEndBox = rangeEndsAfter ? getLeadingTrailingBox(section.system, /*leading*/ false) : endBox;

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

        double x1 = 0.0;
        if (sectionStartBox) {
            x1 = sectionStartBox->pageBoundingRect().left();
        } else {
            x1 = section.startSegment->pagePos().x();
        }

        double x2 = 0.0;
        if (sectionEndBox) {
            x2 = sectionEndBox->pageBoundingRect().right();
        } else {
            x2 = section.endSegment->pageBoundingRect().right();
        }

        const int padding = 0.5 * scoreFirstStaff->spatium(startSegment->tick());
        const double y1 = topY + segmentFirstStaff->y() + section.startSegment->pagePos().y() - padding;
        const double y2 = bottomY + segmentLastStaff->y() + section.endSegment->pagePos().y() + padding;

        if (!sectionStartBox && section.startSegment->measure()->firstEnabled() == section.startSegment) {
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
        while (nextSegment && !nextSegment->visible()) {
            nextSegment = nextSegment->next1MMenabled();
        }

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
