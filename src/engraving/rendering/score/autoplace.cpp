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
#include "dom/score.h"
#include "dom/segment.h"
#include "dom/spanner.h"
#include "dom/staff.h"
#include "dom/system.h"
#include "dom/measure.h"

using namespace muse;
using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

void Autoplace::autoplaceSegmentElement(const EngravingItem* item, EngravingItem::LayoutData* ldata, bool above, bool add)
{
    // TODO: proper item-to-item table for horizontal clearance in skyline
    const double minSkylineHorizontalClearance = item->isArticulationOrFermata() ? 0.0 : item->style().styleAbsolute(
        Sid::skylineMinHorizontalClearance) * item->mag();

    if (item->autoplace() && item->explicitParent()) {
        const Segment* s = toSegment(item->findAncestor(ElementType::SEGMENT));
        IF_ASSERT_FAILED(s) {
            return;
        }

        const Measure* m = s->measure();

        LD_CONDITION(ldata->isSetPos());
        LD_CONDITION(m->ldata()->isSetPos());
        LD_CONDITION(s->ldata()->isSetPos());

        double sp = item->style().spatium();
        staff_idx_t si = item->effectiveStaffIdx();

        // if there's no good staff for this object, obliterate it
        ldata->setIsSkipDraw(si == muse::nidx);
        const_cast<EngravingItem*>(item)->setSelectable(!ldata->isSkipDraw());
        if (ldata->isSkipDraw()) {
            return;
        }

        double mag = item->staff()->staffMag(item);
        sp *= mag;
        double minDistance = item->minDistance().val() * sp;

        SysStaff* ss = m->system()->staff(si);
        Shape shape = item->ldata()->shape().translate(item->systemPos() - item->offset());

        // Adjust bbox Y pos for staffType offset
        shape.translate(item->staffOffset());

        SkylineLine& staffSkyline = above ? ss->skyline().north() : ss->skyline().south();

        SkylineLine filteredSkyline = staffSkyline.getFilteredCopy([item](const ShapeElement& shapeEl) {
            const EngravingItem* skylineItem = shapeEl.item();
            if (!skylineItem) {
                return false;
            }
            return itemsShouldIgnoreEachOther(item, skylineItem);
        });

        if (filteredSkyline.elements().empty()) {
            if (add && item->addToSkyline()) {
                staffSkyline.add(shape.translated(item->offset()));
            }
            return;
        }

        double d = above ? filteredSkyline.minDistanceToShapeAbove(shape, minSkylineHorizontalClearance)
                   : filteredSkyline.minDistanceToShapeBelow(shape, minSkylineHorizontalClearance);

        if (d > -minDistance) {
            double yd = d + minDistance;
            if (above) {
                yd *= -1.0;
            }
            ldata->moveY(yd);
            shape.translate(PointF(0.0, yd));
        }

        if (add && item->addToSkyline()) {
            staffSkyline.add(shape.translated(item->offset()));
        }
    }
}

void Autoplace::autoplaceMeasureElement(const EngravingItem* item, EngravingItem::LayoutData* ldata, bool above, bool add)
{
    if (item->autoplace() && item->explicitParent()) {
        const Measure* m = toMeasure(item->explicitParent());

        LD_CONDITION(ldata->isSetPos());
        LD_CONDITION(ldata->isSetBbox());
        LD_CONDITION(m->ldata()->isSetPos());

        staff_idx_t si = item->effectiveStaffIdx();

        // if there's no good staff for this object, obliterate it
        ldata->setIsSkipDraw(si == muse::nidx);
        const_cast<EngravingItem*>(item)->setSelectable(!ldata->isSkipDraw());
        if (ldata->isSkipDraw()) {
            return;
        }

        double sp = item->style().spatium();
        double minDistance = item->minDistance().val() * sp;

        SysStaff* ss = m->system()->staff(si);
        // shape rather than bbox is good for tuplets especially
        Shape sh = item->shape().translate(m->pos() + item->ldata()->pos() + item->staffOffset());

        SkylineLine sk(!above);
        SkylineLine& staffSkyline = above ? ss->skyline().north() : ss->skyline().south();

        SkylineLine filteredSkyline = staffSkyline.getFilteredCopy([item](const ShapeElement& shapeEl) {
            const EngravingItem* skylineItem = shapeEl.item();
            if (!skylineItem) {
                return false;
            }
            return itemsShouldIgnoreEachOther(item, skylineItem);
        });

        double d;
        if (above) {
            sk.add(sh);
            d = sk.minDistance(filteredSkyline);
        } else {
            sk.add(sh);
            d = filteredSkyline.minDistance(sk);
        }
        minDistance *= item->staff()->staffMag(item);
        if (d > -minDistance) {
            double yd = d + minDistance;
            if (above) {
                yd *= -1.0;
            }
            ldata->moveY(yd);
            sh.translateY(yd);
        }
        if (add && item->addToSkyline()) {
            staffSkyline.add(sh.translated(item->offset()));
        }
    }
}

