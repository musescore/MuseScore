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
#include "notationselection.h"

#include <QMimeData>

#include "engraving/dom/score.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/timesig.h"
#include "engraving/dom/box.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/part.h"
#include "engraving/dom/stringdata.h"
#include "engraving/dom/stafftype.h"

#include "engraving/editing/edittie.h"
#include "engraving/editing/navigation.h"

#include "notationselectionrange.h"
#include "notationerrors.h"

#include "log.h"

using namespace muse;
using namespace mu::notation;
using namespace mu::engraving;

NotationSelection::NotationSelection(IGetScore* getScore, IInteractionForSelection* interaction)
    : m_getScore(getScore), m_interaction(interaction)
{
    m_range = std::make_shared<NotationSelectionRange>(getScore);
}

bool NotationSelection::isNone() const
{
    return score()->selection().isNone();
}

bool NotationSelection::isRange() const
{
    return score()->selection().isRange();
}

SelectionState NotationSelection::state() const
{
    return score()->selection().state();
}

Ret NotationSelection::canCopy() const
{
    if (isNone()) {
        return make_ret(Err::EmptySelection);
    }

    if (!score()->selection().canCopy()) {
        return make_ret(Err::SelectCompleteTupletOrTremolo);
    }

    return muse::make_ok();
}

muse::ByteArray NotationSelection::mimeData() const
{
    return score()->selection().mimeData();
}

QMimeData* NotationSelection::qMimeData() const
{
    QString mimeType = score()->selection().mimeType();
    if (mimeType.isEmpty()) {
        return nullptr;
    }

    QMimeData* mimeData = new QMimeData();
    mimeData->setData(mimeType, score()->selection().mimeData().toQByteArray());

    return mimeData;
}

EngravingItem* NotationSelection::element() const
{
    return score()->selection().element();
}

const std::vector<EngravingItem*>& NotationSelection::elements() const
{
    return score()->selection().elements();
}

std::vector<Note*> NotationSelection::notes(NoteFilter filter) const
{
    switch (filter) {
    case NoteFilter::All: return score()->selection().noteList();
    case NoteFilter::WithTie: return mu::engraving::EditTie::cmdTieNoteList(score()->selection(), false);
    case NoteFilter::WithSlur: {
        NOT_IMPLEMENTED;
        return {};
    }
    }

    return {};
}

muse::RectF NotationSelection::canvasBoundingRect() const
{
    if (isNone()) {
        return RectF();
    }

    if (const mu::engraving::EngravingItem* element = score()->selection().element()) {
        return element->canvasBoundingRect();
    }

    RectF result;

    for (const RectF& rect : range()->boundingArea()) {
        result = result.united(rect);
    }

    return result;
}

INotationSelectionRangePtr NotationSelection::range() const
{
    return m_range;
}

Score* NotationSelection::score() const
{
    return m_getScore->score();
}

void NotationSelection::onElementHit(EngravingItem* el)
{
    m_lastElementHit = el;
}

MeasureBase* NotationSelection::startMeasureBase() const
{
    return score()->selection().startMeasureBase();
}

MeasureBase* NotationSelection::endMeasureBase() const
{
    return score()->selection().endMeasureBase();
}

std::vector<System*> NotationSelection::selectedSystems() const
{
    return score()->selection().selectedSystems();
}

std::vector<mu::engraving::Page*> NotationSelection::pagesContainingSelection() const
{
    return score()->selection().pagesContainingSelection();
}

EngravingItem* NotationSelection::lastElementHit() const
{
    return m_lastElementHit;
}

bool NotationSelection::elementsSelected(const mu::engraving::ElementTypeSet& types) const
{
    return score()->selection().elementsSelected(types);
}

