/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include <gmock/gmock.h>

#include "engraving/dom/score.h"

#include "notation/inotationundostack.h"

namespace mu::notation {
class NotationUndoStackMock : public INotationUndoStack
{
public:
    MOCK_METHOD(bool, canUndo, (), (const, override));
    MOCK_METHOD(void, undo, (mu::engraving::EditData*), (override));

    MOCK_METHOD(bool, canRedo, (), (const, override));
    MOCK_METHOD(void, redo, (mu::engraving::EditData*), (override));

    MOCK_METHOD(void, undoRedoToIndex, (size_t, mu::engraving::EditData*), (override));

    MOCK_METHOD(void, transaction, (const muse::TranslatableString&, std::function<void(mu::engraving::Transaction&)>), (override));

    MOCK_METHOD(void, prepareChanges, (const muse::TranslatableString&), (override));
    MOCK_METHOD(void, rollbackChanges, (), (override));
    MOCK_METHOD(void, commitChanges, (), (override));

    MOCK_METHOD(void, mergeTransactions, (const size_t), (override));

    MOCK_METHOD(bool, isStackClean, (), (const, override));

    MOCK_METHOD(void, lock, (), (override));
    MOCK_METHOD(void, unlock, (), (override));
    MOCK_METHOD(bool, isLocked, (), (const, override));

    MOCK_METHOD(const muse::TranslatableString, topMostUndoActionName, (), (const, override));
    MOCK_METHOD(const muse::TranslatableString, topMostRedoActionName, (), (const, override));
    MOCK_METHOD(size_t, undoRedoActionCount, (), (const, override));
    MOCK_METHOD(size_t, currentStateIndex, (), (const, override));
    MOCK_METHOD(const muse::TranslatableString, lastActionNameAtIdx, (size_t), (const, override));

    MOCK_METHOD(muse::async::Notification, stackChanged, (), (const, override));
    MOCK_METHOD(muse::async::Channel<engraving::ScoreChanges>, changesChannel, (), (const, override));
    MOCK_METHOD(muse::async::Notification, undoRedoNotification, (), (const, override));
};
}
