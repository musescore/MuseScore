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

#pragma once

#include <functional>
#include <memory>

#include "global/types/translatablestring.h"

namespace mu::engraving {
class EditData;
class MasterScore;
class UndoableCommand;
class UndoableTransaction;
class UndoStack;

/// # Transaction and TransactionManager
///
/// These classes are intended to provide an interface for safely
/// performing undoable transactions, without the possibility to forget
/// starting/ending a transaction or to start a nested transaction.
///
/// The aim is to eliminate Score::undo, so that all undoable commands
/// must be performed inside a transaction.
///
/// When the new method is adopted widely, it will directly or indirectly
/// help eliminate a lot of bad patterns currently seen around undoable
/// transactions:
/// * forgetting to start/end a transaction
/// * performing undoable commands outside a transaction
/// * not being able to tell whether a method is undoable or not
/// * calling an undoable method without being aware, leading to overhead
///   or unexpected references in the UndoStack
/// * complex unsafe direct UndoStack manipulation, for example in text
///   editing; by replacing ad-hoc manipulation with proper conceptual
///   solutions, we can introduce safety for more complex behaviour too
/// * there are too many sources of undo changes information, especially
///   inside the Notation module
///
/// Two temporary methods remain for compatibility purposes:
/// * Score::undo for performing an undoable command
/// * TransactionManager::currentOrDummyTransaction for calling methods
///   that do need a `Transaction&` in a context that does not have it yet
///
/// Concepts not yet (completely) covered:
/// * failing transactions, i.e. a replacement for
///   `endTransaction(rollback: true)`; ideally, this would ensure that
///   after failing a transaction, no further commands can be pushed to it
/// * "long-running" transactions, for example like in the style dialog or
///   during text editing or dragging; these will still have to use
///   begin/endTransaction. That's basically fine, but those methods can be
///   made more strict, for example beginTransaction could return some
///   object that must be given back to endTransaction, ensuring correctness
/// * the aforementioned complex unsafe direct UndoStack manipulations
/// * "dummy transactions" will have to be promoted from a temporary
///   compatibility solution into a real concept, for operations during
///   score reading or in situations where one does not care about the undo
///   stack at all
/// * we need to figure out how the plugins/extensions framework can fit
///   into safely, given that we must consider user plugins/extensions
///   unsafe.
///
/// Potential opportunities to explore in the future
/// * Allowing to change the description/actionName mid-transaction via the
///   Transaction type
/// * Moving CmdState etc. out of MasterScore into Transaction
/// * Moving changes channels into TransactionManager
/// * Moving UndoStack ownership to TransactionManager
/// * Nested UndoStacks for "long-running" operations, for example in the
///   case of text editing: this should eliminate the need for
///   UndoStack::mergeTransactions
///
/// Shortcomings
/// The new `transaction` method and `Transaction` type do help prevent
/// calling undoable methods outside an undoable transaction, but they do
/// not help prevent calling non-undoable methods at unexpected moments. For
/// that, we would need another type that can only be constructed by a few
/// trusted friend classes, and acts as a 'password' for calling
/// non-undoable methods. The question is how far we should go with this
/// though, because adding such thing to literally every simple setter
/// anywhere in the module is not feasible. We might reserve it for a few
/// ambiguous places only.

class Transaction
{
public:
    Transaction(const Transaction&) = delete;
    Transaction& operator=(const Transaction&) = delete;

    Transaction(Transaction&&) = default;
    Transaction& operator=(Transaction&&) = default;

    void push(UndoableCommand* cmd);
    void pushWithoutPerforming(UndoableCommand* cmd);

private:
    friend class TransactionManager;
    Transaction(UndoableTransaction* undoableTransaction);

    static Transaction& dummy();

    UndoableTransaction* m_undoableTransaction = nullptr;
    bool m_isDummy = false;
};

class TransactionManager
{
public:
    explicit TransactionManager(MasterScore* masterScore);

    UndoStack* undoStack() const;

    void transaction(const muse::TranslatableString& description, std::function<void(Transaction&)> func);

    void beginTransaction(const muse::TranslatableString& description);
    void endTransaction(bool rollback = false, bool layoutAllParts = false);

    /// Returns the current transaction, if any is active.
    /// Where possible, prefer using `transaction()` instead of this method.
    Transaction* currentTransaction() const;

    Transaction& currentOrDummyTransaction();

    void undoRedo(bool undo, EditData* editData);

private:
    MasterScore* m_masterScore = nullptr;

    std::unique_ptr<Transaction> m_currentTransaction;
};
}
