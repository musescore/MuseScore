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

#ifndef __DYNAMICHAIRPINGROUP_H__
#define __DYNAMICHAIRPINGROUP_H__

#include <memory>
#include <functional>

#include "elementgroup.h"
#include "draw/types/geometry.h"

namespace mu::engraving {
class Dynamic;
class Hairpin;
class HairpinSegment;
class Expression;

//-------------------------------------------------------------------
//   HairpinWithDynamicsDragGroup
///   Sequence of Dynamics and Hairpins
//-------------------------------------------------------------------

class HairpinWithDynamicsDragGroup : public ElementGroup
{
    OBJECT_ALLOCATOR(engraving, HairpinWithDynamicsDragGroup)

    Dynamic* startDynamic;
    HairpinSegment* hairpinSegment;
    Dynamic* endDynamic;

public:
    HairpinWithDynamicsDragGroup(Dynamic* start, HairpinSegment* hs, Dynamic* end)
        : startDynamic(start), hairpinSegment(hs), endDynamic(end) {}

    void startDrag(EditData&) override;
    mu::RectF drag(EditData&) override;
    void endDrag(EditData&) override;

    static std::unique_ptr<ElementGroup> detectFor(HairpinSegment* hs, std::function<bool(const EngravingItem*)> isDragged);
    static std::unique_ptr<ElementGroup> detectFor(Dynamic* d, std::function<bool(const EngravingItem*)> isDragged);
};

//-------------------------------------------------------------------
//   DynamicNearHairpinsDragGroup
//-------------------------------------------------------------------

class DynamicNearHairpinsDragGroup : public ElementGroup
{
    OBJECT_ALLOCATOR(engraving, DynamicNearHairpinsDragGroup)

    Hairpin* leftHairpin;
    Dynamic* dynamic;
    Hairpin* rightHairpin;

public:
    DynamicNearHairpinsDragGroup(Hairpin* left, Dynamic* d, Hairpin* right)
        : leftHairpin(left), dynamic(d), rightHairpin(right) {}

    void startDrag(EditData&) override;
    mu::RectF drag(EditData&) override;
    void endDrag(EditData&) override;

    static std::unique_ptr<ElementGroup> detectFor(Dynamic* d, std::function<bool(const EngravingItem*)> isDragged);
};

//-------------------------------------------------------------------
//   DynamicExpressionDragGroup
//-------------------------------------------------------------------

class DynamicExpressionDragGroup : public ElementGroup
{
    OBJECT_ALLOCATOR(engraving, DynamicNearHairpinsDragGroup)

    Dynamic* dynamic;
    Expression* expression;

public:
    DynamicExpressionDragGroup(Dynamic* d, Expression* e)
        : dynamic(d), expression(e) {}

    void startDrag(EditData& ed) override;
    mu::RectF drag(EditData& ed) override;
    void endDrag(EditData& ed) override;

    static std::unique_ptr<ElementGroup> detectFor(Dynamic* d, std::function<bool(const EngravingItem*)> isDragged);
    static std::unique_ptr<ElementGroup> detectFor(Expression* e, std::function<bool(const EngravingItem*)> isDragged);
};
} // namespace mu::engraving

#endif
