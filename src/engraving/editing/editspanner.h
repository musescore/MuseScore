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

#include "../dom/spanner.h"

namespace mu::engraving {
class ChangeSpannerElements : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeSpannerElements)

    Spanner* spanner = nullptr;
    EngravingItem* startElement = nullptr;
    EngravingItem* endElement = nullptr;

    void flip(EditData*) override;

public:
    ChangeSpannerElements(Spanner* s, EngravingItem* se, EngravingItem* ee)
        : spanner(s), startElement(se), endElement(ee) {}

    UNDO_TYPE(CommandType::ChangeSpannerElements)
    UNDO_NAME("ChangeSpannerElements")
    UNDO_CHANGED_OBJECTS({ spanner })
};

class ChangeStartEndSpanner : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeStartEndSpanner)

    Spanner* spanner = nullptr;
    EngravingItem* start = nullptr;
    EngravingItem* end = nullptr;

    void flip(EditData*) override;

public:
    ChangeStartEndSpanner(Spanner* sp, EngravingItem* s, EngravingItem* e)
        : spanner(sp), start(s), end(e) {}

    UNDO_TYPE(CommandType::ChangeStartEndSpanner)
    UNDO_NAME("ChangeStartEndSpanner")
    UNDO_CHANGED_OBJECTS({ spanner })
};
}
