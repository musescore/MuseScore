/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "undo.h"

#include "containers.h"

#include "editproperty.h"
#include "editstyle.h"
#include "textedit.h"

#include "../dom/fret.h"
#include "../dom/harmony.h"
#include "../dom/note.h"
#include "../dom/stafftextbase.h"

#include "log.h"
#define LOG_UNDO() if (0) LOGD()

using namespace mu;
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
};

std::vector<EngravingObject*> compoundObjects(EngravingObject* object)
{
    std::vector<EngravingObject*> objects;

    if (object->isChord()) {
        const Chord* chord = toChord(object);
        for (const Note* note : chord->notes()) {
            for (Note* compoundNote : note->compoundNotes()) {
                objects.push_back(compoundNote);
            }
        }
    } else if (object->isNote()) {
        const Note* note = toNote(object);
        for (Note* compoundNote : note->compoundNotes()) {
            objects.push_back(compoundNote);
        }
    } else if (object->isFretDiagram()) {
        const FretDiagram* fret = toFretDiagram(object);
        if (fret->harmony()) {
            objects.push_back(fret->harmony());
        }
    }

    objects.push_back(object);

    return objects;
}

void updateStaffTextCache(const StaffTextBase* text, Score* score)
{
    TRACEFUNC;

    if (text->isCapo()) {
        score->updateCapo();
    } else if (text->swing()) {
        score->updateSwing();
    }
}

//---------------------------------------------------------
//   UndoCommand
//---------------------------------------------------------

UndoCommand::~UndoCommand()
{
    for (auto c : m_childCommands) {
        delete c;
    }
}

//---------------------------------------------------------
//   UndoCommand::cleanup
//---------------------------------------------------------

void UndoCommand::cleanup(bool undo)
{
    for (auto c : m_childCommands) {
        c->cleanup(undo);
    }
}

//---------------------------------------------------------
//   undo
//---------------------------------------------------------

void UndoCommand::undo(EditData* ed)
{
    for (auto it = m_childCommands.rbegin(); it != m_childCommands.rend(); ++it) {
        LOG_UNDO() << "<" << (*it)->name() << ">";
        (*it)->undo(ed);
    }
    flip(ed);
}

//---------------------------------------------------------
//   redo
//---------------------------------------------------------

void UndoCommand::redo(EditData* ed)
{
    for (UndoCommand* c : m_childCommands) {
        LOG_UNDO() << "<" << c->name() << ">";
        c->redo(ed);
    }
    flip(ed);
}

//---------------------------------------------------------
//   appendChildren
///   Append children of \p other into this UndoCommand.
///   Ownership over child commands of \p other is
///   transferred to this UndoCommand.
//---------------------------------------------------------

void UndoCommand::appendChildren(UndoCommand& other)
{
    m_childCommands.insert(m_childCommands.end(), other.m_childCommands.cbegin(), other.m_childCommands.cend());
    other.m_childCommands.clear();
}

//---------------------------------------------------------
//   hasFilteredChildren
//---------------------------------------------------------

