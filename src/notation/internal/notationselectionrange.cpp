/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include <cfloat>

#include "notationselectionrange.h"

#include "engraving/dom/segment.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/system.h"
#include "engraving/dom/chordrest.h"
#include "engraving/dom/staff.h"

#include "log.h"

using namespace mu::notation;
using namespace mu::engraving;

NotationSelectionRange::NotationSelectionRange(IGetScore* getScore)
    : m_getScore(getScore)
{
}

staff_idx_t NotationSelectionRange::startStaffIndex() const
{
    const mu::engraving::Selection& selection = score()->selection();
    return selection.staffStart();
}

Fraction NotationSelectionRange::startTick() const
{
    const mu::engraving::Selection& selection = score()->selection();
    return selection.tickStart();
}

staff_idx_t NotationSelectionRange::endStaffIndex() const
{
    const mu::engraving::Selection& selection = score()->selection();
    return selection.staffEnd();
}

Fraction NotationSelectionRange::endTick() const
{
    const mu::engraving::Selection& selection = score()->selection();
    return selection.tickEnd();
}

NotationSelectionRange::MeasureRange NotationSelectionRange::measureRange() const
{
    MeasureRange range;
    score()->selection().measureRange(&range.startMeasure, &range.endMeasure);

    return range;
}

std::vector<muse::RectF> NotationSelectionRange::boundingArea() const
{
    const mu::engraving::Selection& selection = score()->selection();
    if (!selection.isRange()) {
        return {};
    }

    const mu::engraving::Segment* startSegment = rangeStartSegment();
    const mu::engraving::Segment* endSegment = rangeEndSegment();
    if (!endSegment) {
        endSegment = score()->lastSegment();
    }

    if (!startSegment || !endSegment || startSegment->tick() > endSegment->tick()) {
        return {};
    }

    std::vector<RectF> result;

    std::vector<RangeSection> rangeSections = splitRangeBySections(startSegment, endSegment);

    for (const RangeSection& rangeSection: rangeSections) {
        const mu::engraving::System* sectionSystem = rangeSection.system;
        const mu::engraving::Segment* sectionStartSegment = rangeSection.startSegment;
        const mu::engraving::Segment* sectionEndSegment = rangeSection.endSegment;

        staff_idx_t firstStaff = selectionFirstVisibleStaff(sectionSystem);
        staff_idx_t lastStaff = selectionLastVisibleStaff(sectionSystem);
        if (firstStaff == muse::nidx || lastStaff == muse::nidx) {
            continue;
        }

        const mu::engraving::SysStaff* segmentFirstStaff = sectionSystem->staff(firstStaff);
        const mu::engraving::SysStaff* segmentLastStaff = sectionSystem->staff(lastStaff);

        const mu::engraving::Staff* scoreFirstStaff = score()->staff(firstStaff);
        const mu::engraving::Staff* scoreLastStaff = score()->staff(lastStaff);

        double standardStaffHeight = 4 * scoreFirstStaff->spatium(Fraction(0, 1));
        double firstStaffHeight = scoreFirstStaff->staffHeight();
        double lastStaffHeight = scoreLastStaff->staffHeight();

        double topY = 0.0;
        if (firstStaffHeight < standardStaffHeight) {
            double diff = standardStaffHeight - firstStaffHeight;
            topY -= 0.5 * diff;
        }

        double bottomY = lastStaffHeight;
        if (lastStaffHeight < standardStaffHeight) {
            double diff = standardStaffHeight - lastStaffHeight;
            bottomY += 0.5 * diff;
        }

        double x1 = sectionStartSegment->pagePos().x();
        double x2 = sectionEndSegment->pageBoundingRect().topRight().x();
        const int SELECTION_BOX_PADDING = 0.5 * scoreFirstStaff->spatium(startSegment->tick());
        double y1 = topY + segmentFirstStaff->y() + sectionStartSegment->pagePos().y() - SELECTION_BOX_PADDING;
        double y2 = bottomY + segmentLastStaff->y() + sectionStartSegment->pagePos().y() + SELECTION_BOX_PADDING;

        if (sectionStartSegment->measure()->firstEnabled() == sectionStartSegment) {
            x1 = sectionStartSegment->measure()->pagePos().x();
        }

        RectF rect = RectF(PointF(x1, y1), PointF(x2, y2)).translated(sectionSystem->page()->pos());
        result.push_back(rect);
    }

    return result;
}

bool NotationSelectionRange::containsPoint(const PointF& point) const
{
    for (const muse::RectF& area : boundingArea()) {
        if (area.contains(point)) {
            return true;
        }
    }

    return false;
}

