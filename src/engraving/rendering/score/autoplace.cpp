/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited and others
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
#include "autoplace.h"

#include "dom/harmony.h"
#include "style/style.h"

#include "dom/chordrest.h"
#include "dom/lyrics.h"
#include "dom/score.h"
#include "dom/segment.h"
#include "dom/spanner.h"
#include "dom/staff.h"
#include "dom/system.h"
#include "dom/measure.h"

#include "log.h"

using namespace muse;
using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

void Autoplace::autoplaceSegmentElement(const EngravingItem* item, EngravingItem::LayoutData* ldata, bool above, bool add)
{
    const Segment* segment = toSegment(item->findAncestor(ElementType::SEGMENT));
    const Measure* measure = segment ? segment->measure() : nullptr;
    const System* system = measure ? measure->system() : nullptr;
    IF_ASSERT_FAILED(system) {
        return;
    }
    LD_CONDITION(segment->ldata()->isSetPos());
    LD_CONDITION(measure->ldata()->isSetPos());

    const double spatium = item->style().spatium() * item->staff()->staffMag(item);
    // TODO: proper item-to-item table for horizontal clearance in skyline
    const double minSkylineHorizontalClearance = item->isArticulationOrFermata() ? 0.0 : item->style().styleAbsolute(
        Sid::skylineMinHorizontalClearance) * item->mag();

    doAutoplaceElement(item, ldata, system, above, add, minSkylineHorizontalClearance, spatium);
}

void Autoplace::autoplaceMeasureElement(const EngravingItem* item, EngravingItem::LayoutData* ldata, bool above, bool add)
{
    const Measure* measure = toMeasure(item->ownershipParent());
    const System* system = measure ? measure->system() : nullptr;
    IF_ASSERT_FAILED(system) {
        return;
    }
    LD_CONDITION(measure->ldata()->isSetPos());

    const double spatium = item->style().spatium() * item->staff()->staffMag(item);
    // TODO: check this value for measure-owned items
    // const double minSkylineHorizontalClearance = item->isMeasureNumber() ? 0.0 : item->style().styleAbsolute(
    // Sid::skylineMinHorizontalClearance) * item->mag();
    const double minSkylineHorizontalClearance = 0.0;
    doAutoplaceElement(item, ldata, system, above, add, minSkylineHorizontalClearance, spatium);
}

void Autoplace::autoplaceSpannerSegment(const SpannerSegment* item, EngravingItem::LayoutData* ldata)
{
    if (item->spanner()->anchor() == Spanner::Anchor::NOTE) {
        return;
    }
    const System* system = item->system();
    IF_ASSERT_FAILED(system) {
        return;
    }
    const double mag = !item->systemFlag() && !item->spanner()->systemFlag() ? item->staff()->staffMag(item->spanner()->tick()) : 1.0;
    const double spatium = item->style().spatium() * mag;

    const bool above = item->spanner()->placeAbove();
    doAutoplaceElement(item, ldata, system, above, /* add = */ false, /* minSkylineHorizontalClearance = */ 0.0, spatium);
}

// Returns the move away from the staff that keeps the min distance from items at the given distance
static double moveToClear(double distance, bool above, double minDistance)
{
    if (!(distance > -minDistance)) {
        return 0.0;
    }

    return (distance + minDistance) * (above ? -1.0 : 1.0);
}

static double distanceToClearSkyline(const SkylineLine& skyline, const Shape& shape, bool above, double minHorizontalClearance,
                                     double minDistance)
{
    if (skyline.elements().empty()) {
        return 0.0;
    }

    const double d = above ? skyline.minDistanceToShapeAbove(shape, minHorizontalClearance)
                     : skyline.minDistanceToShapeBelow(shape, minHorizontalClearance);

    return moveToClear(d, above, minDistance);
}

static double distanceToClearItems(const Shape& items, const Shape& shape, bool above, double minHorizontalClearance, double minDistance)
{
    if (items.empty()) {
        return 0.0;
    }

    const double d = above ? shape.minVerticalDistance(items, minHorizontalClearance)
                     : items.minVerticalDistance(shape, minHorizontalClearance);

    return moveToClear(d, above, minDistance);
}