void NotationSelection::select(SelectionTarget target)
{
    //! TODO It's better to change the implementation,
    // instead of calling different methods, it's better to do this:
    // 1. get target elements
    // 2. call select method with the elements
    switch (target) {
    case SelectionTarget::Undefined:
        break;
    case SelectionTarget::FirstItem:
        selectFirstElement();
        break;
    case SelectionTarget::LastItem:
        selectLastElement();
        break;
    case SelectionTarget::NextItem:
        moveSelection(MoveDirection::Right, MoveSelectionType::EngravingItem);
        break;
    case SelectionTarget::PrevItem:
        moveSelection(MoveDirection::Left, MoveSelectionType::EngravingItem);
        break;
    case SelectionTarget::NextSegmentItem:
        moveSegmentSelection(MoveDirection::Right);
        break;
    case SelectionTarget::PrevSegmentItem:
        moveSegmentSelection(MoveDirection::Left);
        break;
    case SelectionTarget::NextChord:
        moveSelection(MoveDirection::Right, MoveSelectionType::Chord);
        break;
    case SelectionTarget::PrevChord:
        moveSelection(MoveDirection::Left, MoveSelectionType::Chord);
        break;
    case SelectionTarget::NextMeasure:
        moveSelection(MoveDirection::Right, MoveSelectionType::Measure);
        break;
    case SelectionTarget::PrevMeasure:
        moveSelection(MoveDirection::Left, MoveSelectionType::Measure);
        break;
    case SelectionTarget::NextTrack:
        moveSelection(MoveDirection::Right, MoveSelectionType::Track);
        break;
    case SelectionTarget::PrevTrack:
        moveSelection(MoveDirection::Left, MoveSelectionType::Track);
        break;
    case SelectionTarget::NextFrame:
        moveSelection(MoveDirection::Right, MoveSelectionType::Frame);
        break;
    case SelectionTarget::PrevFrame:
        moveSelection(MoveDirection::Left, MoveSelectionType::Frame);
        break;
    case SelectionTarget::NextSystem:
        moveSelection(MoveDirection::Right, MoveSelectionType::System);
        break;
    case SelectionTarget::PrevSystem:
        moveSelection(MoveDirection::Left, MoveSelectionType::System);
        break;
    case SelectionTarget::UpNoteInChord:
        moveChordNoteSelection(MoveDirection::Up);
        break;
    case SelectionTarget::DownNoteInChord:
        moveChordNoteSelection(MoveDirection::Down);
        break;
    case SelectionTarget::TopNoteInChord:
        selectTopOrBottomOfChord(MoveDirection::Up);
        break;
    case SelectionTarget::BottomNoteInChord:
        selectTopOrBottomOfChord(MoveDirection::Down);
        break;
    case SelectionTarget::Similar:
        selectAllSimilarElements();
        break;
    case SelectionTarget::SimilarInStaff:
        selectAllSimilarElementsInStaff();
        break;
    case SelectionTarget::SimilarInRange:
        selectAllSimilarElementsInRange();
        break;
    case SelectionTarget::NotesInChord:
        selectAllNotesInChord();
        break;
    case SelectionTarget::All:
        selectAll();
        break;
    case SelectionTarget::Section:
        selectSection();
        break;
    }
}

void NotationSelection::select(const std::vector<EngravingItem*>& elements, SelectType type, staff_idx_t staffIndex)
{
    TRACEFUNC;

    const mu::engraving::Selection& selection = score()->selection();
    const std::vector<EngravingItem*> oldSelectedElements = selection.elements();
    const mu::engraving::SelState oldSelectionState = selection.state();

    const Fraction oldStartTick = selection.tickStart();
    const Fraction oldEndTick = selection.tickEnd();

    const staff_idx_t oldStartStaff = selection.staffStart();
    const staff_idx_t oldEndStaff = selection.staffEnd();

    doSelect(elements, type, staffIndex);

    bool rangeChanged = false;
    if (selection.isRange()) {
        const bool ticksChanged = oldStartTick != selection.tickStart() || oldEndTick != selection.tickEnd();
        const bool stavesChanged = oldStartStaff != selection.staffStart() || oldEndStaff != selection.staffEnd();
        rangeChanged = ticksChanged || stavesChanged;
    }

    if (rangeChanged || oldSelectionState != selection.state() || oldSelectedElements != selection.elements()) {
        notifyAboutSelectionChangedIfNeed();
    } else {
        score()->setSelectionChanged(false);
    }
}

