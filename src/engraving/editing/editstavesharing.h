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
class KeyList;
class SharedPart;

using StaveSharingGroup = std::vector<Part*>;
using StaveSharingGroups = std::vector<StaveSharingGroup>;

class EditStaveSharing
{
public:
    static void cmdChangeStaveSharing(Score* score, bool enable);
    static void handleRemovePart(Part* part);

private:
    static void cmdCreateSharedStaves(Score* score);
    static void cmdRemoveSharedStaves(Score* score);

    static StaveSharingGroups computeGroups(Score* score);
    static void createSharedParts(const StaveSharingGroups& groups, Score* score);
    static SharedPart* createSharedPart(Score* score, size_t idx, Part* firstPart, const KeyList& keyList);
};

class ConnectSharedPart : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ConnectSharedPart)

    SharedPart* sharedPart = nullptr;
    Part* originPart = nullptr;

public:
    ConnectSharedPart(SharedPart* s, Part* o)
        : sharedPart(s), originPart(o) {}

    void undo(EditData*) override;
    void redo(EditData*) override;

    UNDO_TYPE(CommandType::ConnectSharedPart)
    UNDO_NAME("Connect shared part")
};

class DisconnectSharedPart : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, DisconnectSharedPart)

    SharedPart* sharedPart = nullptr;
    Part* originPart = nullptr;

public:
    DisconnectSharedPart(SharedPart* s, Part* o)
        : sharedPart(s), originPart(o) {}

    void undo(EditData*) override;
    void redo(EditData*) override;

    UNDO_TYPE(CommandType::DisconnectSharedPart)
    UNDO_NAME("Disconnect shared part")
};
}
