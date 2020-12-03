//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FIT-0NESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef MU_NOTATION_UNDOSTACK
#define MU_NOTATION_UNDOSTACK

#include "inotationundostack.h"
#include "igetscore.h"

namespace Ms {
class Score;
class MasterScore;
class UndoStack;
}

namespace mu::notation {
class NotationUndoStack : public INotationUndoStack
{
public:
    NotationUndoStack(IGetScore* getScore, async::Notification notationChanged);

    bool canUndo() const override;
    void undo() override;

    bool canRedo() const override;
    void redo() override;

    void prepareChanges() override;
    void rollbackChanges() override;
    void commitChanges() override;

    async::Notification stackChanged() const override;

private:
    void notifyAboutNotationChanged();
    void notifyAboutStackStateChanged();

    bool isStackClean() const;

    Ms::Score* score() const;
    Ms::MasterScore* masterScore() const;
    Ms::UndoStack* undoStack() const;

    IGetScore* m_getScore = nullptr;

    async::Notification m_notationChanged;
    async::Notification m_stackStateChanged;
};
}

#endif // MU_NOTATION_UNDOSTACK
