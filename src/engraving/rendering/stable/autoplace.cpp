/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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

#include "libmscore/chordrest.h"
#include "libmscore/segment.h"
#include "libmscore/spanner.h"
#include "libmscore/staff.h"
#include "libmscore/system.h"
#include "libmscore/measure.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::stable;

void Autoplace::autoplaceSegmentElement(const EngravingItem* item, EngravingItem::LayoutData* ldata, bool above, bool add)
{
    // rebase vertical offset on drag
    double rebase = 0.0;
    if (ldata->autoplace.offsetChanged != OffsetChange::NONE) {
        rebase = rebaseOffset(item, ldata);
    }

    if (item->autoplace() && item->explicitParent()) {
        Segment* s = toSegment(item->explicitParent());
        Measure* m = s->measure();

        double sp = item->style().spatium();
        staff_idx_t si = item->staffIdxOrNextVisible();

        // if there's no good staff for this object, obliterate it
        ldata->setIsSkipDraw(si == mu::nidx);
        const_cast<EngravingItem*>(item)->setSelectable(!ldata->isSkipDraw());
        if (ldata->isSkipDraw()) {
            return;
        }

        double mag = item->staff()->staffMag(item);
        sp *= mag;
        double minDistance = item->minDistance().val() * sp;

        SysStaff* ss = m->system()->staff(si);
        RectF r = item->layoutData()->bbox().translated(m->pos() + s->pos() + item->pos());

        // Adjust bbox Y pos for staffType offset
        if (item->staffType()) {
            double stYOffset = item->staffType()->yoffset().val() * sp;
            r.translate(0.0, stYOffset);
        }

        SkylineLine sk(!above);
        double d;
        if (above) {
            sk.add(r.x(), r.bottom(), r.width());
            d = sk.minDistance(ss->skyline().north());
        } else {
            sk.add(r.x(), r.top(), r.width());
            d = ss->skyline().south().minDistance(sk);
        }

        if (d > -minDistance) {
            double yd = d + minDistance;
            if (above) {
                yd *= -1.0;
            }
            if (ldata->autoplace.offsetChanged != OffsetChange::NONE) {
                // user moved element within the skyline
                // we may need to adjust minDistance, yd, and/or offset
                bool inStaff = above ? r.bottom() + rebase > 0.0 : r.top() + rebase < item->staff()->height();
                if (rebaseMinDistance(item, ldata, minDistance, yd, sp, rebase, above, inStaff)) {
                    r.translate(0.0, rebase);
                }
            }
            ldata->moveY(yd);
            r.translate(PointF(0.0, yd));
        }
        if (add && item->addToSkyline()) {
            ss->skyline().add(r);
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
        Measure* m = toMeasure(item->explicitParent());
        staff_idx_t si = item->staffIdxOrNextVisible();

        // if there's no good staff for this object, obliterate it
        ldata->setIsSkipDraw(si == mu::nidx);
        const_cast<EngravingItem*>(item)->setSelectable(!ldata->isSkipDraw());
        if (ldata->isSkipDraw()) {
            return;
        }

        double sp = item->style().spatium();
        double minDistance = item->minDistance().val() * sp;

        SysStaff* ss = m->system()->staff(si);
        // shape rather than bbox is good for tuplets especially
        Shape sh = item->shape().translate(m->pos() + item->pos());

        SkylineLine sk(!above);
        double d;
        if (above) {
            sk.add(sh);
            d = sk.minDistance(ss->skyline().north());
        } else {
            sk.add(sh);
            d = ss->skyline().south().minDistance(sk);
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
                bool inStaff = above ? sh.bottom() + rebase > 0.0 : sh.top() + rebase < item->staff()->height();
                if (rebaseMinDistance(item, ldata, minDistance, yd, sp, rebase, above, inStaff)) {
                    sh.translateY(rebase);
                }
            }
            ldata->moveY(yd);
            sh.translateY(yd);
        }
        if (add && item->addToSkyline()) {
            ss->skyline().add(sh);
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
        sl.add(sh.translated(item->pos()));
        double yd = 0.0;
        staff_idx_t stfIdx = item->systemFlag() ? item->staffIdxOrNextVisible() : item->staffIdx();
        if (stfIdx == mu::nidx) {
            ldata->setIsSkipDraw(true);
            return;
        } else {
            ldata->setIsSkipDraw(false);
        }
        if (above) {
            double d = item->system()->topDistance(stfIdx, sl);
            if (d > -md) {
                yd = -(d + md);
            }
        } else {
            double d = item->system()->bottomDistance(stfIdx, sl);
            if (d > -md) {
                yd = d + md;
            }
        }
        if (yd != 0.0) {
            if (ldata->offsetChanged() != OffsetChange::NONE) {
                // user moved element within the skyline
                // we may need to adjust minDistance, yd, and/or offset
                double adj = item->pos().y() + rebase;
                bool inStaff = above ? sh.bottom() + adj > 0.0 : sh.top() + adj < item->staff()->height();
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
        double staffHeight = item->staff()->height();
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

//---------------------------------------------------------
//   doAutoplace
//    check for collisions
//---------------------------------------------------------
void Autoplace::doAutoplace(const Articulation* item, Articulation::LayoutData* ldata)
{
    // rebase vertical offset on drag
    double rebase = 0.0;
    if (ldata->offsetChanged() != OffsetChange::NONE) {
        rebase = rebaseOffset(item, ldata);
    }

    if (item->autoplace() && item->explicitParent()) {
        Segment* s = item->segment();
        Measure* m = item->measure();
        staff_idx_t si = item->staffIdx();

        double sp = item->style().spatium();
        double md = item->minDistance().val() * sp;

        SysStaff* ss = m->system()->staff(si);

        Shape thisShape = item->shape().translate(item->chordRest()->pos() + m->pos() + s->pos() + item->pos());

        for (ShapeElement& shapeEl : thisShape) {
            RectF r = shapeEl;

            double d = 0.0;
            bool above = item->up();
            SkylineLine sk(!above);
            if (above) {
                sk.add(r.x(), r.bottom(), r.width());
                d = sk.minDistance(ss->skyline().north());
            } else {
                sk.add(r.x(), r.top(), r.width());
                d = ss->skyline().south().minDistance(sk);
            }

            if (d > -md) {
                double yd = d + md;
                if (above) {
                    yd *= -1.0;
                }
                if (ldata->offsetChanged() != OffsetChange::NONE) {
                    // user moved element within the skyline
                    // we may need to adjust minDistance, yd, and/or offset
                    //bool inStaff = placeAbove() ? r.bottom() + rebase > 0.0 : r.top() + rebase < staff()->height();
                    if (rebaseMinDistance(item, ldata, md, yd, sp, rebase, above, true)) {
                        r.translate(0.0, rebase);
                    }
                }
                ldata->moveY(yd);
                thisShape.translateY(yd);
            }
        }
    }
    setOffsetChanged(item, ldata, false);
}
