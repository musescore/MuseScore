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

#include "engraving/dom/masterscore.h"
#include "engraving/dom/undo.h"

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

void NotationUndoStack::undo(mu::engraving::EditData* editData)
{
    IF_ASSERT_FAILED(score()) {
        return;
    }

    score()->undoRedo(true, editData);

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

void NotationUndoStack::redo(mu::engraving::EditData* editData)
{
    IF_ASSERT_FAILED(score()) {
        return;
    }

    score()->undoRedo(false, editData);

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

    if (isLocked()) {
        return;
    }

    score()->startCmd();
}

void NotationUndoStack::rollbackChanges()
{
    IF_ASSERT_FAILED(score()) {
        return;
    }

    if (isLocked()) {
        return;
    }

    score()->endCmd(true);
}

void NotationUndoStack::commitChanges()
{
    IF_ASSERT_FAILED(score()) {
        return;
    }

    if (isLocked()) {
        return;
    }

    score()->endCmd();

    notifyAboutStateChanged();
}

bool NotationUndoStack::isStackClean() const
{
    IF_ASSERT_FAILED(undoStack()) {
        return false;
    }

    return undoStack()->isClean();
}

void NotationUndoStack::lock()
{
    IF_ASSERT_FAILED(undoStack()) {
        return;
    }

    undoStack()->setLocked(true);
}

void NotationUndoStack::unlock()
{
    IF_ASSERT_FAILED(undoStack()) {
        return;
    }

    undoStack()->setLocked(false);
}

bool NotationUndoStack::isLocked() const
{
    return undoStack()->locked();
}

mu::async::Notification NotationUndoStack::stackChanged() const
{
    return m_stackStateChanged;
}

mu::async::Channel<ChangesRange> NotationUndoStack::changesChannel() const
{
    IF_ASSERT_FAILED(score()) {
        return async::Channel<ChangesRange>();
    }

    return score()->changesChannel();
}

mu::engraving::Score* NotationUndoStack::score() const
{
    return m_getScore->score();
}

mu::engraving::MasterScore* NotationUndoStack::masterScore() const
{
    return score() ? score()->masterScore() : nullptr;
}

mu::engraving::UndoStack* NotationUndoStack::undoStack() const
{
    return score() ? score()->undoStack() : nullptr;
}

void NotationUndoStack::notifyAboutNotationChanged()
{
    m_notationChanged.notify();
}

void NotationUndoStack::notifyAboutStateChanged()
{
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
