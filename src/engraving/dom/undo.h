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

/**
 \file
 Definition of undo-related classes and structs.
*/

#include <map>

#include "../style/style.h"
#include "../compat/midi/midipatch.h"

#include "bend.h"
#include "bracket.h"
#include "chord.h"
#include "drumset.h"
#include "fret.h"
#include "harppedaldiagram.h"
#include "input.h"
#include "instrchange.h"
#include "instrument.h"
#include "key.h"
#include "keysig.h"
#include "masterscore.h"
#include "measure.h"
#include "note.h"
#include "noteevent.h"
#include "part.h"
#include "score.h"
#include "scoreorder.h"
#include "select.h"
#include "soundflag.h"
#include "spanner.h"
#include "staff.h"
#include "stafftype.h"
#include "stringdata.h"
#include "stringtunings.h"
#include "tremolotwochord.h"

namespace mu::engraving {
class Bend;
class Chord;
class ChordRest;
class Clef;
class EditData;
class EngravingItem;
class Excerpt;
class Harmony;
class HarpPedalDiagram;
class InstrChannel;
class Instrument;
class InstrumentChange;
class KeySig;
class MStaff;
class Measure;
class MeasureBase;
class Note;
class NoteEvent;
class Part;
class Score;
class Segment;
class Selection;
class Spanner;
class Staff;
class SystemLock;
class Text;
class TremoloBar;

enum class PlayEventType : unsigned char;

#define UNDO_TYPE(t) CommandType type() const override { return t; }
#define UNDO_NAME(a) const char* name() const override { return a; }
#define UNDO_CHANGED_OBJECTS(...) std::vector<EngravingObject*> objectItems() const override { return __VA_ARGS__; }

class UndoCommand
{
    OBJECT_ALLOCATOR(engraving, UndoCommand)

    std::list<UndoCommand*> childList;

protected:
    virtual void flip(EditData*) {}
    void appendChildren(UndoCommand*);

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
    void appendChild(UndoCommand* cmd) { childList.push_back(cmd); }
    UndoCommand* removeChild() { return muse::takeLast(childList); }
    size_t childCount() const { return childList.size(); }
    void unwind();
    const std::list<UndoCommand*>& commands() const { return childList; }
    virtual std::vector<EngravingObject*> objectItems() const { return {}; }
    virtual void cleanup(bool undo);
// #ifndef QT_NO_DEBUG
    virtual const char* name() const { return "UndoCommand"; }
// #endif
    virtual CommandType type() const { return CommandType::Unknown; }

    virtual bool isFiltered(Filter, const EngravingItem* /* target */) const { return false; }
    bool hasFilteredChildren(Filter, const EngravingItem* target) const;
    bool hasUnfilteredChildren(const std::vector<Filter>& filters, const EngravingItem* target) const;
    void filterChildren(UndoCommand::Filter f, EngravingItem* target);
};

//---------------------------------------------------------
//   UndoMacro
//    A root element for undo macro which is stored
//    directly in UndoStack
//---------------------------------------------------------

class UndoMacro : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, UndoMacro)
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

    struct ChangesInfo {
        ElementTypeSet changedObjectTypes;
        std::map<EngravingItem*, std::unordered_set<CommandType> > changedItems;
        StyleIdSet changedStyleIdSet;
        PropertyIdSet changedPropertyIdSet;
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

class InsertPart : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, InsertPart)

    Part* m_part = nullptr;
    size_t m_targetPartIdx = 0;

public:
    InsertPart(Part* p, size_t targetPartIdx);
    void undo(EditData*) override;
    void redo(EditData*) override;
    void cleanup(bool) override;

    UNDO_TYPE(CommandType::InsertPart)
    UNDO_NAME("InsertPart")
    UNDO_CHANGED_OBJECTS({ m_part })
};

class RemovePart : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, RemovePart)

    Part* m_part = nullptr;
    size_t m_partIdx = muse::nidx;

public:
    RemovePart(Part*, size_t partIdx);
    void undo(EditData*) override;
    void redo(EditData*) override;
    void cleanup(bool) override;

    UNDO_TYPE(CommandType::RemovePart)
    UNDO_NAME("RemovePart")
    UNDO_CHANGED_OBJECTS({ m_part })
};

class AddPartToExcerpt : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, AddPartToExcerpt)

    Excerpt* m_excerpt = nullptr;
    Part* m_part = nullptr;
    size_t m_targetPartIdx = 0;

public:
    AddPartToExcerpt(Excerpt* e, Part* p, size_t targetPartIdx);
    void undo(EditData*) override;
    void redo(EditData*) override;
    void cleanup(bool undo) override;

    UNDO_TYPE(CommandType::AddPartToExcerpt)
    UNDO_NAME("AddPartToExcerpt")
    UNDO_CHANGED_OBJECTS({ m_part })
};

class SetSoloist : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, SetSoloist)

    Part* part = nullptr;
    bool soloist = false;

public:
    SetSoloist(Part* p, bool b);
    void undo(EditData*) override;
    void redo(EditData*) override;

    UNDO_TYPE(CommandType::SetSoloist)
    UNDO_NAME("SetSoloist")
    UNDO_CHANGED_OBJECTS({ part })
};

class InsertStaff : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, InsertStaff)

    Staff* staff = nullptr;
    staff_idx_t ridx = muse::nidx;

public:
    InsertStaff(Staff*, staff_idx_t idx);
    void undo(EditData*) override;
    void redo(EditData*) override;
    void cleanup(bool) override;

    UNDO_TYPE(CommandType::InsertStaff)
    UNDO_NAME("InsertStaff")
    UNDO_CHANGED_OBJECTS({ staff })
};