bool NotationSelectionRange::containsItem(const EngravingItem* item) const
{
    Fraction itemTick = item->tick();
    Fraction selectionStartTick = startTick();
    Fraction selectionEndTick = endTick();

    if (item->isMeasure()) {
        return itemTick >= selectionStartTick && itemTick < selectionEndTick;
    }

    if (itemTick < selectionStartTick || itemTick > selectionEndTick) {
        return false;
    }

    if (itemTick == selectionEndTick) {
        return item->rtick() > Fraction(0, 1);
    }

    track_idx_t itemTrack = item->track();
    track_idx_t selectionStartTrack = VOICES * startStaffIndex();
    track_idx_t selectionEndTrack = VOICES * (endStaffIndex() - 1) + VOICES;

    return itemTrack >= selectionStartTrack && itemTrack < selectionEndTrack;
}

std::vector<const Part*> NotationSelectionRange::selectedParts() const
{
    std::vector<const Part*> result;

    if (!score()->selection().isRange()) {
        return result;
    }

    staff_idx_t startStaffIndex = this->startStaffIndex();
    staff_idx_t endStaffIndex = this->endStaffIndex();

    for (staff_idx_t i = startStaffIndex; i < endStaffIndex; ++i) {
        const Staff* staff = score()->staff(i);

        if (staff && staff->part()) {
            result.push_back(staff->part());
        }
    }

    return result;
}

mu::engraving::Score* NotationSelectionRange::score() const
{
    return m_getScore->score();
}

mu::engraving::Segment* NotationSelectionRange::rangeStartSegment() const
{
    mu::engraving::Segment* startSegment = score()->selection().startSegment();

    if (!startSegment) {
        return nullptr;
    }

    if (!startSegment->enabled()) {
        startSegment = startSegment->next1MMenabled();
    }

    if (!startSegment->measure()->system()) {
        const Measure* mmr = startSegment->measure()->coveringMMRestOrThis();
        if (!mmr || mmr->system()) {
            return nullptr;
        }
        startSegment = mmr->first(mu::engraving::SegmentType::ChordRest);
    }

    return startSegment;
}

mu::engraving::Segment* NotationSelectionRange::rangeEndSegment() const
{
    mu::engraving::Segment* endSegment = score()->selection().endSegment();

    if (!endSegment) {
        return nullptr;
    }

    if (!endSegment->enabled()) {
        endSegment = endSegment->next1MMenabled();
    }

    return endSegment;
}

staff_idx_t NotationSelectionRange::selectionLastVisibleStaff(const System* system) const
{
    for (int i = static_cast<int>(score()->selection().staffEnd()) - 1; i >= 0; --i) {
        if (system->staff(i)->show()) {
            return i;
        }
    }

    return muse::nidx;
}

staff_idx_t NotationSelectionRange::selectionFirstVisibleStaff(const System* system) const
{
    for (staff_idx_t i = score()->selection().staffStart(); i < score()->nstaves(); ++i) {
        if (system->staff(i)->show()) {
            return i;
        }
    }

    return muse::nidx;
}

std::vector<NotationSelectionRange::RangeSection> NotationSelectionRange::splitRangeBySections(
    const mu::engraving::Segment* rangeStartSegment,
    const mu::engraving::Segment* rangeEndSegment)
const
{
    std::vector<RangeSection> sections;

    const mu::engraving::Segment* startSegment = rangeStartSegment;
    Fraction rangeEndTick = rangeEndSegment->tick();
    for (const mu::engraving::Segment* segment = rangeStartSegment;
         segment && segment != rangeEndSegment && segment->tick() <= rangeEndTick;) {
        mu::engraving::System* currentSegmentSystem = segment->measure()->system();

        mu::engraving::Segment* nextSegment = segment->next1MMenabled();
        if (!nextSegment) {
            RangeSection section;
            section.system = currentSegmentSystem;
            section.startSegment = startSegment;
            section.endSegment = segment;

            sections.push_back(section);
            break;
        }

        mu::engraving::System* nextSegmentSystem = nextSegment->measure()->system();
        if (!nextSegmentSystem) {
            const Measure* mmr = nextSegment->measure()->coveringMMRestOrThis();
            if (mmr) {
                nextSegmentSystem = mmr->system();
            }
            if (!nextSegmentSystem) {
                break;
            }
        }

        if (nextSegmentSystem != currentSegmentSystem || nextSegment == rangeEndSegment) {
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
