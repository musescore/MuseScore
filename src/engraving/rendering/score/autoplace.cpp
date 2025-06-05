/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
    // rebase vertical offset on drag
    double rebase = 0.0;
    if (ldata->autoplace.offsetChanged != OffsetChange::NONE) {
        rebase = rebaseOffset(item, ldata);
    }

    // TODO: proper item-to-item table for horizontal clearance in skyline
    const double minSkylineHorizontalClearance = item->isArticulationOrFermata() ? 0.0 : item->style().styleMM(
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
        Shape shape = item->ldata()->shape().translate(item->systemPos());
        RectF r = shape.bbox();

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
                staffSkyline.add(shape);
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
            if (ldata->autoplace.offsetChanged != OffsetChange::NONE) {
                // user moved element within the skyline
                // we may need to adjust minDistance, yd, and/or offset
                bool inStaff = above ? r.bottom() + rebase > 0.0 : r.top() + rebase < item->staff()->staffHeight(item->tick());
                if (rebaseMinDistance(item, ldata, minDistance, yd, sp, rebase, above, inStaff)) {
                    shape.translate(PointF(0.0, rebase));
                }
            }
            ldata->moveY(yd);
            shape.translate(PointF(0.0, yd));
        }

        if (add && item->addToSkyline()) {
            staffSkyline.add(shape);
        }
    }
    setOffsetChanged(item, ldata, false);
}

void Autoplace::autoplaceMeasureElement(const EngravingItem* item, EngravingItem::LayoutData* ldata, bool above, bool add)
{
    // rebase vertical offset on drag
    double rebase = 0.0;
    if (ldata->autoplace.offsetChanged != OffsetChange::NONE) {
        rebase = rebaseOffset(item, ldata);
    }

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
        Shape sh = item->shape().translate(m->pos() + item->pos() + item->staffOffset());

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
            if (ldata->autoplace.offsetChanged != OffsetChange::NONE) {
                // user moved element within the skyline
                // we may need to adjust minDistance, yd, and/or offset
                bool inStaff = above ? sh.bottom() + rebase > 0.0 : sh.top() + rebase < item->staff()->staffHeight(item->tick());
                if (rebaseMinDistance(item, ldata, minDistance, yd, sp, rebase, above, inStaff)) {
                    sh.translateY(rebase);
                }
            }
            ldata->moveY(yd);
            sh.translateY(yd);
        }
        if (add && item->addToSkyline()) {
            staffSkyline.add(sh);
        }
    }
    setOffsetChanged(item, ldata, false);
}

void Autoplace::autoplaceSpannerSegment(const SpannerSegment* item, EngravingItem::LayoutData* ldata, double sp)
{
    if (item->isStyled(Pid::OFFSET)) {
        const_cast<SpannerSegment*>(item)->setOffset(item->spanner()->propertyDefault(Pid::OFFSET).value<PointF>());
    }

    if (item->spanner()->anchor() == Spanner::Anchor::NOTE) {
        return;
    }

    // rebase vertical offset on drag
    double rebase = 0.0;
    if (ldata->offsetChanged() != OffsetChange::NONE) {
        rebase = rebaseOffset(item, ldata);
    }

    if (item->autoplace()) {
        if (!item->systemFlag() && !item->spanner()->systemFlag()) {
            sp *= item->staff()->staffMag(item->spanner()->tick());
        }
        double md = item->minDistance().val() * sp;
        bool above = item->spanner()->placeAbove();
        SkylineLine sl(!above);
        Shape sh = item->shape();
        sl.add(sh.translate(item->pos()));
        double yd = 0.0;
        staff_idx_t stfIdx = item->effectiveStaffIdx();
        if (stfIdx == muse::nidx) {
            ldata->setIsSkipDraw(true);
            return;
        } else {
            ldata->setIsSkipDraw(false);
        }
        const System* system = item->system();
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
            if (ldata->offsetChanged() != OffsetChange::NONE) {
                // user moved element within the skyline
                // we may need to adjust minDistance, yd, and/or offset
                double adj = item->pos().y() + rebase;
                bool inStaff = above ? sh.bottom() + adj > 0.0 : sh.top() + adj < item->staff()->staffHeight(item->tick());
                rebaseMinDistance(item, ldata, md, yd, sp, rebase, above, inStaff);
            }
            ldata->moveY(yd);
        }
    }
    setOffsetChanged(item, ldata, false);
}

//---------------------------------------------------------
//   rebaseOffset
//    calculates new offset for moved elements
//    for drag & other actions that result in absolute position, apply the new offset
//    for nudge & other actions that result in relative adjustment, return the vertical difference
//---------------------------------------------------------

