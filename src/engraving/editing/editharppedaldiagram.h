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

#include "undo.h"

#include "../dom/harppedaldiagram.h"

namespace mu::engraving {
class ChangeHarpPedalState : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeHarpPedalState)
    HarpPedalDiagram* diagram;
    std::array<PedalPosition, HARP_STRING_NO> pedalState;

    void flip(EditData*) override;

public:
    ChangeHarpPedalState(HarpPedalDiagram* _diagram, std::array<PedalPosition, HARP_STRING_NO> _pedalState)
        : diagram(_diagram), pedalState(_pedalState) {}

    UNDO_NAME("ChangeHarpPedalState")
//    UNDO_CHANGED_OBJECTS({ diagram })
    std::vector<EngravingObject*> objectItems() const override;
};

class ChangeSingleHarpPedal : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeSingleHarpPedal)

    HarpPedalDiagram* diagram;
    HarpStringType type;
    PedalPosition pos;

    void flip(EditData*) override;

public:
    ChangeSingleHarpPedal(HarpPedalDiagram* _diagram, HarpStringType _type, PedalPosition _pos)
        : diagram(_diagram),
        type(_type),
        pos(_pos)
    {
    }

    UNDO_NAME("ChangeSingleHarpPedal")
//    UNDO_CHANGED_OBJECTS({ diagram });
    std::vector<EngravingObject*> objectItems() const override;
};
}
