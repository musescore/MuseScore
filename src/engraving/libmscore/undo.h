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

#ifndef __UNDO_H__
#define __UNDO_H__

/**
 \file
 Definition of undo-related classes and structs.
*/

#include <map>

#include "style/style.h"
#include "compat/midi/midipatch.h"

#include "bend.h"
#include "bracket.h"
#include "chord.h"
#include "drumset.h"
#include "fret.h"
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
#include "spanner.h"
#include "staff.h"
#include "stafftype.h"
#include "synthesizerstate.h"
#include "text.h"
#include "tremolo.h"
#include "tremolobar.h"

namespace mu::engraving {
class Bend;
class Chord;
class ChordRest;
class Clef;
class EditData;
class EngravingItem;
class Excerpt;
class Harmony;
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
class Text;
class TremoloBar;

enum class PlayEventType : char;

#define UNDO_NAME(a) const char* name() const override { return a; }
#define UNDO_CHANGED_OBJECTS(...) std::vector<const EngravingObject*> objectItems() const override { return __VA_ARGS__; }

//---------------------------------------------------------
//   UndoCommand
//---------------------------------------------------------

class UndoCommand
{
    OBJECT_ALLOCATOR(engraving, UndoCommand)

    std::list<UndoCommand*> childList;

protected:
    virtual void flip(EditData*) {}
    void appendChildren(UndoCommand*);

public:
    enum class Filter {
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
    UndoCommand* removeChild() { return mu::takeLast(childList); }
    size_t childCount() const { return childList.size(); }
    void unwind();
    const std::list<UndoCommand*>& commands() const { return childList; }
    virtual std::vector<const EngravingObject*> objectItems() const { return {}; }
    virtual void cleanup(bool undo);
// #ifndef QT_NO_DEBUG
    virtual const char* name() const { return "UndoCommand"; }
// #endif

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
        staff_idx_t staffStart = mu::nidx;
        staff_idx_t staffEnd = mu::nidx;

        bool isValid() const { return !elements.empty() || staffStart != mu::nidx; }
    };

    UndoMacro(Score* s);
    void undo(EditData*) override;
    void redo(EditData*) override;
    bool empty() const;
    void append(UndoMacro&& other);

    const InputState& undoInputState() const;
    const InputState& redoInputState() const;
    const SelectionInfo& undoSelectionInfo() const;
    const SelectionInfo& redoSelectionInfo() const;

    std::unordered_set<ElementType> changedTypes() const;
    std::vector<const EngravingItem*> changedElements() const;

    static bool canRecordSelectedElement(const EngravingItem* e);

    UNDO_NAME("UndoMacro");

private:
    InputState m_undoInputState;
    InputState m_redoInputState;
    SelectionInfo m_undoSelectionInfo;
    SelectionInfo m_redoSelectionInfo;

    Score* m_score = nullptr;

    static void fillSelectionInfo(SelectionInfo&, const Selection&);
    static void applySelectionInfo(const SelectionInfo&, Selection&);
};

//---------------------------------------------------------
//   UndoStack
//---------------------------------------------------------

class UndoStack
{
    UndoMacro* curCmd;
    std::vector<UndoMacro*> list;
    std::vector<int> stateList;
    int nextState;
    int cleanState;
    size_t curIdx = 0;

    void remove(size_t idx);

public:
    UndoStack();
    ~UndoStack();

    bool active() const { return curCmd != 0; }
    void beginMacro(Score*);
    void endMacro(bool rollback);
    void push(UndoCommand*, EditData*);        // push & execute
    void push1(UndoCommand*);
    void pop();
    void setClean();
    bool canUndo() const { return curIdx > 0; }
    bool canRedo() const { return curIdx < list.size(); }
    int state() const { return stateList[curIdx]; }
    bool isClean() const { return cleanState == state(); }
    size_t getCurIdx() const { return curIdx; }
    bool empty() const { return !canUndo() && !canRedo(); }
    UndoMacro* current() const { return curCmd; }
    UndoMacro* last() const { return curIdx > 0 ? list[curIdx - 1] : 0; }
    UndoMacro* prev() const { return curIdx > 1 ? list[curIdx - 2] : 0; }
    void undo(EditData*);
    void redo(EditData*);
    void rollback();
    void reopen();

    void mergeCommands(size_t startIdx);
    void cleanRedoStack() { remove(curIdx); }
};

//---------------------------------------------------------
//   InsertPart
//---------------------------------------------------------

class InsertPart : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, InsertPart)

    Part* part;
    int idx;

public:
    InsertPart(Part* p, int i);
    void undo(EditData*) override;
    void redo(EditData*) override;

    UNDO_NAME("InsertPart")
    UNDO_CHANGED_OBJECTS({ part });
};

//---------------------------------------------------------
//   RemovePart
//---------------------------------------------------------

class RemovePart : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, RemovePart)

    Part* part;
    staff_idx_t idx;

public:
    RemovePart(Part*, staff_idx_t idx);
    virtual void undo(EditData*) override;
    virtual void redo(EditData*) override;
    UNDO_NAME("RemovePart")
    UNDO_CHANGED_OBJECTS({ part });
};