void Autoplace::autoplaceSpannerSegment(const SpannerSegment* item, EngravingItem::LayoutData* ldata, double sp)
{
    if (item->spanner()->anchor() == Spanner::Anchor::NOTE) {
        return;
    }

    if (item->autoplace()) {
        if (!item->systemFlag() && !item->spanner()->systemFlag()) {
            sp *= item->staff()->staffMag(item->spanner()->tick());
        }
        double md = item->minDistance().val() * sp;
        bool above = item->spanner()->placeAbove();
        SkylineLine sl(!above);
        Shape sh = item->shape();
        sl.add(sh.translate(item->ldata()->pos()));
        double yd = 0.0;
        staff_idx_t stfIdx = item->effectiveStaffIdx();
        if (stfIdx == muse::nidx) {
            ldata->setIsSkipDraw(true);
            return;
        } else {
            ldata->setIsSkipDraw(false);
        }
        const System* system = item->system();
        IF_ASSERT_FAILED(system) {
            return;
        }
        const Skyline& staffSkyline = system->staff(stfIdx)->skyline();
        const SkylineLine& skyline = above ? staffSkyline.north() : staffSkyline.south();
        SkylineLine filteredSkyline = skyline.getFilteredCopy([item](const ShapeElement& shapeEl){
            const EngravingItem* skylineItem = shapeEl.item();
            if (!skylineItem) {
                return false;
            }
            return itemsShouldIgnoreEachOther(item, skylineItem);
        });

        if (above) {
            double d = sl.minDistance(filteredSkyline);
            if (d > -md) {
                yd = -(d + md);
            }
        } else {
            double d =  filteredSkyline.minDistance(sl);
            if (d > -md) {
                yd = d + md;
            }
        }
        if (!RealIsNull(yd)) {
            ldata->moveY(yd);
        }
    }
}

bool Autoplace::itemsShouldIgnoreEachOther(const EngravingItem* itemToAutoplace, const EngravingItem* itemInSkyline)
{
    if (itemToAutoplace == itemInSkyline) {
        return true;
    }

    if (itemInSkyline->isText() && itemInSkyline->explicitParent() && itemInSkyline->parent()->isSLineSegment()) {
        return itemsShouldIgnoreEachOther(itemToAutoplace, itemInSkyline->parentItem());
    }

    ElementType type1 = itemToAutoplace->type();
    ElementType type2 = itemInSkyline->type();

    if (type1 == ElementType::TIMESIG) {
        return type2 != ElementType::KEYSIG;
    }

    if (type1 == ElementType::FRET_DIAGRAM && (type2 == ElementType::FRET_DIAGRAM || type2 == ElementType::HARMONY)) {
        bool isFretDiagAgainstItsOwnHarmony = itemInSkyline->parentItem() == itemToAutoplace;
        bool areOnDifferentSegments = itemToAutoplace->findAncestor(ElementType::SEGMENT)
                                      != itemInSkyline->findAncestor(ElementType::SEGMENT);
        return isFretDiagAgainstItsOwnHarmony || areOnDifferentSegments;
    }

    if ((type1 == ElementType::DYNAMIC || type1 == ElementType::HAIRPIN_SEGMENT)
        && (type2 == ElementType::DYNAMIC || type2 == ElementType::HAIRPIN_SEGMENT)) {
        return true;
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
