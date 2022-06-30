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

#include "engravingitem.h"
#include "note.h"
#include "score.h"
#include "segment.h"
#include "measure.h"
#include "system.h"
#include "select.h"
#include "input.h"
#include "slur.h"
#include "tie.h"
#include "clef.h"
#include "staff.h"
#include "chord.h"
#include "sig.h"
#include "key.h"
#include "barline.h"
#include "volta.h"
#include "tuplet.h"
#include "harmony.h"
#include "pitchspelling.h"
#include "part.h"
#include "beam.h"
#include "dynamic.h"
#include "page.h"
#include "keysig.h"
#include "image.h"
#include "hairpin.h"
#include "rest.h"
#include "bend.h"
#include "tremolobar.h"
#include "articulation.h"
#include "noteevent.h"
#include "slur.h"
#include "tempotext.h"
#include "instrchange.h"
#include "box.h"
#include "stafftype.h"
#include "accidental.h"
#include "layoutbreak.h"
#include "spanner.h"
#include "breath.h"
#include "fingering.h"
#include "rehearsalmark.h"
#include "excerpt.h"
#include "stafftext.h"
#include "chordline.h"
#include "tremolo.h"
#include "scorefont.h"
#include "utils.h"
#include "glissando.h"
#include "stafflines.h"
#include "bracket.h"
#include "fret.h"
#include "textedit.h"
#include "textline.h"
#include "linkedobjects.h"
#include "stafftypechange.h"

#include "masterscore.h"

#include "log.h"
#define LOG_UNDO() if (0) LOGD()

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
extern Measure* tick2measure(int tick);

//---------------------------------------------------------
//   updateNoteLines
//    compute line position of noteheads after
//    clef change
//---------------------------------------------------------

void updateNoteLines(Segment* segment, track_idx_t track)
{
    Staff* staff = segment->score()->staff(track / VOICES);
    if (staff->isDrumStaff(segment->tick()) || staff->isTabStaff(segment->tick())) {
        return;
    }
    for (Segment* s = segment->next1(); s; s = s->next1()) {
        if ((s->segmentType() & (SegmentType::Clef | SegmentType::HeaderClef)) && s->element(track) && !s->element(track)->generated()) {
            break;
        }
        if (!s->isChordRestType()) {
            continue;
        }
        for (track_idx_t t = track; t < track + VOICES; ++t) {
            EngravingItem* e = s->element(t);
            if (e && e->isChord()) {
                Chord* chord = toChord(e);
                for (Note* n : chord->notes()) {
                    n->updateLine();
                }
                chord->sortNotes();
                for (Chord* gc : chord->graceNotes()) {
                    for (Note* gn : gc->notes()) {
                        gn->updateLine();
                    }
                    gc->sortNotes();
                }
            }
        }
    }
}

//---------------------------------------------------------
//   UndoCommand
//---------------------------------------------------------

UndoCommand::~UndoCommand()
{
    for (auto c : childList) {
        delete c;
    }
}

//---------------------------------------------------------
//   UndoCommand::cleanup
//---------------------------------------------------------

void UndoCommand::cleanup(bool undo)
{
    for (auto c : childList) {
        c->cleanup(undo);
    }
}

//---------------------------------------------------------
//   undo
//---------------------------------------------------------

