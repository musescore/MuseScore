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

#ifndef MU_ENGRAVING_DYNAMICHAIRPINGROUP_H
#define MU_ENGRAVING_DYNAMICHAIRPINGROUP_H

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

public:
    HairpinWithDynamicsDragGroup(Dynamic* start, HairpinSegment* hs, Dynamic* end)
        : m_startDynamic(start), m_hairpinSegment(hs), m_endDynamic(end) {}

    void startDrag(EditData&) override;
    muse::RectF drag(EditData&) override;
    void endDrag(EditData&) override;

    static std::unique_ptr<ElementGroup> detectFor(HairpinSegment* hs, std::function<bool(const EngravingItem*)> isDragged);
    static std::unique_ptr<ElementGroup> detectFor(Dynamic* d, std::function<bool(const EngravingItem*)> isDragged);

private:

    Dynamic* m_startDynamic = nullptr;
    HairpinSegment* m_hairpinSegment = nullptr;
    Dynamic* m_endDynamic = nullptr;
};

//-------------------------------------------------------------------
//   DynamicNearHairpinsDragGroup
//-------------------------------------------------------------------

class DynamicNearHairpinsDragGroup : public ElementGroup
{
    OBJECT_ALLOCATOR(engraving, DynamicNearHairpinsDragGroup)

public:
    DynamicNearHairpinsDragGroup(Hairpin* left, Dynamic* d, Hairpin* right)
        : m_leftHairpin(left), m_dynamic(d), m_rightHairpin(right) {}

    void startDrag(EditData&) override;
    muse::RectF drag(EditData&) override;
    void endDrag(EditData&) override;

    static std::unique_ptr<ElementGroup> detectFor(Dynamic* d, std::function<bool(const EngravingItem*)> isDragged);

private:
    Hairpin* m_leftHairpin = nullptr;
    Dynamic* m_dynamic = nullptr;
    Hairpin* m_rightHairpin = nullptr;
};

//-------------------------------------------------------------------
//   DynamicExpressionDragGroup
//-------------------------------------------------------------------

class DynamicExpressionDragGroup : public ElementGroup
{
    OBJECT_ALLOCATOR(engraving, DynamicNearHairpinsDragGroup)

public:
    DynamicExpressionDragGroup(Dynamic* d, Expression* e)
        : m_dynamic(d), m_expression(e) {}

    void startDrag(EditData& ed) override;
    muse::RectF drag(EditData& ed) override;
    void endDrag(EditData& ed) override;

    static std::unique_ptr<ElementGroup> detectFor(Dynamic* d, std::function<bool(const EngravingItem*)> isDragged);
    static std::unique_ptr<ElementGroup> detectFor(Expression* e, std::function<bool(const EngravingItem*)> isDragged);

private:
    Dynamic* m_dynamic = nullptr;
    Expression* m_expression = nullptr;
};
} // namespace mu::engraving

#endif
