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

#include "../dom/score.h"

namespace mu::engraving {
class ChangeMetaTags : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeMetaTags)

    Score* score = nullptr;
    std::map<String, String> metaTags;

    void flip(EditData*) override;

public:
    ChangeMetaTags(Score* s, const std::map<String, String>& m)
        : score(s), metaTags(m) {}

    UNDO_TYPE(CommandType::ChangeMetaInfo)
    UNDO_NAME("ChangeMetaTags")
    UNDO_CHANGED_OBJECTS({ score })
};

class ChangeMetaText : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeMetaText)

    Score* score = nullptr;
    String id;
    String text;

    void flip(EditData*) override;

public:
    ChangeMetaText(Score* s, const String& i, const String& t)
        : score(s), id(i), text(t) {}

    UNDO_TYPE(CommandType::ChangeMetaInfo)
    UNDO_NAME("ChangeMetaText")
    UNDO_CHANGED_OBJECTS({ score })
};

class ChangeScoreOrder : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeScoreOrder)

    Score* score = nullptr;
    ScoreOrder order;
    void flip(EditData*) override;

public:
    ChangeScoreOrder(Score* sc, ScoreOrder so)
        : score(sc), order(so) {}

    UNDO_TYPE(CommandType::ChangeScoreOrder)
    UNDO_NAME("ChangeScoreOrder")
    UNDO_CHANGED_OBJECTS({ score })
};

class ChangePageNumberOffset : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangePageNumberOffset)

    Score* score = nullptr;
    int pageOffset = 0;

    void flip(EditData*) override;

public:
    ChangePageNumberOffset(Score* s, int po)
        : score(s), pageOffset(po) {}

    UNDO_NAME("ChangePageNumberOffset")
    UNDO_CHANGED_OBJECTS({ score })
};
}
