//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "notationselection.h"

#include "libmscore/score.h"
#include "libmscore/segment.h"
#include "libmscore/measure.h"
#include "libmscore/system.h"
#include "libmscore/page.h"
#include "libmscore/chordrest.h"
#include "libmscore/skyline.h"

#include "log.h"

static const int SELECTION_SIDE_PADDING = 8;

using namespace mu::notation;

NotationSelection::NotationSelection(IGetScore* getScore)
    : m_getScore(getScore)
{
}

bool NotationSelection::isNone() const
{
    return score()->selection().isNone();
}

bool NotationSelection::isRange() const
{
    return score()->selection().isRange();
}

bool NotationSelection::canCopy() const
{
    return score()->selection().canCopy();
}

QMimeData* NotationSelection::mimeData() const
{
    QString mimeType = score()->selection().mimeType();
    if (mimeType.isEmpty()) {
        return nullptr;
    }

    QMimeData* mimeData = new QMimeData();
    mimeData->setData(mimeType, score()->selection().mimeData());

    return mimeData;
}

Element* NotationSelection::element() const
{
    return score()->selection().element();
}

std::vector<Element*> NotationSelection::elements() const
{
    std::vector<Element*> els;
    QList<Ms::Element*> list = score()->selection().elements();
    els.reserve(list.count());
    for (Ms::Element* e : list) {
        els.push_back(e);
    }
    return els;
}

SelectionRange NotationSelection::range() const
{
    const Ms::Selection& selection = score()->selection();

    SelectionRange range;
    range.startStaffIndex = selection.staffStart();
    range.endStaffIndex = selection.staffEnd();
    range.startTick = selection.tickStart();
    range.endTick = selection.tickEnd();

    return range;
}

std::vector<QRectF> NotationSelection::rangeBoundingArea() const
{
    if (!isRange()) {
        return {};
    }

    Ms::Segment* startSegment = rangeStartSegment();
    Ms::Segment* endSegment = rangeEndSegment();

    if (!startSegment || !endSegment || startSegment->tick() > endSegment->tick()) {
        return {};
    }

    std::vector<QRectF> result;

    std::vector<RangeSection> rangeSections = splitRangeBySections(startSegment, endSegment);

    int lastStaff = selectionLastVisibleStaff();

    for (const RangeSection& rangeSection: rangeSections) {
        const Ms::System* sectionSystem = rangeSection.system;
        const Ms::Segment* sectionStartSegment = rangeSection.startSegment;
        const Ms::Segment* sectionEndSegment = rangeSection.endSegment;

        Ms::SysStaff* segmentFirstStaff = sectionSystem->staff(score()->selection().staffStart());
        Ms::SysStaff* segmentLastStaff = sectionSystem->staff(lastStaff);

        int topY = sectionElementsMaxY(rangeSection);
        int bottomY = sectionElementsMinY(rangeSection);

        double x1 = sectionStartSegment->pagePos().x() - SELECTION_SIDE_PADDING;
        double x2 = sectionEndSegment->pageBoundingRect().topRight().x();
        double y1 = topY + segmentFirstStaff->y() + sectionStartSegment->pagePos().y() - SELECTION_SIDE_PADDING;
        double y2 = bottomY + segmentLastStaff->y() + sectionStartSegment->pagePos().y() + SELECTION_SIDE_PADDING;

        if (sectionStartSegment->measure()->first() == sectionStartSegment) {
            x1 = sectionStartSegment->measure()->pagePos().x();
        }

        QRectF rect = QRectF(QPointF(x1, y1), QPointF(x2, y2)).translated(sectionSystem->page()->pos());
        result.push_back(rect);
    }

    return result;
}

QRectF NotationSelection::canvasBoundingRect() const
{
    if (isNone()) {
        return QRectF();
    }

    Ms::Element* el = score()->selection().element();
    if (el) {
        return el->canvasBoundingRect();
    }

    QList<Ms::Element*> els = score()->selection().elements();
    if (els.isEmpty()) {
        LOGW() << "selection not none, but no elements";
        return QRectF();
    }

    QRectF rect;
    for (const Ms::Element* elm: els) {
        if (rect.isNull()) {
            rect = elm->canvasBoundingRect();
        } else {
            rect = rect.united(elm->canvasBoundingRect());
        }
    }

    return rect;
}