void NotationSelection::doSelect(const std::vector<EngravingItem*>& elements, SelectType type, staff_idx_t staffIndex)
{
    TRACEFUNC;

    if (elements.size() == 1 && type == SelectType::ADD && QGuiApplication::keyboardModifiers() == Qt::KeyboardModifier::ControlModifier) {
        if (score()->selection().isRange()) {
            score()->selection().setState(mu::engraving::SelState::LIST);
            score()->setUpdateAll();
        }

        if (elements.front()->selected()) {
            score()->deselect(elements.front());
            return;
        }
    }

    if (type == SelectType::REPLACE) {
        score()->deselectAll();
        type = SelectType::ADD;
    }

    if (type == SelectType::SINGLE && elements.size() == 1) {
        const mu::engraving::EngravingItem* element = elements.front();
        mu::engraving::Segment* segment = nullptr;

        if (element->isKeySig()) {
            segment = mu::engraving::toKeySig(element)->segment();
        } else if (element->isTimeSig()) {
            segment = mu::engraving::toTimeSig(element)->segment();
        }

        if (segment) {
            selectElementsWithSameTypeOnSegment(element->type(), segment);
            return;
        }
    }

    score()->select(elements, type, staffIndex);
}

void NotationSelection::clearSelection()
{
    score()->deselectAll();

    notifyAboutSelectionChangedIfNeed();
}

void NotationSelection::notifyAboutSelectionChangedIfNeed()
{
    if (!score()->selectionChanged()) {
        return;
    }

    TRACEFUNC;

    score()->setSelectionChanged(false);

    m_selectionChanged.notify();
}

muse::async::Notification NotationSelection::selectionChanged() const
{
    return m_selectionChanged;
}

void NotationSelection::selectElementsWithSameTypeOnSegment(mu::engraving::ElementType elementType, mu::engraving::Segment* segment)
{
    TRACEFUNC;

    IF_ASSERT_FAILED(segment) {
        return;
    }

    score()->deselectAll();

    std::vector<EngravingItem*> elementsToSelect;

    for (track_idx_t track = 0; track < score()->ntracks(); track += VOICES) {
        EngravingItem* element = segment->element(track);
        if (element && element->type() == elementType) {
            elementsToSelect.push_back(element);
        }
    }

    score()->select(elementsToSelect, SelectType::ADD);
}

void NotationSelection::selectFirstElement(bool frame)
{
    if (EngravingItem* element = Navigation::firstElement(score(), frame)) {
        select({ element }, SelectType::SINGLE, element->staffIdx());
        m_interaction->showItem(element);
    }
}

void NotationSelection::selectLastElement()
{
    if (EngravingItem* element = Navigation::lastElement(score())) {
        select({ element }, SelectType::SINGLE, element->staffIdx());
        m_interaction->showItem(element);
    }
}

void NotationSelection::moveSelection(MoveDirection d, MoveSelectionType type)
{
    IF_ASSERT_FAILED(MoveSelectionType::Undefined != type) {
        return;
    }

    if (type != MoveSelectionType::String) {
        IF_ASSERT_FAILED(MoveDirection::Left == d || MoveDirection::Right == d) {
            return;
        }
    } else {
        IF_ASSERT_FAILED(MoveDirection::Up == d || MoveDirection::Down == d) {
            return;
        }
    }

    if (MoveSelectionType::EngravingItem == type) {
        moveElementSelection(d);
        return;
    }

    if (MoveSelectionType::String == type) {
        moveStringSelection(d);
        return;
    }

    // TODO: rewrite, Score::move( ) logic needs to be here

    auto typeToString = [](MoveSelectionType type) {
        switch (type) {
        case MoveSelectionType::Undefined: return QString();
        case MoveSelectionType::EngravingItem:   return QString();
        case MoveSelectionType::Chord:     return QString("chord");
        case MoveSelectionType::Measure:   return QString("measure");
        case MoveSelectionType::Track:     return QString("track");
        case MoveSelectionType::Frame:     return QString("frame");
        case MoveSelectionType::System:    return QString("system");
        case MoveSelectionType::String:   return QString();
        }
        return QString();
    };

    QString cmd;
    if (MoveDirection::Left == d) {
        cmd = "prev-";
    } else if (MoveDirection::Right == d) {
        cmd = "next-";
    }

    cmd += typeToString(type);

    mu::engraving::EngravingItem* item = Navigation::move(score(), cmd);
    m_interaction->resetHitElementContext();

    notifyAboutSelectionChangedIfNeed();
    m_interaction->showItem(item);
}