class RemoveStaff : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, RemoveStaff)

    Staff* staff = nullptr;
    staff_idx_t ridx = muse::nidx;
    bool wasSystemObjectStaff = false;

public:
    RemoveStaff(Staff*);
    void undo(EditData*) override;
    void redo(EditData*) override;
    void cleanup(bool) override;

    UNDO_TYPE(CommandType::RemoveStaff)
    UNDO_NAME("RemoveStaff")
    UNDO_CHANGED_OBJECTS({ staff })
};

class AddSystemObjectStaff : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, AddSystemObjectStaff)

    Staff* staff = nullptr;

public:
    AddSystemObjectStaff(Staff*);
    void undo(EditData*) override;
    void redo(EditData*) override;

    UNDO_TYPE(CommandType::AddSystemObjectStaff)
    UNDO_NAME("AddSystemObjectStaff")
    UNDO_CHANGED_OBJECTS({ staff })
};

class RemoveSystemObjectStaff : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, RemoveSystemObjectStaff)

    Staff* staff = nullptr;

public:
    RemoveSystemObjectStaff(Staff*);
    void undo(EditData*) override;
    void redo(EditData*) override;

    UNDO_TYPE(CommandType::RemoveSystemObjectStaff)
    UNDO_NAME("RemoveSystemObjectStaff")
    UNDO_CHANGED_OBJECTS({ staff })
};

class InsertMStaff : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, InsertMStaff)

    Measure* measure = nullptr;
    MStaff* mstaff = nullptr;
    staff_idx_t idx = muse::nidx;

public:
    InsertMStaff(Measure*, MStaff*, staff_idx_t);
    void undo(EditData*) override;
    void redo(EditData*) override;

    UNDO_TYPE(CommandType::InsertMStaff)
    UNDO_NAME("InsertMStaff")
    UNDO_CHANGED_OBJECTS({ measure })
};

class RemoveMStaff : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, RemoveMStaff)

    Measure* measure = nullptr;
    MStaff* mstaff = nullptr;
    int idx = 0;

public:
    RemoveMStaff(Measure*, MStaff*, int);
    void undo(EditData*) override;
    void redo(EditData*) override;

    UNDO_TYPE(CommandType::RemoveMStaff)
    UNDO_NAME("RemoveMStaff")
    UNDO_CHANGED_OBJECTS({ measure })
};

class InsertStaves : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, InsertStaves)

    Measure* measure = nullptr;
    staff_idx_t a = muse::nidx;
    staff_idx_t b = muse::nidx;

public:
    InsertStaves(Measure*, staff_idx_t, staff_idx_t);
    void undo(EditData*) override;
    void redo(EditData*) override;

    UNDO_TYPE(CommandType::InsertStaves)
    UNDO_NAME("InsertStaves")
    UNDO_CHANGED_OBJECTS({ measure })
};

class RemoveStaves : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, RemoveStaves)

    Measure* measure = nullptr;
    staff_idx_t a = muse::nidx;
    staff_idx_t b = muse::nidx;

public:
    RemoveStaves(Measure*, staff_idx_t, staff_idx_t);
    void undo(EditData*) override;
    void redo(EditData*) override;

    UNDO_TYPE(CommandType::RemoveStaves)
    UNDO_NAME("RemoveStaves")
    UNDO_CHANGED_OBJECTS({ measure })
};

class SortStaves : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, SortStaves)

    Score* score = nullptr;
    std::vector<staff_idx_t> list;
    std::vector<staff_idx_t> rlist;

public:
    SortStaves(Score*, const std::vector<staff_idx_t>&);
    void undo(EditData*) override;
    void redo(EditData*) override;

    UNDO_TYPE(CommandType::SortStaves)
    UNDO_NAME("SortStaves")
    UNDO_CHANGED_OBJECTS({ score })
};

class ChangePitch : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangePitch)

    Note* note = nullptr;
    int pitch = 0;
    int tpc1 = 0;
    int tpc2 = 0;

    void flip(EditData*) override;

public:
    ChangePitch(Note* note, int pitch, int tpc1, int tpc2);

    UNDO_TYPE(CommandType::ChangePitch)
    UNDO_NAME("ChangePitch")
    UNDO_CHANGED_OBJECTS({ note })
};

class ChangeFretting : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeFretting)

    Note* note = nullptr;
    int pitch = 0;
    int string = 0;
    int fret = 0;
    int tpc1 = 0;
    int tpc2 = 0;

    void flip(EditData*) override;

public:
    ChangeFretting(Note* note, int pitch, int string, int fret, int tpc1, int tpc2);

    UNDO_TYPE(CommandType::ChangeFretting)
    UNDO_NAME("ChangeFretting")
    UNDO_CHANGED_OBJECTS({ note })
};

class ChangeKeySig : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeKeySig)

    KeySig* keysig = nullptr;
    KeySigEvent ks;
    bool showCourtesy = false;
    bool evtInStaff = false;

    void flip(EditData*) override;

public:
    ChangeKeySig(KeySig* k, KeySigEvent newKeySig, bool sc, bool addEvtToStaff = true);

    UNDO_TYPE(CommandType::ChangeKeySig)
    UNDO_NAME("ChangeKeySig")
    UNDO_CHANGED_OBJECTS({ keysig })
};

class ChangeMeasureLen : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeMeasureLen)

    Measure* measure = nullptr;
    Fraction len;

    void flip(EditData*) override;

public:
    ChangeMeasureLen(Measure*, Fraction);

    UNDO_TYPE(CommandType::ChangeMeasureLen)
    UNDO_NAME("ChangeMeasureLen")
    UNDO_CHANGED_OBJECTS({ measure })
};

