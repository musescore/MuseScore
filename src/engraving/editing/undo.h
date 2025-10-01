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

#pragma once

#include <map>

#include "global/containers.h"
#include "global/types/translatablestring.h"

#include "editdata.h"
#include "../dom/input.h"

namespace mu::engraving {
class EngravingObject;
class Score;
class Segment;

enum class CommandType : signed char {
    Unknown = -1,

    // Parts
    InsertPart,
    RemovePart,
    AddPartToExcerpt,
    SetSoloist,
    ChangePart,

    // Staves
    InsertStaff,
    RemoveStaff,
    AddSystemObjectStaff,
    RemoveSystemObjectStaff,
    SortStaves,
    ChangeStaff,
    ChangeStaffType,

    // MStaves
    InsertMStaff,
    RemoveMStaff,
    InsertStaves,
    RemoveStaves,
    ChangeMStaffProperties,
    ChangeMStaffHideIfEmpty,

    // Instruments
    ChangeInstrumentShort,
    ChangeInstrumentLong,
    ChangeInstrument,
    ChangeDrumset,

    // Measures
    RemoveMeasures,
    InsertMeasures,
    ChangeMeasureLen,
    ChangeMMRest,
    ChangeMeasureRepeatCount,

    // Elements
    AddElement,
    RemoveElement,
    Unlink,
    Link,
    ChangeElement,
    ChangeParent,

    // Notes
    ChangePitch,
    ChangeFretting,
    ChangeVelocity,

    // ChordRest
    ChangeChordStaffMove,
    SwapCR,

    // Brackets
    RemoveBracket,
    AddBracket,

    // Fret
    FretDataChange,
    FretDot,
    FretMarker,
    FretBarre,
    FretClear,
    AddFretDiagramToFretBox,
    RemoveFretDiagramFromFretBox,

    // Harmony
    TransposeHarmony,

    // KeySig
    ChangeKeySig,

    // Clef
    ChangeClefType,

    // Tremolo
    MoveTremolo,

    // Spanners
    ChangeSpannerElements,
    InsertTimeUnmanagedSpanner,
    ChangeStartEndSpanner,

    // Ties
    ChangeTieEndPointActive,

    // Style
    ChangeStyle,
    ChangeStyleValues,

    // Property
    ChangeProperty,

    // Voices
    ExchangeVoice,
    CloneVoice,

    // Excerpts
    AddExcerpt,
    RemoveExcerpt,
    SwapExcerpt,
    ChangeExcerptTitle,

    // Meta info
    ChangeMetaInfo,

    // Text
    TextEdit,

    // Other
    InsertTime,
    ChangeScoreOrder,
};

#define UNDO_TYPE(t) CommandType type() const override { return t; }
#define UNDO_NAME(a) const char* name() const override { return a; }
#define UNDO_CHANGED_OBJECTS(...) std::vector<EngravingObject*> objectItems() const override { return __VA_ARGS__; }

class UndoCommand
{
public:
    enum class Filter : unsigned char {
        TextEdit,
        AddElement,
        AddElementLinked,
        Link,
        RemoveElement,
        RemoveElementLinked,
        ChangePropertyLinked,
    };

    virtual ~UndoCommand();
    virtual void undo(EditData*);
    virtual void redo(EditData*);
    void appendChild(UndoCommand* cmd) { m_childCommands.push_back(cmd); }
    UndoCommand* removeChild() { return muse::takeLast(m_childCommands); }
    size_t childCount() const { return m_childCommands.size(); }
    void unwind();
    const std::vector<UndoCommand*>& commands() const { return m_childCommands; }
    virtual std::vector<EngravingObject*> objectItems() const { return {}; }
    virtual void cleanup(bool undo);
    virtual const char* name() const { return "UndoCommand"; }
    virtual CommandType type() const { return CommandType::Unknown; }

