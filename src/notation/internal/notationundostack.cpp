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
    m_cleanOverride = std::nullopt;

    notifyAboutNotationChanged();
    notifyAboutUndo();
    notifyAboutStateChanged(changesRange());
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
    m_cleanOverride = std::nullopt;

    notifyAboutNotationChanged();
    notifyAboutRedo();
    notifyAboutStateChanged(changesRange());
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
    m_cleanOverride = std::nullopt;
}

void NotationUndoStack::commitChanges()
{
    IF_ASSERT_FAILED(m_getScore && m_getScore->score()) {
        return;
    }

    if (m_isLocked) {
        return;
    }

    NotationChangesRange range = changesRange();

    score()->endCmd();
    m_cleanOverride = std::nullopt;

    notifyAboutStateChanged(std::move(range));
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

bool NotationUndoStack::isStackClean() const
{
    if (m_cleanOverride.has_value()) {
        return m_cleanOverride.value();
    }

    IF_ASSERT_FAILED(undoStack()) {
        return false;
    }

    return undoStack()->isClean();
}

void NotationUndoStack::setCleanOverride(std::optional<bool> cleanOverride){
    if (m_cleanOverride == cleanOverride) {
        return;
    }

    m_cleanOverride = cleanOverride;
    m_stackStateChanged.notify();
}

mu::async::Notification NotationUndoStack::stackChanged() const
{
    return m_stackStateChanged;
}

Channel<int, int, int, int> NotationUndoStack::notationChangesRange() const
{
    return m_notationChangesChannel;
}

Ms::Score* NotationUndoStack::score() const
{
    return m_getScore->score();
}

Ms::UndoStack* NotationUndoStack::undoStack() const
{
    return score() ? score()->undoStack() : nullptr;
}

void NotationUndoStack::notifyAboutNotationChanged()
{
    m_notationChanged.notify();
}

void NotationUndoStack::notifyAboutStateChanged(NotationChangesRange&& range)
{
    IF_ASSERT_FAILED(undoStack()) {
        return;
    }

    m_stackStateChanged.notify();
    m_notationChangesChannel.send(range.tickFrom, range.tickTo, range.staffIdxFrom, range.staffIdxTo);
}

void NotationUndoStack::notifyAboutUndo()
{
    m_undoNotification.notify();
}

void NotationUndoStack::notifyAboutRedo()
{
    m_redoNotification.notify();
}

NotationUndoStack::NotationChangesRange NotationUndoStack::changesRange() const
{
    const Ms::CmdState& cmdState = score()->cmdState();
    return { cmdState.startTick().ticks(), cmdState.endTick().ticks(),
             cmdState.startStaff(), cmdState.endStaff() };
}
