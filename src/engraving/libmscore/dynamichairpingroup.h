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
#include "infrastructure/draw/geometry.h"

namespace Ms {
class Dynamic;
class Hairpin;
class HairpinSegment;

//-------------------------------------------------------------------
//   HairpinWithDynamicsDragGroup
///   Sequence of Dynamics and Hairpins
//-------------------------------------------------------------------

class HairpinWithDynamicsDragGroup : public ElementGroup
{
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
} // namespace Ms

#endif
