/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include "global/io/iodevice.h"

#include "transaction/undoablecommand.h"

#include "../dom/score.h"

namespace mu::engraving {
class Transaction;

class ChangeStyle : public UndoableCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeStyle)

    Score* score = nullptr;
    MStyle style;
    bool overlap = false;

    void flip() override;
    void undo() override;

public:
    ChangeStyle(Score*, const MStyle&, const bool overlapOnly = false);

    StyleIdSet changedIds() const;

    UNDO_TYPE(CommandType::ChangeStyle)
    UNDO_NAME("ChangeStyle")
    UNDO_CHANGED_OBJECTS({ score })
};

class ChangeStyleValues : public UndoableCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeStyleValues)

public:
    ChangeStyleValues(Score* s, std::unordered_map<Sid, PropertyValue> values)
        : m_score(s), m_values(std::move(values)) {}

    const std::unordered_map<Sid, PropertyValue>& values() const { return m_values; }

    UNDO_TYPE(CommandType::ChangeStyleValues)
    UNDO_NAME("ChangeStyleValues")
    UNDO_CHANGED_OBJECTS({ m_score })

private:
    void flip() override;

    Score* m_score = nullptr;
    std::unordered_map<Sid, PropertyValue> m_values;
};

class EditStyle
{
public:
    static bool loadStyle(Transaction& tx, Score* score, muse::io::IODevice& dev, bool ign = false, bool overlap = false);
};
}
