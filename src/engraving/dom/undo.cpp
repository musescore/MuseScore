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

#include "iengravingfont.h"

#include "arpeggio.h"
#include "bend.h"
#include "bracket.h"
#include "chord.h"
#include "clef.h"
#include "engravingitem.h"
#include "excerpt.h"
#include "fret.h"
#include "guitarbend.h"
#include "harmony.h"
#include "harppedaldiagram.h"
#include "input.h"
#include "instrchange.h"
#include "key.h"
#include "keysig.h"
#include "linkedobjects.h"
#include "masterscore.h"
#include "measure.h"
#include "note.h"
#include "noteevent.h"
#include "page.h"
#include "part.h"
#include "score.h"
#include "segment.h"
#include "select.h"
#include "spanner.h"
#include "staff.h"
#include "stafflines.h"
#include "stafftype.h"
#include "stafftypechange.h"
#include "system.h"
#include "tempotext.h"
#include "textedit.h"
#include "textline.h"
#include "tie.h"
#include "tuplet.h"
#include "utils.h"

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

static std::vector<EngravingObject*> compoundObjects(EngravingObject* object)
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
    }

    objects.push_back(object);

    return objects;
}

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

static void updateStaffTextCache(const StaffTextBase* text, Score* score)
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
    muse::join(childList, other->childList);
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
        UndoCommand* c = muse::takeLast(childList);
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

            auto item = dynamic_cast<EngravingItem*>(object);
            if (!item) {
                continue;
            }

            result.changedItems[item].insert(type);
        }
    }

    return result;
}

const TranslatableString& UndoMacro::actionName() const
{
    return m_actionName;
}

//---------------------------------------------------------
//   CloneVoice
//---------------------------------------------------------

CloneVoice::CloneVoice(Segment* _sf, const Fraction& _lTick, Segment* _d, track_idx_t _strack, track_idx_t _dtrack, track_idx_t _otrack,
                       bool _linked)
{
    sourceSeg = _sf;            // first source segment
    lTick     = _lTick;         // last tick to clone
    destSeg   = _d;             // first destination segment
    strack    = _strack;
    dtrack    = _dtrack;
    otrack    = _otrack;        // old source track if -1 delete voice in strack after copy
    linked    = _linked;        // if true  add elements in destination segment only
                                // if false add elements in every linked staff
}

void CloneVoice::undo(EditData*)
{
    Score* s = destSeg->score();
    Fraction ticks = destSeg->tick() + lTick - sourceSeg->tick();
    track_idx_t sTrack = otrack == muse::nidx ? dtrack : otrack;   // use the correct source / destination if deleting the source
    track_idx_t dTrack = otrack == muse::nidx ? strack : dtrack;

    // Clear destination voice (in case of not linked and otrack = -1 we would delete our source
    if (otrack != muse::nidx && linked) {
        for (Segment* seg = destSeg; seg && seg->tick() < ticks; seg = seg->next1()) {
            EngravingItem* el = seg->element(dTrack);
            if (el && el->isChordRest()) {
                el->unlink();
                seg->setElement(dTrack, nullptr);
            }
        }
    }

    if (otrack == muse::nidx && !linked) {
        // On the first run get going the undo redo action for adding/deleting elements and slurs
        if (first) {
            s->cloneVoice(sTrack, dTrack, sourceSeg, ticks, linked);
            auto spanners = s->spannerMap().findOverlapping(sourceSeg->tick().ticks(), lTick.ticks());
            for (auto i = spanners.begin(); i < spanners.end(); i++) {
                Spanner* sp = i->value;
                if (sp->isSlur() && (sp->track() == sTrack || sp->track2() == sTrack)) {
                    s->undoRemoveElement(sp);
                }
                if (sp->isHairpin() && (sp->track() == dTrack || sp->track2() == dTrack)) {
                    s->undoRemoveElement(sp);
                }
            }
            for (Segment* seg = destSeg; seg && seg->tick() < ticks; seg = seg->next1()) {
                EngravingItem* el = seg->element(sTrack);
                if (el && el->isChordRest()) {
                    s->undoRemoveElement(el);
                }

                const std::vector<EngravingItem*> annotations = seg->annotations();
                for (EngravingItem* annotation : annotations) {
                    if (annotation && annotation->hasVoiceAssignmentProperties() && annotation->track() == dTrack) {
                        // Remove extra all voice annotations which have been created
                        VoiceAssignment voiceAssignment = annotation->getProperty(Pid::VOICE_ASSIGNMENT).value<VoiceAssignment>();
                        if (voiceAssignment != VoiceAssignment::CURRENT_VOICE_ONLY) {
                            s->undoRemoveElement(annotation);
                        }
                    }
                }
            }
        }
        // Set rests if first voice in a staff
        if (!(sTrack % VOICES)) {
            s->setRest(destSeg->tick(), sTrack, ticks, false, nullptr);
        }
    } else {
        s->cloneVoice(sTrack, dTrack, sourceSeg, ticks, linked);
        if (!linked && !(dTrack % VOICES)) {
            s->setRest(destSeg->tick(), dTrack, ticks, false, nullptr);
        }

        // Remove annotations
        for (Segment* seg = destSeg; seg && seg->tick() < ticks; seg = seg->next1()) {
            const std::vector<EngravingItem*> annotations = seg->annotations();
            for (EngravingItem* annotation : annotations) {
                if (annotation && annotation->hasVoiceAssignmentProperties() && annotation->track() == dTrack) {
                    // Remove extra all voice annotations which have been created
                    VoiceAssignment voiceAssignment = annotation->getProperty(Pid::VOICE_ASSIGNMENT).value<VoiceAssignment>();
                    if (voiceAssignment != VoiceAssignment::CURRENT_VOICE_ONLY) {
                        s->undoRemoveElement(annotation);
                    }
                }
            }
        }

        auto spanners = s->spannerMap().findOverlapping(sourceSeg->tick().ticks(), lTick.ticks());
        for (auto i = spanners.begin(); i < spanners.end(); i++) {
            Spanner* sp = i->value;
            if (sp->hasVoiceAssignmentProperties() && (sp->track() == dTrack || sp->track2() == dTrack)) {
                // Remove extra all voice annotations which have been created
                VoiceAssignment voiceAssignment = sp->getProperty(Pid::VOICE_ASSIGNMENT).value<VoiceAssignment>();
                if (voiceAssignment != VoiceAssignment::CURRENT_VOICE_ONLY) {
                    s->undoRemoveElement(sp);
                }
            }
        }
    }

    first = false;
}

