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

#include "../dom/key.h"
#include "../dom/keysig.h"

namespace mu::engraving {
class ChangeKeySig : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeKeySig)

    KeySig* keysig = nullptr;
    KeySigEvent ks;
    bool showCourtesy = false;
    bool evtInStaff = false;

    void flip(EditData*) override;

public:
    ChangeKeySig(KeySig* k, KeySigEvent newKeySig, bool sc, bool addEvtToStaff = true);

    UNDO_TYPE(CommandType::ChangeKeySig)
    UNDO_NAME("ChangeKeySig")
    UNDO_CHANGED_OBJECTS({ keysig })
};
}
