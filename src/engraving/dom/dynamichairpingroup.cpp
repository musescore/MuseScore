/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "dynamichairpingroup.h"

#include "dynamic.h"
#include "expression.h"
#include "hairpin.h"
#include "score.h"
#include "segment.h"
#include "undo.h"

using namespace mu;

namespace mu::engraving {
static std::pair<Hairpin*, Hairpin*> findAdjacentHairpins(Dynamic* d)
{
    Score* score = d->score();
    const Segment* dSeg = d->segment();
    Hairpin* leftHairpin = nullptr;
    Hairpin* rightHairpin = nullptr;

    const Fraction tick = dSeg->tick();
    const int intTick = tick.ticks();

    const auto& nearSpanners = score->spannerMap().findOverlapping(intTick - 1, intTick + 1);
    for (auto i : nearSpanners) {
        Spanner* s = i.value;
        if (s->track() == d->track() && s->isHairpin()) {
            Hairpin* h = toHairpin(s);
            if (h->tick() == tick) {
                rightHairpin = h;
            } else if (h->tick2() == tick) {
                leftHairpin = h;
            }
        }
    }

    return { leftHairpin, rightHairpin };
}

std::unique_ptr<ElementGroup> HairpinWithDynamicsDragGroup::detectFor(HairpinSegment* hs,
                                                                      std::function<bool(const EngravingItem*)> isDragged)
{
    if (!hs->isSingleType()) {
        return nullptr;
    }

    Hairpin* hairpin = hs->hairpin();

    Segment* startSegment = hairpin->startSegment();
    Segment* endSegment = hairpin->endSegment();
    const track_idx_t track = hs->track();

    Dynamic* startDynamic = toDynamic(startSegment->findAnnotation(ElementType::DYNAMIC, track, track));
    Dynamic* endDynamic = toDynamic(endSegment->findAnnotation(ElementType::DYNAMIC, track, track));

    // Include only dragged dynamics to this group
    if (!isDragged(startDynamic)) {
        startDynamic = nullptr;
    }
    if (!isDragged(endDynamic)) {
        endDynamic = nullptr;
    }

    if (startDynamic || endDynamic) {
        return std::unique_ptr<ElementGroup>(new HairpinWithDynamicsDragGroup(startDynamic, hs, endDynamic));
    }
    return nullptr;
}

std::unique_ptr<ElementGroup> HairpinWithDynamicsDragGroup::detectFor(Dynamic* d, std::function<bool(const EngravingItem*)> isDragged)
{
    Hairpin* leftHairpin = nullptr;
    Hairpin* rightHairpin = nullptr;

    std::tie(leftHairpin, rightHairpin) = findAdjacentHairpins(d);

    // Dynamic will be governed bt HairpinWithDynamicsDragGroup if any of adjacent
    // hairpins is dragged, disable separate drag logic for dynamic in this case.
    if (isDragged(leftHairpin)) {
        return std::unique_ptr<ElementGroup>(new DisabledElementGroup());
    }
    if (isDragged(rightHairpin)) {
        return std::unique_ptr<ElementGroup>(new DisabledElementGroup());
    }

    return nullptr;
}

void HairpinWithDynamicsDragGroup::startDrag(EditData& ed)
{
    if (m_startDynamic) {
        m_startDynamic->startDrag(ed);
    }
    static_cast<EngravingItem*>(m_hairpinSegment)->startDrag(ed);
    if (m_endDynamic) {
        m_endDynamic->startDrag(ed);
    }
}

mu::RectF HairpinWithDynamicsDragGroup::drag(EditData& ed)
{
    RectF r;

    if (m_startDynamic) {
        r.unite(static_cast<EngravingItem*>(m_startDynamic)->drag(ed));
    }
    r.unite(m_hairpinSegment->drag(ed));
    if (m_endDynamic) {
        r.unite(static_cast<EngravingItem*>(m_endDynamic)->drag(ed));
    }

    Hairpin* h = m_hairpinSegment->hairpin();

    const Fraction startTick = m_startDynamic ? m_startDynamic->segment()->tick() : h->tick();
    const Fraction endTick = m_endDynamic ? m_endDynamic->segment()->tick() : h->tick2();

    if (endTick > startTick) {
        if (h->tick() != startTick) {
            h->undoChangeProperty(Pid::SPANNER_TICK, startTick);
        }
        if (h->tick2() != endTick) {
            h->undoChangeProperty(Pid::SPANNER_TICKS, endTick - startTick);
        }
    }

    return r;
}

void HairpinWithDynamicsDragGroup::endDrag(EditData& ed)
{
    if (m_startDynamic) {
        m_startDynamic->endDrag(ed);
        m_startDynamic->triggerLayout();
    }

    m_hairpinSegment->endDrag(ed);
    m_hairpinSegment->triggerLayout();

    if (m_endDynamic) {
        m_endDynamic->endDrag(ed);
        m_endDynamic->triggerLayout();
    }
}

std::unique_ptr<ElementGroup> DynamicNearHairpinsDragGroup::detectFor(Dynamic* d, std::function<bool(const EngravingItem*)> isDragged)
{
    Hairpin* leftHairpin = nullptr;
    Hairpin* rightHairpin = nullptr;

    std::tie(leftHairpin, rightHairpin) = findAdjacentHairpins(d);

    // Drag hairpins according to this rule only if they are not being dragged themselves
    if (isDragged(leftHairpin)) {
        leftHairpin = nullptr;
    }
    if (isDragged(rightHairpin)) {
        rightHairpin = nullptr;
    }

    if (leftHairpin || rightHairpin) {
        return std::unique_ptr<ElementGroup>(new DynamicNearHairpinsDragGroup(leftHairpin, d, rightHairpin));
    }
    return nullptr;
}

void DynamicNearHairpinsDragGroup::startDrag(EditData& ed)
{
    m_dynamic->startDrag(ed);
}

RectF DynamicNearHairpinsDragGroup::drag(EditData& ed)
{
    RectF r(static_cast<EngravingItem*>(m_dynamic)->drag(ed));

    const Fraction tick = m_dynamic->segment()->tick();

    if (m_leftHairpin && m_leftHairpin->tick2() != tick && tick > m_leftHairpin->tick()) {
        m_leftHairpin->undoChangeProperty(Pid::SPANNER_TICKS, tick - m_leftHairpin->tick());
    }

    if (m_rightHairpin && m_rightHairpin->tick() != tick) {
        const Fraction tick2 = m_rightHairpin->tick2();
        if (tick < tick2) {
            m_rightHairpin->undoChangeProperty(Pid::SPANNER_TICK, tick);
            m_rightHairpin->undoChangeProperty(Pid::SPANNER_TICKS, tick2 - tick);
        }
    }

    if (m_leftHairpin || m_rightHairpin) {
        m_dynamic->triggerLayout();
    }

    return r;
}

void DynamicNearHairpinsDragGroup::endDrag(EditData& ed)
{
    m_dynamic->endDrag(ed);
    m_dynamic->triggerLayout();
}

//-------------------------------------------------------
// DynamicExpressionDragGroup
//-------------------------------------------------------

std::unique_ptr<ElementGroup> DynamicExpressionDragGroup::detectFor(Dynamic* d, std::function<bool(const EngravingItem*)> isDragged)
{
    Expression* snappedExpression = d->snappedExpression();
    if (snappedExpression && !isDragged(snappedExpression)) {
        return std::unique_ptr<DynamicExpressionDragGroup>(new DynamicExpressionDragGroup(d, snappedExpression));
    }
    return nullptr;
}

std::unique_ptr<ElementGroup> DynamicExpressionDragGroup::detectFor(Expression* e, std::function<bool(const EngravingItem*)> isDragged)
{
    Dynamic* snappedDynamic = e->snappedDynamic();
    if (snappedDynamic && !isDragged(snappedDynamic)) {
        return std::unique_ptr<DynamicExpressionDragGroup>(new DynamicExpressionDragGroup(snappedDynamic, e));
    }
    return nullptr;
}

void DynamicExpressionDragGroup::startDrag(EditData& ed)
{
    m_dynamic->startDrag(ed);
    m_expression->startDrag(ed);
}

RectF DynamicExpressionDragGroup::drag(EditData& ed)
{
    RectF r = static_cast<EngravingItem*>(m_dynamic)->drag(ed);

    // Dynamic may snap to a different segment upon dragging,
    // in which case move the expression with it
    Segment* newSegment = m_dynamic->segment();
    Segment* oldSegment = toSegment(m_expression->explicitParent());
    staff_idx_t newStaff = m_dynamic->staffIdx();
    staff_idx_t oldStaff = m_expression->staffIdx();

    if (newSegment != oldSegment || newStaff != oldStaff) {
        Score* score = newSegment->score();
        score->undoChangeParent(m_expression, newSegment, newStaff);
    }

    m_dynamic->triggerLayout();
    m_expression->triggerLayout();

    return r;
}

void DynamicExpressionDragGroup::endDrag(EditData& ed)
{
    m_dynamic->endDrag(ed);
    m_dynamic->triggerLayout();
    m_expression->endDrag(ed);
    m_expression->triggerLayout();
}
} // namespace mu::engraving
