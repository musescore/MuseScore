/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "global/types/translatablestring.h"

namespace mu::engraving {
class EditData;
class MasterScore;
class UndoStack;

class TransactionManager
{
public:
    explicit TransactionManager(MasterScore* masterScore);

    UndoStack* undoStack() const;

    void beginTransaction(const muse::TranslatableString& actionName);
    void endTransaction(bool rollback = false, bool layoutAllParts = false);

    void undoRedo(bool undo, EditData* editData);

private:
    MasterScore* m_masterScore = nullptr;
};
}
