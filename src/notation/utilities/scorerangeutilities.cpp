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

static const Box* getLeadingTrailingBox(const System* system, bool leading)
{
    IF_ASSERT_FAILED(system) {
        return nullptr;
    }
    const MeasureBase* mb = leading ? system->first() : system->last();
    return mb && mb->isBox() ? toBox(mb) : nullptr;
}

// Unites the bounding rects in a run of consecutive boxes (until "limit")...
static RectF boxRunBoundingRect(const Box* box, bool forwards, const Box* limit = nullptr)
{
    RectF rect = box->canvasBoundingRect();
    const System* system = box->system();

    const MeasureBase* mb = box;
    while (mb != limit) {
        mb = forwards ? mb->nextMM() : mb->prevMM();
        if (!mb || !mb->isBox() || mb->system() != system) {
            break;
        }
        rect.unite(mb->canvasBoundingRect());
    }

    return rect;
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

    const std::vector<RangeSection> sections = splitRangeBySections(startSegment, endSegment);
    if (sections.empty() && !startBox && !endBox) {
        return {};
    }

    std::vector<RectF> result;

    //! NOTE: endSegment is exclusive, so these are the systems of the first and last segments that are actually inside
    //! the range. They're null when the range contains boxes but no segments...
    const System* firstSegmentSystem = sections.empty() ? nullptr : sections.front().system;
    const System* lastSegmentSystem = sections.empty() ? nullptr : sections.back().system;

    // Collect and add systems consisting solely of boxes...
    const System* startSystem = startBox ? startBox->system() : firstSegmentSystem;
    const System* endSystem = endBox ? endBox->system() : lastSegmentSystem;
    const std::set<const System*> boxSystems = startSystem ? boxOnlySystems(startSystem, endSystem) : std::set<const System*>();
    for (const System* boxOnlySystem : boxSystems) {
        const bool useStartBox = startBox && startBox->system() == boxOnlySystem;
        const MeasureBase* first = useStartBox ? startBox : boxOnlySystem->first();

        const bool useEndBox = endBox && endBox->system() == boxOnlySystem;
        const MeasureBase* last = useEndBox ? endBox : boxOnlySystem->last();

        IF_ASSERT_FAILED(first && first->isBox() && last && last->isBox()) {
            continue;
        }

        result.push_back(boxRunBoundingRect(toBox(first), /*forwards*/ true, toBox(last)));
    }

    if (sections.empty()) {
        // TODO: Entire range consists solely of boxes...
        return result;
    }

    // Handle start/end boxes that exist on a different system to the start/end segment...
    if (startBox && startBox->system() != firstSegmentSystem && boxSystems.find(startBox->system()) == boxSystems.end()) {
        result.push_back(boxRunBoundingRect(startBox, /*forwards*/ true));
    }
    if (endBox && endBox->system() != lastSegmentSystem && boxSystems.find(endBox->system()) == boxSystems.end()) {
        result.push_back(boxRunBoundingRect(endBox, /*forwards*/ false));
    }

    for (size_t i = 0; i < sections.size(); ++i) {
        const RangeSection& section = sections.at(i);

        const staff_idx_t firstStaff = firstVisibleStaffIdx(score, section.system, startStaffIndex);
        const staff_idx_t lastStaff = lastVisibleStaffIdx(score, section.system, endStaffIndex);
        if (firstStaff == muse::nidx || lastStaff == muse::nidx) {
            continue;
        }

        const Box* sectionStartBox = nullptr;
        if (startBox && startBox->system() == section.system) {
            // startBox is on this system - it's the "section start box"...
            sectionStartBox = startBox;
        } else if (i > 0 || startBox) {
            // startBox exists but isn't on this system or the range started earlier - the section start box is the "leading" box...
            sectionStartBox = getLeadingTrailingBox(section.system, /*leading*/ true);
        }
        const Box* sectionEndBox = nullptr;
        if (endBox && endBox->system() == section.system) {
            // endBox is on this system - it's the "section end box"...
            sectionEndBox = endBox;
        } else if (i < sections.size() - 1 || endBox) {
            // endBox exists but isn't on this system or the range is ending later - the section end box is the "trailing" box...
            sectionEndBox = getLeadingTrailingBox(section.system, /*leading*/ false);
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
