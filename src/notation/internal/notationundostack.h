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

#include "inotationundostack.h"
#include "igetscore.h"

namespace mu::engraving {
class Score;
class MasterScore;
class UndoStack;
class EditData;
}

namespace mu::notation {
class NotationUndoStack : public INotationUndoStack
{
public:
    NotationUndoStack(IGetScore* getScore, muse::async::Notification notationChanged);

    bool canUndo() const override;
    void undo(mu::engraving::EditData*) override;

    bool canRedo() const override;
    void redo(mu::engraving::EditData*) override;

    void undoRedoToIndex(size_t idx, mu::engraving::EditData* editData) override;

    void prepareChanges(const muse::TranslatableString& actionName) override;
    void rollbackChanges() override;
    void commitChanges() override;

    bool isStackClean() const override;

    void mergeCommands(size_t startIdx) override;

    void lock() override;
    void unlock() override;
    bool isLocked() const override;

    const muse::TranslatableString topMostUndoActionName() const override;
    const muse::TranslatableString topMostRedoActionName() const override;
    size_t undoRedoActionCount() const override;
    size_t currentStateIndex() const override;
    const muse::TranslatableString lastActionNameAtIdx(size_t idx) const override;

    muse::async::Notification stackChanged() const override;
    muse::async::Channel<ChangesRange> changesChannel() const override;
    muse::async::Notification undoRedoNotification() const override;

private:
    void notifyAboutNotationChanged();
    void notifyAboutStateChanged();
    void notifyAboutUndoRedo();

    mu::engraving::Score* score() const;
    mu::engraving::MasterScore* masterScore() const;
    mu::engraving::UndoStack* undoStack() const;

    IGetScore* m_getScore = nullptr;

    muse::async::Notification m_notationChanged;
    muse::async::Notification m_stackStateChanged;
    muse::async::Notification m_undoRedoNotification;
};
}