//---------------------------------------------------------
//   SetSoloist
//---------------------------------------------------------

class SetSoloist : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, SetSoloist)

    Part* part = nullptr;
    bool soloist = false;

public:
    SetSoloist(Part* p, bool b);
    void undo(EditData*) override;
    void redo(EditData*) override;
    UNDO_NAME("SetSoloist")
    UNDO_CHANGED_OBJECTS({ part });
};

//---------------------------------------------------------
//   InsertStaff
//---------------------------------------------------------

class InsertStaff : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, InsertStaff)

    Staff* staff;
    staff_idx_t ridx;

public:
    InsertStaff(Staff*, staff_idx_t idx);
    virtual void undo(EditData*) override;
    virtual void redo(EditData*) override;
    UNDO_NAME("InsertStaff")
    UNDO_CHANGED_OBJECTS({ staff });
};

//---------------------------------------------------------
//   RemoveStaff
//---------------------------------------------------------

class RemoveStaff : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, RemoveStaff)

    Staff* staff = nullptr;
    staff_idx_t ridx = mu::nidx;

public:
    RemoveStaff(Staff*);
    virtual void undo(EditData*) override;
    virtual void redo(EditData*) override;
    UNDO_NAME("RemoveStaff")
    UNDO_CHANGED_OBJECTS({ staff });
};

//---------------------------------------------------------
//   InsertMStaff
//---------------------------------------------------------

class InsertMStaff : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, InsertMStaff)

    Measure* measure;
    MStaff* mstaff;
    staff_idx_t idx;

public:
    InsertMStaff(Measure*, MStaff*, staff_idx_t);
    virtual void undo(EditData*) override;
    virtual void redo(EditData*) override;
    UNDO_NAME("InsertMStaff")
    UNDO_CHANGED_OBJECTS({ measure });
};

//---------------------------------------------------------
//   RemoveMStaff
//---------------------------------------------------------

class RemoveMStaff : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, RemoveMStaff)

    Measure* measure;
    MStaff* mstaff;
    int idx;

public:
    RemoveMStaff(Measure*, MStaff*, int);
    virtual void undo(EditData*) override;
    virtual void redo(EditData*) override;
    UNDO_NAME("RemoveMStaff")
    UNDO_CHANGED_OBJECTS({ measure });
};

//---------------------------------------------------------
//   InsertStaves
//---------------------------------------------------------

class InsertStaves : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, InsertStaves)

    Measure* measure;
    staff_idx_t a;
    staff_idx_t b;

public:
    InsertStaves(Measure*, staff_idx_t, staff_idx_t);
    virtual void undo(EditData*) override;
    virtual void redo(EditData*) override;
    UNDO_NAME("InsertStaves")
    UNDO_CHANGED_OBJECTS({ measure });
};

//---------------------------------------------------------
//   RemoveStaves
//---------------------------------------------------------

class RemoveStaves : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, RemoveStaves)

    Measure* measure;
    staff_idx_t a;
    staff_idx_t b;

public:
    RemoveStaves(Measure*, staff_idx_t, staff_idx_t);
    virtual void undo(EditData*) override;
    virtual void redo(EditData*) override;
    UNDO_NAME("RemoveStaves")
    UNDO_CHANGED_OBJECTS({ measure });
};

//---------------------------------------------------------
//   SortStaves
//---------------------------------------------------------

class SortStaves : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, SortStaves)

    Score* score = nullptr;
    std::vector<staff_idx_t> list;
    std::vector<staff_idx_t> rlist;

public:
    SortStaves(Score*, const std::vector<staff_idx_t>&);
    virtual void undo(EditData*) override;
    virtual void redo(EditData*) override;
    UNDO_NAME("SortStaves")
    UNDO_CHANGED_OBJECTS({ score });
};

//---------------------------------------------------------
//   MapExcerptTracks
//---------------------------------------------------------

class MapExcerptTracks : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, MapExcerptTracks)

    Score* score = nullptr;
    std::vector<staff_idx_t> list;
    std::vector<staff_idx_t> rlist;

public:
    MapExcerptTracks(Score*, const std::vector<staff_idx_t>&);
    virtual void undo(EditData*) override;
    virtual void redo(EditData*) override;
    UNDO_NAME("MapExcerptTracks")
    UNDO_CHANGED_OBJECTS({ score });
};

//---------------------------------------------------------
//   ChangePitch
//---------------------------------------------------------

class ChangePitch : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangePitch)

    Note* note;
    int pitch;
    int tpc1;
    int tpc2;
    void flip(EditData*) override;

public:
    ChangePitch(Note* note, int pitch, int tpc1, int tpc2);
    UNDO_NAME("ChangePitch")
    UNDO_CHANGED_OBJECTS({ note });
};

//---------------------------------------------------------
//   ChangeFretting
//---------------------------------------------------------

class ChangeFretting : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeFretting)

    Note* note;
    int pitch;
    int string;
    int fret;
    int tpc1;
    int tpc2;
    void flip(EditData*) override;