    virtual bool isFiltered(Filter, const EngravingItem* /* target */) const { return false; }
    bool hasFilteredChildren(Filter, const EngravingItem* target) const;
    bool hasUnfilteredChildren(const std::vector<Filter>& filters, const EngravingItem* target) const;
    void filterChildren(UndoCommand::Filter f, EngravingItem* target);

protected:
    virtual void flip(EditData*) {}
    void appendChildren(UndoCommand& other);

private:
    std::vector<UndoCommand*> m_childCommands;
};

//---------------------------------------------------------
//   UndoMacro
//    A root element for undo macro which is stored
//    directly in UndoStack
//---------------------------------------------------------

class UndoMacro : public UndoCommand
{
public:
    struct SelectionInfo {
        std::vector<EngravingItem*> elements;
        Fraction tickStart;
        Fraction tickEnd;
        staff_idx_t staffStart = muse::nidx;
        staff_idx_t staffEnd = muse::nidx;

        bool isValid() const { return !elements.empty() || staffStart != muse::nidx; }
    };

    UndoMacro(Score* s, const TranslatableString& actionName);
    void undo(EditData*) override;
    void redo(EditData*) override;
    bool empty() const;
    void append(UndoMacro&& other);

    const InputState& undoInputState() const;
    const InputState& redoInputState() const;

    const SelectionInfo& undoSelectionInfo() const;
    const SelectionInfo& redoSelectionInfo() const;

    void excludeElementFromSelectionInfo(EngravingItem* element);

    struct ChangesInfo {
        ElementTypeSet changedObjectTypes;
        std::map<EngravingObject*, std::unordered_set<CommandType> > changedObjects;
        StyleIdSet changedStyleIdSet;
        PropertyIdSet changedPropertyIdSet;
        bool isTextEditing = false;
    };

    ChangesInfo changesInfo(bool undo = false) const;
    const TranslatableString& actionName() const;

    static bool canRecordSelectedElement(const EngravingItem* e);

    UNDO_NAME("UndoMacro")

private:
    InputState m_undoInputState;
    InputState m_redoInputState;
    SelectionInfo m_undoSelectionInfo;
    SelectionInfo m_redoSelectionInfo;
    TranslatableString m_actionName;

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

    bool hasActiveCommand() const { return m_activeCommand != nullptr; }

    void beginMacro(Score*, const TranslatableString& actionName);
    void endMacro(bool rollback);

    void pushAndPerform(UndoCommand*, EditData*);
    void pushWithoutPerforming(UndoCommand*);
    void pop();

    bool canUndo() const { return m_currentIndex > 0; }
    bool canRedo() const { return m_currentIndex < m_macroList.size(); }
    bool isClean() const { return m_cleanState == m_stateList[m_currentIndex]; }

    size_t size() const { return m_macroList.size(); }
    size_t currentIndex() const { return m_currentIndex; }

    UndoMacro* activeCommand() const { return m_activeCommand; }

    UndoMacro* last() const { return m_currentIndex > 0 ? m_macroList[m_currentIndex - 1] : nullptr; }
    UndoMacro* prev() const { return m_currentIndex > 1 ? m_macroList[m_currentIndex - 2] : nullptr; }
    UndoMacro* next() const { return canRedo() ? m_macroList[m_currentIndex] : nullptr; }

    /// Returns the command that led to the state with the given `idx`.
    /// For further discussion of the indices involved in UndoStack, see:
    /// https://github.com/musescore/MuseScore/pull/25389#discussion_r1825782176
    UndoMacro* lastAtIndex(size_t idx) const
    {
        return idx > 0 && idx - 1 < m_macroList.size() ? m_macroList[idx - 1] : nullptr;
    }

    void undo(EditData*);
    void redo(EditData*);
    void reopen();

    void mergeCommands(size_t startIdx);
    void cleanRedoStack() { remove(m_currentIndex); }

private:
    void remove(size_t idx);

    UndoMacro* m_activeCommand = nullptr;
    std::vector<UndoMacro*> m_macroList;
    std::vector<int> m_stateList;
    int m_nextState = 0;
    int m_cleanState = 0;
    size_t m_currentIndex = 0;
    bool m_isLocked = false;
};

std::vector<EngravingObject*> compoundObjects(EngravingObject* object);

class StaffTextBase;
void updateStaffTextCache(const StaffTextBase* text, Score* score);
}