class ChangeElement : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeElement)

    EngravingItem* oldElement = nullptr;
    EngravingItem* newElement = nullptr;

    void flip(EditData*) override;

public:
    ChangeElement(EngravingItem* oldElement, EngravingItem* newElement);

    UNDO_TYPE(CommandType::ChangeElement)
    UNDO_NAME("ChangeElement")
    UNDO_CHANGED_OBJECTS({ oldElement, newElement })
};

class TransposeHarmony : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, TransposeHarmony)

    Harmony* harmony = nullptr;
    int rootTpc = 0;
    int baseTpc = 0;

    void flip(EditData*) override;

public:
    TransposeHarmony(Harmony*, int rootTpc, int baseTpc);

    UNDO_TYPE(CommandType::TransposeHarmony)
    UNDO_NAME("TransposeHarmony")
    UNDO_CHANGED_OBJECTS({ harmony })
};

class ExchangeVoice : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ExchangeVoice)

    Measure* measure = nullptr;
    track_idx_t val1 = muse::nidx;
    track_idx_t val2 = muse::nidx;
    staff_idx_t staff = muse::nidx;

public:
    ExchangeVoice(Measure*, track_idx_t val1, track_idx_t val2, staff_idx_t staff);
    void undo(EditData*) override;
    void redo(EditData*) override;

    UNDO_TYPE(CommandType::ExchangeVoice)
    UNDO_NAME("ExchangeVoice")
    UNDO_CHANGED_OBJECTS({ measure })
};

class CloneVoice : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, CloneVoice)

    Segment* sourceSeg = nullptr;
    Fraction lTick;
    Segment* destSeg = nullptr;               //Destination
    track_idx_t strack = muse::nidx;
    track_idx_t dtrack = muse::nidx;
    track_idx_t otrack;
    bool linked = false;
    bool first = true;        //first redo

public:
    CloneVoice(Segment* sf, const Fraction& lTick, Segment* d, track_idx_t strack, track_idx_t dtrack, track_idx_t otrack,
               bool linked = true);
    void undo(EditData*) override;
    void redo(EditData*) override;

    UNDO_TYPE(CommandType::CloneVoice)
    UNDO_NAME("CloneVoice")
    UNDO_CHANGED_OBJECTS({ sourceSeg, destSeg })
};

class ChangeInstrumentShort : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeInstrumentShort)

    Part* part = nullptr;
    Fraction tick;
    std::list<StaffName> text;

    void flip(EditData*) override;

public:
    ChangeInstrumentShort(const Fraction&, Part*, std::list<StaffName>);

    UNDO_TYPE(CommandType::ChangeInstrumentShort)
    UNDO_NAME("ChangeInstrumentShort")
    UNDO_CHANGED_OBJECTS({ part })
};

class ChangeInstrumentLong : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeInstrumentLong)

    Part* part = nullptr;
    Fraction tick;
    std::list<StaffName> text;

    void flip(EditData*) override;

public:
    ChangeInstrumentLong(const Fraction&, Part*, std::list<StaffName>);

    UNDO_TYPE(CommandType::ChangeInstrumentLong)
    UNDO_NAME("ChangeInstrumentLong")
    UNDO_CHANGED_OBJECTS({ part })
};

class AddElement : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, AddElement)

    EngravingItem* element = nullptr;

    void endUndoRedo(bool) const;
    void undo(EditData*) override;
    void redo(EditData*) override;

public:
    AddElement(EngravingItem*);
    EngravingItem* getElement() const { return element; }
    void cleanup(bool) override;
    const char* name() const override;

    bool isFiltered(UndoCommand::Filter f, const EngravingItem* target) const override;

    std::vector<EngravingObject*> objectItems() const override;

    UNDO_TYPE(CommandType::AddElement)
};

class RemoveElement : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, RemoveElement)

    EngravingItem* element = nullptr;

public:
    RemoveElement(EngravingItem*);
    void undo(EditData*) override;
    void redo(EditData*) override;
    void cleanup(bool) override;
    const char* name() const override;

    bool isFiltered(UndoCommand::Filter f, const EngravingItem* target) const override;

    UNDO_TYPE(CommandType::RemoveElement)
    UNDO_CHANGED_OBJECTS({ element })
};

class AddSystemLock : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, AddSystemLock)

    const SystemLock* m_systemLock;
public:
    AddSystemLock(const SystemLock* systemLock);
    void undo(EditData*) override;
    void redo(EditData*) override;
    void cleanup(bool undo) override;

    UNDO_NAME("AddSystemLock")
    std::vector<EngravingObject*> objectItems() const override;
};

class RemoveSystemLock : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, RemoveSystemLock)

    const SystemLock* m_systemLock;
public:
    RemoveSystemLock(const SystemLock* systemLock);
    void undo(EditData*) override;
    void redo(EditData*) override;
    void cleanup(bool undo) override;

    UNDO_NAME("RemoveSystemLock")
    std::vector<EngravingObject*> objectItems() const override;
};

class ChangePatch : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangePatch)

    Score* score = nullptr;
    InstrChannel* channel = nullptr;
    MidiPatch patch;

    void flip(EditData*) override;

public:
    ChangePatch(Score* s, InstrChannel* c, const MidiPatch& pt)
        : score(s), channel(c), patch(pt) {}
    UNDO_NAME("ChangePatch")
    UNDO_CHANGED_OBJECTS({ score })
};

class SetUserBankController : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, SetUserBankController)

    InstrChannel* channel = nullptr;
    bool val = false;

    void flip(EditData*) override;

public:
    SetUserBankController(InstrChannel* c, bool v)
        : channel(c), val(v) {}
    UNDO_NAME("SetUserBankController")
};