public:
    ChangeFretting(Note* note, int pitch, int string, int fret, int tpc1, int tpc2);
    UNDO_NAME("ChangeFretting")
    UNDO_CHANGED_OBJECTS({ note });
};

//---------------------------------------------------------
//   ChangeKeySig
//---------------------------------------------------------

class ChangeKeySig : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeKeySig)

    KeySig* keysig;
    KeySigEvent ks;
    bool showCourtesy;
    bool evtInStaff;

    void flip(EditData*) override;

public:
    ChangeKeySig(KeySig* k, KeySigEvent newKeySig, bool sc, bool addEvtToStaff = true);
    UNDO_NAME("ChangeKeySig")
    UNDO_CHANGED_OBJECTS({ keysig });
};

//---------------------------------------------------------
//   ChangeMeasureLen
//---------------------------------------------------------

class ChangeMeasureLen : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeMeasureLen)

    Measure* measure;
    Fraction len;
    void flip(EditData*) override;

public:
    ChangeMeasureLen(Measure*, Fraction);
    UNDO_NAME("ChangeMeasureLen")
    UNDO_CHANGED_OBJECTS({ measure });
};

//---------------------------------------------------------
//   ChangeElement
//---------------------------------------------------------

class ChangeElement : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeElement)

    EngravingItem* oldElement;
    EngravingItem* newElement;
    void flip(EditData*) override;

public:
    ChangeElement(EngravingItem* oldElement, EngravingItem* newElement);
    UNDO_NAME("ChangeElement")
    UNDO_CHANGED_OBJECTS({ oldElement, newElement });
};

//---------------------------------------------------------
//   TransposeHarmony
//---------------------------------------------------------

class TransposeHarmony : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, TransposeHarmony)

    Harmony* harmony;
    int rootTpc, baseTpc;
    void flip(EditData*) override;

public:
    TransposeHarmony(Harmony*, int rootTpc, int baseTpc);
    UNDO_NAME("TransposeHarmony")
    UNDO_CHANGED_OBJECTS({ harmony });
};

//---------------------------------------------------------
//   ExchangeVoice
//---------------------------------------------------------

class ExchangeVoice : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ExchangeVoice)

    Measure* measure;
    track_idx_t val1, val2;
    staff_idx_t staff;

public:
    ExchangeVoice(Measure*, track_idx_t val1, track_idx_t val2, staff_idx_t staff);
    virtual void undo(EditData*) override;
    virtual void redo(EditData*) override;
    UNDO_NAME("ExchangeVoice")
    UNDO_CHANGED_OBJECTS({ measure });
};

//---------------------------------------------------------
//   CloneVoice
//---------------------------------------------------------

class CloneVoice : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, CloneVoice)

    Segment* sf;
    Fraction lTick;
    Segment* d;               //Destination
    track_idx_t strack, dtrack;
    track_idx_t otrack;
    bool linked;
    bool first = true;        //first redo

public:
    CloneVoice(Segment* sf, const Fraction& lTick, Segment* d, track_idx_t strack, track_idx_t dtrack, track_idx_t otrack,
               bool linked = true);
    virtual void undo(EditData*) override;
    virtual void redo(EditData*) override;
    UNDO_NAME("CloneVoice")
    UNDO_CHANGED_OBJECTS({ sf, d });
};

//---------------------------------------------------------
//   ChangeInstrumentShort
//---------------------------------------------------------

class ChangeInstrumentShort : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeInstrumentShort)

    Part* part;
    Fraction tick;
    std::list<StaffName> text;
    void flip(EditData*) override;

public:
    ChangeInstrumentShort(const Fraction&, Part*, std::list<StaffName>);
    UNDO_NAME("ChangeInstrumentShort")
    UNDO_CHANGED_OBJECTS({ part });
};

//---------------------------------------------------------
//   ChangeInstrumentLong
//---------------------------------------------------------

class ChangeInstrumentLong : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeInstrumentLong)

    Part* part = nullptr;
    Fraction tick;
    std::list<StaffName> text;
    void flip(EditData*) override;

public:
    ChangeInstrumentLong(const Fraction&, Part*, std::list<StaffName>);
    UNDO_NAME("ChangeInstrumentLong")
    UNDO_CHANGED_OBJECTS({ part });
};

//---------------------------------------------------------
//   ChangeBracketType
//---------------------------------------------------------

class ChangeBracketType : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeBracketType)

    Bracket* bracket;
    BracketType type;
    void flip(EditData*) override;

public:
    ChangeBracketType(Bracket*, BracketType type);
    UNDO_NAME("ChangeBracketType")
    UNDO_CHANGED_OBJECTS({ bracket });
};

//---------------------------------------------------------
//   AddElement
//---------------------------------------------------------

class AddElement : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, AddElement)

    EngravingItem* element;

    void endUndoRedo(bool) const;
    void undo(EditData*) override;
    void redo(EditData*) override;

public:
    AddElement(EngravingItem*);
    EngravingItem* getElement() const { return element; }
    virtual void cleanup(bool) override;
    virtual const char* name() const override;

    bool isFiltered(UndoCommand::Filter f, const EngravingItem* target) const override;

    std::vector<const EngravingObject*> objectItems() const override;
};

