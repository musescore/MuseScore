/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "transaction.h"

#include "../../dom/masterscore.h"

#include "undoablecommand.h"
#include "undostack.h"

#include "log.h"

using namespace mu::engraving;

static ScoreChanges buildScoreChanges(const CmdState& cmdState, const UndoableTransaction::ChangesInfo& changes,
                                      const std::vector<UndoableCommand*>& commands)
{
    int startTick = cmdState.startTick().ticks();
    int endTick = cmdState.endTick().ticks();
    staff_idx_t startStaff = cmdState.startStaff();
    staff_idx_t endStaff = cmdState.endStaff();

    for (const UndoableCommand* cmd : commands) {
        std::optional<ChangedRange> range = cmd->changedRange();
        if (!range) {
            continue;
        }

        int tickFrom = range->tickFrom.ticks();
        int tickTo = range->tickTo.ticks();

        if (startTick == -1 || startTick > tickFrom) {
            startTick = tickFrom;
        }
        if (endTick == -1 || endTick < tickTo) {
            endTick = tickTo;
        }

        if (range->staffIdxFrom != muse::nidx && (startStaff == muse::nidx || startStaff > range->staffIdxFrom)) {
            startStaff = range->staffIdxFrom;
        }
        if (range->staffIdxTo != muse::nidx && (endStaff == muse::nidx || endStaff < range->staffIdxTo)) {
            endStaff = range->staffIdxTo;
        }
    }

    for (const auto& pair : changes.changedObjects) {
        if (!pair.first->isEngravingItem()) {
            continue;
        }

        int tick = toEngravingItem(pair.first)->tick().ticks();

        if (startTick > tick) {
            startTick = tick;
        }

        if (endTick < tick) {
            endTick = tick;
        }
    }

    return { startTick, endTick,
             startStaff, endStaff,
             changes.isTextEditing,
             std::move(changes.changedObjects),
             std::move(changes.changedObjectTypes),
             std::move(changes.changedPropertyIdSet),
             std::move(changes.changedStyleIdSet) };
}

//---------------------------------------------------------
// Transaction
//---------------------------------------------------------

Transaction::Transaction(UndoableTransaction* undoableTransaction)
    : m_undoableTransaction(undoableTransaction)
{
}

Transaction& Transaction::dummy()
{
    static Transaction dummyTransaction(nullptr);
    dummyTransaction.m_isDummy = true;
    return dummyTransaction;
}

void Transaction::push(UndoableCommand* cmd)
{
    if (m_isDummy) {
        cmd->redo();
        delete cmd;
        return;
    }

    // NOTE: append the command _before_ performing it. Some commands' redo()
    // may themselves push further commands; those must end up _after_ this
    // command in the transaction, so that undo reverses them first.
    m_undoableTransaction->appendCommand(cmd);
    cmd->redo();
}

void Transaction::pushWithoutPerforming(UndoableCommand* cmd)
{
    if (m_isDummy) {
        delete cmd;
        return;
    }

    m_undoableTransaction->appendCommand(cmd);
}

//---------------------------------------------------------
// TransactionManager
//---------------------------------------------------------

TransactionManager::TransactionManager(MasterScore* masterScore)
    : m_masterScore(masterScore)
{
}

UndoStack* TransactionManager::undoStack() const
{
    return m_masterScore->undoStack();
}

void TransactionManager::transaction(const muse::TranslatableString& description, std::function<void(Transaction&)> func)
{
    IF_ASSERT_FAILED(!m_currentTransaction) {
        LOGW() << "transaction already active";
        func(*m_currentTransaction);
        return;
    }
    beginTransaction(description);
    func(*m_currentTransaction);
    endTransaction();
}