//---------------------------------------------------------
//   ChangeStaff
//---------------------------------------------------------

class ChangeStaff : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeStaff)

    Staff* staff = nullptr;

    bool visible = false;
    ClefTypeList clefType;
    double userDist = 0.0;
    Staff::HideMode hideMode = Staff::HideMode::AUTO;
    bool showIfEmpty = false;
    bool cutaway = false;
    bool hideSystemBarLine = false;
    AutoOnOff mergeMatchingRests = AutoOnOff::AUTO;
    bool reflectTranspositionInLinkedTab = false;

    void flip(EditData*) override;

public:
    ChangeStaff(Staff*);

    ChangeStaff(Staff*, bool _visible, ClefTypeList _clefType, double userDist, Staff::HideMode _hideMode, bool _showIfEmpty, bool _cutaway,
                bool _hideSystemBarLine, AutoOnOff _mergeRests, bool _reflectTranspositionInLinkedTab);

    UNDO_TYPE(CommandType::ChangeStaff)
    UNDO_NAME("ChangeStaff")
    UNDO_CHANGED_OBJECTS({ staff })
};

//---------------------------------------------------------
//   ChangeStaffType
//---------------------------------------------------------

class ChangeStaffType : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeStaffType)

    Staff* staff = nullptr;
    StaffType staffType;

    void flip(EditData*) override;

public:
    ChangeStaffType(Staff* s, const StaffType& t)
        : staff(s), staffType(t) {}

    UNDO_TYPE(CommandType::ChangeStaffType)
    UNDO_NAME("ChangeStaffType")
    UNDO_CHANGED_OBJECTS({ staff })
};

class ChangePart : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangePart)

    Part* part = nullptr;
    Instrument* instrument = nullptr;
    String partName;

    void flip(EditData*) override;

public:
    ChangePart(Part*, Instrument*, const String& name);

    UNDO_TYPE(CommandType::ChangePart)
    UNDO_NAME("ChangePart")
    UNDO_CHANGED_OBJECTS({ part })
};

class ChangeStyle : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeStyle)

    Score* score = nullptr;
    MStyle style;
    bool overlap = false;

    void flip(EditData*) override;
    void undo(EditData*) override;

public:
    ChangeStyle(Score*, const MStyle&, const bool overlapOnly = false);

    UNDO_TYPE(CommandType::ChangeStyle)
    UNDO_NAME("ChangeStyle")
    UNDO_CHANGED_OBJECTS({ score })
};

class ChangeStyleValues : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeStyleValues)

public:
    ChangeStyleValues(Score* s, std::unordered_map<Sid, PropertyValue> values)
        : m_score(s), m_values(std::move(values)) {}

    const std::unordered_map<Sid, PropertyValue>& values() const { return m_values; }

    UNDO_TYPE(CommandType::ChangeStyleValues)
    UNDO_NAME("ChangeStyleValues")
    UNDO_CHANGED_OBJECTS({ m_score })

private:
    void flip(EditData*) override;

    Score* m_score = nullptr;
    std::unordered_map<Sid, PropertyValue> m_values;
};

class ChangePageNumberOffset : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangePageNumberOffset)

    Score* score = nullptr;
    int pageOffset = 0;

    void flip(EditData*) override;

public:
    ChangePageNumberOffset(Score* s, int po)
        : score(s), pageOffset(po) {}

    UNDO_NAME("ChangePageNumberOffset")
    UNDO_CHANGED_OBJECTS({ score })
};

class ChangeChordStaffMove : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeChordStaffMove)

    ChordRest* chordRest = nullptr;
    int staffMove = 0;

    void flip(EditData*) override;

public:
    ChangeChordStaffMove(ChordRest* cr, int);

    UNDO_TYPE(CommandType::ChangeChordStaffMove)
    UNDO_NAME("ChangeChordStaffMove")
    UNDO_CHANGED_OBJECTS({ chordRest })
};

class ChangeVelocity : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeVelocity)

    Note* note = nullptr;
    int userVelocity = 0;

    void flip(EditData*) override;

public:
    ChangeVelocity(Note*, int);

    UNDO_TYPE(CommandType::ChangeVelocity)
    UNDO_NAME("ChangeVelocity")
    UNDO_CHANGED_OBJECTS({ note })
};

class ChangeMStaffProperties : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeMStaffProperties)

    Measure* measure = nullptr;
    int staffIdx = 0;
    bool visible = false;
    bool stemless = false;

    void flip(EditData*) override;

public:
    ChangeMStaffProperties(Measure*, int staffIdx, bool visible, bool stemless);

    UNDO_TYPE(CommandType::ChangeMStaffProperties)
    UNDO_NAME("ChangeMStaffProperties")
    UNDO_CHANGED_OBJECTS({ measure })
};

class InsertRemoveMeasures : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, InsertRemoveMeasures)

    MeasureBase* fm = nullptr;
    MeasureBase* lm = nullptr;

    static std::vector<Clef*> getCourtesyClefs(Measure* m);

    bool moveStc = true;

protected:
    void removeMeasures();
    void insertMeasures();

public:
    InsertRemoveMeasures(MeasureBase* _fm, MeasureBase* _lm, bool _moveStc)
        : fm(_fm), lm(_lm), moveStc(_moveStc) {}
    virtual void undo(EditData*) override = 0;
    virtual void redo(EditData*) override = 0;
    UNDO_CHANGED_OBJECTS({ fm, lm })
};

class RemoveMeasures : public InsertRemoveMeasures
{
    OBJECT_ALLOCATOR(engraving, RemoveMeasures)
public:
    RemoveMeasures(MeasureBase* m1, MeasureBase* m2, bool moveStc = true)
        : InsertRemoveMeasures(m1, m2, moveStc) {}
    void undo(EditData*) override { insertMeasures(); }
    void redo(EditData*) override { removeMeasures(); }

