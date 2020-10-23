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

NotationUndoStack::NotationUndoStack(IGetScore* getScore)
    : m_getScore(getScore)
{
}

bool NotationUndoStack::canUndo() const
{
    IF_ASSERT_FAILED(m_getScore && m_getScore->score()) {
        return false;
    }

    return m_getScore->score()->undoStack()->canUndo();
}

void NotationUndoStack::undo()
{
    IF_ASSERT_FAILED(m_getScore && m_getScore->score()) {
        return;
    }

    m_getScore->score()->undoRedo(true, nullptr);
    m_getScore->score()->masterScore()->setSaved(isStackClean());
    notifyAboutStackStateChanged();
}

bool NotationUndoStack::canRedo() const
{
    IF_ASSERT_FAILED(m_getScore && m_getScore->score()) {
        return false;
    }

    return m_getScore->score()->undoStack()->canRedo();
}

void NotationUndoStack::redo()
{
    IF_ASSERT_FAILED(m_getScore && m_getScore->score()) {
        return;
    }

    m_getScore->score()->undoRedo(false, nullptr);
    m_getScore->score()->masterScore()->setSaved(isStackClean());
    notifyAboutStackStateChanged();
}

void NotationUndoStack::prepareChanges()
{
    IF_ASSERT_FAILED(m_getScore && m_getScore->score()) {
        return;
    }

    m_getScore->score()->startCmd();
}

void NotationUndoStack::rollbackChanges()
{
    IF_ASSERT_FAILED(m_getScore && m_getScore->score()) {
        return;
    }

    m_getScore->score()->endCmd(false, true);
    m_getScore->score()->masterScore()->setSaved(isStackClean());
    notifyAboutStackStateChanged();
}

void NotationUndoStack::commitChanges()
{
    IF_ASSERT_FAILED(m_getScore && m_getScore->score()) {
        return;
    }

    m_getScore->score()->endCmd();
    m_getScore->score()->masterScore()->setSaved(isStackClean());
    notifyAboutStackStateChanged();
}

mu::async::Notification NotationUndoStack::stackChanged() const
{
    return m_stackStateChanged;
}

void NotationUndoStack::notifyAboutStackStateChanged()
{
    m_stackStateChanged.notify();
}

bool NotationUndoStack::isStackClean() const
{
    IF_ASSERT_FAILED(m_getScore && m_getScore->score()) {
        return true;
    }

    return m_getScore->score()->undoStack()->isClean();
}