void NotationSelection::moveElementSelection(MoveDirection d)
{
    EngravingItem* el = score()->selection().element();
    if (!el && !score()->selection().elements().empty()) {
        el = score()->selection().elements().back();
    }

    if (m_interaction->isTextEditingStarted() && el && el->isTextBase()) {
        m_interaction->navigateToNearText(d);
        return;
    }

    const bool isLeftDirection = MoveDirection::Left == d;
    const bool isHorizontalLayout = score()->isLayoutMode(LayoutMode::LINE) || score()->isLayoutMode(LayoutMode::HORIZONTAL_FIXED);

    // VBoxes are not included in horizontal layouts - skip over them (and their contents) when moving selections...
    const auto nextNonVBox = [this, isLeftDirection](EngravingItem* currElem) -> EngravingItem* {
        IF_ASSERT_FAILED(currElem) {
            return nullptr;
        }

        while (const EngravingItem* vBox = currElem->findAncestor(ElementType::VBOX)) {
            currElem = isLeftDirection ? toVBox(vBox)->prevMM() : toVBox(vBox)->nextMM();
            if (currElem && currElem->isMeasure()) {
                const ChordRest* cr = score()->selection().currentCR();
                const staff_idx_t si = cr ? cr->staffIdx() : 0;
                Measure* mb = toMeasure(currElem);
                currElem = isLeftDirection ? mb->prevElementStaff(si, currElem) : mb->nextElementStaff(si, currElem);
            }

            if (!currElem) {
                break;
            }
        }

        return currElem;
    };

    EngravingItem* toEl = nullptr;

    if (el) {
        toEl = isLeftDirection ? score()->prevElement() : score()->nextElement();
        if (isHorizontalLayout) {
            toEl = nextNonVBox(toEl);
        }
    } else {
        // Nothing currently selected (e.g. because user pressed Esc or clicked on
        // an empty region of the page). Try to restore previous selection.
        if (ChordRest* cr = score()->selection().currentCR()) {
            el = cr->isChord() ? toChord(cr)->upNote() : toEngravingItem(cr);
        }
        if (el) {
            toEl = el; // Restoring previous selection.
        } else {
            toEl = isLeftDirection ? Navigation::lastElement(score()) : Navigation::firstElement(score());
            if (isHorizontalLayout) {
                toEl = nextNonVBox(toEl);
            }
        }
    }

    if (!toEl) {
        return;
    }

    if (m_interaction->isEditingElement()) {
        m_interaction->endEditElement();
    }

    select({ toEl }, SelectType::REPLACE);
    m_interaction->resetHitElementContext();
    m_interaction->showItem(toEl);

    if (toEl->isNote() || toEl->isHarmony()) {
        score()->setPlayNote(true);
    }

    if (toEl->needStartEditingAfterSelecting()) {
        m_interaction->startEditElement(toEl);
    }
}

void NotationSelection::moveStringSelection(MoveDirection d)
{
    mu::engraving::InputState& is = score()->inputState();
    mu::engraving::Staff* staff = score()->staff(track2staff(is.track()));
    int instrStrgs = static_cast<int>(staff->part()->stringData(is.tick(), staff->idx())->strings());
    int delta = (staff->staffType(is.tick())->upsideDown() ? -1 : 1);

    if (MoveDirection::Up == d) {
        delta = -delta;
    }

    int strg = is.string() + delta;
    if (strg >= 0 && strg < instrStrgs && strg != is.string()) {
        is.setString(strg);

        const ChordRest* chordRest = is.cr();
        if (chordRest && chordRest->isChord()) {
            const Chord* chord = toChord(chordRest);

            for (Note* note : chord->notes()) {
                if (note->string() == strg) {
                    select({ note }, SelectType::SINGLE);
                }
            }
        }
    }
}

void NotationSelection::moveSegmentSelection(MoveDirection d)
{
    IF_ASSERT_FAILED(MoveDirection::Left == d || MoveDirection::Right == d) {
        return;
    }

    EngravingItem* e = this->element();
    if (!e && !this->elements().empty()) {
        e = d == MoveDirection::Left ? this->elements().front() : this->elements().back();
    }

    if (!e || (e = d == MoveDirection::Left ? e->prevSegmentElement() : e->nextSegmentElement()) == nullptr) {
        e = d == MoveDirection::Left ? Navigation::firstElement(score()) : Navigation::lastElement(score());
    }

    select({ e }, SelectType::SINGLE);
    m_interaction->showItem(e);
}