    UNDO_TYPE(CommandType::RemoveMeasures)
    UNDO_NAME("RemoveMeasures")
};

class InsertMeasures : public InsertRemoveMeasures
{
    OBJECT_ALLOCATOR(engraving, InsertMeasures)
public:
    InsertMeasures(MeasureBase* m1, MeasureBase* m2, bool moveStc = true)
        : InsertRemoveMeasures(m1, m2, moveStc) {}
    void redo(EditData*) override { insertMeasures(); }
    void undo(EditData*) override { removeMeasures(); }

    UNDO_TYPE(CommandType::InsertMeasures)
    UNDO_NAME("InsertMeasures")
};

class AddExcerpt : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, AddExcerpt)

    Excerpt* excerpt = nullptr;
    bool deleteExcerpt = false;

public:
    AddExcerpt(Excerpt* ex);
    ~AddExcerpt() override;

    void undo(EditData*) override;
    void redo(EditData*) override;

    std::vector<EngravingObject*> objectItems() const override;

    UNDO_TYPE(CommandType::AddExcerpt)
    UNDO_NAME("AddExcerpt")
};

class RemoveExcerpt : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, RemoveExcerpt)

    Excerpt* excerpt = nullptr;
    size_t index = muse::nidx;
    bool deleteExcerpt = false;

public:
    RemoveExcerpt(Excerpt* ex);
    ~RemoveExcerpt() override;

    void undo(EditData*) override;
    void redo(EditData*) override;

    std::vector<EngravingObject*> objectItems() const override;

    UNDO_TYPE(CommandType::RemoveExcerpt)
    UNDO_NAME("RemoveExcerpt")
};

class SwapExcerpt : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, SwapExcerpt)

    MasterScore* score = nullptr;
    int pos1 = 0;
    int pos2 = 0;

    void flip(EditData*) override;

public:
    SwapExcerpt(MasterScore* s, int p1, int p2)
        : score(s), pos1(p1), pos2(p2) {}

    UNDO_TYPE(CommandType::SwapExcerpt)
    UNDO_NAME("SwapExcerpt")
    UNDO_CHANGED_OBJECTS({ score })
};

class ChangeExcerptTitle : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeExcerptTitle)

    Excerpt* excerpt = nullptr;
    String title;

    void flip(EditData*) override;

public:
    ChangeExcerptTitle(Excerpt* x, const String& t)
        : excerpt(x), title(t) {}

    UNDO_TYPE(CommandType::ChangeExcerptTitle)
    UNDO_NAME("ChangeExcerptTitle")
};

class ChangeNoteEventList : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeNoteEventList)

    Note* note = nullptr;
    NoteEventList newEvents;
    PlayEventType newPetype;

    void flip(EditData*) override;

public:
    ChangeNoteEventList(Note* n, NoteEventList& ne)
        : note(n), newEvents(ne), newPetype(PlayEventType::User) {}
    UNDO_NAME("ChangeNoteEventList")
    UNDO_CHANGED_OBJECTS({ note });
};

class ChangeChordPlayEventType : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeChordPlayEventType)

    Chord* chord = nullptr;
    PlayEventType petype;
    std::vector<NoteEventList> events;

    void flip(EditData*) override;

public:
    ChangeChordPlayEventType(Chord* c, PlayEventType pet)
        : chord(c), petype(pet)
    {
        events = c->getNoteEventLists();
    }

    UNDO_NAME("ChangeChordPlayEventType")
    UNDO_CHANGED_OBJECTS({ chord });
};

//---------------------------------------------------------
//   ChangeInstrument
//    change instrument in an InstrumentChange element
//---------------------------------------------------------

class ChangeInstrument : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeInstrument)

    InstrumentChange* is = nullptr;
    Instrument* instrument = nullptr;

    void flip(EditData*) override;

public:
    ChangeInstrument(InstrumentChange* _is, Instrument* i)
        : is(_is), instrument(i) {}

    UNDO_TYPE(CommandType::ChangeInstrument)
    UNDO_NAME("ChangeInstrument")
    UNDO_CHANGED_OBJECTS({ is })
};

extern void updateNoteLines(Segment*, track_idx_t track);

class SwapCR : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, SwapCR)

    ChordRest* cr1 = nullptr;
    ChordRest* cr2 = nullptr;

    void flip(EditData*) override;

public:
    SwapCR(ChordRest* a, ChordRest* b)
        : cr1(a), cr2(b) {}

    UNDO_TYPE(CommandType::SwapCR)
    UNDO_NAME("SwapCR")
    UNDO_CHANGED_OBJECTS({ cr1, cr2 })
};

class ChangeClefType : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeClefType)

    Clef* clef = nullptr;
    ClefType concertClef;
    ClefType transposingClef;

    void flip(EditData*) override;

public:
    ChangeClefType(Clef*, ClefType cl, ClefType tc);

    UNDO_TYPE(CommandType::ChangeClefType)
    UNDO_NAME("ChangeClef")
    UNDO_CHANGED_OBJECTS({ clef })
};

class ChangeProperty : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeProperty)
protected:
    EngravingObject* element = nullptr;
    Pid id;
    PropertyValue property;
    PropertyFlags flags;

    void flip(EditData*) override;

