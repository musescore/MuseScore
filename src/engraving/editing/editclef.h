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

#include "transaction/undoablecommand.h"

#include "../dom/clef.h"

namespace mu::engraving {
class ChangeClefType : public UndoableCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeClefType)

    Clef* clef = nullptr;
    ClefType concertClef;
    ClefType transposingClef;

    void flip() override;

public:
    ChangeClefType(Clef*, ClefType cl, ClefType tc);

    UNDO_TYPE(CommandType::ChangeClefType)
    UNDO_NAME("ChangeClef")
    UNDO_CHANGED_OBJECTS({ clef })
};

class ChordRest;
class EngravingItem;
class Score;
class Staff;
class Transaction;

class EditClef
{
public:
    static bool canInsertClef(const Score* score, ClefType type);
    static void insertClef(Transaction& tx, Score* score, ClefType type);
    static void insertClef(Transaction& tx, Score* score, Clef* clef, ChordRest* cr);
    static void undoChangeClef(Transaction& tx, Score* score, Staff* ostaff, EngravingItem* e, ClefType ct,
                               bool forInstrumentChange = false, Clef* clefToRelink = nullptr);
};
}