double Autoplace::rebaseOffset(const EngravingItem* item, EngravingItem::LayoutData* ldata, bool nox)
{
    LD_CONDITION(ldata->isSetPos());

    PointF off = item->offset();
    PointF p = ldata->autoplace.changedPos - item->pos();
    if (nox) {
        p.rx() = 0.0;
    }
    //OffsetChange saveChangedValue = _offsetChanged;

    bool staffRelative = item->staff() && item->explicitParent() && !(item->explicitParent()->isNote() || item->explicitParent()->isRest());
    if (staffRelative && item->propertyFlags(Pid::PLACEMENT) != PropertyFlags::NOSTYLE) {
        // check if flipped
        // TODO: elements that support PLACEMENT but not as a styled property (add supportsPlacement() method?)
        // TODO: refactor to take advantage of existing cmdFlip() algorithms
        // TODO: adjustPlacement() (from read206.cpp) on read for 3.0 as well
        RectF r = ldata->bbox().translated(ldata->autoplace.changedPos);
        double staffHeight = item->staff()->staffHeight(item->tick());
        const EngravingItem* e = item->isSpannerSegment() ? toSpannerSegment(item)->spanner() : item;
        bool multi = e->isSpanner() && toSpanner(e)->spannerSegments().size() > 1;
        bool above = e->placeAbove();
        bool flipped = above ? r.top() > staffHeight : r.bottom() < 0.0;
        if (flipped && !multi) {
            off.ry() += above ? -staffHeight : staffHeight;
            const_cast<EngravingItem*>(item)->undoChangeProperty(Pid::OFFSET, PropertyValue::fromValue(off + p));
            ldata->autoplace.offsetChanged = OffsetChange::ABSOLUTE_OFFSET;             //saveChangedValue;
            ldata->moveY(above ? staffHeight : -staffHeight);
            PropertyFlags pf = e->propertyFlags(Pid::PLACEMENT);
            if (pf == PropertyFlags::STYLED) {
                pf = PropertyFlags::UNSTYLED;
            }
            PlacementV place = above ? PlacementV::BELOW : PlacementV::ABOVE;
            const_cast<EngravingItem*>(e)->undoChangeProperty(Pid::PLACEMENT, int(place), pf);
            const_cast<EngravingItem*>(item)->undoResetProperty(Pid::MIN_DISTANCE);
            return 0.0;
        }
    }

    if (ldata->autoplace.offsetChanged == OffsetChange::ABSOLUTE_OFFSET) {
        const_cast<EngravingItem*>(item)->undoChangeProperty(Pid::OFFSET, PropertyValue::fromValue(off + p));
        ldata->autoplace.offsetChanged = OffsetChange::ABSOLUTE_OFFSET;                 //saveChangedValue;
        // allow autoplace to manage min distance even when not needed
        const_cast<EngravingItem*>(item)->undoResetProperty(Pid::MIN_DISTANCE);
        return 0.0;
    }

    // allow autoplace to manage min distance even when not needed
    const_cast<EngravingItem*>(item)->undoResetProperty(Pid::MIN_DISTANCE);
    return p.y();
}

//---------------------------------------------------------
//   rebaseMinDistance
//    calculates new minDistance for moved elements
//    if necessary, also rebases offset
//    updates md, yd
//    returns true if shape needs to be rebased
//---------------------------------------------------------

bool Autoplace::rebaseMinDistance(const EngravingItem* item, EngravingItem::LayoutData* ldata, double& md, double& yd, double sp,
                                  double rebase, bool above, bool fix)
{
    bool rc = false;
    PropertyFlags pf = item->propertyFlags(Pid::MIN_DISTANCE);
    if (pf == PropertyFlags::STYLED) {
        pf = PropertyFlags::UNSTYLED;
    }
    double adjustedY = item->pos().y() + yd;
    double diff = ldata->autoplace.changedPos.y() - adjustedY;
    if (fix) {
        const_cast<EngravingItem*>(item)->undoChangeProperty(Pid::MIN_DISTANCE, -999.0, pf);
        yd = 0.0;
    } else if (!item->isStyled(Pid::MIN_DISTANCE)) {
        md = (above ? md + yd : md - yd) / sp;
        const_cast<EngravingItem*>(item)->undoChangeProperty(Pid::MIN_DISTANCE, md, pf);
        yd += diff;
    } else {
        // min distance still styled
        // user apparently moved element into skyline
        // but perhaps not really, if performing a relative adjustment
        if (ldata->autoplace.offsetChanged == OffsetChange::RELATIVE_OFFSET) {
            // relative movement (cursor): fix only if moving vertically into direction of skyline
            if ((above && diff > 0.0) || (!above && diff < 0.0)) {
                // rebase offset
                PointF p = item->offset();
                p.ry() += rebase;
                const_cast<EngravingItem*>(item)->undoChangeProperty(Pid::OFFSET, p);
                md = (above ? md - diff : md + diff) / sp;
                const_cast<EngravingItem*>(item)->undoChangeProperty(Pid::MIN_DISTANCE, md, pf);
                rc = true;
                yd = 0.0;
            }
        } else {
            // absolute movement (drag): fix unconditionally
            md = (above ? md + yd : md - yd) / sp;
            const_cast<EngravingItem*>(item)->undoChangeProperty(Pid::MIN_DISTANCE, md, pf);
            yd = 0.0;
        }
    }
    return rc;
}

void Autoplace::setOffsetChanged(const EngravingItem* item, EngravingItem::LayoutData* ldata, bool v, bool absolute, const PointF& diff)
{
    if (v) {
        ldata->autoplace.offsetChanged = absolute ? OffsetChange::ABSOLUTE_OFFSET : OffsetChange::RELATIVE_OFFSET;
    } else {
        ldata->autoplace.offsetChanged = OffsetChange::NONE;
    }
    ldata->autoplace.changedPos = item->pos() + diff;
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

    if (type1 == ElementType::FRET_DIAGRAM && type2 == ElementType::HARMONY) {
        return true;
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
            ElementType::FOOTNOTE,
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

    if (type1 == ElementType::FERMATA && itemInSkyline->isArticulationOrFermata()) {
        // Fermata should ignore articulation on other segments
        return itemToAutoplace->parent() != itemInSkyline->parent();
    }

    return itemToAutoplace->ldata()->itemSnappedBefore() == itemInSkyline || itemToAutoplace->ldata()->itemSnappedAfter() == itemInSkyline;
}