public:
    ChangeProperty(EngravingObject* e, Pid i, const PropertyValue& v, PropertyFlags ps = PropertyFlags::NOSTYLE)
        : element(e), id(i), property(v), flags(ps) {}

    Pid getId() const { return id; }
    EngravingObject* getElement() const { return element; }
    PropertyValue data() const { return property; }

    UNDO_TYPE(CommandType::ChangeProperty)
    UNDO_NAME("ChangeProperty")

    std::vector<EngravingObject*> objectItems() const override;

    bool isFiltered(UndoCommand::Filter f, const EngravingItem* target) const override
    {
        return f == UndoCommand::Filter::ChangePropertyLinked && muse::contains(target->linkList(), element);
    }
};

class ChangeBracketProperty : public ChangeProperty
{
    OBJECT_ALLOCATOR(engraving, ChangeBracketProperty)

    Staff* staff = nullptr;
    size_t level = 0;

    void flip(EditData*) override;

public:
    ChangeBracketProperty(Staff* s, size_t l, Pid i, const PropertyValue& v, PropertyFlags ps = PropertyFlags::NOSTYLE)
        : ChangeProperty(nullptr, i, v, ps), staff(s), level(l) {}
    UNDO_NAME("ChangeBracketProperty")
    UNDO_CHANGED_OBJECTS({ staff })
};

class ChangeTextLineProperty : public ChangeProperty
{
    OBJECT_ALLOCATOR(engraving, ChangeTextLineProperty)

    void flip(EditData*) override;

public:
    ChangeTextLineProperty(EngravingObject* e, PropertyValue v)
        : ChangeProperty(e, Pid::SYSTEM_FLAG, v, PropertyFlags::NOSTYLE) {}
    UNDO_NAME("ChangeTextLineProperty")
};

class ChangeMetaText : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeMetaText)

    Score* score = nullptr;
    String id;
    String text;

    void flip(EditData*) override;

public:
    ChangeMetaText(Score* s, const String& i, const String& t)
        : score(s), id(i), text(t) {}

    UNDO_TYPE(CommandType::ChangeMetaInfo)
    UNDO_NAME("ChangeMetaText")
    UNDO_CHANGED_OBJECTS({ score })
};

class RemoveBracket : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, RemoveBracket)

    Staff* staff = nullptr;
    size_t level = 0;
    BracketType bracketType = BracketType::NORMAL;
    size_t span = 0;

    void undo(EditData*) override;
    void redo(EditData*) override;

public:
    RemoveBracket(Staff* s, size_t l, BracketType t, size_t sp)
        : staff(s), level(l), bracketType(t), span(sp) {}

    UNDO_TYPE(CommandType::RemoveBracket)
    UNDO_NAME("RemoveBracket")
    UNDO_CHANGED_OBJECTS({ staff })
};

class AddBracket : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, AddBracket)

    Staff* staff = nullptr;
    size_t level = 0;
    BracketType bracketType = BracketType::NORMAL;
    size_t span = 0;

    void undo(EditData*) override;
    void redo(EditData*) override;

public:
    AddBracket(Staff* s, size_t l, BracketType t, size_t sp)
        : staff(s), level(l), bracketType(t), span(sp) {}

    UNDO_TYPE(CommandType::AddBracket)
    UNDO_NAME("AddBracket")
    UNDO_CHANGED_OBJECTS({ staff })
};

class ChangeSpannerElements : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeSpannerElements)

    Spanner* spanner = nullptr;
    EngravingItem* startElement = nullptr;
    EngravingItem* endElement = nullptr;

    void flip(EditData*) override;

public:
    ChangeSpannerElements(Spanner* s, EngravingItem* se, EngravingItem* ee)
        : spanner(s), startElement(se), endElement(ee) {}

    UNDO_TYPE(CommandType::ChangeSpannerElements)
    UNDO_NAME("ChangeSpannerElements")
    UNDO_CHANGED_OBJECTS({ spanner })
};

class ChangeParent : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeParent)

    EngravingItem* element = nullptr;
    EngravingItem* parent = nullptr;
    staff_idx_t staffIdx = muse::nidx;

    void flip(EditData*) override;

public:
    ChangeParent(EngravingItem* e, EngravingItem* p, staff_idx_t si)
        : element(e), parent(p), staffIdx(si) {}

    UNDO_TYPE(CommandType::ChangeParent)
    UNDO_NAME("ChangeParent")
    UNDO_CHANGED_OBJECTS({ element })
};

class ChangeMMRest : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeMMRest)

    Measure* m;
    Measure* mmrest;

    void flip(EditData*) override;

public:
    ChangeMMRest(Measure* _m, Measure* _mmr)
        : m(_m), mmrest(_mmr) {}

    UNDO_TYPE(CommandType::ChangeMMRest)
    UNDO_NAME("ChangeMMRest")
    UNDO_CHANGED_OBJECTS({ m, mmrest })
};

class ChangeMeasureRepeatCount : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeMeasureRepeatCount)

    Measure* m = nullptr;
    int count = 0;
    staff_idx_t staffIdx = muse::nidx;

    void flip(EditData*) override;

public:
    ChangeMeasureRepeatCount(Measure* _m, int _count, staff_idx_t _staffIdx)
        : m(_m), count(_count), staffIdx(_staffIdx) {}

    UNDO_TYPE(CommandType::ChangeMeasureRepeatCount)
    UNDO_NAME("ChangeMeasureRepeatCount")
    UNDO_CHANGED_OBJECTS({ m })
};

class InsertTime : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, InsertTime)

    Score* score = nullptr;
    Fraction tick;
    Fraction len;

    void redo(EditData*) override;
    void undo(EditData*) override;

public:
    InsertTime(Score* _score, const Fraction& _tick, const Fraction& _len)
        : score(_score), tick(_tick), len(_len) {}

    UNDO_TYPE(CommandType::InsertTime)
    UNDO_NAME("InsertTime")
    UNDO_CHANGED_OBJECTS({ score })
};

