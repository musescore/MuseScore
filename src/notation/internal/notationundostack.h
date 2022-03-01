/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

namespace Ms {
class Score;
class MasterScore;
class UndoStack;
class EditData;
}

namespace mu::notation {
class NotationUndoStack : public INotationUndoStack
{
public:
    NotationUndoStack(IGetScore* getScore, async::Notification notationChanged);

    bool canUndo() const override;
    void undo(Ms::EditData*) override;
    async::Notification undoNotification() const override;

    bool canRedo() const override;
    void redo(Ms::EditData*) override;
    async::Notification redoNotification() const override;

    void prepareChanges() override;
    void rollbackChanges() override;
    void commitChanges() override;

    void lock() override;
    void unlock() override;
    bool isLocked() const override;

    async::Notification stackChanged() const override;
    async::Channel<int /*tickFrom*/, int /*tickTo*/,
                   int /*staffIdxFrom*/, int /*staffIdxTo*/> notationChangesRange() const override;

private:
    struct NotationChangesRange {
        int tickFrom = -1;
        int tickTo = -1;
        int staffIdxFrom = -1;
        int staffIdxTo = -1;

        bool isValid() const
        {
            return tickFrom != -1
                   && tickTo != -1
                   && staffIdxFrom != -1
                   && staffIdxTo != -1;
        }
    };

    void notifyAboutNotationChanged();
    void notifyAboutStateChanged(NotationChangesRange&& range);
    void notifyAboutUndo();
    void notifyAboutRedo();

    bool isStackClean() const;

    NotationChangesRange changesRange() const;
    Ms::Score* score() const;
    Ms::MasterScore* masterScore() const;
    Ms::UndoStack* undoStack() const;

    IGetScore* m_getScore = nullptr;

    async::Notification m_notationChanged;
    async::Notification m_stackStateChanged;
    async::Notification m_undoNotification;
    async::Notification m_redoNotification;
    async::Channel<int /*tickFrom*/, int /*tickTo*/, int /*staffIdxFrom*/, int /*staffIdxTo*/> m_notationChangesChannel;

    bool m_isLocked = false;
};
}

#endif // MU_NOTATION_UNDOSTACK
