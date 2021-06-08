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

#include "draw/geometry.h"

namespace Ms {
class Element;
class EditData;

//-------------------------------------------------------------------
//   ElementGroup
///   Base class for implementing logic to handle groups of elements
///   together in certain operations.
//-------------------------------------------------------------------

class ElementGroup
{
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
public:
    bool enabled() const override { return false; }

    void startDrag(EditData&) override {}
    mu::RectF drag(EditData&) override { return mu::RectF(); }
    void endDrag(EditData&) override {}
};

//-------------------------------------------------------------------
//   SingleElementGroup
///   Element group for single element.
//-------------------------------------------------------------------

class SingleElementGroup final : public ElementGroup
{
    Element* e;
public:
    SingleElementGroup(Element* el)
        : e(el) {}

    void startDrag(EditData& ed) override;
    mu::RectF drag(EditData& ed) override;
    void endDrag(EditData& ed) override;
};
} // namespace Ms

#endif