void CloneVoice::redo(EditData*)
{
    Score* s = destSeg->score();
    Fraction ticks = destSeg->tick() + lTick - sourceSeg->tick();

    // Clear destination voice (in case of not linked and otrack = -1 we would delete our source
    if (otrack != muse::nidx && linked) {
        for (Segment* seg = destSeg; seg && seg->tick() < ticks; seg = seg->next1()) {
            EngravingItem* el = seg->element(dtrack);
            if (el && el->isChordRest()) {
                el->unlink();
                seg->setElement(dtrack, nullptr);
            }
        }
    }

    if (otrack == muse::nidx && !linked) {
        // On the first run get going the undo redo action for adding/deleting elements and slurs
        if (first) {
            s->cloneVoice(strack, dtrack, sourceSeg, ticks, linked);
            auto spanners = s->spannerMap().findOverlapping(sourceSeg->tick().ticks(), lTick.ticks());
            for (auto i = spanners.begin(); i < spanners.end(); i++) {
                Spanner* sp = i->value;
                if (sp->isSlur() && (sp->track() == strack || sp->track2() == strack)) {
                    s->undoRemoveElement(sp);
                }
            }
            for (Segment* seg = destSeg; seg && seg->tick() < ticks; seg = seg->next1()) {
                EngravingItem* el = seg->element(strack);
                if (el && el->isChordRest()) {
                    s->undoRemoveElement(el);
                }

                const std::vector<EngravingItem*> annotations = seg->annotations();
                for (EngravingItem* annotation : annotations) {
                    if (annotation && annotation->track() == strack) {
                        if (annotation->hasVoiceAssignmentProperties()) {
                            VoiceAssignment voiceAssignment = annotation->getProperty(Pid::VOICE_ASSIGNMENT).value<VoiceAssignment>();
                            if (voiceAssignment != VoiceAssignment::CURRENT_VOICE_ONLY) {
                                continue;
                            }
                        }
                        s->undoRemoveElement(annotation);
                    }
                }
            }
        }
        // Set rests if first voice in a staff
        if (!(strack % VOICES)) {
            s->setRest(destSeg->tick(), strack, ticks, false, nullptr);
        }
    } else {
        s->cloneVoice(strack, dtrack, sourceSeg, ticks, linked, first);
    }
}

//---------------------------------------------------------
//   AddElement
//---------------------------------------------------------

AddElement::AddElement(EngravingItem* e)
{
    DO_ASSERT_X(!e->generated(), String(u"Generated item %1 passed to AddElement").arg(String::fromAscii(e->typeName())));
    element = e;
}

//---------------------------------------------------------
//   AddElement::cleanup
//---------------------------------------------------------