//---------------------------------------------------------
//   RemoveElement
//---------------------------------------------------------

class RemoveElement : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, RemoveElement)

    EngravingItem* element;

public:
    RemoveElement(EngravingItem*);
    virtual void undo(EditData*) override;
    virtual void redo(EditData*) override;
    virtual void cleanup(bool) override;
    virtual const char* name() const override;

    bool isFiltered(UndoCommand::Filter f, const EngravingItem* target) const override;

    UNDO_CHANGED_OBJECTS({ element });
};

//---------------------------------------------------------
//   EditText
//---------------------------------------------------------

class EditText : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, EditText)

    Text* text;
    String oldText;
    //int undoLevel;

    void undoRedo();

public:
    EditText(Text* t, const String& ot, int /*l*/)
        : text(t), oldText(ot) /*, undoLevel(l)*/ {}
    virtual void undo(EditData*) override;
    virtual void redo(EditData*) override;
    UNDO_NAME("EditText")
    UNDO_CHANGED_OBJECTS({ text });
};

//---------------------------------------------------------
//   ChangePatch
//---------------------------------------------------------

class ChangePatch : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangePatch)

    Score* score;
    InstrChannel* channel;
    MidiPatch patch;

    void flip(EditData*) override;

public:
    ChangePatch(Score* s, InstrChannel* c, const MidiPatch* pt)
        : score(s), channel(c), patch(*pt) {}
    UNDO_NAME("ChangePatch")
    UNDO_CHANGED_OBJECTS({ score });
};

//---------------------------------------------------------
//   SetUserBankController
//---------------------------------------------------------

class SetUserBankController : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, SetUserBankController)

    InstrChannel* channel;
    bool val;

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
    bool mergeMatchingRests = false;

    void flip(EditData*) override;

public:
    ChangeStaff(Staff*);

    ChangeStaff(Staff*, bool _visible, ClefTypeList _clefType, double userDist, Staff::HideMode _hideMode, bool _showIfEmpty, bool _cutaway,
                bool _hideSystemBarLine, bool _mergeRests);
    UNDO_NAME("ChangeStaff")
    UNDO_CHANGED_OBJECTS({ staff });
};

//---------------------------------------------------------
//   ChangeStaffType
//---------------------------------------------------------

class ChangeStaffType : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeStaffType)

    Staff* staff;
    StaffType staffType;

    void flip(EditData*) override;

public:
    ChangeStaffType(Staff* s, const StaffType& t)
        : staff(s), staffType(t) {}
    UNDO_NAME("ChangeStaffType")
    UNDO_CHANGED_OBJECTS({ staff });
};

//---------------------------------------------------------
//   ChangePart
//---------------------------------------------------------

class ChangePart : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangePart)

    Part* part;
    Instrument* instrument;
    String partName;

    void flip(EditData*) override;

public:
    ChangePart(Part*, Instrument*, const String& name);
    UNDO_NAME("ChangePart")
    UNDO_CHANGED_OBJECTS({ part });
};

//---------------------------------------------------------
//   ChangeStyle
//---------------------------------------------------------

class ChangeStyle : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeStyle)

    Score* score;
    MStyle style;
    bool overlap = false;
    void flip(EditData*) override;
    void undo(EditData*) override;

public:
    ChangeStyle(Score*, const MStyle&, const bool overlapOnly = false);
    UNDO_NAME("ChangeStyle")
    UNDO_CHANGED_OBJECTS({ score });
};

//---------------------------------------------------------
//   ChangeStyleVal
//---------------------------------------------------------

class ChangeStyleVal : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeStyleVal)

    Score* score;
    Sid idx;
    PropertyValue value;

    void flip(EditData*) override;

public:
    ChangeStyleVal(Score* s, Sid i, const PropertyValue& v)
        : score(s), idx(i), value(v) {}
    UNDO_NAME("ChangeStyleVal")
    UNDO_CHANGED_OBJECTS({ score });
};

//---------------------------------------------------------
//   ChangePageNumberOffset
//---------------------------------------------------------

class ChangePageNumberOffset : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangePageNumberOffset)

    Score* score;
    int pageOffset;

    void flip(EditData*) override;

public:
    ChangePageNumberOffset(Score* s, int po)
        : score(s), pageOffset(po) {}
    UNDO_NAME("ChangePageNumberOffset")
    UNDO_CHANGED_OBJECTS({ score });
};

//---------------------------------------------------------
//   ChangeChordStaffMove
//---------------------------------------------------------

class ChangeChordStaffMove : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeChordStaffMove)

    ChordRest* chordRest;
    int staffMove;
    void flip(EditData*) override;

public:
    ChangeChordStaffMove(ChordRest* cr, int);
    UNDO_NAME("ChangeChordStaffMove")
    UNDO_CHANGED_OBJECTS({ chordRest });
};

//---------------------------------------------------------
//   ChangeVelocity
//---------------------------------------------------------

class ChangeVelocity : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeVelocity)

    Note* note;
    VeloType veloType;
    int veloOffset;
    void flip(EditData*) override;

