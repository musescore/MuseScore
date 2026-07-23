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

#pragma once

#include <map>

#include "global/containers.h"
#include "global/types/translatablestring.h"

#include "../../dom/input.h"

namespace mu::engraving {
class EngravingObject;
class Score;
class Segment;

class EditData;

class UndoableCommand;
enum class UndoableCommandFilter : unsigned char;
enum class CommandType : signed char;

class UndoableTransaction
{
public:
    UndoableTransaction(Score* s, const muse::TranslatableString& actionName);
    ~UndoableTransaction();

    void undo();
    void redo();

    void appendCommand(UndoableCommand* cmd) { m_commands.push_back(cmd); }
    void append(UndoableTransaction&& other);

    void unwind(bool cleanUp = true);
    void cleanup(bool wasDone);

    const std::vector<UndoableCommand*>& commands() const { return m_commands; }
    bool empty() const { return m_commands.empty(); }

    bool hasCommandsMatchingFilter(UndoableCommandFilter, const EngravingItem* target) const;
    bool hasCommandsNotMatchingFilters(const std::vector<UndoableCommandFilter>& filters, const EngravingItem* target) const;
    void removeCommandsMatchingFilter(UndoableCommandFilter f, EngravingItem* target);

    const InputState& undoInputState() const;
    const InputState& redoInputState() const;

    struct SelectionInfo {
        std::vector<EngravingItem*> elements;
        Fraction tickStart;
        Fraction tickEnd;
        staff_idx_t staffStart = muse::nidx;
        staff_idx_t staffEnd = muse::nidx;

        bool isValid() const { return !elements.empty() || staffStart != muse::nidx; }
    };

    const SelectionInfo& undoSelectionInfo() const;
    const SelectionInfo& redoSelectionInfo() const;

    void excludeElementFromSelectionInfo(EngravingItem* element);
    static bool canRecordSelectedElement(const EngravingItem* e);

    struct ChangesInfo {
        ElementTypeSet changedObjectTypes;
        std::map<EngravingObject*, std::unordered_set<CommandType> > changedObjects;
        StyleIdSet changedStyleIdSet;
        PropertyIdSet changedPropertyIdSet;
        bool isTextEditing = false;
    };

    ChangesInfo changesInfo(bool undo = false) const;
    const muse::TranslatableString& actionName() const;

private:
    void appendCommands(UndoableTransaction& other);

    std::vector<UndoableCommand*> m_commands;

    InputState m_undoInputState;
    InputState m_redoInputState;
    SelectionInfo m_undoSelectionInfo;
    SelectionInfo m_redoSelectionInfo;
    muse::TranslatableString m_actionName;

    Score* m_score = nullptr;

    static void fillSelectionInfo(SelectionInfo&, const Selection&);
    static void applySelectionInfo(const SelectionInfo&, Selection&);
};

class UndoStack
{
public:
    UndoStack();
    ~UndoStack();

    bool isLocked() const;
    void setLocked(bool locked);

    bool hasActiveTransaction() const { return m_activeTransaction != nullptr; }

    void beginTransaction(Score*, const muse::TranslatableString& actionName);
    void endTransaction(bool rollback);

    bool canUndo() const { return m_currentIndex > 0; }
    bool canRedo() const { return m_currentIndex < m_transactions.size(); }
    bool isClean() const { return m_cleanState == m_states[m_currentIndex]; }

    size_t size() const { return m_transactions.size(); }
    size_t currentIndex() const { return m_currentIndex; }

    UndoableTransaction* activeTransaction() const { return m_activeTransaction; }

    UndoableTransaction* last() const { return m_currentIndex > 0 ? m_transactions[m_currentIndex - 1] : nullptr; }
    UndoableTransaction* prev() const { return m_currentIndex > 1 ? m_transactions[m_currentIndex - 2] : nullptr; }
    UndoableTransaction* next() const { return canRedo() ? m_transactions[m_currentIndex] : nullptr; }

    /// Returns the transaction that led to the state with the given `idx`.
    /// For further discussion of the indices involved in UndoStack, see:
    /// https://github.com/musescore/MuseScore/pull/25389#discussion_r1825782176
    UndoableTransaction* lastAtIndex(size_t idx) const
    {
        return idx > 0 && idx - 1 < m_transactions.size() ? m_transactions[idx - 1] : nullptr;
    }

    void undo(EditData*);
    void redo();
    void reopen();

    void mergeTransactions(size_t startIdx);
    void cleanRedoStack() { remove(m_currentIndex); }

private:
    void remove(size_t idx);

    UndoableTransaction* m_activeTransaction = nullptr;
    std::vector<UndoableTransaction*> m_transactions;
    std::vector<int> m_states;
    int m_nextState = 0;
    int m_cleanState = 0;
    size_t m_currentIndex = 0;
    bool m_isLocked = false;
};
}