void AddElement::cleanup(bool undo)
{
    if (!undo) {
        delete element;
        element = nullptr;
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
    Score* score = element->score();

    if (!element->isTuplet()) {
        score->removeElement(element);
    }

    if (element->isStaffTextBase()) {
        updateStaffTextCache(toStaffTextBase(element), score);
    }

    if (element->isHarmony() && !toHarmony(element)->isInFretBox()) {
        score->rebuildFretBox();
    }

    if (element->isFretDiagram() && !toFretDiagram(element)->isInFretBox()) {
        score->rebuildFretBox();
    }

    endUndoRedo(true);
}

//---------------------------------------------------------
//   redo
//---------------------------------------------------------

void AddElement::redo(EditData*)
{
    Score* score = element->score();

    if (!element->isTuplet()) {
        score->addElement(element);
    }

    if (element->isStaffTextBase()) {
        updateStaffTextCache(toStaffTextBase(element), score);
    }

    if (element->isHarmony() && !toHarmony(element)->isInFretBox()) {
        score->rebuildFretBox();
    }

    if (element->isFretDiagram() && !toFretDiagram(element)->isInFretBox()) {
        score->rebuildFretBox();
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
        return muse::contains(target->linkList(), static_cast<EngravingObject*>(element));
    default:
        break;
    }
    return false;
}

std::vector<EngravingObject*> AddElement::objectItems() const
{
    return compoundObjects(element);
}

//---------------------------------------------------------
//   removeNote
//    Helper function for RemoveElement class
//---------------------------------------------------------

static void removeNote(const Note* note)
{
    Score* score = note->score();
    Tie* tieFor = note->tieFor();
    Tie* tieBack = note->tieBack();
    if (tieFor && tieFor->endNote()) {
        score->doUndoRemoveElement(tieFor);
    }
    if (tieBack) {
        if (tieBack->tieJumpPoints() && tieBack->tieJumpPoints()->size() > 1) {
            Tie::changeTieType(tieBack);
        } else {
            score->doUndoRemoveElement(tieBack);
        }
    }
    for (Spanner* s : note->spannerBack()) {
        score->doUndoRemoveElement(s);
    }
    for (Spanner* s : note->spannerFor()) {
        score->doUndoRemoveElement(s);
    }
}

//---------------------------------------------------------
//   RemoveElement
//---------------------------------------------------------

RemoveElement::RemoveElement(EngravingItem* e)
{
    DO_ASSERT_X(!e->generated(), String(u"Generated item %1 passed to RemoveElement").arg(String::fromAscii(e->typeName())));
    element = e;

    Score* score = element->score();
    if (element->isChordRest()) {
        ChordRest* cr = toChordRest(element);
        if (cr->tuplet() && cr->tuplet()->elements().size() <= 1) {
            score->doUndoRemoveElement(cr->tuplet());
        }
        if (e->isChord()) {
            Chord* chord = toChord(e);
            // remove tremolo between 2 notes
            if (chord->tremoloTwoChord()) {
                TremoloTwoChord* tremolo = chord->tremoloTwoChord();
                score->doUndoRemoveElement(tremolo);
            }
            // Move arpeggio down to next available note
            if (chord->arpeggio()) {
                chord->arpeggio()->rebaseStartAnchor(AnchorRebaseDirection::DOWN);
            } else {
                // If this chord is the end of an arpeggio, move the end of the arpeggio upwards to the next available chord
                Arpeggio* spanArp = chord->spanArpeggio();
                if (spanArp && chord->track() == spanArp->endTrack()) {
                    spanArp->rebaseEndAnchor(AnchorRebaseDirection::UP);
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
        element = nullptr;
    }
}

//---------------------------------------------------------
//   undo
//---------------------------------------------------------

void RemoveElement::undo(EditData*)
{
    Score* score = element->score();

    if (!element->isTuplet()) {
        score->addElement(element);
    }

    if (element->isStaffTextBase()) {
        updateStaffTextCache(toStaffTextBase(element), score);
    } else if (element->isChordRest()) {
        if (element->isChord()) {
            Chord* chord = toChord(element);
            for (Note* note : chord->notes()) {
                note->connectTiedNotes();
            }
        }
        undoAddTuplet(toChordRest(element));
    } else if (element->isClef()) {
        score->setLayout(element->staff()->nextClefTick(element->tick()), element->staffIdx());
    } else if (element->isKeySig()) {
        score->setLayout(element->staff()->nextKeyTick(element->tick()), element->staffIdx());
    }

    if (element->isHarmony() && !toHarmony(element)->isInFretBox()) {
        score->rebuildFretBox();
    }

    if (element->isFretDiagram() && !toFretDiagram(element)->isInFretBox()) {
        score->rebuildFretBox();
    }
}

//---------------------------------------------------------
//   redo
//---------------------------------------------------------

void RemoveElement::redo(EditData*)
{
    Score* score = element->score();

    if (!element->isTuplet()) {
        score->removeElement(element);
    }

    if (element->isStaffTextBase()) {
        updateStaffTextCache(toStaffTextBase(element), score);
    } else if (element->isChordRest()) {
        undoRemoveTuplet(toChordRest(element));
        if (element->isChord()) {
            Chord* chord = toChord(element);
            for (Note* note : chord->notes()) {
                note->disconnectTiedNotes();
            }
        }
    } else if (element->isClef()) {
        score->setLayout(element->staff()->nextClefTick(element->tick()), element->staffIdx());
    } else if (element->isKeySig()) {
        score->setLayout(element->staff()->nextKeyTick(element->tick()), element->staffIdx());
    }

    if (element->isHarmony() && !toHarmony(element)->isInFretBox()) {
        score->rebuildFretBox();
    }

    if (element->isFretDiagram() && !toFretDiagram(element)->isInFretBox()) {
        score->rebuildFretBox();
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
        return muse::contains(target->linkList(), static_cast<EngravingObject*>(element));
    default:
        break;
    }
    return false;
}

//---------------------------------------------------------
//   InsertPart
//---------------------------------------------------------

InsertPart::InsertPart(Part* p, size_t targetPartIdx)
{
    m_part = p;
    m_targetPartIdx = targetPartIdx;
}

void InsertPart::undo(EditData*)
{
    m_part->score()->removePart(m_part);
}

void InsertPart::redo(EditData*)
{
    m_part->score()->insertPart(m_part, m_targetPartIdx);
}

void InsertPart::cleanup(bool undo)
{
    if (!undo) {
        delete m_part;
        m_part = nullptr;
    }
}

//---------------------------------------------------------
//   RemovePart
//---------------------------------------------------------

RemovePart::RemovePart(Part* p, size_t partIdx)
{
    m_part = p;
    m_partIdx = partIdx;

    if (m_partIdx == muse::nidx) {
        m_partIdx = muse::indexOf(m_part->score()->parts(), m_part);
    }
}

void RemovePart::undo(EditData*)
{
    m_part->score()->insertPart(m_part, m_partIdx);
}

void RemovePart::redo(EditData*)
{
    m_part->score()->removePart(m_part);
}

void RemovePart::cleanup(bool undo)
{
    if (undo) {
        delete m_part;
        m_part = nullptr;
    }
}

//---------------------------------------------------------
//   AddPartToExcerpt
//---------------------------------------------------------

AddPartToExcerpt::AddPartToExcerpt(Excerpt* e, Part* p, size_t targetPartIdx)
    : m_excerpt(e), m_part(p), m_targetPartIdx(targetPartIdx)
{
    assert(m_excerpt);
    assert(m_part);
}

void AddPartToExcerpt::undo(EditData*)
{
    muse::remove(m_excerpt->parts(), m_part);

    if (Score* score = m_excerpt->excerptScore()) {
        score->removePart(m_part);
    }
}

void AddPartToExcerpt::redo(EditData*)
{
    std::vector<Part*>& excerptParts = m_excerpt->parts();
    if (m_targetPartIdx < excerptParts.size()) {
        excerptParts.insert(excerptParts.begin() + m_targetPartIdx, m_part);
    } else {
        excerptParts.push_back(m_part);
    }

    if (Score* score = m_excerpt->excerptScore()) {
        score->insertPart(m_part, m_targetPartIdx);
    }
}

void AddPartToExcerpt::cleanup(bool undo)
{
    if (!undo) {
        delete m_part;
        m_part = nullptr;
    }
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

void InsertStaff::cleanup(bool undo)
{
    if (!undo) {
        delete staff;
        staff = nullptr;
    }
}

//---------------------------------------------------------
//   RemoveStaff
//---------------------------------------------------------

RemoveStaff::RemoveStaff(Staff* p)
{
    staff = p;
    ridx  = staff->rstaff();
    wasSystemObjectStaff = staff->score()->isSystemObjectStaff(staff);
}

void RemoveStaff::undo(EditData*)
{
    staff->score()->insertStaff(staff, ridx);
    if (wasSystemObjectStaff) {
        staff->score()->addSystemObjectStaff(staff);
    }
}

void RemoveStaff::redo(EditData*)
{
    staff->score()->removeStaff(staff);
    if (wasSystemObjectStaff) {
        staff->score()->removeSystemObjectStaff(staff);
    }
}

void RemoveStaff::cleanup(bool undo)
{
    if (undo) {
        delete staff;
        staff = nullptr;
    }
}

AddSystemObjectStaff::AddSystemObjectStaff(Staff* s)
    : staff(s)
{
}

void AddSystemObjectStaff::undo(EditData*)
{
    staff->score()->removeSystemObjectStaff(staff);
}

void AddSystemObjectStaff::redo(EditData*)
{
    staff->score()->addSystemObjectStaff(staff);
}

RemoveSystemObjectStaff::RemoveSystemObjectStaff(Staff* s)
    : staff(s)
{
}

void RemoveSystemObjectStaff::undo(EditData*)
{
    staff->score()->addSystemObjectStaff(staff);
}

void RemoveSystemObjectStaff::redo(EditData*)
{
    staff->score()->removeSystemObjectStaff(staff);
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
        rlist.push_back(muse::indexOf(l, i));
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

    newElement->setTrack(oldElement->track());
    if (newElement->isSpanner()) {
        toSpanner(newElement)->setTrack2(toSpanner(oldElement)->track2());
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

    if (oldElement->explicitParent() == nullptr) {
        newElement->setScore(score);
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
    } else if (newElement->isTempoText()) {
        TempoText* t = toTempoText(oldElement);
        score->setTempo(t->segment(), t->tempo());
    }

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

    if (newElement->isStaffTextBase()) {
        updateStaffTextCache(toStaffTextBase(newElement), score);
    }

    std::swap(oldElement, newElement);
    oldElement->triggerLayout();
    newElement->triggerLayout();
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

TransposeHarmony::TransposeHarmony(Harmony* h, Interval interval, bool doubleSharpsFlats)
{
    m_harmony = h;
    m_interval = interval;
    m_useDoubleSharpsFlats = doubleSharpsFlats;
}

void TransposeHarmony::flip(EditData*)
{
    m_harmony->realizedHarmony().setDirty(true);   // harmony should be re-realized after transposition

    for (HarmonyInfo* info : m_harmony->chords()) {
        info->setRootTpc(transposeTpc(info->rootTpc(), m_interval, m_useDoubleSharpsFlats));
        info->setBassTpc(transposeTpc(info->bassTpc(), m_interval, m_useDoubleSharpsFlats));
    }

    m_harmony->setXmlText(m_harmony->harmonyName());
    m_harmony->triggerLayout();
    m_interval.flip();
}

//---------------------------------------------------------
//   TransposeHarmonyDiatonic
//---------------------------------------------------------

void TransposeHarmonyDiatonic::flip(EditData*)
{
    m_harmony->realizedHarmony().setDirty(true);   // harmony should be re-realized after transposition

    Fraction tick = Fraction(0, 1);
    Segment* seg = toSegment(m_harmony->findAncestor(ElementType::SEGMENT));
    if (seg) {
        tick = seg->tick();
    }
    Key key = !m_harmony->staff() ? Key::C : m_harmony->staff()->key(tick);

    for (HarmonyInfo* info : m_harmony->chords()) {
        info->setRootTpc(transposeTpcDiatonicByKey(info->rootTpc(), m_interval, key, m_transposeKeys, m_useDoubleSharpsFlats));
        info->setBassTpc(transposeTpcDiatonicByKey(info->bassTpc(), m_interval, key, m_transposeKeys, m_useDoubleSharpsFlats));
    }

    m_harmony->setXmlText(m_harmony->harmonyName());
    m_harmony->triggerLayout();

    m_interval *= -1;
}

TransposeHarmonyDiatonic::TransposeHarmonyDiatonic(Harmony* h, int interval, bool useDoubleSharpsFlats, bool transposeKeys)
{
    m_harmony = h;
    m_interval = interval;
    m_useDoubleSharpsFlats = useDoubleSharpsFlats;
    m_transposeKeys = transposeKeys;
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
    cutaway = staff->cutaway();
    hideSystemBarLine = staff->hideSystemBarLine();
    mergeMatchingRests = staff->mergeMatchingRests();
    reflectTranspositionInLinkedTab = staff->reflectTranspositionInLinkedTab();
}

ChangeStaff::ChangeStaff(Staff* _staff, bool _visible, ClefTypeList _clefType, Spatium _userDist, bool _cutaway, bool _hideSystemBarLine,
                         AutoOnOff _mergeMatchingRests, bool _reflectTranspositionInLinkedTab)
{
    staff       = _staff;
    visible     = _visible;
    clefType    = _clefType;
    userDist    = _userDist;
    cutaway     = _cutaway;
    hideSystemBarLine  = _hideSystemBarLine;
    mergeMatchingRests = _mergeMatchingRests;
    reflectTranspositionInLinkedTab = _reflectTranspositionInLinkedTab;
}

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeStaff::flip(EditData*)
{
    bool oldVisible = staff->visible();
    ClefTypeList oldClefType = staff->defaultClefType();
    Spatium oldUserDist   = staff->userDist();
    bool oldCutaway     = staff->cutaway();
    bool oldHideSystemBarLine  = staff->hideSystemBarLine();
    AutoOnOff oldMergeMatchingRests = staff->mergeMatchingRests();
    bool oldReflectTranspositionInLinkedTab = staff->reflectTranspositionInLinkedTab();

    staff->setVisible(visible);
    staff->setDefaultClefType(clefType);
    staff->setUserDist(userDist);
    staff->setCutaway(cutaway);
    staff->setHideSystemBarLine(hideSystemBarLine);
    staff->setMergeMatchingRests(mergeMatchingRests);
    staff->setReflectTranspositionInLinkedTab(reflectTranspositionInLinkedTab);

    visible     = oldVisible;
    clefType    = oldClefType;
    userDist    = oldUserDist;
    cutaway     = oldCutaway;
    hideSystemBarLine  = oldHideSystemBarLine;
    mergeMatchingRests = oldMergeMatchingRests;
    reflectTranspositionInLinkedTab = oldReflectTranspositionInLinkedTab;

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
    bool fromTabToStandard = oldStaffType.isTabStaff() && !staffType.isTabStaff();

    staffType = oldStaffType;

    Score* score = staff->score();
    if (invisibleChanged) {
        staff_idx_t staffIdx = staff->idx();
        for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
            m->staffLines(staffIdx)->setVisible(!staff->isLinesInvisible(Fraction(0, 1)));
        }
    }

    if (fromTabToStandard) {
        GuitarBend::adaptBendsFromTabToStandardStaff(staff);
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

static void changeChordStyle(Score* score)
{
    const MStyle& style = score->style();
    score->chordList()->unload();
    double emag = style.styleD(Sid::chordExtensionMag);
    double eadjust = style.styleD(Sid::chordExtensionAdjust);
    double mmag = style.styleD(Sid::chordModifierMag);
    double madjust = style.styleD(Sid::chordModifierAdjust);
    double stackedmmag = style.styleD(Sid::chordStackedModiferMag);
    bool mstackModifiers = style.styleB(Sid::verticallyStackModifiers);
    bool mexcludeModsHAlign = style.styleB(Sid::chordAlignmentExcludeModifiers);
    String msymbolFont = style.styleSt(Sid::musicalTextFont);
    score->chordList()->configureAutoAdjust(emag, eadjust, mmag, madjust, stackedmmag, mstackModifiers, mexcludeModsHAlign, msymbolFont);
    if (score->style().styleB(Sid::chordsXmlFile)) {
        score->chordList()->read(u"chords.xml");
    }
    score->chordList()->read(style.styleSt(Sid::chordDescriptionFile));
    score->chordList()->setCustomChordList(style.styleV(Sid::chordStyle).value<ChordStylePreset>() == ChordStylePreset::CUSTOM);
}

//---------------------------------------------------------
//   ChangeStyle
//----------------------------------------------------------

ChangeStyle::ChangeStyle(Score* s, const MStyle& st, const bool overlapOnly)
    : score(s), style(st), overlap(overlapOnly)
{
}

StyleIdSet ChangeStyle::changedIds() const
{
    StyleIdSet result;
    for (int _sid = 0; _sid < static_cast<int>(Sid::STYLES); ++_sid) {
        Sid sid = static_cast<Sid>(_sid);
        if (score->style().styleV(sid) != style.value(sid)) {
            result.insert(sid);
        }
    }
    return result;
}

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeStyle::flip(EditData*)
{
    MStyle tmp = score->style();

    if (score->style().styleV(Sid::concertPitch) != style.value(Sid::concertPitch)) {
        score->cmdConcertPitchChanged(style.value(Sid::concertPitch).toBool());
    }
    if (score->style().styleV(Sid::musicalSymbolFont) != style.value(Sid::musicalSymbolFont)) {
        score->setEngravingFont(score->engravingFonts()->fontByName(style.styleSt(Sid::musicalSymbolFont).toStdString()));
    }

    score->setStyle(style, overlap);
    changeChordStyle(score);
    if (tmp.spatium() != style.spatium()) {
        score->spatiumChanged(tmp.spatium(), style.spatium());
    }
    score->styleChanged();
    style = tmp;
}

void ChangeStyle::undo(EditData* ed)
{
    overlap = false;
    UndoCommand::undo(ed);
}

static void changeStyleValue(Score* score, Sid idx, const PropertyValue& oldValue, const PropertyValue& newValue)
{
    score->style().set(idx, newValue);
    switch (idx) {
    case Sid::chordExtensionMag:
    case Sid::chordExtensionAdjust:
    case Sid::chordModifierMag:
    case Sid::chordModifierAdjust:
    case Sid::chordDescriptionFile:
    case Sid::verticallyStackModifiers:
    case Sid::chordAlignmentExcludeModifiers:
    case Sid::musicalTextFont: {
        changeChordStyle(score);
    }
    break;
    case Sid::spatium:
        score->spatiumChanged(oldValue.toDouble(), newValue.toDouble());
        break;
    case Sid::defaultsVersion:
        score->style().setDefaultStyleVersion(newValue.toInt());
        break;
    case Sid::createMultiMeasureRests:
        if (oldValue.toBool() == true && newValue.toBool() == false) {
            score->removeSystemLocksContainingMMRests();
        }
        break;
    default:
        break;
    }
}

void ChangeStyleValues::flip(EditData*)
{
    if (!m_score) {
        return;
    }

    bool styleChanged = false;
    const MStyle& style = m_score->style();

    for (auto& pair : m_values) {
        PropertyValue oldValue = style.styleV(pair.first);
        if (oldValue == pair.second) {
            continue;
        }

        changeStyleValue(m_score, pair.first, oldValue, pair.second);
        pair.second = oldValue;

        styleChanged = true;
    }

    if (styleChanged) {
        m_score->styleChanged();
    }
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
    staff_idx_t oldStaff = chordRest->vStaffIdx();

    chordRest->setStaffMove(staffMove);
    chordRest->checkStaffMoveValidity();
    chordRest->triggerLayout();
    if (chordRest->vStaffIdx() == oldStaff) {
        return;
    }

    for (EngravingObject* e : chordRest->linkList()) {
        ChordRest* cr = toChordRest(e);
        if (cr == chordRest) {
            continue;
        }
        cr->setStaffMove(staffMove);
        cr->checkStaffMoveValidity();
        cr->triggerLayout();
    }
    staffMove = v;
}

//---------------------------------------------------------
//   ChangeVelocity
//---------------------------------------------------------

ChangeVelocity::ChangeVelocity(Note* n, int o)
    : note(n), userVelocity(o)
{
}

void ChangeVelocity::flip(EditData*)
{
    int v = note->userVelocity();
    note->setUserVelocity(userVelocity);
    userVelocity = v;
}

//---------------------------------------------------------
//   ChangeMStaffProperties
//---------------------------------------------------------

ChangeMStaffProperties::ChangeMStaffProperties(Measure* m, staff_idx_t i, bool v, bool s)
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
    visible = v;
    stemless = s;
}

//---------------------------------------------------------
//   ChangeMStaffHideIfEmpty
//---------------------------------------------------------

ChangeMStaffHideIfEmpty::ChangeMStaffHideIfEmpty(Measure* m, staff_idx_t i, AutoOnOff h)
    : measure(m), staffIdx(i), hideIfEmpty(h)
{
}

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeMStaffHideIfEmpty::flip(EditData*)
{
    AutoOnOff h = measure->hideStaffIfEmpty(staffIdx);
    measure->setHideStaffIfEmpty(staffIdx, hideIfEmpty);
    measure->triggerLayout(staffIdx);
    hideIfEmpty = h;
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

    // move subsequent StaffTypeChanges
    Fraction tickStart = fm->tick();
    Fraction tickEnd = lm->endTick();
    if (moveStc && tickEnd > tickStart) {
        for (Staff* staff : score->staves()) {
            // loop backwards until the insert point
            auto stRange = staff->staffTypeRange(score->lastMeasure()->tick());
            int moveTick = stRange.first;

            while (moveTick >= tickStart.ticks() && moveTick > 0) {
                Fraction tick = Fraction::fromTicks(moveTick);
                Fraction tickNew = tick + tickEnd - tickStart;

                // measures were inserted already so icon is at differnt place, as staffTypeChange itslef
                Measure* measure = score->tick2measure(tickNew);
                bool stIcon = false;

                staff->moveStaffType(tick, tickNew);

                for (EngravingItem* el : measure->el()) {
                    if (el && el->isStaffTypeChange() && el->track() == staff->idx() * VOICES) {
                        StaffTypeChange* stc = toStaffTypeChange(el);
                        stc->setStaffType(staff->staffType(tickNew), false);
                        stIcon = true;
                        break;
                    }
                }

                if (!stIcon) {
                    LOG_UNDO() << "StaffTypeChange icon is missing in measure " << measure->no();
                }

                stRange = staff->staffTypeRange(tick);
                moveTick = stRange.first;
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
            if (!e || !e->isChord()) {
                continue;
            }
            Chord* chord = toChord(e);
            for (Note* n : chord->notes()) {
                Tie* tie = n->tieForNonPartial();
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

    if (fm->isMeasure() && lm->isMeasure()) {
        // remove beams from chordrests in affected area, they will be rebuilt later but we need
        // to avoid situations where notes from deleted measures remain in beams
        // when undoing, we need to check the previous measure as well as there could be notes in there
        // that need to have their beams recalculated (esp. when adding time signature)
        MeasureBase* prev = fm->prev();
        Segment* first = toMeasure(prev && prev->isMeasure() ? prev : fm)->first();
        for (Segment* s = first; s && s != toMeasure(lm)->last(); s = s->next1()) {
            if (!s) {
                break;
            }
            if (!s->isChordRestType()) {
                continue;
            }

            for (track_idx_t track = 0; track < score->ntracks(); ++track) {
                EngravingItem* e = s->element(track);
                if (e && e->isChordRest()) {
                    toChordRest(e)->removeDeleteBeam(false);
                }
            }
        }
    }

    std::list<System*> systemList;
    for (MeasureBase* mb = lm;; mb = mb->prev()) {
        System* system = mb->system();
        if (system) {
            if (!muse::contains(systemList, system)) {
                systemList.push_back(system);
            }
            system->removeMeasure(mb);
        }
        if (mb == fm) {
            break;
        }
    }

    // move subsequent StaffTypeChanges
    if (moveStc) {
        for (Staff* staff : score->staves()) {
            Fraction tickStart = tick1;
            Fraction tickEnd = tick2;

            // loop trhu, until the last one
            auto stRange = staff->staffTypeRange(tickEnd);
            int moveTick = stRange.first == tickEnd.ticks() ? stRange.first : stRange.second;

            while (moveTick != -1) {
                Fraction tick = Fraction::fromTicks(moveTick);
                Fraction newTick = tick + tickStart - tickEnd;

                Measure* measure = score->tick2measure(tick);
                bool stIcon = false;

                staff->moveStaffType(tick, newTick);

                for (EngravingItem* el : measure->el()) {
                    if (el && el->isStaffTypeChange() && el->track() == staff->idx() * VOICES) {
                        StaffTypeChange* stc = toStaffTypeChange(el);
                        stc->setStaffType(staff->staffType(newTick), false);
                        stIcon = true;
                        break;
                    }
                }

                if (!stIcon) {
                    LOG_UNDO() << "StaffTypeChange icon is missing in measure " << measure->no();
                }

                stRange = staff->staffTypeRange(tick);
                moveTick = stRange.second;
            }
        }
    }

    score->measures()->remove(fm, lm);

    if (fm->isMeasure()) {
        score->setUpTempoMap();
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

        std::set<Spanner*> spannersCopy = score->unmanagedSpanners();
        for (Spanner* sp : spannersCopy) {
            if ((sp->tick() >= tick1 && sp->tick() < tick2) || (sp->tick2() >= tick1 && sp->tick2() < tick2)) {
                sp->removeUnmanaged();
            }
        }
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

std::vector<EngravingObject*> AddExcerpt::objectItems() const
{
    if (excerpt) {
        if (MasterScore* score = excerpt->masterScore()) {
            return { score };
        }
    }

    return {};
}

//---------------------------------------------------------
//   RemoveExcerpt
//---------------------------------------------------------

RemoveExcerpt::RemoveExcerpt(Excerpt* ex)
    : excerpt(ex)
{
    index = muse::indexOf(excerpt->masterScore()->excerpts(), excerpt);
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

std::vector<EngravingObject*> RemoveExcerpt::objectItems() const
{
    if (excerpt) {
        if (MasterScore* score = excerpt->masterScore()) {
            return { score };
        }
    }

    return {};
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

    if (cr1->isChord() && cr2->isChord() && toChord(cr1)->tremoloTwoChord()
        && (toChord(cr1)->tremoloTwoChord() == toChord(cr2)->tremoloTwoChord())) {
        TremoloTwoChord* t = toChord(cr1)->tremoloTwoChord();
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

    if (element->isStaffTextBase()) {
        updateStaffTextCache(toStaffTextBase(element), element->score());
    }

    property = v;
    flags = ps;
}

std::vector<EngravingObject*> ChangeProperty::objectItems() const
{
    return compoundObjects(element);
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

void AddBracket::redo(EditData*)
{
    staff->setBracketType(level, bracketType);
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
    staff->setBracketType(level, bracketType);
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
    bool isPartialSpanner = spanner->isPartialTie() || spanner->isLaissezVib();
    if (spanner->anchor() == Spanner::Anchor::NOTE) {
        // be sure new spanner elements are of the right type
        if (!isPartialSpanner && (!startElement || !startElement->isNote() || !endElement || !endElement->isNote())) {
            return;
        }
        Note* oldStartNote = toNote(oldStartElement);
        Note* oldEndNote = toNote(oldEndElement);
        Note* newStartNote = toNote(startElement);
        Note* newEndNote = toNote(endElement);
        // update spanner's start and end notes
        if ((newStartNote && newEndNote) || (isPartialSpanner && (newStartNote || newEndNote))) {
            spanner->setNoteSpan(newStartNote, newEndNote);
            if (spanner->isTie()) {
                Tie* tie = toTie(spanner);
                if (oldStartNote && newStartNote) {
                    oldStartNote->setTieFor(nullptr);
                    newStartNote->setTieFor(tie);
                }
                if (oldEndNote && newEndNote) {
                    oldEndNote->setTieBack(nullptr);
                    newEndNote->setTieBack(tie);
                }
            } else {
                oldStartNote->removeSpannerFor(spanner);
                oldEndNote->removeSpannerBack(spanner);
                newStartNote->addSpannerFor(spanner);
                newEndNote->addSpannerBack(spanner);
                if (spanner->isGlissando()) {
                    oldEndNote->chord()->updateEndsNoteAnchoredLine();
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
    element->setTrack(staffIdx * VOICES + element->voice());
    parent->add(element);
    staffIdx = si;
    parent = p;

    element->triggerLayout();
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
        std::set<Spanner*> spannersCopy = s->unmanagedSpanners();
        for (Spanner* sp : spannersCopy) {
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
        le->front()->setLinks(nullptr);
        mustDelete = true;
    }

    e->setLinks(nullptr);
}

//---------------------------------------------------------
//   Link
//    link e1 to e2
//---------------------------------------------------------

Link::Link(EngravingObject* e1, EngravingObject* e2)
{
    assert(e1->links() == nullptr);
    le = e2->links();
    if (!le) {
        le = new LinkedObjects();
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

    if (part->staves().size() > 0) {
        part->score()->setLayout(Fraction(0, 1), part->score()->endTick(), part->staves().front()->idx(), part->staves().back()->idx());
    }
}

//---------------------------------------------------------
//   FretDataChange
//---------------------------------------------------------

void FretDataChange::redo(EditData*)
{
    m_undoData = FretUndoData(m_diagram);

    m_diagram->updateDiagram(m_harmonyName);
}

void FretDataChange::undo(EditData*)
{
    m_undoData.updateDiagram();
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
    trem->chord1()->setTremoloTwoChord(nullptr);
    trem->chord2()->setTremoloTwoChord(nullptr);

    // Delete old tremolo on c1 and c2, if present
    if (c1->tremoloTwoChord() && (c1->tremoloTwoChord() != trem)) {
        if (c2->tremoloTwoChord() == c1->tremoloTwoChord()) {
            c2->tremoloTwoChord()->setChords(c1, c2);
        } else {
            c1->tremoloTwoChord()->setChords(c1, nullptr);
        }
        TremoloTwoChord* oldTremolo  = c1->tremoloTwoChord();
        c1->setTremoloTwoChord(nullptr);
        delete oldTremolo;
    }
    if (c2->tremoloTwoChord() && (c2->tremoloTwoChord() != trem)) {
        c2->tremoloTwoChord()->setChords(nullptr, c2);
        TremoloTwoChord* oldTremolo  = c2->tremoloTwoChord();
        c2->setTremoloTwoChord(nullptr);
        delete oldTremolo;
    }

    // Move tremolo to new chords
    c1->setTremoloTwoChord(trem);
    c2->setTremoloTwoChord(trem);
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
    trem->chord1()->setTremoloTwoChord(nullptr);
    trem->chord2()->setTremoloTwoChord(nullptr);
    oldC1->setTremoloTwoChord(trem);
    oldC2->setTremoloTwoChord(trem);
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

//---------------------------------------------------------
//   ChangeHarpPedalState
//---------------------------------------------------------

void ChangeHarpPedalState::flip(EditData*)
{
    std::array<PedalPosition, HARP_STRING_NO> f_state = diagram->getPedalState();
    if (f_state == pedalState) {
        return;
    }

    diagram->setPedalState(pedalState);
    pedalState = f_state;

    diagram->triggerLayout();
}

std::vector<EngravingObject*> ChangeHarpPedalState::objectItems() const
{
    Part* part = diagram->part();
    std::vector<EngravingObject*> objs{ diagram };
    if (!part) {
        return objs;
    }

    HarpPedalDiagram* nextDiagram = part->nextHarpDiagram(diagram->tick());
    if (nextDiagram) {
        objs.push_back(nextDiagram);
    } else {
        objs.push_back(diagram->score()->lastElement());
    }
    return objs;
}

void ChangeSingleHarpPedal::flip(EditData*)
{
    HarpStringType f_type = type;
    PedalPosition f_pos = diagram->getPedalState()[type];
    if (f_pos == pos) {
        return;
    }

    diagram->setPedal(type, pos);
    type = f_type;
    pos = f_pos;

    diagram->triggerLayout();
}

std::vector<EngravingObject*> ChangeSingleHarpPedal::objectItems() const
{
    Part* part = diagram->part();
    std::vector<EngravingObject*> objs{ diagram };
    if (!part) {
        return objs;
    }

    HarpPedalDiagram* nextDiagram = part->nextHarpDiagram(diagram->tick());
    if (nextDiagram) {
        objs.push_back(nextDiagram);
    } else {
        objs.push_back(diagram->score()->lastElement());
    }
    return objs;
}
}

void ChangeStringData::flip(EditData*)
{
    const StringData* stringData =  m_stringTunings ? m_stringTunings->stringData() : m_instrument->stringData();
    int frets = stringData->frets();
    std::vector<instrString> stringList = stringData->stringList();

    if (m_stringTunings) {
        m_stringTunings->setStringData(m_stringData);
    } else {
        m_instrument->setStringData(m_stringData);
    }

    m_stringData.set(StringData(frets, stringList));
}

void ChangeSoundFlag::flip(EditData*)
{
    IF_ASSERT_FAILED(m_soundFlag) {
        return;
    }

    SoundFlag::PresetCodes presets = m_soundFlag->soundPresets();
    SoundFlag::PlayingTechniqueCode technique = m_soundFlag->playingTechnique();

    m_soundFlag->setSoundPresets(m_presets);
    m_soundFlag->setPlayingTechnique(m_playingTechnique);

    m_presets = std::move(presets);
    m_playingTechnique = std::move(technique);
}

void ChangeSpanArpeggio::flip(EditData*)
{
    Arpeggio* f_spanArp = m_chord->spanArpeggio();

    m_chord->setSpanArpeggio(m_spanArpeggio);
    m_spanArpeggio = f_spanArp;
}

AddSystemLock::AddSystemLock(const SystemLock* systemLock)
    : m_systemLock(systemLock) {}

void AddSystemLock::undo(EditData*)
{
    Score* score = m_systemLock->startMB()->score();
    score->removeSystemLock(m_systemLock);
}

void AddSystemLock::redo(EditData*)
{
    Score* score = m_systemLock->startMB()->score();
    score->addSystemLock(m_systemLock);
}

void AddSystemLock::cleanup(bool undo)
{
    if (!undo) {
        delete m_systemLock;
        m_systemLock = nullptr;
    }
}

std::vector<EngravingObject*> AddSystemLock::objectItems() const
{
    return { m_systemLock->startMB(), m_systemLock->endMB() };
}

RemoveSystemLock::RemoveSystemLock(const SystemLock* systemLock)
    : m_systemLock(systemLock) {}

void RemoveSystemLock::undo(EditData*)
{
    Score* score = m_systemLock->startMB()->score();
    score->addSystemLock(m_systemLock);
}

void RemoveSystemLock::redo(EditData*)
{
    Score* score = m_systemLock->startMB()->score();
    score->removeSystemLock(m_systemLock);
}

void RemoveSystemLock::cleanup(bool undo)
{
    if (undo) {
        delete m_systemLock;
        m_systemLock = nullptr;
    }
}

std::vector<EngravingObject*> RemoveSystemLock::objectItems() const
{
    return { m_systemLock->startMB(), m_systemLock->endMB() };
}

void ChangeTieJumpPointActive::flip(EditData*)
{
    TieJumpPoint* jumpPoint = m_jumpPointList->findJumpPoint(m_id);
    if (!jumpPoint) {
        return;
    }
    bool oldActive = jumpPoint->active();

    jumpPoint->setActive(m_active);
    m_active = oldActive;
}

FretLinkHarmony::FretLinkHarmony(FretDiagram* diagram, Harmony* harmony, bool unlink)
{
    m_fretDiagram = diagram;
    m_harmony = harmony;
    m_unlink = unlink;
}

void FretLinkHarmony::undo(EditData*)
{
    if (m_unlink) {
        m_fretDiagram->linkHarmony(m_harmony);
    } else {
        m_fretDiagram->unlinkHarmony();
    }
}

void FretLinkHarmony::redo(EditData*)
{
    if (m_unlink) {
        m_fretDiagram->unlinkHarmony();
    } else {
        m_fretDiagram->linkHarmony(m_harmony);
    }
}