public:
    ChangeVelocity(Note*, VeloType, int);
    UNDO_NAME("ChangeVelocity")
    UNDO_CHANGED_OBJECTS({ note });
};

//---------------------------------------------------------
//   ChangeMStaffProperties
//---------------------------------------------------------

class ChangeMStaffProperties : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeMStaffProperties)

    Measure* measure;
    int staffIdx;
    bool visible;
    bool stemless;
    void flip(EditData*) override;

public:
    ChangeMStaffProperties(Measure*, int staffIdx, bool visible, bool stemless);
    UNDO_NAME("ChangeMStaffProperties")
    UNDO_CHANGED_OBJECTS({ measure });
};

//---------------------------------------------------------
//   InsertRemoveMeasures
//---------------------------------------------------------

class InsertRemoveMeasures : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, InsertRemoveMeasures)

    MeasureBase* fm;
    MeasureBase* lm;

    static std::vector<Clef*> getCourtesyClefs(Measure* m);

protected:
    void removeMeasures();
    void insertMeasures();

public:
    InsertRemoveMeasures(MeasureBase* _fm, MeasureBase* _lm)
        : fm(_fm), lm(_lm) {}
    virtual void undo(EditData*) override = 0;
    virtual void redo(EditData*) override = 0;
    UNDO_CHANGED_OBJECTS({ fm, lm });
};

//---------------------------------------------------------
//   RemoveMeasures
//---------------------------------------------------------

class RemoveMeasures : public InsertRemoveMeasures
{
    OBJECT_ALLOCATOR(engraving, RemoveMeasures)
public:
    RemoveMeasures(MeasureBase* m1, MeasureBase* m2)
        : InsertRemoveMeasures(m1, m2) {}
    virtual void undo(EditData*) override { insertMeasures(); }
    virtual void redo(EditData*) override { removeMeasures(); }
    UNDO_NAME("RemoveMeasures")
};

//---------------------------------------------------------
//   InsertMeasures
//---------------------------------------------------------

class InsertMeasures : public InsertRemoveMeasures
{
    OBJECT_ALLOCATOR(engraving, InsertMeasures)
public:
    InsertMeasures(MeasureBase* m1, MeasureBase* m2)
        : InsertRemoveMeasures(m1, m2) {}
    virtual void redo(EditData*) override { insertMeasures(); }
    virtual void undo(EditData*) override { removeMeasures(); }
    UNDO_NAME("InsertMeasures")
};

//---------------------------------------------------------
//   AddExcerpt
//---------------------------------------------------------

class AddExcerpt : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, AddExcerpt)

    Excerpt* excerpt = nullptr;
    bool deleteExcerpt = false;

public:
    AddExcerpt(Excerpt* ex);
    ~AddExcerpt() override;

    virtual void undo(EditData*) override;
    virtual void redo(EditData*) override;
    UNDO_NAME("AddExcerpt")
};

//---------------------------------------------------------
//   RemoveExcerpt
//---------------------------------------------------------

class RemoveExcerpt : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, RemoveExcerpt)

    Excerpt* excerpt = nullptr;
    size_t index = mu::nidx;
    bool deleteExcerpt = false;

public:
    RemoveExcerpt(Excerpt* ex);
    ~RemoveExcerpt() override;

    virtual void undo(EditData*) override;
    virtual void redo(EditData*) override;
    UNDO_NAME("RemoveExcerpt")
};

//---------------------------------------------------------
//   SwapExcerpt
//---------------------------------------------------------

class SwapExcerpt : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, SwapExcerpt)

    MasterScore* score;
    int pos1;
    int pos2;

    void flip(EditData*) override;

public:
    SwapExcerpt(MasterScore* s, int p1, int p2)
        : score(s), pos1(p1), pos2(p2) {}
    UNDO_NAME("SwapExcerpt")
    UNDO_CHANGED_OBJECTS({ score });
};

//---------------------------------------------------------
//   ChangeExcerptTitle
//---------------------------------------------------------

class ChangeExcerptTitle : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeExcerptTitle)

    Excerpt* excerpt;
    String title;

    void flip(EditData*) override;

public:
    ChangeExcerptTitle(Excerpt* x, const String& t)
        : excerpt(x), title(t) {}
    UNDO_NAME("ChangeExcerptTitle")
};

//---------------------------------------------------------
//   ChangeBend
//---------------------------------------------------------

class ChangeBend : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeBend)

    Bend* bend;
    PitchValues points;

    void flip(EditData*) override;

public:
    ChangeBend(Bend* b, PitchValues p)
        : bend(b), points(p) {}
    UNDO_NAME("ChangeBend")
    UNDO_CHANGED_OBJECTS({ bend });
};

//---------------------------------------------------------
//   ChangeTremoloBar
//---------------------------------------------------------

class ChangeTremoloBar : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeTremoloBar)

    TremoloBar* bend;
    PitchValues points;

    void flip(EditData*) override;

public:
    ChangeTremoloBar(TremoloBar* b, PitchValues p)
        : bend(b), points(p) {}
    UNDO_NAME("ChangeTremoloBar")
    UNDO_CHANGED_OBJECTS({ bend });
};