class InsertTimeUnmanagedSpanner : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, InsertTimeUnmanagedSpanner)

    Score* score = nullptr;
    Fraction tick;
    Fraction len;

    void flip(EditData*) override;

public:
    InsertTimeUnmanagedSpanner(Score* s, const Fraction& _tick, const Fraction& _len)
        : score(s), tick(_tick), len(_len) {}

    UNDO_TYPE(CommandType::InsertTimeUnmanagedSpanner)
    UNDO_NAME("InsertTimeUnmanagedSpanner")
    UNDO_CHANGED_OBJECTS({ score })
};

class ChangeNoteEvent : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeNoteEvent)

    Note* note = nullptr;
    NoteEvent* oldEvent = nullptr;
    NoteEvent newEvent;
    PlayEventType newPetype;

    void flip(EditData*) override;

public:
    ChangeNoteEvent(Note* n, NoteEvent* oe, const NoteEvent& ne)
        : note(n), oldEvent(oe), newEvent(ne), newPetype(PlayEventType::User) {}
    UNDO_NAME("ChangeNoteEvent")
    UNDO_CHANGED_OBJECTS({ note })
};

class LinkUnlink : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, LinkUnlink)

    bool mustDelete  { false };

protected:
    LinkedObjects* le = nullptr;
    EngravingObject* e = nullptr;

    void link();
    void unlink();

public:
    LinkUnlink() {}
    ~LinkUnlink();
};

class Unlink : public LinkUnlink
{
    OBJECT_ALLOCATOR(engraving, Unlink)
public:
    Unlink(EngravingObject*);
    void undo(EditData*) override { link(); }
    void redo(EditData*) override { unlink(); }

    UNDO_TYPE(CommandType::Unlink)
    UNDO_NAME("Unlink")
};

class Link : public LinkUnlink
{
    OBJECT_ALLOCATOR(engraving, Link)
public:
    Link(EngravingObject*, EngravingObject*);

    void undo(EditData*) override { unlink(); }
    void redo(EditData*) override { link(); }

    UNDO_TYPE(CommandType::Link)
    UNDO_NAME("Link")

    bool isFiltered(UndoCommand::Filter f, const EngravingItem* target) const override;
};

class ChangeStartEndSpanner : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeStartEndSpanner)

    Spanner* spanner = nullptr;
    EngravingItem* start = nullptr;
    EngravingItem* end = nullptr;

    void flip(EditData*) override;

public:
    ChangeStartEndSpanner(Spanner* sp, EngravingItem* s, EngravingItem* e)
        : spanner(sp), start(s), end(e) {}

    UNDO_TYPE(CommandType::ChangeStartEndSpanner)
    UNDO_NAME("ChangeStartEndSpanner")
    UNDO_CHANGED_OBJECTS({ spanner })
};

class ChangeMetaTags : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeMetaTags)

    Score* score = nullptr;
    std::map<String, String> metaTags;

    void flip(EditData*) override;

public:
    ChangeMetaTags(Score* s, const std::map<String, String>& m)
        : score(s), metaTags(m) {}

    UNDO_TYPE(CommandType::ChangeMetaInfo)
    UNDO_NAME("ChangeMetaTags")
    UNDO_CHANGED_OBJECTS({ score })
};

class ChangeDrumset : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeDrumset)

    Instrument* instrument = nullptr;
    Drumset drumset;
    Part* part = nullptr;

    void flip(EditData*) override;

public:
    ChangeDrumset(Instrument* i, const Drumset& d, Part* p)
        : instrument(i), drumset(d), part(p) {}

    UNDO_TYPE(CommandType::ChangeDrumset)
    UNDO_NAME("ChangeDrumset")
};

class FretDataChange : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, FretDataChange)

    FretDiagram* m_diagram = nullptr;
    FretUndoData m_undoData;
    String m_harmonyName;

    void redo(EditData*) override;
    void undo(EditData*) override;

public:
    FretDataChange(FretDiagram* d, const String& harmonyName)
        : m_diagram(d), m_harmonyName(harmonyName) {}

    UNDO_TYPE(CommandType::FretDataChange)

    UNDO_NAME("FretDataChange")

    UNDO_CHANGED_OBJECTS({ m_diagram })
};

class FretDot : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, FretDot)

    FretDiagram* diagram = nullptr;
    int string = 0;
    int fret = 0;
    bool add = 0;
    FretDotType dtype;
    FretUndoData undoData;

    void redo(EditData*) override;
    void undo(EditData*) override;

public:
    FretDot(FretDiagram* d, int _string, int _fret, bool _add = false, FretDotType _dtype = FretDotType::NORMAL)
        : diagram(d), string(_string), fret(_fret), add(_add), dtype(_dtype) {}

    UNDO_TYPE(CommandType::FretDot)
    UNDO_NAME("FretDot")
    UNDO_CHANGED_OBJECTS({ diagram })
};

class FretMarker : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, FretMarker)

    FretDiagram* diagram = nullptr;
    int string = 0;
    FretMarkerType mtype;
    FretUndoData undoData;

    void redo(EditData*) override;
    void undo(EditData*) override;

public:
    FretMarker(FretDiagram* d, int _string, FretMarkerType _mtype)
        : diagram(d), string(_string), mtype(_mtype) {}

    UNDO_TYPE(CommandType::FretMarker)
    UNDO_NAME("FretMarker")
    UNDO_CHANGED_OBJECTS({ diagram })
};

class FretBarre : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, FretBarre)

    FretDiagram* diagram = nullptr;
    int string = 0;
    int fret = 0;
    bool add = 0;
    FretUndoData undoData;

    void redo(EditData*) override;
    void undo(EditData*) override;

