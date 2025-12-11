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

#pragma once

#include <functional>
#include "../types/types.h"

namespace mu::engraving {
class Note;

class Staff;
class StringData;

class EditCapo
{
public:
    static void updateNotationForCapoChange(const CapoParams& oldParams, const CapoParams& newParams, const Staff* staff, int startTick,
                                            int endTick);

private:
    struct UpdateCtx {
        const Staff* staff = nullptr;
        const StringData* stringData = nullptr;
        CapoParams params = {};
        int noteOffset = 0;
        bool updateIgnoredStrings = false;
        bool possibleFretConflict = false;
    };
    static void applyCapoTranspose(int startTick, int endTick, UpdateCtx& ctx);
    static void update(Note* note, const UpdateCtx& ctx);
    static void updateString(Note* note, const UpdateCtx& ctx);

    static void handleModeChange(const CapoParams& oldParams, const CapoParams& newParams, int startTick, int endTick, UpdateCtx& ctx);
    static void handleFretChange(const CapoParams& oldParams, const CapoParams& newParams, int startTick, int endTick, UpdateCtx& ctx);
    static void handleStringChanged(const CapoParams& newParams, int startTick, int endTick, UpdateCtx& ctx);
    static void handleActiveChanged(const CapoParams& newParams, int startTick, int endTick, UpdateCtx& ctx);
};
} // namespace mu::engraving