//---------------------------------------------------------
//   ChangeNoteEvents
//---------------------------------------------------------

class ChangeNoteEvents : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeNoteEvents)

    Chord* chord = nullptr;
    std::list<NoteEvent*> events;

    void flip(EditData*) override;

public:
    ChangeNoteEvents(Chord* n, const std::list<NoteEvent*>& l)
        : chord(n), events(l) {}
    UNDO_NAME("ChangeNoteEvents")
    UNDO_CHANGED_OBJECTS({ chord });
};

//---------------------------------------------------------
//   ChangeNoteEventList
//---------------------------------------------------------

class ChangeNoteEventList : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeNoteEventList)

    Note* note;
    NoteEventList newEvents;
    PlayEventType newPetype;

    void flip(EditData*) override;

public:
    ChangeNoteEventList(Note* n, NoteEventList& ne)
        : note(n), newEvents(ne), newPetype(PlayEventType::User) {}
    UNDO_NAME("ChangeNoteEventList")
    UNDO_CHANGED_OBJECTS({ note });
};

//---------------------------------------------------------
//   ChangeChordPlayEventType
//---------------------------------------------------------

class ChangeChordPlayEventType : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeChordPlayEventType)

    Chord* chord;
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

    InstrumentChange* is;
    Instrument* instrument;

    void flip(EditData*) override;

public:
    ChangeInstrument(InstrumentChange* _is, Instrument* i)
        : is(_is), instrument(i) {}
    UNDO_NAME("ChangeInstrument")
    UNDO_CHANGED_OBJECTS({ is });
};

extern void updateNoteLines(Segment*, track_idx_t track);

//---------------------------------------------------------
//   SwapCR
//---------------------------------------------------------

class SwapCR : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, SwapCR)

    ChordRest* cr1;
    ChordRest* cr2;

    void flip(EditData*) override;

public:
    SwapCR(ChordRest* a, ChordRest* b)
        : cr1(a), cr2(b) {}
    UNDO_NAME("SwapCR")
    UNDO_CHANGED_OBJECTS({ cr1, cr2 });
};

//---------------------------------------------------------
//   ChangeClefType
//---------------------------------------------------------

class ChangeClefType : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeClefType)

    Clef* clef;
    ClefType concertClef;
    ClefType transposingClef;
    void flip(EditData*) override;

public:
    ChangeClefType(Clef*, ClefType cl, ClefType tc);
    UNDO_NAME("ChangeClef")
    UNDO_CHANGED_OBJECTS({ clef });
};

//---------------------------------------------------------
//   ChangeProperty
//---------------------------------------------------------

class ChangeProperty : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeProperty)
protected:
    EngravingObject* element;
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
    UNDO_NAME("ChangeProperty")

    std::vector<const EngravingObject*> objectItems() const override;

    bool isFiltered(UndoCommand::Filter f, const EngravingItem* target) const override
    {
        return f == UndoCommand::Filter::ChangePropertyLinked && mu::contains(target->linkList(), element);
    }
};

//---------------------------------------------------------
//   ChangeBracketProperty
//---------------------------------------------------------

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
    UNDO_CHANGED_OBJECTS({ staff });
};

//---------------------------------------------------------
//   ChangeTextLineProperty
//---------------------------------------------------------

class ChangeTextLineProperty : public ChangeProperty
{
    OBJECT_ALLOCATOR(engraving, ChangeTextLineProperty)

    void flip(EditData*) override;

public:
    ChangeTextLineProperty(EngravingObject* e, PropertyValue v)
        : ChangeProperty(e, Pid::SYSTEM_FLAG, v, PropertyFlags::NOSTYLE) {}
    UNDO_NAME("ChangeTextLineProperty")
};

//---------------------------------------------------------
//   ChangeMetaText
//---------------------------------------------------------

class ChangeMetaText : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeMetaText)

    Score* score;
    String id;
    String text;

    void flip(EditData*) override;

public:
    ChangeMetaText(Score* s, const String& i, const String& t)
        : score(s), id(i), text(t) {}
    UNDO_NAME("ChangeMetaText")
    UNDO_CHANGED_OBJECTS({ score });
};

//---------------------------------------------------------
//   ChangeSynthesizerState
//---------------------------------------------------------

class ChangeSynthesizerState : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeSynthesizerState)

    Score* score;
    SynthesizerState state;

    void flip(EditData*) override;

public:
    ChangeSynthesizerState(Score* s, const SynthesizerState& st)
        : score(s), state(st) {}
    UNDO_NAME("ChangeSynthesizerState")
    UNDO_CHANGED_OBJECTS({ score });
};

//---------------------------------------------------------
//   RemoveBracket
//---------------------------------------------------------

class RemoveBracket : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, RemoveBracket)

    Staff* staff;
    size_t level;
    BracketType type;
    size_t span;

    virtual void undo(EditData*) override;
    virtual void redo(EditData*) override;

public:
    RemoveBracket(Staff* s, size_t l, BracketType t, size_t sp)
        : staff(s), level(l), type(t), span(sp) {}
    UNDO_NAME("RemoveBracket")
    UNDO_CHANGED_OBJECTS({ staff });
};