static bool clearsItems(const Shape& items, const Shape& shape, bool above, double minHorizontalClearance, double minDistance)
{
    // Items further from the staff than the shape may come closer than items between the shape and the staff
    const double outerMinDistance = minDistance / 2;
    const double minDistanceAbove = above ? outerMinDistance : minDistance;
    const double minDistanceBelow = above ? minDistance : outerMinDistance;
    for (const ShapeElement& itemEl : items.elements()) {
        if (itemEl.height() <= 0.0) {
            continue;
        }
        for (const ShapeElement& el : shape.elements()) {
            if (el.height() <= 0.0
                || !mu::engraving::intersects(el.left(), el.right(), itemEl.left(), itemEl.right(), minHorizontalClearance)) {
                continue;
            }
            const bool clearAbove = muse::RealIsEqualOrLess(itemEl.bottom() + minDistanceAbove, el.top());
            const bool clearBelow = muse::RealIsEqualOrMore(itemEl.top(), el.bottom() + minDistanceBelow);
            if (!clearAbove && !clearBelow) {
                return false;
            }
        }
    }
    return true;
}

// Returns the smallest move away from the staff that clears the skyline, which may leave the shape in a gap between items
static double distanceToNearestGap(const SkylineLine& skyline, const Shape& shape, bool above, double minHorizontalClearance,
                                   double minDistance)
{
    // Only items that overlap the shape horizontally can collide with it: collect them in one pass, split into
    // those between the shape and the staff and those beyond it
    const double itemEdge = above ? shape.top() : shape.bottom();
    const RectF& bbox = shape.bbox();
    Shape nearItems;
    Shape farItems;
    for (const ShapeElement& el : skyline.elements()) {
        if (!mu::engraving::intersects(el.left(), el.right(), bbox.left(), bbox.right(), minHorizontalClearance)) {
            continue;
        }
        const bool isBeyondItem = above ? el.bottom() <= itemEdge : el.top() >= itemEdge;
        (isBeyondItem ? farItems : nearItems).add(el);
    }

    // Clear whatever lies between the shape and the staff, then try just past each item beyond it, nearest first
    const double nearYd = distanceToClearItems(nearItems, shape, above, minHorizontalClearance, minDistance);
    std::vector<double> candidates { nearYd };
    for (const ShapeElement& el : farItems.elements()) {
        candidates.push_back(above ? std::min(nearYd, el.top() - minDistance - shape.bottom())
                             : std::max(nearYd, el.bottom() + minDistance - shape.top()));
    }
    std::sort(candidates.begin(), candidates.end(), [above](double a, double b) { return above ? a > b : a < b; });

    for (double yd : candidates) {
        if (clearsItems(farItems, shape.translated(PointF(0.0, yd)), above, minHorizontalClearance, minDistance)) {
            return yd;
        }
    }

    // Not expected: the last candidate is beyond every item
    return distanceToClearSkyline(skyline, shape, above, minHorizontalClearance, minDistance);
}

double Autoplace::distanceToMoveItem(const EngravingItem* item, const SkylineLine& staffSkyline, const Shape& shape, bool above,
                                     double minSkylineHorizontalClearance, double minDistance)
{
    const SkylineLine filteredSkyline = staffSkyline.getFilteredCopy([item](const ShapeElement& shapeEl) {
        const EngravingItem* skylineItem = shapeEl.item();
        if (!skylineItem) {
            return false;
        }
        return itemsShouldIgnoreEachOther(item, skylineItem);
    });

    if (item->isMeasureNumber()
        && item->style().styleV(Sid::measureNumberAvoidance).value<MeasureNumberAvoidance>() == MeasureNumberAvoidance::FIT_IN_GAPS) {
        return distanceToNearestGap(filteredSkyline, shape, above, minSkylineHorizontalClearance, minDistance);
    }

    return distanceToClearSkyline(filteredSkyline, shape, above, minSkylineHorizontalClearance, minDistance);
}

void Autoplace::doAutoplaceElement(const EngravingItem* item, EngravingItem::LayoutData* ldata, const System* system, bool above, bool add,
                                   double minSkylineHorizontalClearance, double spatium)
{
    DO_ASSERT(autoplaceAppliesToType(item->type()));
    IF_ASSERT_FAILED(system) {
        return;
    }

    if (!item->autoplace() || !item->ownershipParent()) {
        return;
    }

    const staff_idx_t staffIdx = item->effectiveStaffIdx();
    if (staffIdx == muse::nidx) {
        return;
    }

    LD_CONDITION(ldata->isSetPos());
    LD_CONDITION(ldata->isSetBbox());

    const double minDistance = item->minDistance().toAbsolute(spatium);

    SysStaff* sysStaff = system->staff(staffIdx);
    SkylineLine& staffSkyline = above ? sysStaff->skyline().north() : sysStaff->skyline().south();
    // TODO [J.M] staffOffset will be applied twice for spanner segments.
    // We really need to sort out staffOffset and have it applied as early and infrequently as possible. Can't keep up with ~30 callsites
    // Shape shape = item->ldata()->shape().translate(item->systemPos() - item->offset() + item->staffOffset());
    Shape shape = item->ldata()->shape();
    if (item->isSpannerSegment()) {
        shape.translate(item->ldata()->pos());
    } else {
        shape.translate(item->systemPos() - item->offset() + item->staffOffset());
    }

    const double yd = distanceToMoveItem(item, staffSkyline, shape, above, minSkylineHorizontalClearance, minDistance);

    ldata->moveY(yd);
    shape.translateY(yd);

    if (add && item->addToSkyline()) {
        staffSkyline.add(shape.translated(item->offset()));
    }
}

