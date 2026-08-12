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

/**
 \file
 Implementation of undo functions.

 The undo system requires calling startUndo() when starting a GUI command
 and calling endUndo() when ending the command. All changes to a score
 in response to a GUI command must be undoable/redoable by executing
 a sequence of low-level undo operations. This sequence is built by the code
 handling the command, by calling one or more undoOp()'s
 between startUndo() and endUndo().
*/

#include "undostack.h"

#include "containers.h"

#include "editing/editproperty.h"
#include "editing/editstyle.h"
#include "editing/textedit.h"
#include "editing/transaction/undoablecommand.h"

#include "log.h"

#define LOG_UNDO() if constexpr (false) LOGD()

using namespace mu::engraving;

namespace mu::engraving {
extern Measure* tick2measure(int tick);

static const std::unordered_map<CommandType, CommandType> COMMAND_TYPE_INVERSION {
    { CommandType::AddElement, CommandType::RemoveElement },
    { CommandType::RemoveElement, CommandType::AddElement },

    { CommandType::AddBracket, CommandType::RemoveBracket },
    { CommandType::RemoveBracket, CommandType::AddBracket },

    { CommandType::AddExcerpt, CommandType::RemoveExcerpt },
    { CommandType::RemoveExcerpt, CommandType::AddExcerpt },

    { CommandType::AddSystemObjectStaff, CommandType::RemoveSystemObjectStaff },
    { CommandType::RemoveSystemObjectStaff, CommandType::AddSystemObjectStaff },

    { CommandType::InsertMeasures, CommandType::RemoveMeasures },
    { CommandType::RemoveMeasures, CommandType::InsertMeasures },

    { CommandType::InsertStaff, CommandType::RemoveStaff },
    { CommandType::RemoveStaff, CommandType::InsertStaff },

    { CommandType::InsertPart, CommandType::RemovePart },
    { CommandType::RemovePart, CommandType::InsertPart },

    { CommandType::Link, CommandType::Unlink },
    { CommandType::Unlink, CommandType::Link },

    { CommandType::AddNoteParenthesesInfo, CommandType::RemoveNoteParenthesesInfo },
    { CommandType::RemoveNoteParenthesesInfo, CommandType::AddNoteParenthesesInfo },

    { CommandType::ConnectSharedPart, CommandType::DisconnectSharedPart },
    { CommandType::DisconnectSharedPart, CommandType::ConnectSharedPart },
};

//---------------------------------------------------------
//   UndoStack
//---------------------------------------------------------

UndoStack::UndoStack()
{
    m_activeTransaction = nullptr;
    m_currentIndex = 0;
    m_cleanState = 0;
    m_states.push_back(m_cleanState);
    m_nextState = 1;
}

UndoStack::~UndoStack()
{
    size_t idx = 0;
    for (auto c : m_transactions) {
        c->cleanup(idx++ < m_currentIndex);
    }
    muse::DeleteAll(m_transactions);
}

bool UndoStack::isLocked() const
{
    return m_isLocked;
}

void UndoStack::setLocked(bool locked)
{
    m_isLocked = locked;
}

void UndoStack::beginTransaction(Score* score, const TranslatableString& actionName)
{
    if (m_isLocked) {
        return;
    }

    IF_ASSERT_FAILED(!m_activeTransaction) {
        LOGE() << "Transaction already in progress";
        return;
    }

    m_activeTransaction = new UndoableTransaction(score, actionName);
}

void UndoStack::remove(size_t idx)
{
    assert(idx <= m_currentIndex);
    assert(m_currentIndex != muse::nidx);
    // remove redo stack
    while (m_transactions.size() > m_currentIndex) {
        UndoableTransaction* transaction = muse::takeLast(m_transactions);
        m_states.pop_back();
        transaction->cleanup(false); // delete elements for which UndoableCommand() holds ownership
        delete transaction;
//            --curIdx;
    }
    while (m_transactions.size() > idx) {
        UndoableTransaction* transaction = muse::takeLast(m_transactions);
        m_states.pop_back();
        transaction->cleanup(true);
        delete transaction;
    }
    m_currentIndex = idx;
}

void UndoStack::mergeTransactions(size_t startIdx)
{
    assert(startIdx <= m_currentIndex);

    if (startIdx >= m_transactions.size()) {
        return;
    }

    UndoableTransaction* startTransaction = m_transactions[startIdx];

    for (size_t idx = startIdx + 1; idx < m_currentIndex; ++idx) {
        startTransaction->append(std::move(*m_transactions[idx]));
    }
    remove(startIdx + 1);   // TODO: remove from startIdx to curIdx only
}

void UndoStack::endTransaction(bool rollback)
{
    if (m_isLocked) {
        return;
    }

    IF_ASSERT_FAILED(m_activeTransaction) {
        LOGE() << "No transaction in progress";
        return;
    }

    if (rollback) {
        delete m_activeTransaction;
    } else {
        // remove redo stack
        while (m_transactions.size() > m_currentIndex) {
            UndoableTransaction* transaction = muse::takeLast(m_transactions);
            m_states.pop_back();
            transaction->cleanup(false); // delete elements for which UndoableCommand() holds ownership
            delete transaction;
        }
        m_transactions.push_back(m_activeTransaction);
        m_states.push_back(m_nextState++);
        ++m_currentIndex;
    }
    m_activeTransaction = nullptr;
}

void UndoStack::reopen()
{
    if (m_isLocked) {
        return;
    }

    LOG_UNDO() << "curIdx: " << m_currentIndex << ", size: " << m_transactions.size();
    assert(m_activeTransaction == nullptr);
    assert(m_currentIndex > 0);
    --m_currentIndex;
    m_activeTransaction = muse::takeAt(m_transactions, m_currentIndex);
    m_states.erase(m_states.begin() + m_currentIndex + 1);
    for (auto i : m_activeTransaction->commands()) {
        LOG_UNDO() << "   " << i->name();
    }
}

void UndoStack::undo(EditData* ed)
{
    LOG_UNDO() << "called";
    // Are we currently editing text?
    if (ed && ed->element && ed->element->isTextBase()) {
        TextEditData* ted = dynamic_cast<TextEditData*>(ed->getData(ed->element).get());
        if (ted && ted->startUndoIdx == m_currentIndex) {
            // No edits to undo, so do nothing
            return;
        }
    }
    if (m_currentIndex) {
        --m_currentIndex;
        assert(m_currentIndex < m_transactions.size());
        m_transactions[m_currentIndex]->undo();
    }
}

void UndoStack::redo()
{
    LOG_UNDO() << "called";
    if (canRedo()) {
        m_transactions[m_currentIndex++]->redo();
    }
}

//---------------------------------------------------------
//   UndoableTransaction
//---------------------------------------------------------

UndoableTransaction::UndoableTransaction(Score* s, const TranslatableString& actionName)
    : m_undoInputState(s->inputState()), m_actionName(actionName), m_score(s)
{
    fillSelectionInfo(m_undoSelectionInfo, s->selection());
}

UndoableTransaction::~UndoableTransaction()
{
    for (UndoableCommand* command : m_commands) {
        delete command;
    }
}

void UndoableTransaction::cleanup(bool wasDone)
{
    for (UndoableCommand* command : m_commands) {
        command->cleanup(wasDone);
    }
}

void UndoableTransaction::unwind(bool cleanUp)
{
    while (!m_commands.empty()) {
        UndoableCommand* command = muse::takeLast(m_commands);
        LOG_UNDO() << "unwind: " << command->name();
        command->undo();
        if (cleanUp) {
            command->cleanup(false);
        }
        delete command;
    }
}

void UndoableTransaction::appendCommands(UndoableTransaction& other)
{
    m_commands.insert(m_commands.end(), other.m_commands.cbegin(), other.m_commands.cend());
    other.m_commands.clear();
}

bool UndoableTransaction::hasCommandsMatchingFilter(UndoableCommandFilter f, const EngravingItem* target) const
{
    for (UndoableCommand* command : m_commands) {
        if (command->matchesFilter(f, target)) {
            return true;
        }
    }
    return false;
}

bool UndoableTransaction::hasCommandsNotMatchingFilters(const std::vector<UndoableCommandFilter>& filters,
                                                        const EngravingItem* target) const
{
    for (UndoableCommand* command : m_commands) {
        bool filtered = false;
        for (UndoableCommandFilter f : filters) {
            if (command->matchesFilter(f, target)) {
                filtered = true;
                break;
            }
        }
        if (!filtered) {
            return true;
        }
    }
    return false;
}

void UndoableTransaction::removeCommandsMatchingFilter(UndoableCommandFilter f, EngravingItem* target)
{
    std::vector<UndoableCommand*> acceptedList;
    for (UndoableCommand* command : m_commands) {
        if (command->matchesFilter(f, target)) {
            delete command;
        } else {
            acceptedList.push_back(command);
        }
    }
    m_commands = std::move(acceptedList);
}

bool UndoableTransaction::canRecordSelectedElement(const EngravingItem* e)
{
    if (e->generated()) {
        return false;
    }

    return e->isNote() || (e->isChordRest() && !e->isChord())
           || (e->isTextBase() && !e->isInstrumentName() && !e->isHammerOnPullOffText())
           || e->isFretDiagram() || e->isSoundFlag();
}

void UndoableTransaction::fillSelectionInfo(SelectionInfo& info, const Selection& sel)
{
    info.staffStart = info.staffEnd = muse::nidx;
    info.elements.clear();

    if (sel.isList()) {
        for (EngravingItem* e : sel.elements()) {
            if (canRecordSelectedElement(e)) {
                info.elements.push_back(e);
            } else {
                // don't remember selection we are unable to restore
                info.elements.clear();
                return;
            }
        }
    } else if (sel.isRange()) {
        info.staffStart = sel.staffStart();
        info.staffEnd = sel.staffEnd();
        info.tickStart = sel.tickStart();
        info.tickEnd = sel.tickEnd();
    }
}

void UndoableTransaction::applySelectionInfo(const SelectionInfo& info, Selection& sel)
{
    if (!info.elements.empty()) {
        for (EngravingItem* e : info.elements) {
            sel.add(e);
        }
    } else if (info.staffStart != muse::nidx) {
        sel.setRangeTicks(info.tickStart, info.tickEnd, info.staffStart, info.staffEnd);
    }
}

void UndoableTransaction::undo()
{
    m_redoInputState = m_score->inputState();
    fillSelectionInfo(m_redoSelectionInfo, m_score->selection());
    m_score->deselectAll();

    for (auto it = m_commands.rbegin(); it != m_commands.rend(); ++it) {
        LOG_UNDO() << "<" << (*it)->name() << ">";
        (*it)->undo();
    }

    m_score->setInputState(m_undoInputState);
    if (m_undoSelectionInfo.isValid()) {
        m_score->deselectAll();
        applySelectionInfo(m_undoSelectionInfo, m_score->selection());
    }
}

void UndoableTransaction::redo()
{
    m_undoInputState = m_score->inputState();
    fillSelectionInfo(m_undoSelectionInfo, m_score->selection());
    m_score->deselectAll();

    for (UndoableCommand* command : m_commands) {
        LOG_UNDO() << "<" << command->name() << ">";
        command->redo();
    }

    m_score->setInputState(m_redoInputState);
    if (m_redoSelectionInfo.isValid()) {
        m_score->deselectAll();
        applySelectionInfo(m_redoSelectionInfo, m_score->selection());
    }
}

void UndoableTransaction::append(UndoableTransaction&& other)
{
    appendCommands(other);
    if (m_score == other.m_score) {
        m_redoInputState = std::move(other.m_redoInputState);
        m_redoSelectionInfo = std::move(other.m_redoSelectionInfo);
    }
}

const InputState& UndoableTransaction::undoInputState() const
{
    return m_undoInputState;
}

const InputState& UndoableTransaction::redoInputState() const
{
    return m_redoInputState;
}

void UndoableTransaction::excludeElementFromSelectionInfo(EngravingItem* element)
{
    if (m_undoSelectionInfo.isValid()) {
        muse::remove(m_undoSelectionInfo.elements, element);
    }

    if (m_redoSelectionInfo.isValid()) {
        muse::remove(m_redoSelectionInfo.elements, element);
    }
}

const UndoableTransaction::SelectionInfo& UndoableTransaction::undoSelectionInfo() const
{
    return m_undoSelectionInfo;
}

const UndoableTransaction::SelectionInfo& UndoableTransaction::redoSelectionInfo() const
{
    return m_redoSelectionInfo;
}

UndoableTransaction::ChangesInfo UndoableTransaction::changesInfo(bool undo) const
{
    ChangesInfo result;

    for (const UndoableCommand* command : commands()) {
        CommandType type = command->type();

        if (type == CommandType::ChangeProperty) {
            auto changeProperty = static_cast<const ChangeProperty*>(command);
            result.changedPropertyIdSet.insert(changeProperty->getId());
        } else if (type == CommandType::ChangeStyleValues) {
            auto changeStyle = static_cast<const ChangeStyleValues*>(command);
            for (const auto& pair : changeStyle->values()) {
                result.changedStyleIdSet.insert(pair.first);
            }
        } else if (type == CommandType::ChangeStyle) {
            auto changeStyle = static_cast<const ChangeStyle*>(command);
            const StyleIdSet styleIds = changeStyle->changedIds();
            result.changedStyleIdSet.insert(styleIds.cbegin(), styleIds.cend());
        } else if (type == CommandType::TextEdit) {
            result.isTextEditing |= static_cast<const TextEditUndoableCommand*>(command)->cursor().editing();
        }

        if (undo) {
            auto it = COMMAND_TYPE_INVERSION.find(type);
            if (it != COMMAND_TYPE_INVERSION.end()) {
                type = it->second;
            }
        }

        for (EngravingObject* object : command->objectItems()) {
            if (!object) {
                continue;
            }

            result.changedObjectTypes.insert(object->type());
            result.changedObjects[object].insert(type);
        }
    }

    return result;
}

const TranslatableString& UndoableTransaction::actionName() const
{
    return m_actionName;
}
}