//---------------------------------------------------------
//   AddBracket
//---------------------------------------------------------

class AddBracket : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, AddBracket)

    Staff* staff;
    int level;
    BracketType type;
    size_t span;

    virtual void undo(EditData*) override;
    virtual void redo(EditData*) override;

public:
    AddBracket(Staff* s, int l, BracketType t, size_t sp)
        : staff(s), level(l), type(t), span(sp) {}
    UNDO_NAME("AddBracket")
    UNDO_CHANGED_OBJECTS({ staff });
};

//---------------------------------------------------------
//   ChangeSpannerElements
//---------------------------------------------------------

class ChangeSpannerElements : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeSpannerElements)

    Spanner* spanner;
    EngravingItem* startElement;
    EngravingItem* endElement;

    void flip(EditData*) override;

public:
    ChangeSpannerElements(Spanner* s, EngravingItem* se, EngravingItem* ee)
        : spanner(s), startElement(se), endElement(ee) {}
    UNDO_NAME("ChangeSpannerElements")
    UNDO_CHANGED_OBJECTS({ spanner });
};

//---------------------------------------------------------
//   ChangeParent
//---------------------------------------------------------

class ChangeParent : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeParent)

    EngravingItem* element;
    EngravingItem* parent;
    staff_idx_t staffIdx;

    void flip(EditData*) override;

public:
    ChangeParent(EngravingItem* e, EngravingItem* p, staff_idx_t si)
        : element(e), parent(p), staffIdx(si) {}
    UNDO_NAME("ChangeParent")
    UNDO_CHANGED_OBJECTS({ element });
};

//---------------------------------------------------------
//   ChangeMMRest
//---------------------------------------------------------

class ChangeMMRest : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeMMRest)

    Measure* m;
    Measure* mmrest;

    void flip(EditData*) override;

public:
    ChangeMMRest(Measure* _m, Measure* _mmr)
        : m(_m), mmrest(_mmr) {}
    UNDO_NAME("ChangeMMRest")
    UNDO_CHANGED_OBJECTS({ m, mmrest });
};

//---------------------------------------------------------
//   ChangeMeasureRepeatCount
//---------------------------------------------------------

class ChangeMeasureRepeatCount : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeMeasureRepeatCount)

    Measure* m;
    int count;
    staff_idx_t staffIdx;

    void flip(EditData*) override;

public:
    ChangeMeasureRepeatCount(Measure* _m, int _count, staff_idx_t _staffIdx)
        : m(_m), count(_count), staffIdx(_staffIdx) {}
    UNDO_NAME("ChangeMeasureRepeatCount")
    UNDO_CHANGED_OBJECTS({ m });
};

//---------------------------------------------------------
//   InsertTime
//---------------------------------------------------------

class InsertTime : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, InsertTime)

    Score* score;
    Fraction tick;
    Fraction len;

    void redo(EditData*) override;
    void undo(EditData*) override;

public:
    InsertTime(Score* _score, const Fraction& _tick, const Fraction& _len)
        : score(_score), tick(_tick), len(_len) {}
    UNDO_NAME("InsertTime")
    UNDO_CHANGED_OBJECTS({ score });
};

//---------------------------------------------------------
//   InsertTimeUnmanagedSpanner
//---------------------------------------------------------

class InsertTimeUnmanagedSpanner : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, InsertTimeUnmanagedSpanner)

    Score* score;
    Fraction tick;
    Fraction len;

    void flip(EditData*) override;

public:
    InsertTimeUnmanagedSpanner(Score* s, const Fraction& _tick, const Fraction& _len)
        : score(s), tick(_tick), len(_len) {}
    UNDO_NAME("InsertTimeUnmanagedSpanner")
    UNDO_CHANGED_OBJECTS({ score });
};

//---------------------------------------------------------
//   ChangeNoteEvent
//---------------------------------------------------------

class ChangeNoteEvent : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeNoteEvent)

    Note* note;
    NoteEvent* oldEvent;
    NoteEvent newEvent;
    PlayEventType newPetype;

    void flip(EditData*) override;

public:
    ChangeNoteEvent(Note* n, NoteEvent* oe, const NoteEvent& ne)
        : note(n), oldEvent(oe), newEvent(ne), newPetype(PlayEventType::User) {}
    UNDO_NAME("ChangeNoteEvent")
    UNDO_CHANGED_OBJECTS({ note });
};

//---------------------------------------------------------
//   LinkUnlink
//---------------------------------------------------------

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

//---------------------------------------------------------
//   Unlink
//---------------------------------------------------------

class Unlink : public LinkUnlink
{
    OBJECT_ALLOCATOR(engraving, Unlink)
public:
    Unlink(EngravingObject*);
    virtual void undo(EditData*) override { link(); }
    virtual void redo(EditData*) override { unlink(); }
    UNDO_NAME("Unlink")
};

//---------------------------------------------------------
//   Link
//---------------------------------------------------------

class Link : public LinkUnlink
{
    OBJECT_ALLOCATOR(engraving, Link)
public:
    Link(EngravingObject*, EngravingObject*);
    virtual void undo(EditData*) override { unlink(); }
    virtual void redo(EditData*) override { link(); }
    UNDO_NAME("Link")