void NotationSelection::moveChordNoteSelection(MoveDirection d)
{
    IF_ASSERT_FAILED(MoveDirection::Up == d || MoveDirection::Down == d) {
        return;
    }

    EngravingItem* current = this->element();
    if (!current || !(current->isNote() || current->isRest())) {
        return;
    }

    EngravingItem* chordElem;
    if (d == MoveDirection::Up) {
        chordElem = Navigation::chordNoteAbove(score(), current);
    } else {
        chordElem = Navigation::chordNoteBelow(score(), current);
    }

    if (chordElem == current) {
        return;
    }

    select({ chordElem }, SelectType::SINGLE, chordElem->staffIdx());
    m_interaction->showItem(chordElem);
}

void NotationSelection::selectTopOrBottomOfChord(MoveDirection d)
{
    IF_ASSERT_FAILED(MoveDirection::Up == d || MoveDirection::Down == d) {
        return;
    }

    EngravingItem* current = this->element();
    if (!current || !current->isNote()) {
        return;
    }

    EngravingItem* target = d == MoveDirection::Up
                            ? Navigation::topNoteInChord(toNote(current)) : Navigation::bottomNoteInChord(toNote(current));

    if (target == current) {
        return;
    }

    select({ target }, SelectType::SINGLE);
    m_interaction->showItem(target);
}

FilterElementsOptions NotationSelection::elementsFilterOptions(const EngravingItem* element) const
{
    TRACEFUNC;
    FilterElementsOptions options;
    options.elementType = element->type();

    if (element->isNote()) {
        const mu::engraving::Note* note = dynamic_cast<const mu::engraving::Note*>(element);
        if (note->chord()->isGrace()) {
            options.subtype = -1;
        } else {
            options.subtype = element->subtype();
        }
    } else if (element->isHairpinSegment() || element->isHarmony()) {
        options.subtype = element->subtype();
        options.bySubtype = true;
    }

    return options;
}

void NotationSelection::selectAllSimilarElements()
{
    TRACEFUNC;
    auto notationElements = m_interaction->elements();
    if (!notationElements) {
        return;
    }

    EngravingItem* selectedElement = this->element();
    if (!selectedElement) {
        return;
    }

    FilterElementsOptions options = elementsFilterOptions(selectedElement);
    std::vector<EngravingItem*> elements = notationElements->elements(options);
    if (elements.empty()) {
        return;
    }

    clearSelection();

    select(elements, SelectType::ADD);
}

void NotationSelection::selectAllSimilarElementsInStaff()
{
    TRACEFUNC;
    auto notationElements = m_interaction->elements();
    if (!notationElements) {
        return;
    }

    EngravingItem* selectedElement = this->element();
    if (!selectedElement) {
        return;
    }

    FilterElementsOptions options = elementsFilterOptions(selectedElement);
    options.staffStart = static_cast<int>(selectedElement->staffIdx());
    options.staffEnd = options.staffStart + 1;

    std::vector<EngravingItem*> elements = notationElements->elements(options);
    if (elements.empty()) {
        return;
    }

    clearSelection();

    select(elements, SelectType::ADD);
}

void NotationSelection::selectAllSimilarElementsInRange()
{
    auto elements = m_interaction->elements();
    if (!elements) {
        return;
    }

    mu::engraving::EngravingItem* lastHit = this->lastElementHit();
    if (!lastHit) {
        return;
    }

    mu::engraving::Score* score = elements->msScore();
    score->selectSimilarInRange(lastHit);
    notifyAboutSelectionChangedIfNeed();
}

void NotationSelection::selectAllNotesInChord()
{
    TRACEFUNC;

    const std::vector<EngravingItem*>& selectedElements = this->elements();
    if (selectedElements.empty()) {
        return;
    }

    std::set<const Chord*> chords;
    for (const EngravingItem* item : selectedElements) {
        if (item->isNote()) {
            chords.insert(toNote(item)->chord());
        }
    }

    if (chords.empty()) {
        return;
    }

    std::vector<EngravingItem*> allNotes;
    for (const Chord* chord : chords) {
        for (Note* note : chord->notes()) {
            allNotes.push_back(note);
        }
    }

    clearSelection();
    select(allNotes, SelectType::ADD);
}

void NotationSelection::selectAll()
{
    if (m_interaction->isTextEditingStarted()) {
        m_interaction->selectAllText();
    } else {
        score()->cmdSelectAll();
    }

    notifyAboutSelectionChangedIfNeed();
}

void NotationSelection::selectSection()
{
    score()->cmdSelectSection();

    notifyAboutSelectionChangedIfNeed();
}