public:
    FretBarre(FretDiagram* d, int _string, int _fret, bool _add = false)
        : diagram(d), string(_string), fret(_fret), add(_add) {}

    UNDO_TYPE(CommandType::FretBarre)
    UNDO_NAME("FretBarre")
    UNDO_CHANGED_OBJECTS({ diagram })
};

class FretClear : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, FretClear)

    FretDiagram* diagram = nullptr;
    FretUndoData undoData;

    void redo(EditData*) override;
    void undo(EditData*) override;

public:
    FretClear(FretDiagram* d)
        : diagram(d) {}

    UNDO_TYPE(CommandType::FretClear)
    UNDO_NAME("FretClear")
    UNDO_CHANGED_OBJECTS({ diagram })
};

class MoveTremolo : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, MoveTremolo)

    Score* score = nullptr;
    Fraction chord1Tick;
    Fraction chord2Tick;
    TremoloTwoChord* trem = nullptr;
    track_idx_t track = 0;

    Chord* oldC1 = nullptr;
    Chord* oldC2 = nullptr;

    void undo(EditData*) override;
    void redo(EditData*) override;

public:
    MoveTremolo(Score* s, Fraction c1, Fraction c2, TremoloTwoChord* tr, track_idx_t t)
        : score(s), chord1Tick(c1), chord2Tick(c2), trem(tr), track(t) {}

    UNDO_TYPE(CommandType::MoveTremolo)
    UNDO_NAME("MoveTremolo")
    UNDO_CHANGED_OBJECTS({ trem })
};

class ChangeScoreOrder : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeScoreOrder)

    Score* score = nullptr;
    ScoreOrder order;
    void flip(EditData*) override;

public:
    ChangeScoreOrder(Score* sc, ScoreOrder so)
        : score(sc), order(so) {}

    UNDO_TYPE(CommandType::ChangeScoreOrder)
    UNDO_NAME("ChangeScoreOrder")
    UNDO_CHANGED_OBJECTS({ score })
};

//---------------------------------------------------------
//   ChangeHarpPedalState
//---------------------------------------------------------

class ChangeHarpPedalState : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeHarpPedalState)
    HarpPedalDiagram* diagram;
    std::array<PedalPosition, HARP_STRING_NO> pedalState;

    void flip(EditData*) override;

public:
    ChangeHarpPedalState(HarpPedalDiagram* _diagram, std::array<PedalPosition, HARP_STRING_NO> _pedalState)
        : diagram(_diagram), pedalState(_pedalState) {}

    UNDO_NAME("ChangeHarpPedalState")
//    UNDO_CHANGED_OBJECTS({ diagram })
    std::vector<EngravingObject*> objectItems() const override;
};

//---------------------------------------------------------
//   ChangeSingleHarpPedal
//---------------------------------------------------------

class ChangeSingleHarpPedal : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeSingleHarpPedal)

    HarpPedalDiagram* diagram;
    HarpStringType type;
    PedalPosition pos;

    void flip(EditData*) override;

public:
    ChangeSingleHarpPedal(HarpPedalDiagram* _diagram, HarpStringType _type, PedalPosition _pos)
        : diagram(_diagram),
        type(_type),
        pos(_pos)
    {
    }

    UNDO_NAME("ChangeSingleHarpPedal")
//    UNDO_CHANGED_OBJECTS({ diagram });
    std::vector<EngravingObject*> objectItems() const override;
};

class ChangeStringData : public UndoCommand
{
    Instrument* m_instrument = nullptr;
    StringTunings* m_stringTunings = nullptr;
    StringData m_stringData;

public:
    ChangeStringData(StringTunings* stringTunings, const StringData& stringData)
        : m_stringTunings(stringTunings), m_stringData(stringData) {}
    ChangeStringData(Instrument* instrument, const StringData& stringData)
        : m_instrument(instrument), m_stringData(stringData) {}

    void flip(EditData*) override;
    UNDO_NAME("ChangeStringData")
};

class ChangeSoundFlag : public UndoCommand
{
    SoundFlag* m_soundFlag = nullptr;
    SoundFlag::PresetCodes m_presets;
    SoundFlag::PlayingTechniqueCode m_playingTechnique;

public:
    ChangeSoundFlag(SoundFlag* soundFlag, const SoundFlag::PresetCodes& presets, const SoundFlag::PlayingTechniqueCode& technique)
        : m_soundFlag(soundFlag), m_presets(presets), m_playingTechnique(technique) {}

    void flip(EditData*) override;
    UNDO_NAME("ChangeSoundFlag")
    UNDO_CHANGED_OBJECTS({ m_soundFlag })
};

class ChangeSpanArpeggio : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeSpanArpeggio)

    Chord* m_chord = nullptr;
    Arpeggio* m_spanArpeggio = nullptr;

    void flip(EditData*) override;
public:
    ChangeSpanArpeggio(Chord* chord, Arpeggio* spanArp)
        : m_chord(chord), m_spanArpeggio(spanArp) {}

    UNDO_NAME("ChangeSpanArpeggio")
    UNDO_CHANGED_OBJECTS({ m_chord })
};

class ChangeTieJumpPointActive : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeTieJumpPointActive)

    TieJumpPointList* m_jumpPointList = nullptr;
    String m_id;
    bool m_active = false;

    void flip(EditData*) override;

public:
    ChangeTieJumpPointActive(TieJumpPointList* jumpPointList, String& id, bool active)
        : m_jumpPointList(jumpPointList), m_id(id), m_active(active) {}

    UNDO_TYPE(CommandType::ChangeTieEndPointActive)
    UNDO_NAME("ChangeTieEndPointActive")
};
} // namespace mu::engraving