    bool isFiltered(UndoCommand::Filter f, const EngravingItem* target) const override;
};

//---------------------------------------------------------
//   ChangeStartEndSpanner
//---------------------------------------------------------

class ChangeStartEndSpanner : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeStartEndSpanner)

    Spanner* spanner;
    EngravingItem* start;
    EngravingItem* end;

    void flip(EditData*) override;

public:
    ChangeStartEndSpanner(Spanner* sp, EngravingItem* s, EngravingItem* e)
        : spanner(sp), start(s), end(e) {}
    UNDO_NAME("ChangeStartEndSpanner")
    UNDO_CHANGED_OBJECTS({ spanner });
};

//---------------------------------------------------------
//   ChangeMetaTags
//---------------------------------------------------------

class ChangeMetaTags : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeMetaTags)

    Score* score;
    std::map<String, String> metaTags;

    void flip(EditData*) override;

public:
    ChangeMetaTags(Score* s, const std::map<String, String>& m)
        : score(s), metaTags(m) {}
    UNDO_NAME("ChangeMetaTags")
    UNDO_CHANGED_OBJECTS({ score });
};

//---------------------------------------------------------
//   ChangeDrumset
//---------------------------------------------------------

class ChangeDrumset : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeDrumset)

    Instrument* instrument;
    Drumset drumset;

    void flip(EditData*) override;

public:
    ChangeDrumset(Instrument* i, const Drumset* d)
        : instrument(i), drumset(*d) {}
    UNDO_NAME("ChangeDrumset")
};

//---------------------------------------------------------
//   FretDot
//---------------------------------------------------------

class FretDot : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, FretDot)

    FretDiagram* diagram;
    int string;
    int fret;
    bool add;
    FretDotType dtype;
    FretUndoData undoData;

    void redo(EditData*) override;
    void undo(EditData*) override;

public:
    FretDot(FretDiagram* d, int _string, int _fret, bool _add = false, FretDotType _dtype = FretDotType::NORMAL)
        : diagram(d), string(_string), fret(_fret), add(_add), dtype(_dtype) {}
    UNDO_NAME("FretDot")
    UNDO_CHANGED_OBJECTS({ diagram });
};

//---------------------------------------------------------
//   FretMarker
//---------------------------------------------------------

class FretMarker : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, FretMarker)

    FretDiagram* diagram;
    int string;
    FretMarkerType mtype;
    FretUndoData undoData;

    void redo(EditData*) override;
    void undo(EditData*) override;

public:
    FretMarker(FretDiagram* d, int _string, FretMarkerType _mtype)
        : diagram(d), string(_string), mtype(_mtype) {}
    UNDO_NAME("FretMarker")
    UNDO_CHANGED_OBJECTS({ diagram });
};

//---------------------------------------------------------
//   FretBarre
//---------------------------------------------------------

class FretBarre : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, FretBarre)

    FretDiagram* diagram;
    int string;
    int fret;
    bool add;
    FretUndoData undoData;

    void redo(EditData*) override;
    void undo(EditData*) override;

public:
    FretBarre(FretDiagram* d, int _string, int _fret, bool _add = false)
        : diagram(d), string(_string), fret(_fret), add(_add) {}
    UNDO_NAME("FretBarre")
    UNDO_CHANGED_OBJECTS({ diagram });
};

//---------------------------------------------------------
//   FretClear
//---------------------------------------------------------

class FretClear : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, FretClear)

    FretDiagram* diagram;
    FretUndoData undoData;

    void redo(EditData*) override;
    void undo(EditData*) override;

public:
    FretClear(FretDiagram* d)
        : diagram(d) {}
    UNDO_NAME("FretClear")
    UNDO_CHANGED_OBJECTS({ diagram });
};

//---------------------------------------------------------
//   MoveTremolo
//---------------------------------------------------------

class MoveTremolo : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, MoveTremolo)

    Score* score { nullptr };
    Fraction chord1Tick;
    Fraction chord2Tick;
    Tremolo* trem { nullptr };
    int track { 0 };

    Chord* oldC1 { nullptr };
    Chord* oldC2 { nullptr };

    void undo(EditData*) override;
    void redo(EditData*) override;

public:
    MoveTremolo(Score* s, Fraction c1, Fraction c2, Tremolo* tr, int t)
        : score(s), chord1Tick(c1), chord2Tick(c2), trem(tr), track(t) {}
    UNDO_NAME("MoveTremolo")
    UNDO_CHANGED_OBJECTS({ trem });
};

//---------------------------------------------------------
//   ChangeScoreOrder
//---------------------------------------------------------

class ChangeScoreOrder : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeScoreOrder)

    Score* score;
    ScoreOrder order;
    void flip(EditData*) override;

public:
    ChangeScoreOrder(Score* sc, ScoreOrder so)
        : score(sc), order(so) {}
    UNDO_NAME("ChangeScoreOrder")
    UNDO_CHANGED_OBJECTS({ score });
};
} // namespace mu::engraving
#endif
