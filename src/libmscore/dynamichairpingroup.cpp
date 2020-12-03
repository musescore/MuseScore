//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "dynamichairpingroup.h"
#include "dynamic.h"
#include "hairpin.h"
#include "score.h"
#include "segment.h"

namespace Ms {
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
                                                                      std::function<bool(const Element*)> isDragged)
{
    if (!hs->isSingleType()) {
        return nullptr;
    }

    Hairpin* hairpin = hs->hairpin();

    Segment* startSegment = hairpin->startSegment();
    Segment* endSegment = hairpin->endSegment();
    const int track = hs->track();

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

std::unique_ptr<ElementGroup> HairpinWithDynamicsDragGroup::detectFor(Dynamic* d,
                                                                      std::function<bool(const Element*)> isDragged)
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
    if (startDynamic) {
        startDynamic->startDrag(ed);
    }
    static_cast<Element*>(hairpinSegment)->startDrag(ed);
    if (endDynamic) {
        endDynamic->startDrag(ed);
    }
}

QRectF HairpinWithDynamicsDragGroup::drag(EditData& ed)
{
    QRectF r;

    if (startDynamic) {
        r |= static_cast<Element*>(startDynamic)->drag(ed);
    }
    r |= hairpinSegment->drag(ed);
    if (endDynamic) {
        r |= static_cast<Element*>(endDynamic)->drag(ed);
    }

    Hairpin* h = hairpinSegment->hairpin();

    const Fraction startTick = startDynamic ? startDynamic->segment()->tick() : h->tick();
    const Fraction endTick = endDynamic ? endDynamic->segment()->tick() : h->tick2();

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
    if (startDynamic) {
        startDynamic->endDrag(ed);
        startDynamic->triggerLayout();
    }

    hairpinSegment->endDrag(ed);
    hairpinSegment->triggerLayout();

    if (endDynamic) {
        endDynamic->endDrag(ed);
        endDynamic->triggerLayout();
    }
}

std::unique_ptr<ElementGroup> DynamicNearHairpinsDragGroup::detectFor(Dynamic* d,
                                                                      std::function<bool(const Element*)> isDragged)
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
    dynamic->startDrag(ed);
}

QRectF DynamicNearHairpinsDragGroup::drag(EditData& ed)
{
    QRectF r(static_cast<Element*>(dynamic)->drag(ed));

    const Fraction tick = dynamic->segment()->tick();

    if (leftHairpin && leftHairpin->tick2() != tick && tick > leftHairpin->tick()) {
        leftHairpin->undoChangeProperty(Pid::SPANNER_TICKS, tick - leftHairpin->tick());
    }

    if (rightHairpin && rightHairpin->tick() != tick) {
        const Fraction tick2 = rightHairpin->tick2();
        if (tick < tick2) {
            rightHairpin->undoChangeProperty(Pid::SPANNER_TICK, tick);
            rightHairpin->undoChangeProperty(Pid::SPANNER_TICKS, tick2 - tick);
        }
    }

    if (leftHairpin || rightHairpin) {
        dynamic->triggerLayout();
    }

    return r;
}

void DynamicNearHairpinsDragGroup::endDrag(EditData& ed)
{
    dynamic->endDrag(ed);
    dynamic->triggerLayout();
}
} // namespace Ms
