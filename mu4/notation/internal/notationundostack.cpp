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

#include "notationundostack.h"

#include "log.h"

#include "libmscore/score.h"
#include "libmscore/undo.h"

using namespace mu::notation;
using namespace mu::async;

NotationUndoStack::NotationUndoStack(IGetScore* getScore, Notification notationChanged)
    : m_getScore(getScore), m_notationChanged(notationChanged)
{
}

bool NotationUndoStack::canUndo() const
{
    IF_ASSERT_FAILED(undoStack()) {
        return false;
    }

    return undoStack()->canUndo();
}

void NotationUndoStack::undo()
{
    IF_ASSERT_FAILED(score()) {
        return;
    }

    score()->undoRedo(true, nullptr);
    masterScore()->setSaved(isStackClean());

    notifyAboutNotationChanged();
    notifyAboutStackStateChanged();
}

bool NotationUndoStack::canRedo() const
{
    IF_ASSERT_FAILED(undoStack()) {
        return false;
    }

    return undoStack()->canRedo();
}

void NotationUndoStack::redo()
{
    IF_ASSERT_FAILED(score()) {
        return;
    }

    score()->undoRedo(false, nullptr);
    masterScore()->setSaved(isStackClean());

    notifyAboutNotationChanged();
    notifyAboutStackStateChanged();
}

void NotationUndoStack::prepareChanges()
{
    IF_ASSERT_FAILED(score()) {
        return;
    }

    score()->startCmd();
}

void NotationUndoStack::rollbackChanges()
{
    IF_ASSERT_FAILED(score()) {
        return;
    }

    score()->endCmd(false, true);
    masterScore()->setSaved(isStackClean());

    notifyAboutStackStateChanged();
}

void NotationUndoStack::commitChanges()
{
    IF_ASSERT_FAILED(m_getScore && m_getScore->score()) {
        return;
    }

    score()->endCmd();
    masterScore()->setSaved(isStackClean());

    notifyAboutStackStateChanged();
}

mu::async::Notification NotationUndoStack::stackChanged() const
{
    return m_stackStateChanged;
}

Ms::Score* NotationUndoStack::score() const
{
    return m_getScore->score();
}

Ms::MasterScore* NotationUndoStack::masterScore() const
{
    return score() ? score()->masterScore() : nullptr;
}

Ms::UndoStack* NotationUndoStack::undoStack() const
{
    return score() ? score()->undoStack() : nullptr;
}

void NotationUndoStack::notifyAboutNotationChanged()
{
    m_notationChanged.notify();
}

void NotationUndoStack::notifyAboutStackStateChanged()
{
    m_stackStateChanged.notify();
}

bool NotationUndoStack::isStackClean() const
{
    IF_ASSERT_FAILED(undoStack()) {
        return false;
    }

    return undoStack()->isClean();
}
