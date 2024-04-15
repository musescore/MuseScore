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

#include "editdata.h"

#include "engravingitem.h"

using namespace mu::engraving;

void EditData::clear()
{
    *this = EditData(m_view);
}

std::shared_ptr<ElementEditData> EditData::getData(const EngravingItem* e) const
{
    for (std::shared_ptr<ElementEditData> ed : m_data) {
        if (ed->e == e) {
            return ed;
        }
    }
    return 0;
}

bool EditData::control(bool textEditing) const
{
    if (textEditing) {
        return modifiers & TextEditingControlModifier;
    } else {
        return modifiers & ControlModifier;
    }
}
