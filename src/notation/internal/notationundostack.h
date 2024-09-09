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

#ifndef MU_NOTATION_UNDOSTACK
#define MU_NOTATION_UNDOSTACK

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
    muse::async::Notification undoNotification() const override;

    bool canRedo() const override;
    void redo(mu::engraving::EditData*) override;
    muse::async::Notification redoNotification() const override;

    void prepareChanges(const muse::TranslatableString& actionName) override;
    void rollbackChanges() override;
    void commitChanges() override;

    bool isStackClean() const override;

    void lock() override;
    void unlock() override;
    bool isLocked() const override;

    muse::TranslatableString topMostUndoActionName() const override;
    muse::TranslatableString topMostRedoActionName() const override;

    muse::async::Notification stackChanged() const override;
    muse::async::Channel<ChangesRange> changesChannel() const override;

private:
    void notifyAboutNotationChanged();
    void notifyAboutStateChanged();
    void notifyAboutUndo();
    void notifyAboutRedo();

    mu::engraving::Score* score() const;
    mu::engraving::MasterScore* masterScore() const;
    mu::engraving::UndoStack* undoStack() const;

    IGetScore* m_getScore = nullptr;

    muse::async::Notification m_notationChanged;
    muse::async::Notification m_stackStateChanged;
    muse::async::Notification m_undoNotification;
    muse::async::Notification m_redoNotification;
};
}

#endif // MU_NOTATION_UNDOSTACK