void TransactionManager::beginTransaction(const muse::TranslatableString& description)
{
    UndoStack* stack = undoStack();

    if (stack->isLocked()) {
        return;
    }

    if (stack->hasActiveTransaction()) {
        LOGD() << "cmd already active";
        return;
    }

    MScore::setError(MsError::MS_NO_ERROR);

    m_masterScore->cmdState().reset();
    stack->beginTransaction(m_masterScore, description);
    m_currentTransaction = std::unique_ptr<Transaction>(new Transaction(stack->activeTransaction()));
}

void TransactionManager::endTransaction(bool rollback, bool layoutAllParts, bool keepRolledBackElements)
{
    UndoStack* stack = undoStack();

    if (stack->isLocked()) {
        return;
    }

    if (!stack->hasActiveTransaction()) {
        LOGW() << "no command active";
        m_masterScore->update();
        return;
    }

    if (m_masterScore->readOnly() || (MScore::_error != MsError::MS_NO_ERROR && !MScore::_errorIsWarning)) {
        rollback = true;
    }

    if (rollback) {
        /* When keepRolledBackElements is true, unwind removes the rolled-back elements from
         * the score but does not free them: an external owner is responsible for the deferred
         * deletion, since the elements are still referenced after this returns. */
        stack->activeTransaction()->unwind(!keepRolledBackElements);
    }

    // NOTE: perform update and re-layout within the transaction, so that
    // undoable commands generated by layout are recorded correctly.
    m_masterScore->update(false, layoutAllParts);

    ScoreChanges changes;
    if (!rollback) {
        changes = buildScoreChanges(m_masterScore->cmdState(), stack->activeTransaction()->changesInfo(),
                                    stack->activeTransaction()->commands());
    }

    LOGD() << "Undo stack current transaction commands count: " << stack->activeTransaction()->commands().size();

    const bool isCurrentTransactionEmpty = stack->activeTransaction()->empty(); // nothing to undo?
    stack->endTransaction(isCurrentTransactionEmpty);
    m_currentTransaction.reset();

    if (m_masterScore->dirty()) {
        m_masterScore->invalidateRepeatList(); // TODO: flag individual operations
    }

    m_masterScore->cmdState().reset();

    if (!isCurrentTransactionEmpty && !rollback) {
        m_masterScore->updateAutomation(changes);
        m_masterScore->changesChannel().send(changes);
    }
}

Transaction* TransactionManager::currentTransaction() const
{
    return m_currentTransaction.get();
}

Transaction& TransactionManager::currentOrDummyTransaction()
{
    if (m_currentTransaction) {
        return *m_currentTransaction;
    } else {
        return Transaction::dummy();
    }
}

void TransactionManager::undoRedo(bool undo, EditData* editData)
{
    if (m_masterScore->readOnly()) {
        return;
    }

    UndoStack* stack = undoStack();

    IF_ASSERT_FAILED(!stack->hasActiveTransaction()) {
        LOGW() << "cannot undo/redo while transaction is active";
        return;
    }

    if (undo) {
        IF_ASSERT_FAILED(stack->canUndo()) {
            LOGW() << "cannot undo";
            return;
        }
    } else {
        IF_ASSERT_FAILED(stack->canRedo()) {
            LOGW() << "cannot redo";
            return;
        }
    }

    m_masterScore->cmdState().reset();

    //! NOTE: the order of operations is very important here
    //! 1. for the undo operation, the list of changed elements is available before undo()
    //! 2. for the redo operation, the list of changed elements will be available after redo()
    UndoableTransaction::ChangesInfo changes;
    UndoableTransaction* transaction = nullptr;

    if (undo) {
        transaction = stack->last();
        changes = transaction->changesInfo(true);
        stack->undo(editData);
    } else {
        stack->redo();
        transaction = stack->last();
        changes = transaction->changesInfo(false);
    }

    m_masterScore->update(false);
    m_masterScore->invalidateRepeatList(); // TODO: flag individual operations
    m_masterScore->updateSelection();

    ScoreChanges result = buildScoreChanges(m_masterScore->cmdState(), changes, transaction->commands());
    m_masterScore->updateAutomation(result);
    m_masterScore->changesChannel().send(result);
}
