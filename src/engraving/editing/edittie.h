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

namespace mu::engraving {
class TieJumpPointList;
class ChangeTieJumpPointActive : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeTieJumpPointActive)

    TieJumpPointList* m_jumpPointList = nullptr;
    String m_id;
    bool m_active = false;

    void flip(EditData*) override;

public:
    ChangeTieJumpPointActive(TieJumpPointList* jumpPointList, String& id, bool active)
        : m_jumpPointList(jumpPointList), m_id(id), m_active(active) {}

    UNDO_TYPE(CommandType::ChangeTieEndPointActive)
    UNDO_NAME("ChangeTieEndPointActive")
};
}