void UndoCommand::undo(EditData* ed)
{
    for (std::list<UndoCommand*>::reverse_iterator it = childList.rbegin(); it != childList.rend(); ++it) {
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
    for (UndoCommand* c : childList) {
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

void UndoCommand::appendChildren(UndoCommand* other)
{
    mu::join(childList, other->childList);
    other->childList.clear();
}

//---------------------------------------------------------
//   hasFilteredChildren
//---------------------------------------------------------

bool UndoCommand::hasFilteredChildren(UndoCommand::Filter f, const EngravingItem* target) const
{
    for (UndoCommand* cmd : childList) {
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
    for (UndoCommand* cmd : childList) {
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
    std::list<UndoCommand*> acceptedList;
    for (UndoCommand* cmd : childList) {
        if (cmd->isFiltered(f, target)) {
            delete cmd;
        } else {
            acceptedList.push_back(cmd);
        }
    }
    childList = std::move(acceptedList);
}

//---------------------------------------------------------
//   unwind
//---------------------------------------------------------

void UndoCommand::unwind()
{
    while (!childList.empty()) {
        UndoCommand* c = mu::takeLast(childList);
        LOG_UNDO() << "unwind: " << c->name();
        c->undo(0);
        delete c;
    }
}

//---------------------------------------------------------
//   UndoStack
//---------------------------------------------------------

UndoStack::UndoStack()
{
    curCmd   = 0;
    curIdx   = 0;
    cleanState = 0;
    stateList.push_back(cleanState);
    nextState = 1;
}

//---------------------------------------------------------
//   UndoStack
//---------------------------------------------------------

UndoStack::~UndoStack()
{
    size_t idx = 0;
    for (auto c : list) {
        c->cleanup(idx++ < curIdx);
    }
    DeleteAll(list);
}

//---------------------------------------------------------
//   beginMacro
//---------------------------------------------------------

void UndoStack::beginMacro(Score* score)
{
    if (curCmd) {
        LOGW("already active");
        return;
    }
    curCmd = new UndoMacro(score);
}

//---------------------------------------------------------
//   push
//---------------------------------------------------------

void UndoStack::push(UndoCommand* cmd, EditData* ed)
{
    if (!curCmd) {
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
    curCmd->appendChild(cmd);
    cmd->redo(ed);
}

//---------------------------------------------------------
//   push1
//---------------------------------------------------------

void UndoStack::push1(UndoCommand* cmd)
{
    if (!curCmd) {
        if (!ScoreLoad::loading()) {
            LOGW("no active command, UndoStack %p", this);
        }
        return;
    }
    curCmd->appendChild(cmd);
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void UndoStack::remove(size_t idx)
{
    assert(idx <= curIdx);
    assert(curIdx != mu::nidx);
    // remove redo stack
    while (list.size() > curIdx) {
        UndoCommand* cmd = mu::takeLast(list);
        stateList.pop_back();
        cmd->cleanup(false);      // delete elements for which UndoCommand() holds ownership
        delete cmd;
//            --curIdx;
    }
    while (list.size() > idx) {
        UndoCommand* cmd = mu::takeLast(list);
        stateList.pop_back();
        cmd->cleanup(true);
        delete cmd;
    }
    curIdx = idx;
}

//---------------------------------------------------------
//   mergeCommands
//---------------------------------------------------------

void UndoStack::mergeCommands(size_t startIdx)
{
    assert(startIdx <= curIdx);

    if (startIdx >= list.size()) {
        return;
    }

    UndoMacro* startMacro = list[startIdx];

    for (size_t idx = startIdx + 1; idx < curIdx; ++idx) {
        startMacro->append(std::move(*list[idx]));
    }
    remove(startIdx + 1);   // TODO: remove from startIdx to curIdx only
}

//---------------------------------------------------------
//   pop
//---------------------------------------------------------

void UndoStack::pop()
{
    if (!curCmd) {
        if (!ScoreLoad::loading()) {
            LOGW("no active command");
        }
        return;
    }
    UndoCommand* cmd = curCmd->removeChild();
    cmd->undo(0);
}

//---------------------------------------------------------
//   rollback
//---------------------------------------------------------

void UndoStack::rollback()
{
    LOG_UNDO() << "called";
    assert(curCmd == 0);
    assert(curIdx > 0);
    size_t idx = curIdx - 1;
    list[idx]->unwind();
    remove(idx);
}

//---------------------------------------------------------
//   endMacro
//---------------------------------------------------------

void UndoStack::endMacro(bool rollback)
{
    if (curCmd == 0) {
        LOGW("not active");
        return;
    }
    if (rollback) {
        delete curCmd;
    } else {
        // remove redo stack
        while (list.size() > curIdx) {
            UndoCommand* cmd = mu::takeLast(list);
            stateList.pop_back();
            cmd->cleanup(false);        // delete elements for which UndoCommand() holds ownership
            delete cmd;
        }
        list.push_back(curCmd);
        stateList.push_back(nextState++);
        ++curIdx;
    }
    curCmd = 0;
}

//---------------------------------------------------------
//   reopen
//---------------------------------------------------------

void UndoStack::reopen()
{
    LOG_UNDO() << "curIdx: " << curIdx << ", size: " << list.size();
    assert(curCmd == 0);
    assert(curIdx > 0);
    --curIdx;
    curCmd = mu::takeAt(list, curIdx);
    stateList.erase(stateList.begin() + curIdx);
    for (auto i : curCmd->commands()) {
        LOG_UNDO() << "   " << i->name();
    }
}

//---------------------------------------------------------
//   setClean
//---------------------------------------------------------

void UndoStack::setClean()
{
    cleanState = state();
}

//---------------------------------------------------------
//   undo
//---------------------------------------------------------

void UndoStack::undo(EditData* ed)
{
    LOG_UNDO() << "called";
    // Are we currently editing text?
    if (ed && ed->element && ed->element->isTextBase()) {
        TextEditData* ted = static_cast<TextEditData*>(ed->getData(ed->element).get());
        if (ted && ted->startUndoIdx == curIdx) {
            // No edits to undo, so do nothing
            return;
        }
    }
    if (curIdx) {
        --curIdx;
        assert(curIdx < list.size());
        list[curIdx]->undo(ed);
    }
}

//---------------------------------------------------------
//   redo
//---------------------------------------------------------

void UndoStack::redo(EditData* ed)
{
    LOG_UNDO() << "called";
    if (canRedo()) {
        list[curIdx++]->redo(ed);
    }
}

//---------------------------------------------------------
//   UndoMacro
//---------------------------------------------------------

bool UndoMacro::canRecordSelectedElement(const EngravingItem* e)
{
    return e->isNote() || (e->isChordRest() && !e->isChord()) || (e->isTextBase() && !e->isInstrumentName()) || e->isFretDiagram();
}

void UndoMacro::fillSelectionInfo(SelectionInfo& info, const Selection& sel)
{
    info.staffStart = info.staffEnd = mu::nidx;
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
    } else if (info.staffStart != mu::nidx) {
        sel.setRangeTicks(info.tickStart, info.tickEnd, info.staffStart, info.staffEnd);
    }
}

UndoMacro::UndoMacro(Score* s)
    : m_undoInputState(s->inputState()), m_score(s)
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
    appendChildren(&other);
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

const UndoMacro::SelectionInfo& UndoMacro::undoSelectionInfo() const
{
    return m_undoSelectionInfo;
}

const UndoMacro::SelectionInfo& UndoMacro::redoSelectionInfo() const
{
    return m_redoSelectionInfo;
}

std::unordered_set<ElementType> UndoMacro::changedTypes() const
{
    std::unordered_set<ElementType> result;

    for (const UndoCommand* command : commands()) {
        for (const EngravingObject* object : command->objectItems()) {
            if (!object) {
                continue;
            }

            result.insert(object->type());
        }
    }

    return result;
}

//---------------------------------------------------------
//   CloneVoice
//---------------------------------------------------------

CloneVoice::CloneVoice(Segment* _sf, const Fraction& _lTick, Segment* _d, track_idx_t _strack, track_idx_t _dtrack, track_idx_t _otrack,
                       bool _linked)
{
    sf      = _sf;            // first source segment
    lTick   = _lTick;         // last tick to clone
    d       = _d;             // first destination segment
    strack  = _strack;
    dtrack  = _dtrack;
    otrack  = _otrack;        // old source track if -1 delete voice in strack after copy
    linked  = _linked;        // if true  add elements in destination segment only
                              // if false add elements in every linked staff
}

void CloneVoice::undo(EditData*)
{
    Score* s = d->score();
    Fraction ticks = d->tick() + lTick - sf->tick();
    track_idx_t sTrack = otrack == mu::nidx ? dtrack : otrack;   // use the correct source / destination if deleting the source
    track_idx_t dTrack = otrack == mu::nidx ? strack : dtrack;

    // Clear destination voice (in case of not linked and otrack = -1 we would delete our source
    if (otrack != mu::nidx && linked) {
        for (Segment* seg = d; seg && seg->tick() < ticks; seg = seg->next1()) {
            EngravingItem* el = seg->element(dTrack);
            if (el && el->isChordRest()) {
                el->unlink();
                seg->setElement(dTrack, 0);
            }
        }
    }

    if (otrack == mu::nidx && !linked) {
        // On the first run get going the undo redo action for adding/deleting elements and slurs
        if (first) {
            s->cloneVoice(sTrack, dTrack, sf, ticks, linked);
            auto spanners = s->spannerMap().findOverlapping(sf->tick().ticks(), lTick.ticks());
            for (auto i = spanners.begin(); i < spanners.end(); i++) {
                Spanner* sp = i->value;
                if (sp->isSlur() && (sp->track() == sTrack || sp->track2() == sTrack)) {
                    s->undoRemoveElement(sp);
                }
            }
            for (Segment* seg = d; seg && seg->tick() < ticks; seg = seg->next1()) {
                EngravingItem* el = seg->element(sTrack);
                if (el && el->isChordRest()) {
                    s->undoRemoveElement(el);
                }
            }
        }
        // Set rests if first voice in a staff
        if (!(sTrack % VOICES)) {
            s->setRest(d->tick(), sTrack, ticks, false, 0);
        }
    } else {
        s->cloneVoice(sTrack, dTrack, sf, ticks, linked);
        if (!linked && !(dTrack % VOICES)) {
            s->setRest(d->tick(), dTrack, ticks, false, 0);
        }
    }

    first = false;
}

void CloneVoice::redo(EditData*)
{
    Score* s = d->score();
    Fraction ticks = d->tick() + lTick - sf->tick();

    // Clear destination voice (in case of not linked and otrack = -1 we would delete our source
    if (otrack != mu::nidx && linked) {
        for (Segment* seg = d; seg && seg->tick() < ticks; seg = seg->next1()) {
            EngravingItem* el = seg->element(dtrack);
            if (el && el->isChordRest()) {
                el->unlink();
                seg->setElement(dtrack, 0);
            }
        }
    }

    if (otrack == mu::nidx && !linked) {
        // On the first run get going the undo redo action for adding/deleting elements and slurs
        if (first) {
            s->cloneVoice(strack, dtrack, sf, ticks, linked);
            auto spanners = s->spannerMap().findOverlapping(sf->tick().ticks(), lTick.ticks());
            for (auto i = spanners.begin(); i < spanners.end(); i++) {
                Spanner* sp = i->value;
                if (sp->isSlur() && (sp->track() == strack || sp->track2() == strack)) {
                    s->undoRemoveElement(sp);
                }
            }
            for (Segment* seg = d; seg && seg->tick() < ticks; seg = seg->next1()) {
                EngravingItem* el = seg->element(strack);
                if (el && el->isChordRest()) {
                    s->undoRemoveElement(el);
                }
            }
        }
        // Set rests if first voice in a staff
        if (!(strack % VOICES)) {
            s->setRest(d->tick(), strack, ticks, false, 0);
        }
    } else {
        s->cloneVoice(strack, dtrack, sf, ticks, linked, first);
    }
}

//---------------------------------------------------------
//   AddElement
//---------------------------------------------------------

AddElement::AddElement(EngravingItem* e)
{
    element = e;
}

//---------------------------------------------------------
//   AddElement::cleanup
//---------------------------------------------------------

void AddElement::cleanup(bool undo)
{
    if (!undo) {
        delete element;
        element = 0;
    }
}

//---------------------------------------------------------
//   undoRemoveTuplet
//---------------------------------------------------------

static void undoRemoveTuplet(DurationElement* cr)
{
    if (cr->tuplet()) {
        cr->tuplet()->remove(cr);
        if (cr->tuplet()->elements().empty()) {
            undoRemoveTuplet(cr->tuplet());
        }
    }
}

//---------------------------------------------------------
//   undoAddTuplet
//---------------------------------------------------------

static void undoAddTuplet(DurationElement* cr)
{
    if (cr->tuplet()) {
        cr->tuplet()->add(cr);
        if (cr->tuplet()->elements().size() == 1) {
            undoAddTuplet(cr->tuplet());
        }
    }
}

//---------------------------------------------------------
//   endUndoRedo
//---------------------------------------------------------

void AddElement::endUndoRedo(bool isUndo) const
{
    if (element->isChordRest()) {
        if (isUndo) {
            undoRemoveTuplet(toChordRest(element));
        } else {
            undoAddTuplet(toChordRest(element));
        }
    } else if (element->isClef()) {
        element->triggerLayout();
        element->score()->setLayout(element->staff()->nextClefTick(element->tick()), element->staffIdx());
    } else if (element->isKeySig()) {
        element->triggerLayout();
        element->score()->setLayout(element->staff()->nextKeyTick(element->tick()), element->staffIdx());
    }
}

//---------------------------------------------------------
//   undo
//---------------------------------------------------------

void AddElement::undo(EditData*)
{
    if (!element->isTuplet()) {
        element->score()->removeElement(element);
    }
    endUndoRedo(true);
}

//---------------------------------------------------------
//   redo
//---------------------------------------------------------

void AddElement::redo(EditData*)
{
    if (!element->isTuplet()) {
        element->score()->addElement(element);
    }
    endUndoRedo(false);
}

//---------------------------------------------------------
//   name
//---------------------------------------------------------

const char* AddElement::name() const
{
    static char buffer[64];
    if (element->isTextBase()) {
        snprintf(buffer, 64, "Add:    %s <%s> %p", element->typeName(),
                 muPrintable(toTextBase(element)->plainText()), element);
    } else if (element->isSegment()) {
        snprintf(buffer, 64, "Add:    <%s-%s> %p", element->typeName(), toSegment(element)->subTypeName(), element);
    } else {
        snprintf(buffer, 64, "Add:    <%s> %p", element->typeName(), element);
    }
    return buffer;
}

//---------------------------------------------------------
//   AddElement::isFiltered
//---------------------------------------------------------

bool AddElement::isFiltered(UndoCommand::Filter f, const EngravingItem* target) const
{
    using Filter = UndoCommand::Filter;
    switch (f) {
    case Filter::AddElement:
        return target == element;
    case Filter::AddElementLinked:
        return mu::contains(target->linkList(), static_cast<EngravingObject*>(element));
    default:
        break;
    }
    return false;
}

//---------------------------------------------------------
//   removeNote
//    Helper function for RemoveElement class
//---------------------------------------------------------

static void removeNote(const Note* note)
{
    Score* score = note->score();
    if (note->tieFor() && note->tieFor()->endNote()) {
        score->undo(new RemoveElement(note->tieFor()));
    }
    if (note->tieBack()) {
        score->undo(new RemoveElement(note->tieBack()));
    }
    for (Spanner* s : note->spannerBack()) {
        score->undo(new RemoveElement(s));
    }
    for (Spanner* s : note->spannerFor()) {
        score->undo(new RemoveElement(s));
    }
}

//---------------------------------------------------------
//   RemoveElement
//---------------------------------------------------------

RemoveElement::RemoveElement(EngravingItem* e)
{
    element = e;

    Score* score = element->score();
    if (element->isChordRest()) {
        ChordRest* cr = toChordRest(element);
        if (cr->tuplet() && cr->tuplet()->elements().size() <= 1) {
            score->undo(new RemoveElement(cr->tuplet()));
        }
        if (e->isChord()) {
            Chord* chord = toChord(e);
            // remove tremolo between 2 notes
            if (chord->tremolo()) {
                Tremolo* tremolo = chord->tremolo();
                if (tremolo->twoNotes()) {
                    score->undo(new RemoveElement(tremolo));
                }
            }
            for (const Note* note : chord->notes()) {
                removeNote(note);
            }
        }
    } else if (element->isNote()) {
        // Removing an individual note within a chord
        const Note* note = toNote(element);
        removeNote(note);
    }
}

//---------------------------------------------------------
//   RemoveElement::cleanup
//---------------------------------------------------------

void RemoveElement::cleanup(bool undo)
{
    if (undo) {
        delete element;
        element = 0;
    }
}

//---------------------------------------------------------
//   undo
//---------------------------------------------------------

void RemoveElement::undo(EditData*)
{
    if (!element->isTuplet()) {
        element->score()->addElement(element);
    }
    if (element->isChordRest()) {
        if (element->isChord()) {
            Chord* chord = toChord(element);
            for (Note* note : chord->notes()) {
                note->connectTiedNotes();
            }
        }
        undoAddTuplet(toChordRest(element));
    } else if (element->isClef()) {
        element->score()->setLayout(element->staff()->nextClefTick(element->tick()), element->staffIdx());
    } else if (element->isKeySig()) {
        element->score()->setLayout(element->staff()->nextKeyTick(element->tick()), element->staffIdx());
    }
}

//---------------------------------------------------------
//   redo
//---------------------------------------------------------

void RemoveElement::redo(EditData*)
{
    if (!element->isTuplet()) {
        element->score()->removeElement(element);
    }
    if (element->isChordRest()) {
        undoRemoveTuplet(toChordRest(element));
        if (element->isChord()) {
            Chord* chord = toChord(element);
            for (Note* note : chord->notes()) {
                note->disconnectTiedNotes();
            }
        }
    } else if (element->isClef()) {
        element->score()->setLayout(element->staff()->nextClefTick(element->tick()), element->staffIdx());
    } else if (element->isKeySig()) {
        element->score()->setLayout(element->staff()->nextKeyTick(element->tick()), element->staffIdx());
    }
}

//---------------------------------------------------------
//   name
//---------------------------------------------------------

const char* RemoveElement::name() const
{
    static char buffer[64];
    if (element->isTextBase()) {
        snprintf(buffer, 64, "Remove: %s <%s> %p", element->typeName(),
                 muPrintable(toTextBase(element)->plainText()), element);
    } else if (element->isSegment()) {
        snprintf(buffer, 64, "Remove: <%s-%s> %p", element->typeName(), toSegment(element)->subTypeName(), element);
    } else {
        snprintf(buffer, 64, "Remove: %s %p", element->typeName(), element);
    }
    return buffer;
}

//---------------------------------------------------------
//   RemoveElement::isFiltered
//---------------------------------------------------------

bool RemoveElement::isFiltered(UndoCommand::Filter f, const EngravingItem* target) const
{
    using Filter = UndoCommand::Filter;
    switch (f) {
    case Filter::RemoveElement:
        return target == element;
    case Filter::RemoveElementLinked:
        return mu::contains(target->linkList(), static_cast<EngravingObject*>(element));
    default:
        break;
    }
    return false;
}

//---------------------------------------------------------
//   InsertPart
//---------------------------------------------------------

InsertPart::InsertPart(Part* p, int i)
{
    part = p;
    idx  = i;
}

void InsertPart::undo(EditData*)
{
    part->score()->removePart(part);
}

void InsertPart::redo(EditData*)
{
    part->score()->insertPart(part, idx);
}

//---------------------------------------------------------
//   RemovePart
//---------------------------------------------------------

RemovePart::RemovePart(Part* p, staff_idx_t i)
{
    part = p;
    idx  = i;
}

void RemovePart::undo(EditData*)
{
    part->score()->insertPart(part, idx);
}

void RemovePart::redo(EditData*)
{
    part->score()->removePart(part);
}

//---------------------------------------------------------
//   SetSoloist
//---------------------------------------------------------

SetSoloist::SetSoloist(Part* p, bool b)
{
    part = p;
    soloist  = b;
}

void SetSoloist::undo(EditData*)
{
    part->setSoloist(!soloist);
}

void SetSoloist::redo(EditData*)
{
    part->setSoloist(soloist);
}

//---------------------------------------------------------
//   InsertStaff
//---------------------------------------------------------

InsertStaff::InsertStaff(Staff* p, staff_idx_t _ridx)
{
    staff = p;
    ridx  = _ridx;
}

void InsertStaff::undo(EditData*)
{
    staff->score()->removeStaff(staff);
}

void InsertStaff::redo(EditData*)
{
    staff->score()->insertStaff(staff, ridx);
}

//---------------------------------------------------------
//   RemoveStaff
//---------------------------------------------------------

RemoveStaff::RemoveStaff(Staff* p)
{
    staff = p;
    ridx  = staff->rstaff();
}

void RemoveStaff::undo(EditData*)
{
    staff->score()->insertStaff(staff, ridx);
}

void RemoveStaff::redo(EditData*)
{
    staff->score()->removeStaff(staff);
}

//---------------------------------------------------------
//   InsertMStaff
//---------------------------------------------------------

InsertMStaff::InsertMStaff(Measure* m, MStaff* ms, staff_idx_t i)
{
    measure = m;
    mstaff  = ms;
    idx     = i;
}

void InsertMStaff::undo(EditData*)
{
    measure->removeMStaff(mstaff, idx);
}

void InsertMStaff::redo(EditData*)
{
    measure->insertMStaff(mstaff, idx);
}

//---------------------------------------------------------
//   RemoveMStaff
//---------------------------------------------------------

RemoveMStaff::RemoveMStaff(Measure* m, MStaff* ms, int i)
{
    measure = m;
    mstaff  = ms;
    idx     = i;
}

void RemoveMStaff::undo(EditData*)
{
    measure->insertMStaff(mstaff, idx);
}

void RemoveMStaff::redo(EditData*)
{
    measure->removeMStaff(mstaff, idx);
}

//---------------------------------------------------------
//   SortStaves
//---------------------------------------------------------

SortStaves::SortStaves(Score* s, const std::vector<staff_idx_t>& l)
{
    score = s;

    for (staff_idx_t i = 0; i < l.size(); i++) {
        rlist.push_back(mu::indexOf(l, i));
    }
    list  = l;
}

void SortStaves::redo(EditData*)
{
    score->sortStaves(list);
}

void SortStaves::undo(EditData*)
{
    score->sortStaves(rlist);
}

//---------------------------------------------------------
//   MapExcerptTracks
//---------------------------------------------------------

MapExcerptTracks::MapExcerptTracks(Score* s, const std::vector<staff_idx_t>& l)
{
    score = s;

    /*
     *    In list l [x] represents the previous index of the staffIdx x.
     *    If the a staff x is a newly added staff, l[x] = -1.
     *    For the "undo" all staves which value -1 are *not* remapped since
     *    it is assumed this staves are removed later.
     */
    staff_idx_t maxIndex = 0;
    for (staff_idx_t i : l) {
        maxIndex = std::max(i, maxIndex);
    }

    for (staff_idx_t i = 0; i <= maxIndex; ++i) {
        rlist.push_back(mu::nidx);
    }

    for (staff_idx_t i = 0; i < l.size(); ++i) {
        if (l[i] != mu::nidx) {
            rlist[l[i]] = i;
        }
    }
    list = l;
}

void MapExcerptTracks::redo(EditData*)
{
    score->mapExcerptTracks(list);
}

void MapExcerptTracks::undo(EditData*)
{
    score->mapExcerptTracks(rlist);
}

//---------------------------------------------------------
//   ChangePitch
//---------------------------------------------------------

ChangePitch::ChangePitch(Note* _note, int _pitch, int _tpc1, int _tpc2)
{
    note  = _note;
    pitch = _pitch;
    tpc1  = _tpc1;
    tpc2  = _tpc2;
}

void ChangePitch::flip(EditData*)
{
    int f_pitch = note->pitch();
    int f_tpc1  = note->tpc1();
    int f_tpc2  = note->tpc2();
    // do not change unless necessary
    if (f_pitch == pitch && f_tpc1 == tpc1 && f_tpc2 == tpc2) {
        return;
    }

    note->setPitch(pitch, tpc1, tpc2);
    pitch = f_pitch;
    tpc1  = f_tpc1;
    tpc2  = f_tpc2;

    note->triggerLayout();
}

//---------------------------------------------------------
//   ChangeFretting
//
//    To use with tablatures to force a specific note fretting;
//    Pitch, string and fret must be changed all together; otherwise,
//    if they are not consistent among themselves, the refretting algorithm may re-assign
//    fret and string numbers for (potentially) all the notes of all the chords of a segment.
//---------------------------------------------------------

ChangeFretting::ChangeFretting(Note* _note, int _pitch, int _string, int _fret, int _tpc1, int _tpc2)
{
    note  = _note;
    pitch = _pitch;
    string= _string;
    fret  = _fret;
    tpc1  = _tpc1;
    tpc2  = _tpc2;
}

void ChangeFretting::flip(EditData*)
{
    int f_pitch = note->pitch();
    int f_string= note->string();
    int f_fret  = note->fret();
    int f_tpc1  = note->tpc1();
    int f_tpc2  = note->tpc2();
    // do not change unless necessary
    if (f_pitch == pitch && f_string == string && f_fret == fret && f_tpc1 == tpc1 && f_tpc2 == tpc2) {
        return;
    }

    note->setPitch(pitch, tpc1, tpc2);
    note->setString(string);
    note->setFret(fret);
    pitch = f_pitch;
    string= f_string;
    fret  = f_fret;
    tpc1  = f_tpc1;
    tpc2  = f_tpc2;
    note->triggerLayout();
}

//---------------------------------------------------------
//   ChangeElement
//---------------------------------------------------------

ChangeElement::ChangeElement(EngravingItem* oe, EngravingItem* ne)
{
    oldElement = oe;
    newElement = ne;
}

void ChangeElement::flip(EditData*)
{
    const LinkedObjects* links = oldElement->links();
    if (links) {
        newElement->linkTo(oldElement);
        oldElement->unlink();
    }

    Score* score = oldElement->score();
    if (!score->selection().isRange()) {
        if (oldElement->selected()) {
            score->deselect(oldElement);
        }
        if (newElement->selected()) {
            score->select(newElement, SelectType::ADD);
        }
    }
    if (oldElement->explicitParent() == 0) {
        score->removeElement(oldElement);
        score->addElement(newElement);
    } else {
        oldElement->parentItem()->change(oldElement, newElement);
    }

    if (newElement->isKeySig()) {
        KeySig* ks = toKeySig(newElement);
        if (!ks->generated()) {
            ks->staff()->setKey(ks->tick(), ks->keySigEvent());
        }
    } else if (newElement->isDynamic()) {
        newElement->score()->addLayoutFlags(LayoutFlag::FIX_PITCH_VELO);
    } else if (newElement->isTempoText()) {
        TempoText* t = toTempoText(oldElement);
        score->setTempo(t->segment(), t->tempo());
    }
//      if (newElement->isSegmentFlag()) {
    if (newElement->isSpannerSegment()) {
        SpannerSegment* os = toSpannerSegment(oldElement);
        SpannerSegment* ns = toSpannerSegment(newElement);
        if (os->system()) {
            os->system()->remove(os);
        }
        if (ns->system()) {
            ns->system()->add(ns);
        }
    }
    std::swap(oldElement, newElement);
    oldElement->triggerLayout();
    newElement->triggerLayout();
    // score->setLayoutAll();
}

//---------------------------------------------------------
//   InsertStaves
//---------------------------------------------------------

InsertStaves::InsertStaves(Measure* m, staff_idx_t _a, staff_idx_t _b)
{
    measure = m;
    a       = _a;
    b       = _b;
}

void InsertStaves::undo(EditData*)
{
    measure->removeStaves(a, b);
}

void InsertStaves::redo(EditData*)
{
    measure->insertStaves(a, b);
}

//---------------------------------------------------------
//   RemoveStaves
//---------------------------------------------------------

RemoveStaves::RemoveStaves(Measure* m, staff_idx_t _a, staff_idx_t _b)
{
    measure = m;
    a       = _a;
    b       = _b;
}

void RemoveStaves::undo(EditData*)
{
    measure->insertStaves(a, b);
}

void RemoveStaves::redo(EditData*)
{
    measure->removeStaves(a, b);
}

//---------------------------------------------------------
//   ChangeKeySig
//---------------------------------------------------------

ChangeKeySig::ChangeKeySig(KeySig* k, KeySigEvent newKeySig, bool sc, bool addEvtToStaff)
    : keysig(k), ks(newKeySig), showCourtesy(sc), evtInStaff(addEvtToStaff)
{}

void ChangeKeySig::flip(EditData*)
{
    Segment* segment = keysig->segment();
    Fraction tick = segment->tick();
    Staff* staff = keysig->staff();

    const bool curEvtInStaff = (staff->currentKeyTick(tick) == tick);
    KeySigEvent curKey = keysig->keySigEvent();
    const bool curShowCourtesy = keysig->showCourtesy();

    keysig->setKeySigEvent(ks);
    keysig->setShowCourtesy(showCourtesy);

    // Add/remove the corresponding key events, if appropriate.
    if (evtInStaff) {
        staff->setKey(tick, ks);     // replace
    } else if (curEvtInStaff) {
        staff->removeKey(tick);     // if nothing to add instead, just remove.
    }
    // If no keysig event corresponds to the key signature then this keysig
    // is probably generated. Otherwise it is probably added manually.
    // Set segment flags according to this, layout will change it if needed.
    segment->setEnabled(evtInStaff);
    segment->setHeader(!evtInStaff && segment->rtick() == Fraction(0, 1));

    showCourtesy = curShowCourtesy;
    ks           = curKey;
    evtInStaff   = curEvtInStaff;
    keysig->triggerLayout();
    keysig->score()->setLayout(keysig->staff()->nextKeyTick(tick), keysig->staffIdx());
}

//---------------------------------------------------------
//   ChangeMeasureLen
//---------------------------------------------------------

ChangeMeasureLen::ChangeMeasureLen(Measure* m, Fraction l)
{
    measure     = m;
    len         = l;
}

void ChangeMeasureLen::flip(EditData*)
{
    Fraction oLen = measure->ticks();

    //
    // move EndBarLine and TimeSigAnnounce
    // to end of measure:
    //

    std::list<Segment*> sl;
    for (Segment* s = measure->first(); s; s = s->next()) {
        if (!s->isEndBarLineType() && !s->isTimeSigAnnounceType()) {
            continue;
        }
        s->setRtick(len);
        sl.push_back(s);
        measure->remove(s);
    }
    measure->setTicks(len);
    measure->score()->setUpTempoMap();
    len = oLen;
}

//---------------------------------------------------------
//   TransposeHarmony
//---------------------------------------------------------

TransposeHarmony::TransposeHarmony(Harmony* h, int rtpc, int btpc)
{
    harmony = h;
    rootTpc = rtpc;
    baseTpc = btpc;
}

void TransposeHarmony::flip(EditData*)
{
    harmony->realizedHarmony().setDirty(true);   //harmony should be re-realized after transposition
    int baseTpc1 = harmony->baseTpc();
    int rootTpc1 = harmony->rootTpc();
    harmony->setBaseTpc(baseTpc);
    harmony->setRootTpc(rootTpc);
    harmony->setXmlText(harmony->harmonyName());
    harmony->render();
    rootTpc = rootTpc1;
    baseTpc = baseTpc1;
}

//---------------------------------------------------------
//   ExchangeVoice
//---------------------------------------------------------

ExchangeVoice::ExchangeVoice(Measure* m, track_idx_t _val1, track_idx_t _val2, staff_idx_t _staff)
{
    measure = m;
    val1    = _val1;
    val2    = _val2;
    staff   = _staff;
}

void ExchangeVoice::undo(EditData*)
{
    measure->exchangeVoice(val2, val1, staff);
    measure->checkMultiVoices(staff);
}

void ExchangeVoice::redo(EditData*)
{
    measure->exchangeVoice(val1, val2, staff);
}

//---------------------------------------------------------
//   ChangeInstrumentShort
//---------------------------------------------------------

ChangeInstrumentShort::ChangeInstrumentShort(const Fraction& _tick, Part* p, std::list<StaffName> t)
{
    tick = _tick;
    part = p;
    text = t;
}

void ChangeInstrumentShort::flip(EditData*)
{
    std::list<StaffName> s = part->shortNames(tick);
    part->setShortNames(text, tick);
    text = s;
    part->score()->setLayoutAll();
}

//---------------------------------------------------------
//   ChangeInstrumentLong
//---------------------------------------------------------

ChangeInstrumentLong::ChangeInstrumentLong(const Fraction& _tick, Part* p, std::list<StaffName> t)
{
    tick = _tick;
    part = p;
    text = t;
}

void ChangeInstrumentLong::flip(EditData*)
{
    std::list<StaffName> s = part->longNames(tick);
    part->setLongNames(text, tick);
    text = s;
    part->score()->setLayoutAll();
}

//---------------------------------------------------------
//   EditText::undo
//---------------------------------------------------------

void EditText::undo(EditData*)
{
/*      if (!text->styled()) {
            for (int i = 0; i < undoLevel; ++i)
                  text->undo();
            }
      */
    undoRedo();
}

//---------------------------------------------------------
//   EditText::redo
//---------------------------------------------------------

void EditText::redo(EditData*)
{
/*
      if (!text->styled()) {
            for (int i = 0; i < undoLevel; ++i)
                  text->redo();
            }
      */
    undoRedo();
}

//---------------------------------------------------------
//   EditText::undoRedo
//---------------------------------------------------------

void EditText::undoRedo()
{
    String s = text->xmlText();
    text->setXmlText(oldText);
    oldText = s;
    text->triggerLayout();
}

//---------------------------------------------------------
//   ChangePatch
//---------------------------------------------------------

void ChangePatch::flip(EditData*)
{
    MidiPatch op;
    op.prog          = channel->program();
    op.bank          = channel->bank();
    op.synti         = channel->synti();

    channel->setProgram(patch.prog);
    channel->setBank(patch.bank);
    channel->setSynti(patch.synti);

    patch            = op;

    //! TODO Needs porting for MU4
//    if (MScore::seq == 0) {
//        LOGW("no seq");
//        return;
//    }
//
//    NPlayEvent event;
//    event.setType(ME_CONTROLLER);
//    event.setChannel(channel->channel());
//
//    int hbank = (channel->bank() >> 7) & 0x7f;
//    int lbank = channel->bank() & 0x7f;
//
//    event.setController(CTRL_HBANK);
//    event.setValue(hbank);
//    MScore::seq->sendEvent(event);
//
//    event.setController(CTRL_LBANK);
//    event.setValue(lbank);
//    MScore::seq->sendEvent(event);
//
//    event.setController(CTRL_PROGRAM);
//    event.setValue(channel->program());
//
    score->setInstrumentsChanged(true);
//
//    MScore::seq->sendEvent(event);
}

//---------------------------------------------------------
//   SetUserBankController
//---------------------------------------------------------

void SetUserBankController::flip(EditData*)
{
    bool oldVal = channel->userBankController();
    channel->setUserBankController(val);
    val = oldVal;
}

//---------------------------------------------------------
//   ChangeStaff
//---------------------------------------------------------

ChangeStaff::ChangeStaff(Staff* _staff)
    : staff(_staff)
{
    visible = staff->visible();
    clefType = staff->defaultClefType();
    userDist = staff->userDist();
    hideMode = staff->hideWhenEmpty();
    showIfEmpty = staff->showIfEmpty();
    cutaway = staff->cutaway();
    hideSystemBarLine = staff->hideSystemBarLine();
    mergeMatchingRests = staff->mergeMatchingRests();
}

ChangeStaff::ChangeStaff(Staff* _staff, bool _visible, ClefTypeList _clefType,
                         double _userDist, Staff::HideMode _hideMode, bool _showIfEmpty, bool _cutaway,
                         bool _hideSystemBarLine, bool _mergeMatchingRests)
{
    staff       = _staff;
    visible     = _visible;
    clefType    = _clefType;
    userDist    = _userDist;
    hideMode    = _hideMode;
    showIfEmpty = _showIfEmpty;
    cutaway     = _cutaway;
    hideSystemBarLine  = _hideSystemBarLine;
    mergeMatchingRests = _mergeMatchingRests;
}

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeStaff::flip(EditData*)
{
    bool oldVisible = staff->visible();
    ClefTypeList oldClefType = staff->defaultClefType();
    double oldUserDist   = staff->userDist();
    Staff::HideMode oldHideMode    = staff->hideWhenEmpty();
    bool oldShowIfEmpty = staff->showIfEmpty();
    bool oldCutaway     = staff->cutaway();
    bool oldHideSystemBarLine  = staff->hideSystemBarLine();
    bool oldMergeMatchingRests = staff->mergeMatchingRests();

    staff->setVisible(visible);
    staff->setDefaultClefType(clefType);
    staff->setUserDist(Millimetre(userDist));
    staff->setHideWhenEmpty(hideMode);
    staff->setShowIfEmpty(showIfEmpty);
    staff->setCutaway(cutaway);
    staff->setHideSystemBarLine(hideSystemBarLine);
    staff->setMergeMatchingRests(mergeMatchingRests);

    visible     = oldVisible;
    clefType    = oldClefType;
    userDist    = oldUserDist;
    hideMode    = oldHideMode;
    showIfEmpty = oldShowIfEmpty;
    cutaway     = oldCutaway;
    hideSystemBarLine  = oldHideSystemBarLine;
    mergeMatchingRests = oldMergeMatchingRests;

    staff->triggerLayout();
    staff->masterScore()->rebuildMidiMapping();
    staff->score()->setPlaylistDirty();
}

//---------------------------------------------------------
//   ChangeStaffType::flip
//---------------------------------------------------------

void ChangeStaffType::flip(EditData*)
{
    StaffType oldStaffType = *staff->staffType(Fraction(0, 1));        // TODO

    staff->setStaffType(Fraction(0, 1), staffType);

    bool invisibleChanged = oldStaffType.invisible() != staffType.invisible();

    staffType = oldStaffType;

    Score* score = staff->score();
    if (invisibleChanged) {
        staff_idx_t staffIdx = staff->idx();
        for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
            m->staffLines(staffIdx)->setVisible(!staff->isLinesInvisible(Fraction(0, 1)));
        }
    }

    staff->triggerLayout();
}

//---------------------------------------------------------
//   ChangePart
//---------------------------------------------------------

ChangePart::ChangePart(Part* _part, Instrument* i, const String& s)
{
    instrument = i;
    part       = _part;
    partName   = s;
}

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangePart::flip(EditData*)
{
    Instrument* oi = part->instrument(); //tick?
    String s      = part->partName();
    part->setInstrument(instrument);
    part->setPartName(partName);

    part->updateHarmonyChannels(false);

    Score* score = part->score();
    score->masterScore()->rebuildMidiMapping();
    score->setInstrumentsChanged(true);
    score->setPlaylistDirty();

    // check if notes need to be updated
    // true if changing into or away from TAB or from one TAB type to another

    score->setLayoutAll();

    partName   = s;
    instrument = oi;
}

//---------------------------------------------------------
//   ChangeStyle
//---------------------------------------------------------

ChangeStyle::ChangeStyle(Score* s, const MStyle& st, const bool overlapOnly)
    : score(s), style(st), overlap(overlapOnly)
{
}

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeStyle::flip(EditData*)
{
    MStyle tmp = score->style();

    if (score->styleV(Sid::concertPitch) != style.value(Sid::concertPitch)) {
        score->cmdConcertPitchChanged(style.value(Sid::concertPitch).toBool());
    }
    if (score->styleV(Sid::MusicalSymbolFont) != style.value(Sid::MusicalSymbolFont)) {
        score->setScoreFont(ScoreFont::fontByName(style.styleSt(Sid::MusicalSymbolFont)));
    }

    score->setStyle(style, overlap);
    score->styleChanged();
    style = tmp;
}

void ChangeStyle::undo(EditData* ed)
{
    overlap = false;
    UndoCommand::undo(ed);
}

//---------------------------------------------------------
//   ChangeStyleVal::flip
//---------------------------------------------------------

void ChangeStyleVal::flip(EditData*)
{
    PropertyValue v = score->styleV(idx);
    if (v != value) {
        score->style().set(idx, value);
        switch (idx) {
        case Sid::chordExtensionMag:
        case Sid::chordExtensionAdjust:
        case Sid::chordModifierMag:
        case Sid::chordModifierAdjust:
        case Sid::chordDescriptionFile: {
            score->chordList()->unload();
            double emag = score->styleD(Sid::chordExtensionMag);
            double eadjust = score->styleD(Sid::chordExtensionAdjust);
            double mmag = score->styleD(Sid::chordModifierMag);
            double madjust = score->styleD(Sid::chordModifierAdjust);
            score->chordList()->configureAutoAdjust(emag, eadjust, mmag, madjust);
            if (score->styleB(Sid::chordsXmlFile)) {
                score->chordList()->read(u"chords.xml");
            }
            score->chordList()->read(score->styleSt(Sid::chordDescriptionFile));
            score->chordList()->setCustomChordList(score->styleSt(Sid::chordStyle) == "custom");
        }
        break;
        case Sid::spatium:
            score->spatiumChanged(v.toDouble(), value.toDouble());
            break;
        case Sid::defaultsVersion:
            score->style().setDefaultStyleVersion(value.toInt());
            break;
        default:
            break;
        }
        score->styleChanged();
    }
    value = v;
}

//---------------------------------------------------------
//   ChangePageNumberOffset::flip
//---------------------------------------------------------

void ChangePageNumberOffset::flip(EditData*)
{
    int po = score->pageNumberOffset();

    score->setPageNumberOffset(pageOffset);
    score->setLayoutAll();

    pageOffset = po;
}

//---------------------------------------------------------
//   ChangeChordStaffMove
//---------------------------------------------------------

ChangeChordStaffMove::ChangeChordStaffMove(ChordRest* cr, int v)
    : chordRest(cr), staffMove(v)
{
}

void ChangeChordStaffMove::flip(EditData*)
{
    int v = chordRest->staffMove();
    for (EngravingObject* e : chordRest->linkList()) {
        ChordRest* cr = toChordRest(e);
        cr->setStaffMove(staffMove);
        cr->triggerLayout();
    }
    staffMove = v;
}

//---------------------------------------------------------
//   ChangeVelocity
//---------------------------------------------------------

ChangeVelocity::ChangeVelocity(Note* n, VeloType t, int o)
    : note(n), veloType(t), veloOffset(o)
{
}

void ChangeVelocity::flip(EditData*)
{
    VeloType t = note->veloType();
    int o       = note->veloOffset();
    note->setVeloType(veloType);
    note->setVeloOffset(veloOffset);
    veloType   = t;
    veloOffset = o;
}

//---------------------------------------------------------
//   ChangeMStaffProperties
//---------------------------------------------------------

ChangeMStaffProperties::ChangeMStaffProperties(Measure* m, int i, bool v, bool s)
    : measure(m), staffIdx(i), visible(v), stemless(s)
{
}

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeMStaffProperties::flip(EditData*)
{
    bool v = measure->visible(staffIdx);
    bool s = measure->stemless(staffIdx);
    measure->setStaffVisible(staffIdx, visible);
    measure->setStaffStemless(staffIdx, stemless);
    visible    = v;
    stemless = s;
}

//---------------------------------------------------------
//   getCourtesyClefs
//    remember clefs at the end of previous measure
//---------------------------------------------------------

std::vector<Clef*> InsertRemoveMeasures::getCourtesyClefs(Measure* m)
{
    Score* score = m->score();
    std::vector<Clef*> startClefs;
    if (m->prev() && m->prev()->isMeasure()) {
        Measure* prevMeasure = toMeasure(m->prev());
        const Segment* clefSeg = prevMeasure->findSegmentR(SegmentType::Clef | SegmentType::HeaderClef, prevMeasure->ticks());
        if (clefSeg) {
            for (size_t st = 0; st < score->nstaves(); ++st) {
                EngravingItem* clef = clefSeg->element(staff2track(static_cast<int>(st)));
                if (clef && clef->isClef()) {
                    startClefs.push_back(toClef(clef));
                }
            }
        }
    }
    return startClefs;
}

//---------------------------------------------------------
//   insertMeasures
//---------------------------------------------------------

void InsertRemoveMeasures::insertMeasures()
{
    Score* score = fm->score();

    std::list<Clef*> clefs;
    std::vector<Clef*> prevMeasureClefs;
    std::list<KeySig*> keys;
    Segment* fs = nullptr;
    Segment* ls = nullptr;
    if (fm->isMeasure()) {
        score->setPlaylistDirty();
        fs = toMeasure(fm)->first();
        ls = toMeasure(lm)->last();
        for (Segment* s = fs; s && s != ls; s = s->next1()) {
            if (!s->enabled() || !(s->segmentType() & (SegmentType::Clef | SegmentType::HeaderClef | SegmentType::KeySig))) {
                continue;
            }
            for (track_idx_t track = 0; track < score->ntracks(); track += VOICES) {
                EngravingItem* e = s->element(track);
                if (!e || e->generated()) {
                    continue;
                }
                if (e->isClef()) {
                    clefs.push_back(toClef(e));
                } else if (e->isKeySig()) {
                    keys.push_back(toKeySig(e));
                }
            }
        }
        prevMeasureClefs = getCourtesyClefs(toMeasure(fm));
    }
    score->measures()->insert(fm, lm);

    if (fm->isMeasure()) {
        score->setUpTempoMap();
        score->insertTime(fm->tick(), lm->endTick() - fm->tick());

        // move ownership of Instrument back to part
        for (Segment* s = fs; s && s != ls; s = s->next1()) {
            for (EngravingItem* e : s->annotations()) {
                if (e->isInstrumentChange()) {
                    e->part()->setInstrument(toInstrumentChange(e)->instrument(), s->tick());
                }
            }
        }
        for (Clef* clef : prevMeasureClefs) {
            clef->staff()->setClef(clef);
        }
        for (Clef* clef : clefs) {
            clef->staff()->setClef(clef);
        }
        for (KeySig* key : keys) {
            key->staff()->setKey(key->segment()->tick(), key->keySigEvent());
        }
    }

    score->setLayoutAll();

    if (Measure* nextMeasure = lm->nextMeasure()) {
        for (staff_idx_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
            Staff* staff = score->staff(staffIdx);
            if (staff->isStaffTypeStartFrom(fm->tick())) {
                staff->moveStaffType(fm->tick(), nextMeasure->tick());

                for (auto el : nextMeasure->el()) {
                    if (el && el->isStaffTypeChange()) {
                        toStaffTypeChange(el)->setStaffType(staff->staffType(nextMeasure->tick()), false);
                    }
                }
            }
        }
    }

    //
    // connect ties
    //

    if (!fm->isMeasure() || !fm->prevMeasure()) {
        return;
    }
    Measure* m = fm->prevMeasure();
    for (Segment* seg = m->first(); seg; seg = seg->next()) {
        for (track_idx_t track = 0; track < score->ntracks(); ++track) {
            EngravingItem* e = seg->element(track);
            if (e == 0 || !e->isChord()) {
                continue;
            }
            Chord* chord = toChord(e);
            for (Note* n : chord->notes()) {
                Tie* tie = n->tieFor();
                if (!tie) {
                    continue;
                }
                if (!tie->endNote() || tie->endNote()->chord()->segment()->measure() != m) {
                    Note* nn = searchTieNote(n);
                    if (nn) {
                        tie->setEndNote(nn);
                        nn->setTieBack(tie);
                    }
                }
            }
        }
    }
}

//---------------------------------------------------------
//   removeMeasures
//---------------------------------------------------------

void InsertRemoveMeasures::removeMeasures()
{
    Score* score = fm->score();

    Fraction tick1 = fm->tick();
    Fraction tick2 = lm->endTick();

    std::list<System*> systemList;
    for (MeasureBase* mb = lm;; mb = mb->prev()) {
        System* system = mb->system();
        if (system) {
            if (!mu::contains(systemList, system)) {
                systemList.push_back(system);
            }
            system->removeMeasure(mb);
        }
        if (mb == fm) {
            break;
        }
    }

    if (Measure* nextMeasure = lm->nextMeasure()) {
        for (size_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
            Staff* staff = score->staff(staffIdx);
            if (staff->isStaffTypeStartFrom(nextMeasure->tick())) {
                staff->moveStaffType(nextMeasure->tick(), fm->tick());

                for (auto el : nextMeasure->el()) {
                    if (el && el->isStaffTypeChange()) {
                        toStaffTypeChange(el)->setStaffType(staff->staffType(fm->tick()), false);
                    }
                }
            }
        }
    }

    score->measures()->remove(fm, lm);

    score->setUpTempoMap();
    if (fm->isMeasure()) {
        score->setPlaylistDirty();

        // check if there is a clef at the end of last measure
        // remove clef from staff cleflist

        if (lm->isMeasure()) {
            Measure* m = toMeasure(lm);
            Segment* s = m->findSegment(SegmentType::Clef, tick2);
            if (s) {
                for (EngravingItem* e : s->elist()) {
                    Clef* clef = toClef(e);
                    if (clef) {
                        score->staff(clef->staffIdx())->removeClef(clef);
                    }
                }
            }
        }

        // remember clefs at the end of previous measure
        const auto clefs = getCourtesyClefs(toMeasure(fm));

        score->insertTime(tick1, -(tick2 - tick1));

        // Restore clefs that were backed up. Events for them could be lost
        // as a result of the recent insertTime() call.
        for (Clef* clef : clefs) {
            clef->staff()->setClef(clef);
        }

        for (Spanner* sp : score->unmanagedSpanners()) {
            if ((sp->tick() >= tick1 && sp->tick() < tick2) || (sp->tick2() >= tick1 && sp->tick2() < tick2)) {
                sp->removeUnmanaged();
            }
        }
        score->connectTies(true);       // ??
    }

    // remove empty systems

    for (System* s : systemList) {
        if (s->measures().empty()) {
            Page* page = s->page();
            if (page) {
                // erase system from page
                std::vector<System*>& sl = page->systems();
                auto i = std::find(sl.begin(), sl.end(), s);
                if (i != sl.end()) {
                    sl.erase(i);
                }
                // erase system from score
                auto k = std::find(score->systems().begin(), score->systems().end(), s);
                if (k != score->systems().end()) {
                    score->systems().erase(k);
                }
                // finally delete system
                score->deleteLater(s);
            }
        }
    }

    score->setLayoutAll();
}

AddExcerpt::AddExcerpt(Excerpt* ex)
    : excerpt(ex)
{}

AddExcerpt::~AddExcerpt()
{
    if (deleteExcerpt) {
        delete excerpt;
        excerpt = nullptr;
    }
}

//---------------------------------------------------------
//   AddExcerpt::undo
//---------------------------------------------------------

void AddExcerpt::undo(EditData*)
{
    deleteExcerpt = true;
    excerpt->masterScore()->removeExcerpt(excerpt);
}

//---------------------------------------------------------
//   AddExcerpt::redo
//---------------------------------------------------------

void AddExcerpt::redo(EditData*)
{
    deleteExcerpt = false;
    excerpt->masterScore()->addExcerpt(excerpt);
}

//---------------------------------------------------------
//   RemoveExcerpt
//---------------------------------------------------------

RemoveExcerpt::RemoveExcerpt(Excerpt* ex)
    : excerpt(ex)
{
    index = mu::indexOf(excerpt->masterScore()->excerpts(), excerpt);
}

RemoveExcerpt::~RemoveExcerpt()
{
    if (deleteExcerpt) {
        delete excerpt;
        excerpt = nullptr;
    }
}

//---------------------------------------------------------
//   RemoveExcerpt::undo()
//---------------------------------------------------------

void RemoveExcerpt::undo(EditData*)
{
    deleteExcerpt = false;
    excerpt->masterScore()->addExcerpt(excerpt, index);
}

//---------------------------------------------------------
//   RemoveExcerpt::redo()
//---------------------------------------------------------

void RemoveExcerpt::redo(EditData*)
{
    deleteExcerpt = true;
    excerpt->masterScore()->removeExcerpt(excerpt);
}

//---------------------------------------------------------
//   SwapExcerpt::flip
//---------------------------------------------------------

void SwapExcerpt::flip(EditData*)
{
    Excerpt* tmp = score->excerpts().at(pos1);
    score->excerpts()[pos1] = score->excerpts().at(pos2);
    score->excerpts()[pos2] = tmp;
    score->setExcerptsChanged(true);
}

//---------------------------------------------------------
//   ChangeExcerptTitle::flip
//---------------------------------------------------------

void ChangeExcerptTitle::flip(EditData*)
{
    String s = title;
    title = excerpt->name();
    excerpt->setName(s);
    excerpt->masterScore()->setExcerptsChanged(true);
}

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeBend::flip(EditData*)
{
    PitchValues pv = bend->points();
    bend->score()->addRefresh(bend->canvasBoundingRect());
    bend->setPoints(points);
    points = pv;
    bend->layout();
    bend->score()->addRefresh(bend->canvasBoundingRect());
}

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeTremoloBar::flip(EditData*)
{
    PitchValues pv = bend->points();
    bend->setPoints(points);
    points = pv;
}

//---------------------------------------------------------
//   ChangeNoteEvents::flip
//---------------------------------------------------------

void ChangeNoteEvents::flip(EditData*)
{
/*TODO:      std::list<NoteEvent*> e = chord->playEvents();
      chord->setPlayEvents(events);
      events = e;
      */
}

//---------------------------------------------------------
//   ChangeNoteEventList::flip
//---------------------------------------------------------

void ChangeNoteEventList::flip(EditData*)
{
    note->score()->setPlaylistDirty();
    // Get copy of current list.
    NoteEventList nel = note->playEvents();
    // Replace current copy with new list.
    note->setPlayEvents(newEvents);
    // Save copy of replaced list.
    newEvents = nel;
    // Get a copy of the current playEventType.
    PlayEventType petval = note->chord()->playEventType();
    // Replace current setting with new setting.
    note->chord()->setPlayEventType(newPetype);
    // Save copy of old setting.
    newPetype = petval;
}

//---------------------------------------------------------
//   ChangeNoteEventList::flip
//---------------------------------------------------------

void ChangeChordPlayEventType::flip(EditData*)
{
    chord->score()->setPlaylistDirty();
    // Flips data between NoteEventList's.
    size_t n = chord->notes().size();
    for (size_t i = 0; i < n; ++i) {
        Note* note = chord->notes()[i];
        note->playEvents().swap(events[int(i)]);
    }
    // Flips PlayEventType between chord and undo.
    PlayEventType curPetype = chord->playEventType();
    chord->setPlayEventType(petype);
    petype = curPetype;
}

//---------------------------------------------------------
//   ChangeInstrument::flip
//---------------------------------------------------------

void ChangeInstrument::flip(EditData*)
{
    Part* part = is->staff()->part();
    Fraction tickStart = is->segment()->tick();
    Instrument* oi = is->instrument();    //new Instrument(*is->instrument());

    // set instrument in both part and instrument change element
    is->setInstrument(instrument);        //*instrument
    part->setInstrument(instrument, tickStart);

    // update score
    is->masterScore()->rebuildMidiMapping();
    is->masterScore()->updateChannel();
    is->score()->setInstrumentsChanged(true);
    is->triggerLayoutAll();

    // remember original instrument
    instrument = oi;
}

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void SwapCR::flip(EditData*)
{
    Segment* s1 = cr1->segment();
    Segment* s2 = cr2->segment();
    track_idx_t track = cr1->track();

    if (cr1->isChord() && cr2->isChord() && toChord(cr1)->tremolo()
        && (toChord(cr1)->tremolo() == toChord(cr2)->tremolo())) {
        Tremolo* t = toChord(cr1)->tremolo();
        Chord* c1 = t->chord1();
        Chord* c2 = t->chord2();
        t->setParent(toChord(c2));
        t->setChords(toChord(c2), toChord(c1));
    }

    EngravingItem* cr = s1->element(track);
    s1->setElement(track, s2->element(track));
    s2->setElement(track, cr);
    cr1->score()->setLayout(s1->tick(), cr1->staffIdx(), cr1);
    cr1->score()->setLayout(s2->tick(), cr1->staffIdx(), cr1);
}

//---------------------------------------------------------
//   ChangeClefType
//---------------------------------------------------------

ChangeClefType::ChangeClefType(Clef* c, ClefType cl, ClefType tc)
{
    clef            = c;
    concertClef     = cl;
    transposingClef = tc;
}

//---------------------------------------------------------
//   ChangeClefType::flip
//---------------------------------------------------------

void ChangeClefType::flip(EditData*)
{
    ClefType ocl = clef->concertClef();
    ClefType otc = clef->transposingClef();

    clef->setConcertClef(concertClef);
    clef->setTransposingClef(transposingClef);

    clef->staff()->setClef(clef);
    Segment* segment = clef->segment();
    updateNoteLines(segment, clef->track());
    clef->triggerLayoutAll();        // TODO: reduce layout to clef range

    concertClef     = ocl;
    transposingClef = otc;
    // layout the clef to align the currentClefType with the actual one immediately
    clef->layout();
}

//---------------------------------------------------------
//   ChangeProperty::flip
//---------------------------------------------------------

void ChangeProperty::flip(EditData*)
{
    LOG_UNDO() << element->typeName() << int(id) << "(" << propertyName(id) << ")" << element->getProperty(id) << "->" << property;

    PropertyValue v       = element->getProperty(id);
    PropertyFlags ps = element->propertyFlags(id);

    element->setProperty(id, property);
    element->setPropertyFlags(id, flags);
    property = v;
    flags = ps;
}

//---------------------------------------------------------
//   ChangeBracketProperty::flip
//---------------------------------------------------------

void ChangeBracketProperty::flip(EditData* ed)
{
    if (!staff) {
        return;
    }

    element = staff->brackets()[level];
    ChangeProperty::flip(ed);
    level = toBracketItem(element)->column();
}

//---------------------------------------------------------
//   ChangeTextLineProperty::flip
//---------------------------------------------------------

void ChangeTextLineProperty::flip(EditData* ed)
{
    ChangeProperty::flip(ed);
    if (element->isTextLine()) {
        toTextLine(element)->initStyle();
    }
}

//---------------------------------------------------------
//   ChangeMetaText::flip
//---------------------------------------------------------

void ChangeMetaText::flip(EditData*)
{
    String s = score->metaTag(id);
    score->setMetaTag(id, text);
    text = s;
}

//---------------------------------------------------------
//   ChangeSynthesizerState::flip
//---------------------------------------------------------

void ChangeSynthesizerState::flip(EditData*)
{
    std::swap(state, score->_synthesizerState);
}

void AddBracket::redo(EditData*)
{
    staff->setBracketType(level, type);
    staff->setBracketSpan(level, span);
    staff->triggerLayout();
}

void AddBracket::undo(EditData*)
{
    staff->setBracketType(level, BracketType::NO_BRACKET);
    staff->triggerLayout();
}

void RemoveBracket::redo(EditData*)
{
    staff->setBracketType(level, BracketType::NO_BRACKET);
    staff->triggerLayout();
}

void RemoveBracket::undo(EditData*)
{
    staff->setBracketType(level, type);
    staff->setBracketSpan(level, span);
    staff->triggerLayout();
}

//---------------------------------------------------------
//   ChangeSpannerElements
//---------------------------------------------------------

void ChangeSpannerElements::flip(EditData*)
{
    EngravingItem* oldStartElement   = spanner->startElement();
    EngravingItem* oldEndElement     = spanner->endElement();
    if (spanner->anchor() == Spanner::Anchor::NOTE) {
        // be sure new spanner elements are of the right type
        if (!startElement || !startElement->isNote() || !endElement || !endElement->isNote()) {
            return;
        }
        Note* oldStartNote = toNote(oldStartElement);
        Note* oldEndNote = toNote(oldEndElement);
        Note* newStartNote = toNote(startElement);
        Note* newEndNote = toNote(endElement);
        // update spanner's start and end notes
        if (newStartNote && newEndNote) {
            spanner->setNoteSpan(newStartNote, newEndNote);
            if (spanner->isTie()) {
                Tie* tie = toTie(spanner);
                oldStartNote->setTieFor(nullptr);
                oldEndNote->setTieBack(nullptr);
                newStartNote->setTieFor(tie);
                newEndNote->setTieBack(tie);
            } else {
                oldStartNote->removeSpannerFor(spanner);
                oldEndNote->removeSpannerBack(spanner);
                newStartNote->addSpannerFor(spanner);
                newEndNote->addSpannerBack(spanner);
                if (spanner->isGlissando()) {
                    oldEndNote->chord()->updateEndsGlissando();
                }
            }
        }
    } else {
        spanner->setStartElement(startElement);
        spanner->setEndElement(endElement);
    }
    startElement = oldStartElement;
    endElement   = oldEndElement;
    spanner->triggerLayout();
}

//---------------------------------------------------------
//   ChangeParent
//---------------------------------------------------------

void ChangeParent::flip(EditData*)
{
    EngravingItem* p = element->parentItem();
    staff_idx_t si = element->staffIdx();
    p->remove(element);
    element->setParent(parent);
    element->setTrack(staffIdx * VOICES);
    parent->add(element);
    staffIdx = si;
    parent = p;
}

//---------------------------------------------------------
//   ChangeMMRest
//---------------------------------------------------------

void ChangeMMRest::flip(EditData*)
{
    Measure* mmr = m->mmRest();
    m->setMMRest(mmrest);
    mmrest = mmr;
}

//---------------------------------------------------------
//   ChangeMeasureRepeatCount
//---------------------------------------------------------

void ChangeMeasureRepeatCount::flip(EditData*)
{
    int oldCount = m->measureRepeatCount(staffIdx);
    m->setMeasureRepeatCount(count, staffIdx);
    count = oldCount;
}

//---------------------------------------------------------
//   InsertTime
//---------------------------------------------------------

void InsertTime::redo(EditData*)
{
    score->insertTime(tick, len);
}

void InsertTime::undo(EditData*)
{
    score->insertTime(tick, -len);
}

//---------------------------------------------------------
//   InsertTimeUnmanagedSpanner::flip
//---------------------------------------------------------

void InsertTimeUnmanagedSpanner::flip(EditData*)
{
    for (Score* s : score->scoreList()) {
        const auto unmanagedSpanners(s->unmanagedSpanners());
        for (Spanner* sp : unmanagedSpanners) {
            sp->insertTimeUnmanaged(tick, len);
        }
    }
    len = -len;
}

//---------------------------------------------------------
//   ChangeNoteEvent::flip
//---------------------------------------------------------

void ChangeNoteEvent::flip(EditData*)
{
    note->score()->setPlaylistDirty();
    NoteEvent e = *oldEvent;
    *oldEvent   = newEvent;
    newEvent    = e;
    // Get a copy of the current playEventType.
    PlayEventType petval = note->chord()->playEventType();
    // Replace current setting with new setting.
    note->chord()->setPlayEventType(newPetype);
    // Save copy of old setting.
    newPetype = petval;
}

//---------------------------------------------------------
//   LinkUnlink
//---------------------------------------------------------

LinkUnlink::~LinkUnlink()
{
    if (le && mustDelete) {
        assert(le->size() <= 1);
        delete le;
    }
}

void LinkUnlink::link()
{
    if (le->size() == 1) {
        le->front()->setLinks(le);
    }
    mustDelete = false;
    le->push_back(e);
    e->setLinks(le);
}

void LinkUnlink::unlink()
{
    assert(le->contains(e));
    le->remove(e);
    if (le->size() == 1) {
        le->front()->setLinks(0);
        mustDelete = true;
    }

    e->setLinks(0);
}

//---------------------------------------------------------
//   Link
//    link e1 to e2
//---------------------------------------------------------

Link::Link(EngravingObject* e1, EngravingObject* e2)
{
    assert(e1->links() == 0);
    le = e2->links();
    if (!le) {
        if (e1->isStaff()) {
            le = new LinkedObjects(e1->score(), -1);
        } else {
            le = new LinkedObjects(e1->score());
        }
        le->push_back(e2);
    }
    e = e1;
}

//---------------------------------------------------------
//   Link::isFiltered
//---------------------------------------------------------

bool Link::isFiltered(UndoCommand::Filter f, const EngravingItem* target) const
{
    using Filter = UndoCommand::Filter;
    if (f == Filter::Link) {
        return e == target || le->contains(const_cast<EngravingItem*>(target));
    }
    return false;
}

//---------------------------------------------------------
//   Unlink
//---------------------------------------------------------

Unlink::Unlink(EngravingObject* _e)
{
    e  = _e;
    le = e->links();
    assert(le);
}

//---------------------------------------------------------
//   ChangeStartEndSpanner::flip
//---------------------------------------------------------

void ChangeStartEndSpanner::flip(EditData*)
{
    EngravingItem* s = spanner->startElement();
    EngravingItem* e = spanner->endElement();
    spanner->setStartElement(start);
    spanner->setEndElement(end);
    start = s;
    end   = e;
}

//---------------------------------------------------------
//   ChangeMetaTags::flip
//---------------------------------------------------------

void ChangeMetaTags::flip(EditData*)
{
    std::map<String, String> t = score->metaTags();
    score->setMetaTags(metaTags);
    metaTags = t;
}

//---------------------------------------------------------
//   ChangeDrumset::flip
//---------------------------------------------------------

void ChangeDrumset::flip(EditData*)
{
    Drumset d = *instrument->drumset();
    instrument->setDrumset(&drumset);
    drumset = d;
}

//---------------------------------------------------------
//   FretDot
//---------------------------------------------------------

void FretDot::redo(EditData*)
{
    undoData = FretUndoData(diagram);

    diagram->setDot(string, fret, add, dtype);
    diagram->triggerLayout();
}

void FretDot::undo(EditData*)
{
    undoData.updateDiagram();
    diagram->triggerLayout();
}

//---------------------------------------------------------
//   FretMarker
//---------------------------------------------------------

void FretMarker::redo(EditData*)
{
    undoData = FretUndoData(diagram);

    diagram->setMarker(string, mtype);
    diagram->triggerLayout();
}

void FretMarker::undo(EditData*)
{
    undoData.updateDiagram();
    diagram->triggerLayout();
}

//---------------------------------------------------------
//   FretBarre
//---------------------------------------------------------

void FretBarre::redo(EditData*)
{
    undoData = FretUndoData(diagram);

    diagram->setBarre(string, fret, add);
    diagram->triggerLayout();
}

void FretBarre::undo(EditData*)
{
    undoData.updateDiagram();
    diagram->triggerLayout();
}

//---------------------------------------------------------
//   FretClear
//---------------------------------------------------------

void FretClear::redo(EditData*)
{
    undoData = FretUndoData(diagram);

    diagram->clear();
    diagram->triggerLayout();
}

void FretClear::undo(EditData*)
{
    undoData.updateDiagram();
    diagram->triggerLayout();
}

//---------------------------------------------------------
//   MoveTremolo
//---------------------------------------------------------

void MoveTremolo::redo(EditData*)
{
    // Find new tremolo chords
    Measure* m1 = score->tick2measure(chord1Tick);
    Measure* m2 = score->tick2measure(chord2Tick);
    IF_ASSERT_FAILED(m1 && m2) {
        return;
    }
    Chord* c1 = m1->findChord(chord1Tick, track);
    Chord* c2 = m2->findChord(chord2Tick, track);
    IF_ASSERT_FAILED(c1 && c2) {
        return;
    }

    // Remember the old tremolo chords
    oldC1 = trem->chord1();
    oldC2 = trem->chord2();

    // Move tremolo away from old chords
    trem->chord1()->setTremolo(nullptr);
    trem->chord2()->setTremolo(nullptr);

    // Delete old tremolo on c1 and c2, if present
    if (c1->tremolo() && (c1->tremolo() != trem)) {
        if (c2->tremolo() == c1->tremolo()) {
            c2->tremolo()->setChords(c1, c2);
        } else {
            c1->tremolo()->setChords(c1, nullptr);
        }
        Tremolo* oldTremolo  = c1->tremolo();
        c1->setTremolo(nullptr);
        delete oldTremolo;
    }
    if (c2->tremolo() && (c2->tremolo() != trem)) {
        c2->tremolo()->setChords(nullptr, c2);
        Tremolo* oldTremolo  = c2->tremolo();
        c2->setTremolo(nullptr);
        delete oldTremolo;
    }

    // Move tremolo to new chords
    c1->setTremolo(trem);
    c2->setTremolo(trem);
    trem->setChords(c1, c2);
    trem->setParent(c1);

    // Tremolo would cross barline, so remove it
    if (m1 != m2) {
        score->undoRemoveElement(trem);
        return;
    }
    // One of the notes crosses a barline, so remove the tremolo
    if (c1->ticks() != c2->ticks()) {
        score->undoRemoveElement(trem);
    }
}

void MoveTremolo::undo(EditData*)
{
    // Move tremolo to old position
    trem->chord1()->setTremolo(nullptr);
    trem->chord2()->setTremolo(nullptr);
    oldC1->setTremolo(trem);
    oldC2->setTremolo(trem);
    trem->setChords(oldC1, oldC2);
    trem->setParent(oldC1);
}

//---------------------------------------------------------
//   ChangeScoreOrder
//---------------------------------------------------------

void ChangeScoreOrder::flip(EditData*)
{
    ScoreOrder s = score->scoreOrder();
    score->setScoreOrder(order);
    order = s;
}
}
