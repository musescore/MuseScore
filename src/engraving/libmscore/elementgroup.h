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

#ifndef __ELEMENTGROUP_H__
#define __ELEMENTGROUP_H__

#include "global/allocator.h"
#include "draw/types/geometry.h"

namespace mu::engraving {
class EngravingItem;
class EditData;

//-------------------------------------------------------------------
//   ElementGroup
///   Base class for implementing logic to handle groups of elements
///   together in certain operations.
//-------------------------------------------------------------------

class ElementGroup
{
    OBJECT_ALLOCATOR(engraving, ElementGroup)
public:
    virtual ~ElementGroup() {}

    virtual void startDrag(EditData&) = 0;
    virtual mu::RectF drag(EditData&) = 0;
    virtual void endDrag(EditData&) = 0;

    virtual bool enabled() const { return true; }
};

//-------------------------------------------------------------------
//   DisabledElementGroup
//-------------------------------------------------------------------

class DisabledElementGroup final : public ElementGroup
{
    OBJECT_ALLOCATOR(engraving, DisabledElementGroup)
public:
    bool enabled() const override { return false; }

    void startDrag(EditData&) override {}
    mu::RectF drag(EditData&) override { return mu::RectF(); }
    void endDrag(EditData&) override {}
};

//-------------------------------------------------------------------
//   SingleElementGroup
///   EngravingItem group for single element.
//-------------------------------------------------------------------

class SingleElementGroup final : public ElementGroup
{
    OBJECT_ALLOCATOR(engraving, SingleElementGroup)

    EngravingItem* e;
public:
    SingleElementGroup(EngravingItem* el)
        : e(el) {}

    void startDrag(EditData& ed) override;
    mu::RectF drag(EditData& ed) override;
    void endDrag(EditData& ed) override;
};
} // namespace mu::engraving

#endif
