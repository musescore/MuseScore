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
#include "dom/measurenumber.h"
#include "dom/tuplet.h"

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

    doAutoplaceElement(item, ldata, system, above, add, minSkylineHorizontalClearance, spatium, CollisionStrategy::MOVE_CLEAR);
}

void Autoplace::autoplaceMeasureElement(const EngravingItem* item, EngravingItem::LayoutData* ldata, bool above, bool add)
{
    doAutoplaceMeasureElement(item, ldata, above, add, CollisionStrategy::MOVE_CLEAR);
}

void Autoplace::doAutoplaceMeasureElement(const EngravingItem* item, EngravingItem::LayoutData* ldata, bool above, bool add,
                                          CollisionStrategy strategy)
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
    doAutoplaceElement(item, ldata, system, above, add, minSkylineHorizontalClearance, spatium, strategy);
}

void Autoplace::autoplaceMeasureNumber(const MeasureNumber* item, EngravingItem::LayoutData* ldata)
{
    const bool fitInGaps = item->style().styleV(Sid::measureNumberAvoidance).value<MeasureNumberAvoidance>()
                           == MeasureNumberAvoidance::FIT_IN_GAPS;
    doAutoplaceMeasureElement(item, ldata, item->placeAbove(), /* add = */ true,
                              fitInGaps ? CollisionStrategy::FIT_IN_GAPS : CollisionStrategy::MOVE_CLEAR);
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
    doAutoplaceElement(item, ldata, system, above, /* add = */ false, /* minSkylineHorizontalClearance = */ 0.0, spatium,
                       CollisionStrategy::MOVE_CLEAR);
}

double Autoplace::distanceToNearestGap(const EngravingItem* item, const SkylineLine& skyline, const RectF& bbox,
                                       bool above, double minHorizontalClearance, double minDistance)
{
    const auto innerEdge = [above](const RectF& r) { return above ? -r.bottom() : r.top(); };
    const auto outerEdge = [above](const RectF& r) { return above ? -r.top() : r.bottom(); };

    // Only elements that overlap the item horizontally can collide with it
    std::vector<const ShapeElement*> obstacles;
    for (const ShapeElement& el : skyline.elements()) {
        if (el.height() <= 0.0
            || (el.item() && Autoplace::itemsShouldIgnoreEachOther(item, el.item()))
            || !mu::engraving::intersects(el.left(), el.right(), bbox.left(), bbox.right(), minHorizontalClearance)) {
            continue;
        }
        obstacles.push_back(&el);
    }

    std::sort(obstacles.begin(), obstacles.end(), [&innerEdge](const ShapeElement* a, const ShapeElement* b) {
        return innerEdge(*a) < innerEdge(*b);
    });

    const double outerMinDistance = minDistance / 2;

    double move = 0.0;
    for (const ShapeElement* el : obstacles) {
        if (muse::RealIsEqualOrLess(outerEdge(*el) + minDistance, innerEdge(bbox) + move)) {
            // Between the item and the staff and already clear; moving further out keeps it clear
            continue;
        }
        if (muse::RealIsEqualOrLess(outerEdge(bbox) + move + outerMinDistance, innerEdge(*el))) {
            // The item fits in the gap on the staff side of this element, and every remaining element is further out
            break;
        }
        move = std::max(move, outerEdge(*el) + minDistance - innerEdge(bbox));
    }

    return above ? -move : move;
}

double Autoplace::distanceToMoveItem(const EngravingItem* item, const SkylineLine& staffSkyline, const Shape& shape, bool above,
                                     double minSkylineHorizontalClearance, double minDistance, CollisionStrategy strategy)
{
    if (strategy == CollisionStrategy::FIT_IN_GAPS) {
        return distanceToNearestGap(item, staffSkyline, shape.bbox(), above, minSkylineHorizontalClearance, minDistance);
    }

    const SkylineLine filteredSkyline = staffSkyline.getFilteredCopy([item](const ShapeElement& shapeEl) {
        const EngravingItem* skylineItem = shapeEl.item();
        if (!skylineItem) {
            return false;
        }
        return itemsShouldIgnoreEachOther(item, skylineItem);
    });

    if (filteredSkyline.elements().empty()) {
        return 0.0;
    }

    const double d = above ? filteredSkyline.minDistanceToShapeAbove(shape, minSkylineHorizontalClearance)
                     : filteredSkyline.minDistanceToShapeBelow(shape, minSkylineHorizontalClearance);

    if (!(d > -minDistance)) {
        return 0.0;
    }

    const double yd = (d + minDistance) * (above ? -1.0 : 1.0);

    return yd;
}

void Autoplace::doAutoplaceElement(const EngravingItem* item, EngravingItem::LayoutData* ldata, const System* system, bool above, bool add,
                                   double minSkylineHorizontalClearance, double spatium, CollisionStrategy strategy)
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

    const double yd = distanceToMoveItem(item, staffSkyline, shape, above, minSkylineHorizontalClearance, minDistance, strategy);

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

    if (itemToAutoplace->isTuplet() && itemInSkyline->isTuplet()) {
        // A tuplet avoids the tuplets nested inside it
        for (const Tuplet* t = toTuplet(itemInSkyline)->tuplet(); t; t = t->tuplet()) {
            if (t == itemToAutoplace) {
                return false;
            }
        }
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
