/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#include "engraving/editing/transaction/transaction.h"
#include "engraving/editing/transaction/undostack.h"

using namespace mu::notation;
using namespace muse::async;

NotationUndoStack::NotationUndoStack(IGetScore* getScore, Channel<muse::RectF> notationChanged)
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

    transactionManager()->undoRedo(true, editData);

    notifyAboutNotationChanged();
    notifyAboutUndoRedo();
    notifyAboutStateChanged();
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

    transactionManager()->undoRedo(false, editData);

    notifyAboutNotationChanged();
    notifyAboutUndoRedo();
    notifyAboutStateChanged();
}

void NotationUndoStack::undoRedoToIndex(size_t idx, mu::engraving::EditData* editData)
{
    auto stack = undoStack();

    IF_ASSERT_FAILED(stack) {
        return;
    }

    if (stack->currentIndex() == idx) {
        return;
    }

    while (stack->currentIndex() > idx && stack->canUndo()) {
        transactionManager()->undoRedo(true, editData);
    }
    while (stack->currentIndex() < idx && stack->canRedo()) {
        transactionManager()->undoRedo(false, editData);
    }

    notifyAboutNotationChanged();
    notifyAboutUndoRedo();
    notifyAboutStateChanged();
}

void NotationUndoStack::transaction(const muse::TranslatableString& actionName, std::function<void(mu::engraving::Transaction&)> func)
{
    IF_ASSERT_FAILED(score()) {
        return;
    }

    transactionManager()->transaction(actionName, func);

    notifyAboutStateChanged();
}

void NotationUndoStack::prepareChanges(const muse::TranslatableString& actionName)
{
    IF_ASSERT_FAILED(score()) {
        return;
    }

    if (isLocked()) {
        return;
    }

    transactionManager()->beginTransaction(actionName);
}

void NotationUndoStack::rollbackChanges()
{
    IF_ASSERT_FAILED(score()) {
        return;
    }

    if (isLocked()) {
        return;
    }

    transactionManager()->endTransaction(true);
}

void NotationUndoStack::commitChanges()
{
    IF_ASSERT_FAILED(score()) {
        return;
    }

    if (isLocked()) {
        return;
    }

    transactionManager()->endTransaction(false);

    notifyAboutStateChanged();
}

bool NotationUndoStack::isStackClean() const
{
    IF_ASSERT_FAILED(undoStack()) {
        return false;
    }

    return undoStack()->isClean();
}

void NotationUndoStack::mergeTransactions(size_t startIdx)
{
    IF_ASSERT_FAILED(undoStack()) {
        return;
    }

    undoStack()->mergeTransactions(startIdx);
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
    return undoStack()->isLocked();
}

const muse::TranslatableString NotationUndoStack::topMostUndoActionName() const
{
    IF_ASSERT_FAILED(undoStack()) {
        return {};
    }

    if (auto action = undoStack()->last()) {
        return action->actionName();
    }

    return {};
}

const muse::TranslatableString NotationUndoStack::topMostRedoActionName() const
{
    IF_ASSERT_FAILED(undoStack()) {
        return {};
    }

    if (auto action = undoStack()->next()) {
        return action->actionName();
    }

    return {};
}

size_t NotationUndoStack::undoRedoActionCount() const
{
    IF_ASSERT_FAILED(undoStack()) {
        return 0;
    }

    return undoStack()->size();
}

size_t NotationUndoStack::currentStateIndex() const
{
    IF_ASSERT_FAILED(undoStack()) {
        return muse::nidx;
    }

    return undoStack()->currentIndex();
}

const muse::TranslatableString NotationUndoStack::lastActionNameAtIdx(size_t idx) const
{
    IF_ASSERT_FAILED(undoStack()) {
        return {};
    }

    if (auto action = undoStack()->lastAtIndex(idx)) {
        return action->actionName();
    }

    return {};
}

muse::async::Notification NotationUndoStack::stackChanged() const
{
    return m_stackStateChanged;
}

muse::async::Channel<mu::engraving::ScoreChanges> NotationUndoStack::changesChannel() const
{
    IF_ASSERT_FAILED(score()) {
        return muse::async::Channel<engraving::ScoreChanges>();
    }

    return score()->changesChannel();
}

Notification NotationUndoStack::undoRedoNotification() const
{
    return m_undoRedoNotification;
}

mu::engraving::Score* NotationUndoStack::score() const
{
    return m_getScore->score();
}

mu::engraving::MasterScore* NotationUndoStack::masterScore() const
{
    return score() ? score()->masterScore() : nullptr;
}

mu::engraving::TransactionManager* NotationUndoStack::transactionManager() const
{
    return score() ? score()->masterScore()->transactionManager() : nullptr;
}

mu::engraving::UndoStack* NotationUndoStack::undoStack() const
{
    return score() ? score()->masterScore()->undoStack() : nullptr;
}

void NotationUndoStack::notifyAboutNotationChanged()
{
    m_notationChanged.send(muse::RectF());
}

void NotationUndoStack::notifyAboutStateChanged()
{
    m_stackStateChanged.notify();
}

void NotationUndoStack::notifyAboutUndoRedo()
{
    m_undoRedoNotification.notify();
}