bool Autoplace::itemsShouldIgnoreEachOther(const EngravingItem* itemToAutoplace, const EngravingItem* itemInSkyline)
{
    if (itemToAutoplace == itemInSkyline) {
        return true;
    }

    const EngravingItem* skylineItemOwner = itemInSkyline->ownershipParentItem();
    if (itemInSkyline->isText() && skylineItemOwner && skylineItemOwner->isSLineSegment()) {
        return itemsShouldIgnoreEachOther(itemToAutoplace, skylineItemOwner);
    }

    ElementType type1 = itemToAutoplace->type();
    ElementType type2 = itemInSkyline->type();

    if (type1 == ElementType::TIMESIG) {
        return type2 != ElementType::KEYSIG;
    }

    if (type1 == ElementType::FRET_DIAGRAM && (type2 == ElementType::FRET_DIAGRAM || type2 == ElementType::HARMONY)) {
        bool isFretDiagAgainstItsOwnHarmony = skylineItemOwner == itemToAutoplace;
        bool areOnDifferentSegments = itemToAutoplace->findAncestor(ElementType::SEGMENT)
                                      != itemInSkyline->findAncestor(ElementType::SEGMENT);
        return isFretDiagAgainstItsOwnHarmony || areOnDifferentSegments;
    }

    if ((type1 == ElementType::DYNAMIC || type1 == ElementType::HAIRPIN_SEGMENT)
        && (type2 == ElementType::DYNAMIC || type2 == ElementType::HAIRPIN_SEGMENT)) {
        return true;
    }

    if (itemToAutoplace->isLyricsLineSegment() && itemInSkyline->isLyricsLineSegment()) {
        const LyricsLineSegment* a = toLyricsLineSegment(itemToAutoplace);
        const LyricsLineSegment* b = toLyricsLineSegment(itemInSkyline);
        return a->verse() == b->verse() && a->lyricsPlaceAbove() == b->lyricsPlaceAbove();
    }

    if (type1 == type2) {
        // Items of same type should ignore each other in most cases
        static const std::set<ElementType> TEXT_BASED_TYPES_WHICH_IGNORE_EACH_OTHER {
            ElementType::DYNAMIC,
            ElementType::EXPRESSION,
            ElementType::STICKING
        };
        return !itemToAutoplace->isTextBase() || muse::contains(TEXT_BASED_TYPES_WHICH_IGNORE_EACH_OTHER, type1);
    }

    if ((type1 == ElementType::DYNAMIC || type1 == ElementType::EXPRESSION)
        && (type2 == ElementType::DYNAMIC || type2 == ElementType::EXPRESSION)) {
        // Dynamics and expressions should ignore each other if on the same segment
        return itemToAutoplace->parent() == itemInSkyline->parent();
    }

    if ((type1 == ElementType::TUPLET || type1 == ElementType::STAFF_LINES)
        && (type2 == ElementType::STAFF_LINES || type2 == ElementType::TUPLET)) {
        const Score* score = itemToAutoplace->score();
        const bool outOfStaff = score ? !score->style().styleB(Sid::tupletOutOfStaff) : false;
        return outOfStaff;
    }

    if ((type1 == ElementType::FIGURED_BASS || type1 == ElementType::FIGURED_BASS_ITEM)
        && (type2 == ElementType::FIGURED_BASS || type2 == ElementType::FIGURED_BASS_ITEM)) {
        return true;
    }

    if (itemToAutoplace->isArticulationOrFermata() && itemInSkyline->isArticulationOrFermata()) {
        // Ignore fermatas and articulations on other segments
        return itemToAutoplace->findAncestor(ElementType::SEGMENT) != itemInSkyline->findAncestor(ElementType::SEGMENT);
    }

    return itemToAutoplace->ldata()->itemSnappedBefore() == itemInSkyline || itemToAutoplace->ldata()->itemSnappedAfter() == itemInSkyline;
}