Ms::Score* NotationSelection::score() const
{
    return m_getScore->score();
}

Ms::Segment* NotationSelection::rangeStartSegment() const
{
    Ms::Segment* startSegment = score()->selection().startSegment();

    startSegment->measure()->firstEnabled();

    if (!startSegment) {
        return nullptr;
    }

    if (!startSegment->enabled()) {
        startSegment = startSegment->next1MMenabled();
    }

    if (!startSegment->measure()->system()) {
        const Measure* mmr = startSegment->measure()->mmRest1();
        if (!mmr || mmr->system()) {
            return nullptr;
        }
        startSegment = mmr->first(Ms::SegmentType::ChordRest);
    }

    return startSegment;
}

Ms::Segment* NotationSelection::rangeEndSegment() const
{
    Ms::Segment* endSegment = score()->selection().endSegment();

    if (!endSegment) {
        return nullptr;
    }

    if (!endSegment->enabled()) {
        endSegment = endSegment->next1MMenabled();
    }

    return endSegment;
}

int NotationSelection::selectionLastVisibleStaff() const
{
    for (int i = score()->selection().staffEnd() - 1; i >= 0; --i) {
        if (score()->staff(i)->show()) {
            return i;
        }
    }

    return 0;
}

std::vector<NotationSelection::RangeSection> NotationSelection::splitRangeBySections(const Ms::Segment* rangeStartSegment,
                                                                                     const Ms::Segment* rangeEndSegment) const
{
    std::vector<RangeSection> sections;

    const Ms::Segment* startSegment = rangeStartSegment;
    for (const Ms::Segment* segment = rangeStartSegment; segment && (segment != rangeEndSegment);) {
        Ms::System* currentSegmentSystem = segment->measure()->system();

        Ms::Segment* nextSegment = segment->next1MMenabled();
        Ms::System* nextSegmentSystem = nextSegment->measure()->system();

        if (!nextSegmentSystem) {
            const Measure* mmr = nextSegment->measure()->mmRest1();
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

int NotationSelection::sectionElementsMaxY(const NotationSelection::RangeSection& selection) const
{
    const Ms::System* segmentSystem = selection.system;
    const Ms::Segment* startSegment = selection.startSegment;
    const Ms::Segment* endSegment = selection.endSegment;

    Ms::SysStaff* segmentFirstStaff = segmentSystem->staff(score()->selection().staffStart());

    Ms::SkylineLine north = segmentFirstStaff->skyline().north();
    int maxY = INT_MAX;
    for (Ms::SkylineSegment segment: north) {
        bool ok = segment.x >= startSegment->pagePos().x() && segment.x <= endSegment->pagePos().x();
        if (!ok) {
            continue;
        }

        if (segment.y < maxY) {
            maxY = segment.y;
        }
    }

    if (maxY == INT_MAX) {
        maxY = 0;
    }

    return maxY;
}

int NotationSelection::sectionElementsMinY(const NotationSelection::RangeSection& selection) const
{
    const Ms::System* segmentSystem = selection.system;
    const Ms::Segment* startSegment = selection.startSegment;
    const Ms::Segment* endSegment = selection.endSegment;

    int lastStaff = selectionLastVisibleStaff();
    Ms::SysStaff* segmentLastStaff = segmentSystem->staff(lastStaff);

    Ms::SkylineLine south = segmentLastStaff->skyline().south();
    int minY = INT_MIN;
    for (Ms::SkylineSegment segment: south) {
        bool ok = segment.x >= startSegment->pagePos().x() && segment.x <= endSegment->pagePos().x();
        if (!ok) {
            continue;
        }

        if (segment.y > minY) {
            minY = segment.y;
        }
    }

    if (minY == INT_MIN) {
        minY = segmentLastStaff->bbox().height();
    }

    return minY;
}
