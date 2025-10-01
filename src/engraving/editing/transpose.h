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

#include "../dom/harmony.h"
#include "../dom/interval.h"

namespace mu::engraving {
class TransposeHarmony : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, TransposeHarmony)

    Harmony* m_harmony = nullptr;

    Interval m_interval = Interval(0, 0);
    bool m_useDoubleSharpsFlats = false;

    void flip(EditData*) override;

public:
    TransposeHarmony(Harmony*, Interval interval, bool useDoubleSharpsFlats);

    UNDO_TYPE(CommandType::TransposeHarmony)
    UNDO_NAME("TransposeHarmony")
    UNDO_CHANGED_OBJECTS({ m_harmony })
};

class TransposeHarmonyDiatonic : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, TransposeHarmonyDiatonic)

    Harmony* m_harmony = nullptr;
    int m_interval = 0;
    bool m_useDoubleSharpsFlats = false;
    bool m_transposeKeys = false;

    void flip(EditData*) override;

public:
    TransposeHarmonyDiatonic(Harmony*, int interval, bool useDoubleSharpsFlats, bool transposeKeys);

    UNDO_TYPE(CommandType::TransposeHarmony)
    UNDO_NAME("TransposeHarmonyDiatonic")
    UNDO_CHANGED_OBJECTS({ m_harmony })
};
}
