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

#include "notationundostack.h"

#include "log.h"

#include "libmscore/masterscore.h"
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

void NotationUndoStack::undo(Ms::EditData* editData)
{
    IF_ASSERT_FAILED(score()) {
        return;
    }

    score()->undoRedo(true, editData);
    masterScore()->setSaved(isStackClean());

    notifyAboutNotationChanged();
    notifyAboutUndo();
    notifyAboutStateChanged();
}

Notification NotationUndoStack::undoNotification() const
{
    return m_undoNotification;
}

bool NotationUndoStack::canRedo() const
{
    IF_ASSERT_FAILED(undoStack()) {
        return false;
    }

    return undoStack()->canRedo();
}

void NotationUndoStack::redo(Ms::EditData* editData)
{
    IF_ASSERT_FAILED(score()) {
        return;
    }

    score()->undoRedo(false, editData);
    masterScore()->setSaved(isStackClean());

    notifyAboutNotationChanged();
    notifyAboutRedo();
    notifyAboutStateChanged();
}

Notification NotationUndoStack::redoNotification() const
{
    return m_redoNotification;
}

void NotationUndoStack::prepareChanges()
{
    IF_ASSERT_FAILED(score()) {
        return;
    }

    if (m_isLocked) {
        return;
    }

    score()->startCmd();
}

void NotationUndoStack::rollbackChanges()
{
    IF_ASSERT_FAILED(score()) {
        return;
    }

    if (m_isLocked) {
        return;
    }

    score()->endCmd(true);
    masterScore()->setSaved(isStackClean());
}

void NotationUndoStack::commitChanges()
{
    IF_ASSERT_FAILED(m_getScore && m_getScore->score()) {
        return;
    }

    if (m_isLocked) {
        return;
    }

    score()->endCmd();
    masterScore()->setSaved(isStackClean());

    notifyAboutStateChanged();
}

void NotationUndoStack::lock()
{
    m_isLocked = true;
}

void NotationUndoStack::unlock()
{
    m_isLocked = false;
}

bool NotationUndoStack::isLocked() const
{
    return m_isLocked;
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

void NotationUndoStack::notifyAboutStateChanged()
{
    IF_ASSERT_FAILED(undoStack()) {
        return;
    }

    m_stackStateChanged.notify();
}

void NotationUndoStack::notifyAboutUndo()
{
    m_undoNotification.notify();
}

void NotationUndoStack::notifyAboutRedo()
{
    m_redoNotification.notify();
}

bool NotationUndoStack::isStackClean() const
{
    IF_ASSERT_FAILED(undoStack()) {
        return false;
    }

    return undoStack()->isClean();
}
