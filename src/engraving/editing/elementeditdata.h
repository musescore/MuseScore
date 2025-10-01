/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#pragma once

#include "../dom/engravingitem.h"
#include "../dom/property.h"

namespace mu::engraving {
//-----------------------------------------------------------------------------
//   ElementEditData
//    holds element specific data during element editing:
//
//    startDragGrip(EditData&)    creates data and attaches it to EditData
//         dragGrip(EditData&)
//      endDragGrip(EditData&)    use data to create undo records
//-----------------------------------------------------------------------------

enum class EditDataType : signed char {
    ElementEditData,
    TextEditData,
    BarLineEditData,
    BeamEditData,
    NoteEditData,
};

class ElementEditData
{
public:
    virtual ~ElementEditData() = default;

    struct PropertyData {
        Pid id;
        PropertyValue data;
        PropertyFlags f;
    };

    EngravingItem* e = nullptr;
    std::vector<PropertyData> propertyData;

    void pushProperty(Pid pid)
    {
        propertyData.emplace_back(PropertyData { pid, e->getProperty(pid), e->propertyFlags(pid) });
    }

    virtual EditDataType type() { return EditDataType::ElementEditData; }
};
}