bool UndoCommand::hasFilteredChildren(UndoCommand::Filter f, const EngravingItem* target) const
{
    for (UndoCommand* cmd : m_childCommands) {
        if (cmd->isFiltered(f, target)) {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------
//   hasUnfilteredChildren
//---------------------------------------------------------

bool UndoCommand::hasUnfilteredChildren(const std::vector<UndoCommand::Filter>& filters, const EngravingItem* target) const
{
    for (UndoCommand* cmd : m_childCommands) {
        bool filtered = false;
        for (UndoCommand::Filter f : filters) {
            if (cmd->isFiltered(f, target)) {
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

//---------------------------------------------------------
//   filterChildren
//---------------------------------------------------------

void UndoCommand::filterChildren(UndoCommand::Filter f, EngravingItem* target)
{
    std::vector<UndoCommand*> acceptedList;
    for (UndoCommand* cmd : m_childCommands) {
        if (cmd->isFiltered(f, target)) {
            delete cmd;
        } else {
            acceptedList.push_back(cmd);
        }
    }
    m_childCommands = std::move(acceptedList);
}

//---------------------------------------------------------
//   unwind
//---------------------------------------------------------

void UndoCommand::unwind()
{
    while (!m_childCommands.empty()) {
        UndoCommand* c = muse::takeLast(m_childCommands);
        LOG_UNDO() << "unwind: " << c->name();
        c->undo(nullptr);
        delete c;
    }
}

//---------------------------------------------------------
//   UndoStack
//---------------------------------------------------------

UndoStack::UndoStack()
{
    m_activeCommand = nullptr;
    m_currentIndex = 0;
    m_cleanState = 0;
    m_stateList.push_back(m_cleanState);
    m_nextState = 1;
}

//---------------------------------------------------------
//   UndoStack
//---------------------------------------------------------

UndoStack::~UndoStack()
{
    size_t idx = 0;
    for (auto c : m_macroList) {
        c->cleanup(idx++ < m_currentIndex);
    }
    muse::DeleteAll(m_macroList);
}

bool UndoStack::isLocked() const
{
    return m_isLocked;
}

void UndoStack::setLocked(bool locked)
{
    m_isLocked = locked;
}

//---------------------------------------------------------
//   beginMacro
//---------------------------------------------------------

void UndoStack::beginMacro(Score* score, const TranslatableString& actionName)
{
    if (m_isLocked) {
        return;
    }

    if (m_activeCommand) {
        LOGW("already active");
        return;
    }
    m_activeCommand = new UndoMacro(score, actionName);
}

//---------------------------------------------------------
//   push
//---------------------------------------------------------

void UndoStack::pushAndPerform(UndoCommand* cmd, EditData* ed)
{
    if (!m_activeCommand) {
        // this can happen for layout() outside of a command (load)
        if (!ScoreLoad::loading()) {
            LOG_UNDO() << "no active command, UndoStack";
        }

        cmd->redo(ed);
        delete cmd;
        return;
    }
#ifndef QT_NO_DEBUG
    if (!strcmp(cmd->name(), "ChangeProperty")) {
        ChangeProperty* cp = static_cast<ChangeProperty*>(cmd);
        LOG_UNDO() << cmd->name() << " id: " << int(cp->getId()) << ", property: " << propertyName(cp->getId());
    } else {
        LOG_UNDO() << cmd->name();
    }
#endif
    m_activeCommand->appendChild(cmd);
    cmd->redo(ed);
}

//---------------------------------------------------------
//   push1
//---------------------------------------------------------

void UndoStack::pushWithoutPerforming(UndoCommand* cmd)
{
    if (!m_activeCommand) {
        if (!ScoreLoad::loading()) {
            LOGW("no active command, UndoStack %p", this);
        }
        return;
    }
    m_activeCommand->appendChild(cmd);
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void UndoStack::remove(size_t idx)
{
    assert(idx <= m_currentIndex);
    assert(m_currentIndex != muse::nidx);
    // remove redo stack
    while (m_macroList.size() > m_currentIndex) {
        UndoCommand* cmd = muse::takeLast(m_macroList);
        m_stateList.pop_back();
        cmd->cleanup(false);      // delete elements for which UndoCommand() holds ownership
        delete cmd;
//            --curIdx;
    }
    while (m_macroList.size() > idx) {
        UndoCommand* cmd = muse::takeLast(m_macroList);
        m_stateList.pop_back();
        cmd->cleanup(true);
        delete cmd;
    }
    m_currentIndex = idx;
}

//---------------------------------------------------------
//   mergeCommands
//---------------------------------------------------------

void UndoStack::mergeCommands(size_t startIdx)
{
    assert(startIdx <= m_currentIndex);

    if (startIdx >= m_macroList.size()) {
        return;
    }

    UndoMacro* startMacro = m_macroList[startIdx];

    for (size_t idx = startIdx + 1; idx < m_currentIndex; ++idx) {
        startMacro->append(std::move(*m_macroList[idx]));
    }
    remove(startIdx + 1);   // TODO: remove from startIdx to curIdx only
}

//---------------------------------------------------------
//   pop
//---------------------------------------------------------

void UndoStack::pop()
{
    if (!m_activeCommand) {
        if (!ScoreLoad::loading()) {
            LOGW("no active command");
        }
        return;
    }
    UndoCommand* cmd = m_activeCommand->removeChild();
    cmd->undo(nullptr);
}

//---------------------------------------------------------
//   endMacro
//---------------------------------------------------------

void UndoStack::endMacro(bool rollback)
{
    if (m_isLocked) {
        return;
    }

    if (m_activeCommand == nullptr) {
        LOGW("not active");
        return;
    }
    if (rollback) {
        delete m_activeCommand;
    } else {
        // remove redo stack
        while (m_macroList.size() > m_currentIndex) {
            UndoCommand* cmd = muse::takeLast(m_macroList);
            m_stateList.pop_back();
            cmd->cleanup(false);        // delete elements for which UndoCommand() holds ownership
            delete cmd;
        }
        m_macroList.push_back(m_activeCommand);
        m_stateList.push_back(m_nextState++);
        ++m_currentIndex;
    }
    m_activeCommand = nullptr;
}

//---------------------------------------------------------
//   reopen
//---------------------------------------------------------

void UndoStack::reopen()
{
    if (m_isLocked) {
        return;
    }

    LOG_UNDO() << "curIdx: " << m_currentIndex << ", size: " << m_macroList.size();
    assert(m_activeCommand == nullptr);
    assert(m_currentIndex > 0);
    --m_currentIndex;
    m_activeCommand = muse::takeAt(m_macroList, m_currentIndex);
    m_stateList.erase(m_stateList.begin() + m_currentIndex);
    for (auto i : m_activeCommand->commands()) {
        LOG_UNDO() << "   " << i->name();
    }
}

//---------------------------------------------------------
//   undo
//---------------------------------------------------------

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
        assert(m_currentIndex < m_macroList.size());
        m_macroList[m_currentIndex]->undo(ed);
    }
}

//---------------------------------------------------------
//   redo
//---------------------------------------------------------

void UndoStack::redo(EditData* ed)
{
    LOG_UNDO() << "called";
    if (canRedo()) {
        m_macroList[m_currentIndex++]->redo(ed);
    }
}

//---------------------------------------------------------
//   UndoMacro
//---------------------------------------------------------

bool UndoMacro::canRecordSelectedElement(const EngravingItem* e)
{
    if (e->generated()) {
        return false;
    }

    return e->isNote() || (e->isChordRest() && !e->isChord())
           || (e->isTextBase() && !e->isInstrumentName() && !e->isHammerOnPullOffText())
           || e->isFretDiagram() || e->isSoundFlag();
}

void UndoMacro::fillSelectionInfo(SelectionInfo& info, const Selection& sel)
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

void UndoMacro::applySelectionInfo(const SelectionInfo& info, Selection& sel)
{
    if (!info.elements.empty()) {
        for (EngravingItem* e : info.elements) {
            sel.add(e);
        }
    } else if (info.staffStart != muse::nidx) {
        sel.setRangeTicks(info.tickStart, info.tickEnd, info.staffStart, info.staffEnd);
    }
}

UndoMacro::UndoMacro(Score* s, const TranslatableString& actionName)
    : m_undoInputState(s->inputState()), m_actionName(actionName), m_score(s)
{
    fillSelectionInfo(m_undoSelectionInfo, s->selection());
}

void UndoMacro::undo(EditData* ed)
{
    m_redoInputState = m_score->inputState();
    fillSelectionInfo(m_redoSelectionInfo, m_score->selection());
    m_score->deselectAll();

    // Undo for child commands.
    UndoCommand::undo(ed);

    m_score->setInputState(m_undoInputState);
    if (m_undoSelectionInfo.isValid()) {
        m_score->deselectAll();
        applySelectionInfo(m_undoSelectionInfo, m_score->selection());
    }
}

void UndoMacro::redo(EditData* ed)
{
    m_undoInputState = m_score->inputState();
    fillSelectionInfo(m_undoSelectionInfo, m_score->selection());
    m_score->deselectAll();

    // Redo for child commands.
    UndoCommand::redo(ed);

    m_score->setInputState(m_redoInputState);
    if (m_redoSelectionInfo.isValid()) {
        m_score->deselectAll();
        applySelectionInfo(m_redoSelectionInfo, m_score->selection());
    }
}

bool UndoMacro::empty() const
{
    return childCount() == 0;
}

void UndoMacro::append(UndoMacro&& other)
{
    appendChildren(other);
    if (m_score == other.m_score) {
        m_redoInputState = std::move(other.m_redoInputState);
        m_redoSelectionInfo = std::move(other.m_redoSelectionInfo);
    }
}

const InputState& UndoMacro::undoInputState() const
{
    return m_undoInputState;
}

const InputState& UndoMacro::redoInputState() const
{
    return m_redoInputState;
}

void UndoMacro::excludeElementFromSelectionInfo(EngravingItem* element)
{
    if (m_undoSelectionInfo.isValid()) {
        muse::remove(m_undoSelectionInfo.elements, element);
    }

    if (m_redoSelectionInfo.isValid()) {
        muse::remove(m_redoSelectionInfo.elements, element);
    }
}

const UndoMacro::SelectionInfo& UndoMacro::undoSelectionInfo() const
{
    return m_undoSelectionInfo;
}

const UndoMacro::SelectionInfo& UndoMacro::redoSelectionInfo() const
{
    return m_redoSelectionInfo;
}

UndoMacro::ChangesInfo UndoMacro::changesInfo(bool undo) const
{
    ChangesInfo result;

    for (const UndoCommand* command : commands()) {
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
            result.isTextEditing |= static_cast<const TextEditUndoCommand*>(command)->cursor().editing();
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

const TranslatableString& UndoMacro::actionName() const
{
    return m_actionName;
}
}
