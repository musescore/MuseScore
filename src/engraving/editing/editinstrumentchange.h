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

#include "../dom/instrchange.h"

namespace mu::engraving {
/// change instrument in an InstrumentChange element
class ChangeInstrument : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeInstrument)

    InstrumentChange* is = nullptr;
    Instrument* instrument = nullptr;

    void flip(EditData*) override;

public:
    ChangeInstrument(InstrumentChange* _is, Instrument* i)
        : is(_is), instrument(i) {}

    UNDO_TYPE(CommandType::ChangeInstrument)
    UNDO_NAME("ChangeInstrument")
    UNDO_CHANGED_OBJECTS({ is })
};
}
