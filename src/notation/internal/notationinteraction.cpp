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
#include "notationinteraction.h"

#include "log.h"

#include <memory>
#include <QRectF>
#include <QPainter>
#include <QClipboard>
#include <QApplication>
#include <QKeyEvent>
#include <QMimeData>
#include <QDrag>

#include "defer.h"
#include "ptrutils.h"
#include "containers.h"

#include "engraving/rw/xml.h"
#include "draw/types/pen.h"
#include "draw/types/painterpath.h"
#include "engraving/internal/qmimedataadapter.h"

#include "libmscore/actionicon.h"
#include "libmscore/bracket.h"
#include "libmscore/chord.h"
#include "libmscore/drumset.h"
#include "libmscore/elementgroup.h"
#include "libmscore/factory.h"
#include "libmscore/figuredbass.h"
#include "libmscore/image.h"
#include "libmscore/instrchange.h"
#include "libmscore/keysig.h"
#include "libmscore/lasso.h"
#include "libmscore/layoutbreak.h"
#include "libmscore/linkedobjects.h"
#include "libmscore/lyrics.h"
#include "libmscore/masterscore.h"
#include "libmscore/measure.h"
#include "libmscore/mscore.h"
#include "libmscore/navigate.h"
#include "libmscore/page.h"
#include "libmscore/part.h"
#include "libmscore/rest.h"
#include "libmscore/shadownote.h"
#include "libmscore/slur.h"
#include "libmscore/staff.h"
#include "libmscore/stafflines.h"
#include "libmscore/stafftype.h"
#include "libmscore/system.h"
#include "libmscore/textedit.h"
#include "libmscore/textframe.h"
#include "libmscore/tuplet.h"
#include "libmscore/undo.h"

#include "masternotation.h"
#include "scorecallbacks.h"
#include "notationnoteinput.h"
#include "notationselection.h"
#include "notationerrors.h"

using namespace mu::io;
using namespace mu::notation;
using namespace mu::framework;
using namespace mu::engraving;

static mu::engraving::KeyboardModifier keyboardModifier(Qt::KeyboardModifiers km)
{
    return mu::engraving::KeyboardModifier(int(km));
}

static qreal nudgeDistance(const mu::engraving::EditData& editData)
{
    qreal spatium = editData.element->spatium();

    if (editData.element->isBeam()) {
        if (editData.modifiers & Qt::ControlModifier) {
            return spatium;
        } else if (editData.modifiers & Qt::AltModifier) {
            return spatium * 4;
        }

        return spatium * 0.25;
    }

    if (editData.modifiers & Qt::ControlModifier) {
        return spatium * mu::engraving::MScore::nudgeStep10;
    } else if (editData.modifiers & Qt::AltModifier) {
        return spatium * mu::engraving::MScore::nudgeStep50;
    }

    return spatium * mu::engraving::MScore::nudgeStep;
}

static qreal nudgeDistance(const mu::engraving::EditData& editData, qreal raster)
{
    qreal distance = nudgeDistance(editData);
    if (raster > 0) {
        raster = editData.element->spatium() / raster;
        if (distance < raster) {
            distance = raster;
        }
    }

    return distance;
}

static PointF bindCursorPosToText(const PointF& cursorPos, const EngravingItem* text)
{
    if (!text || !text->isTextBase()) {
        return PointF();
    }

    mu::RectF bbox = text->canvasBoundingRect();
    mu::PointF boundPos = mu::PointF(
        cursorPos.x() < bbox.left() ? bbox.left()
        : cursorPos.x() >= bbox.right() ? bbox.right() - 1 : cursorPos.x(),
        cursorPos.y() < bbox.top() ? bbox.top()
        : cursorPos.y() >= bbox.bottom() ? bbox.bottom() - 1 : cursorPos.y());

    return boundPos;
}

NotationInteraction::NotationInteraction(Notation* notation, INotationUndoStackPtr undoStack)
    : m_notation(notation), m_undoStack(undoStack), m_editData(&m_scoreCallbacks)
{
    m_noteInput = std::make_shared<NotationNoteInput>(notation, this, m_undoStack);
    m_selection = std::make_shared<NotationSelection>(notation);

    m_noteInput->stateChanged().onNotify(this, [this]() {
        if (!m_noteInput->isNoteInputMode()) {
            hideShadowNote();
        }
    });

    m_undoStack->undoNotification().onNotify(this, [this]() {
        endEditElement();
    });

    m_undoStack->redoNotification().onNotify(this, [this]() {
        endEditElement();
    });

    m_undoStack->stackChanged().onNotify(this, [this]() {
        notifyAboutSelectionChangedIfNeed();
    });

    m_dragData.ed = mu::engraving::EditData(&m_scoreCallbacks);
    m_dropData.ed = mu::engraving::EditData(&m_scoreCallbacks);

    m_scoreCallbacks.setNotationInteraction(this);

    m_notation->scoreInited().onNotify(this, [this]() {
        onScoreInited();
    });
}

mu::engraving::Score* NotationInteraction::score() const
{
    return m_notation->score();
}

void NotationInteraction::onScoreInited()
{
    if (!score()) {
        return;
    }

    m_scoreCallbacks.setScore(score());

    score()->elementDestroyed().onReceive(this, [this](mu::engraving::EngravingItem* element) {
        onElementDestroyed(element);
    });
}

void NotationInteraction::startEdit()
{
    m_notifyAboutDropChanged = false;

    m_undoStack->prepareChanges();
}

void NotationInteraction::apply()
{
    m_undoStack->commitChanges();

    if (m_notifyAboutDropChanged) {
        notifyAboutDropChanged();
    } else {
        notifyAboutNotationChanged();
    }
}

void NotationInteraction::rollback()
{
    m_undoStack->rollbackChanges();
}

void NotationInteraction::notifyAboutDragChanged()
{
    m_dragChanged.notify();
}

void NotationInteraction::notifyAboutDropChanged()
{
    m_dropChanged.notify();
}

void NotationInteraction::notifyAboutNotationChanged()
{
    TRACEFUNC;

    m_notation->notifyAboutNotationChanged();
}

void NotationInteraction::notifyAboutTextEditingStarted()
{
    m_textEditingStarted.notify();
}

void NotationInteraction::notifyAboutTextEditingChanged()
{
    m_textEditingChanged.notify();
}

void NotationInteraction::notifyAboutTextEditingEnded()
{
    m_textEditingEnded.notify();
}

void NotationInteraction::notifyAboutSelectionChangedIfNeed()
{
    if (!score()->selectionChanged()) {
        return;
    }

    TRACEFUNC;

    score()->setSelectionChanged(false);

    m_selectionChanged.notify();
}

void NotationInteraction::notifyAboutNoteInputStateChanged()
{
    m_noteInput->stateChanged().notify();
}

void NotationInteraction::paint(mu::draw::Painter* painter)
{
    score()->shadowNote().draw(painter);

    drawAnchorLines(painter);
    drawTextEditMode(painter);
    drawSelectionRange(painter);
    drawGripPoints(painter);

    if (m_lasso && !m_lasso->isEmpty()) {
        m_lasso->draw(painter);
    }

    if (m_dropData.dropRect.isValid()) {
        painter->fillRect(m_dropData.dropRect, configuration()->dropRectColor());
    }
}

INotationNoteInputPtr NotationInteraction::noteInput() const
{
    return m_noteInput;
}

void NotationInteraction::showShadowNote(const PointF& pos)
{
    const mu::engraving::InputState& inputState = score()->inputState();
    mu::engraving::ShadowNote& shadowNote = score()->shadowNote();

    mu::engraving::Position position;
    if (!score()->getPosition(&position, pos, inputState.voice())) {
        shadowNote.setVisible(false);
        return;
    }

    Staff* staff = score()->staff(position.staffIdx);
    const mu::engraving::Instrument* instr = staff->part()->instrument();

    mu::engraving::Segment* segment = position.segment;
    qreal segmentSkylineTopY = 0;
    qreal segmentSkylineBottomY = 0;

    mu::engraving::Segment* shadowNoteActualSegment = position.segment->prev1enabled();
    if (shadowNoteActualSegment) {
        segment = shadowNoteActualSegment;
        segmentSkylineTopY = shadowNoteActualSegment->elementsTopOffsetFromSkyline(position.staffIdx);
        segmentSkylineBottomY = shadowNoteActualSegment->elementsBottomOffsetFromSkyline(position.staffIdx);
    }

    Fraction tick = segment->tick();
    qreal mag = staff->staffMag(tick);

    // in any empty measure, pos will be right next to barline
    // so pad this by barNoteDistance
    qreal relX = position.pos.x() - position.segment->measure()->canvasPos().x();
    position.pos.rx() -= qMin(relX - score()->styleMM(mu::engraving::Sid::barNoteDistance) * mag, 0.0);

    mu::engraving::NoteHeadGroup noteheadGroup = mu::engraving::NoteHeadGroup::HEAD_NORMAL;
    mu::engraving::NoteHeadType noteHead = inputState.duration().headType();
    int line = position.line;

    if (instr->useDrumset()) {
        const mu::engraving::Drumset* ds  = instr->drumset();
        int pitch = inputState.drumNote();
        if (pitch >= 0 && ds->isValid(pitch)) {
            line = ds->line(pitch);
            noteheadGroup = ds->noteHead(pitch);
        }
    }

    voice_idx_t voice = 0;
    if (inputState.drumNote() != -1 && inputState.drumset() && inputState.drumset()->isValid(inputState.drumNote())) {
        voice = inputState.drumset()->voice(inputState.drumNote());
    } else {
        voice = inputState.voice();
    }

    shadowNote.setVisible(true);
    shadowNote.setMag(mag);
    shadowNote.setTick(tick);
    shadowNote.setStaffIdx(position.staffIdx);
    shadowNote.setVoice(voice);
    shadowNote.setLineIndex(line);

    mu::engraving::SymId symNotehead;
    mu::engraving::TDuration duration(inputState.duration());

    if (inputState.rest()) {
        int yo = 0;
        mu::engraving::Rest* rest = mu::engraving::Factory::createRest(mu::engraving::gpaletteScore->dummy()->segment(), duration.type());
        rest->setTicks(duration.fraction());
        symNotehead = rest->getSymbol(inputState.duration().type(), 0, staff->lines(position.segment->tick()), &yo);
        shadowNote.setState(symNotehead, duration, true, segmentSkylineTopY, segmentSkylineBottomY);
        delete rest;
    } else {
        if (mu::engraving::NoteHeadGroup::HEAD_CUSTOM == noteheadGroup) {
            symNotehead = instr->drumset()->noteHeads(inputState.drumNote(), noteHead);
        } else {
            symNotehead = Note::noteHead(0, noteheadGroup, noteHead);
        }

        shadowNote.setState(symNotehead, duration, false, segmentSkylineTopY, segmentSkylineBottomY);
    }

    shadowNote.layout();
    shadowNote.setPos(position.pos);
}

void NotationInteraction::hideShadowNote()
{
    score()->shadowNote().setVisible(false);
}

void NotationInteraction::toggleVisible()
{
    startEdit();

    // TODO: Update `score()->cmdToggleVisible()` and call that here?
    for (EngravingItem* el : selection()->elements()) {
        if (el->isBracket()) {
            continue;
        }
        el->undoChangeProperty(mu::engraving::Pid::VISIBLE, !el->visible());
    }

    apply();
}

EngravingItem* NotationInteraction::hitElement(const PointF& pos, float width) const
{
    std::vector<mu::engraving::EngravingItem*> elements = hitElements(pos, width);
    if (elements.empty()) {
        return nullptr;
    }
    m_selection->onElementHit(elements.back());
    return elements.back();
}

Staff* NotationInteraction::hitStaff(const PointF& pos) const
{
    return hitMeasure(pos).staff;
}

mu::engraving::Page* NotationInteraction::point2page(const PointF& p) const
{
    if (score()->linearMode()) {
        return score()->pages().empty() ? 0 : score()->pages().front();
    }
    for (mu::engraving::Page* page : score()->pages()) {
        if (page->bbox().translated(page->pos()).contains(p)) {
            return page;
        }
    }
    return nullptr;
}

std::vector<EngravingItem*> NotationInteraction::elementsAt(const PointF& p) const
{
    mu::engraving::Page* page = point2page(p);
    if (!page) {
        return {};
    }

    std::vector<EngravingItem*> el = page->items(p - page->pos());
    if (el.empty()) {
        return {};
    }

    std::sort(el.begin(), el.end(), NotationInteraction::elementIsLess);

    return el;
}

EngravingItem* NotationInteraction::elementAt(const PointF& p) const
{
    std::vector<EngravingItem*> el = elementsAt(p);
    return el.empty() || el.back()->isPage() ? nullptr : el.back();
}

std::vector<mu::engraving::EngravingItem*> NotationInteraction::hitElements(const PointF& p_in, float w) const
{
    mu::engraving::Page* page = point2page(p_in);
    if (!page) {
        return {};
    }

    std::vector<mu::engraving::EngravingItem*> ll;

    PointF p = p_in - page->pos();

    if (isTextEditingStarted()) {
        auto editW = w * 2;
        RectF hitRect(p.x() - editW, p.y() - editW, 2.0 * editW, 2.0 * editW);
        if (m_editData.element->intersects(hitRect)) {
            ll.push_back(m_editData.element);
            return ll;
        }
    }

    RectF r(p.x() - w, p.y() - w, 3.0 * w, 3.0 * w);

    std::vector<mu::engraving::EngravingItem*> elements = page->items(r);

    for (int i = 0; i < mu::engraving::MAX_HEADERS; ++i) {
        if (score()->headerText(i) != nullptr) { // gives the ability to select the header
            elements.push_back(score()->headerText(i));
        }
    }

    for (int i = 0; i < mu::engraving::MAX_FOOTERS; ++i) {
        if (score()->footerText(i) != nullptr) { // gives the ability to select the footer
            elements.push_back(score()->footerText(i));
        }
    }

    for (mu::engraving::EngravingItem* element : elements) {
        element->itemDiscovered = 0;
        if (!element->selectable() || element->isPage()) {
            continue;
        }

        if (!element->isInteractionAvailable()) {
            continue;
        }

        if (element->contains(p)) {
            ll.push_back(element);
        }
    }

    if (ll.empty() || (ll.size() == 1 && ll.front()->isMeasure())) {
        //
        // if no relevant element hit, look nearby
        //
        for (mu::engraving::EngravingItem* element : elements) {
            if (element->isPage() || !element->selectable()) {
                continue;
            }

            if (!element->isInteractionAvailable()) {
                continue;
            }

            if (element->intersects(r)) {
                ll.push_back(element);
            }
        }
    }

    if (!ll.empty()) {
        std::sort(ll.begin(), ll.end(), NotationInteraction::elementIsLess);
    } else {
        mu::engraving::Measure* measure = hitMeasure(p_in).measure;
        if (measure) {
            ll.push_back(measure);
        }
    }

    return ll;
}

NotationInteraction::HitMeasureData NotationInteraction::hitMeasure(const PointF& pos) const
{
    mu::engraving::staff_idx_t staffIndex = mu::nidx;
    mu::engraving::Segment* segment = nullptr;
    PointF offset;
    Measure* measure = score()->pos2measure(pos, &staffIndex, 0, &segment, &offset);

    HitMeasureData result;
    if (measure && measure->staffLines(staffIndex)->canvasBoundingRect().contains(pos)) {
        result.measure = measure;
        result.staff = score()->staff(staffIndex);
    }

    return result;
}

bool NotationInteraction::elementIsLess(const EngravingItem* e1, const EngravingItem* e2)
{
    if (e1->selectable() && !e2->selectable()) {
        return false;
    }
    if (!e1->selectable() && e2->selectable()) {
        return true;
    }
    if (e1->isNote() && (e2->isStem() || e2->isHook())) {
        return false;
    }
    if (e2->isNote() && (e1->isStem() || e1->isHook())) {
        return true;
    }
    if (e1->isStem() && e2->isHook()) {
        return false;
    }
    if (e2->isStem() && e1->isHook()) {
        return true;
    }
    if (e1->isText() && e2->isBox()) {
        return false;
    }
    if (e1->isBox() && e2->isText()) {
        return true;
    }
    if (e1->z() == e2->z()) {
        // same stacking order, prefer non-hidden elements
        if (e1->type() == e2->type()) {
            if (e1->isNoteDot()) {
                const NoteDot* n1 = toNoteDot(e1);
                const NoteDot* n2 = toNoteDot(e2);
                if (n1->note() && n1->note()->hidden()) {
                    return false;
                } else if (n2->note() && n2->note()->hidden()) {
                    return true;
                }
            } else if (e1->isNote()) {
                const Note* n1 = toNote(e1);
                const Note* n2 = toNote(e2);
                if (n1->hidden()) {
                    return false;
                } else if (n2->hidden()) {
                    return true;
                }
            }
        }
        // different types, or same type but nothing hidden - use track
        return e1->track() < e2->track();
    }

    // default case, use stacking order
    return e1->z() < e2->z();
}

const NotationInteraction::HitElementContext& NotationInteraction::hitElementContext() const
{
    return m_hitElementContext;
}

void NotationInteraction::setHitElementContext(const HitElementContext& context)
{
    m_hitElementContext = context;
}

void NotationInteraction::moveChordNoteSelection(MoveDirection d)
{
    IF_ASSERT_FAILED(MoveDirection::Up == d || MoveDirection::Down == d) {
        return;
    }

    EngravingItem* current = selection()->element();
    if (!current || !(current->isNote() || current->isRest())) {
        return;
    }

    EngravingItem* chordElem;
    if (d == MoveDirection::Up) {
        chordElem = score()->upAlt(current);
    } else {
        chordElem = score()->downAlt(current);
    }

    if (chordElem == current) {
        return;
    }

    select({ chordElem }, SelectType::SINGLE, chordElem->staffIdx());
    showItem(chordElem);
}

void NotationInteraction::moveSegmentSelection(MoveDirection d)
{
    IF_ASSERT_FAILED(MoveDirection::Left == d || MoveDirection::Right == d) {
        return;
    }

    EngravingItem* e = selection()->element();
    if (!e && !selection()->elements().empty()) {
        e = d == MoveDirection::Left ? selection()->elements().front() : selection()->elements().back();
    }

    if (!e || (e = d == MoveDirection::Left ? e->prevSegmentElement() : e->nextSegmentElement()) == nullptr) {
        e = d == MoveDirection::Left ? score()->firstElement() : score()->lastElement();
    }

    select({ e }, SelectType::SINGLE);
    showItem(e);
}

void NotationInteraction::selectTopOrBottomOfChord(MoveDirection d)
{
    IF_ASSERT_FAILED(MoveDirection::Up == d || MoveDirection::Down == d) {
        return;
    }

    EngravingItem* current = selection()->element();
    if (!current || !current->isNote()) {
        return;
    }

    EngravingItem* target = d == MoveDirection::Up
                            ? score()->upAltCtrl(toNote(current)) : score()->downAltCtrl(toNote(current));

    if (target == current) {
        return;
    }

    select({ target }, SelectType::SINGLE);
    showItem(target);
}

void NotationInteraction::select(const std::vector<EngravingItem*>& elements, SelectType type, staff_idx_t staffIndex)
{
    TRACEFUNC;

    const mu::engraving::Selection& selection = score()->selection();
    std::vector<EngravingItem*> oldSelectedElements = selection.elements();
    mu::engraving::SelState oldSelectionState = selection.state();

    doSelect(elements, type, staffIndex);

    if (oldSelectedElements != selection.elements() || oldSelectionState != selection.state()) {
        notifyAboutSelectionChangedIfNeed();
    } else {
        score()->setSelectionChanged(false);
    }
}

void NotationInteraction::doSelect(const std::vector<EngravingItem*>& elements, SelectType type, staff_idx_t staffIndex)
{
    TRACEFUNC;

    if (needEndTextEditing(elements)) {
        endEditText();
    } else if (isElementEditStarted() && (elements.size() != 1 || elements.front() != score()->selection().element())) {
        endEditElement();
    }

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

    for (EngravingItem* element: elements) {
        score()->select(element, type, staffIndex);
    }
}

void NotationInteraction::selectElementsWithSameTypeOnSegment(mu::engraving::ElementType elementType, mu::engraving::Segment* segment)
{
    TRACEFUNC;

    IF_ASSERT_FAILED(segment) {
        return;
    }

    score()->deselectAll();

    for (size_t staffIdx = 0; staffIdx < score()->nstaves(); ++staffIdx) {
        EngravingItem* element = segment->element(staffIdx * mu::engraving::VOICES);
        if (element && element->type() == elementType) {
            score()->select(element, SelectType::ADD);
        }
    }
}

void NotationInteraction::selectAll()
{
    if (isTextEditingStarted()) {
        auto textBase = toTextBase(m_editData.element);
        textBase->selectAll(textBase->cursorFromEditData(m_editData));
    } else {
        score()->cmdSelectAll();
    }

    notifyAboutSelectionChangedIfNeed();
}

void NotationInteraction::selectSection()
{
    score()->cmdSelectSection();

    notifyAboutSelectionChangedIfNeed();
}

void NotationInteraction::selectFirstElement(bool frame)
{
    if (EngravingItem* element = score()->firstElement(frame)) {
        select({ element }, SelectType::SINGLE, element->staffIdx());
    }
}

void NotationInteraction::selectLastElement()
{
    if (EngravingItem* element = score()->lastElement()) {
        select({ element }, SelectType::SINGLE, element->staffIdx());
    }
}

INotationSelectionPtr NotationInteraction::selection() const
{
    return m_selection;
}

void NotationInteraction::clearSelection()
{
    TRACEFUNC;

    if (isElementEditStarted()) {
        endEditElement();
    } else if (m_editData.element) {
        m_editData.element = nullptr;
    }

    if (isDragStarted()) {
        doEndDrag();
        rollback();
        notifyAboutNotationChanged();
    }

    if (selection()->isNone()) {
        return;
    }

    score()->deselectAll();

    notifyAboutSelectionChangedIfNeed();

    setHitElementContext(HitElementContext());
}

mu::async::Notification NotationInteraction::selectionChanged() const
{
    return m_selectionChanged;
}

bool NotationInteraction::isSelectionTypeFiltered(SelectionFilterType type) const
{
    return score()->selectionFilter().isFiltered(type);
}

void NotationInteraction::setSelectionTypeFiltered(SelectionFilterType type, bool filtered)
{
    score()->selectionFilter().setFiltered(type, filtered);
    if (selection()->isRange()) {
        score()->selection().updateSelectedElements();
        notifyAboutSelectionChangedIfNeed();
    }
}

bool NotationInteraction::isDragStarted() const
{
    if (m_dragData.dragGroups.size() > 0) {
        return true;
    }

    if (m_lasso && !m_lasso->isEmpty()) {
        return true;
    }

    return false;
}

void NotationInteraction::DragData::reset()
{
    beginMove = QPointF();
    elementOffset = QPointF();
    ed = mu::engraving::EditData(ed.view());
    dragGroups.clear();
}

void NotationInteraction::startDrag(const std::vector<EngravingItem*>& elems,
                                    const PointF& eoffset,
                                    const IsDraggable& isDraggable)
{
    m_dragData.reset();
    m_dragData.elements = elems;
    m_dragData.elementOffset = eoffset;
    m_editData.modifiers = keyboardModifier(QGuiApplication::keyboardModifiers());

    for (EngravingItem* e : m_dragData.elements) {
        bool draggable = isDraggable(e);

        if (!draggable && e->isSpanner()) {
            draggable = isDraggable(toSpanner(e)->frontSegment());
        }

        if (!draggable) {
            continue;
        }

        std::unique_ptr<mu::engraving::ElementGroup> g = e->getDragGroup(isDraggable);
        if (g && g->enabled()) {
            m_dragData.dragGroups.push_back(std::move(g));
        }
    }

    startEdit();

    qreal scaling = m_notation->viewState()->matrix().m11();
    qreal proximity = configuration()->selectionProximity() * 0.5f / scaling;
    m_scoreCallbacks.setSelectionProximity(proximity);

    if (isGripEditStarted()) {
        m_editData.element->startEditDrag(m_editData);
        return;
    }

    m_dragData.ed.modifiers = keyboardModifier(QGuiApplication::keyboardModifiers());
    for (auto& group : m_dragData.dragGroups) {
        group->startDrag(m_dragData.ed);
    }
}

void NotationInteraction::doDragLasso(const PointF& pt)
{
    if (!m_lasso) {
        m_lasso = new mu::engraving::Lasso(score());
    }

    score()->addRefresh(m_lasso->canvasBoundingRect());
    RectF r;
    r.setCoords(m_dragData.beginMove.x(), m_dragData.beginMove.y(), pt.x(), pt.y());
    m_lasso->setbbox(r.normalized());
    score()->addRefresh(m_lasso->canvasBoundingRect());
    score()->lassoSelect(m_lasso->bbox());
    score()->update();
}

void NotationInteraction::endLasso()
{
    if (!m_lasso) {
        return;
    }

    score()->addRefresh(m_lasso->canvasBoundingRect());
    m_lasso->setbbox(RectF());
    score()->lassoSelectEnd(m_dragData.mode != DragMode::LassoList);
    score()->update();
}

void NotationInteraction::drag(const PointF& fromPos, const PointF& toPos, DragMode mode)
{
    if (m_dragData.beginMove.isNull()) {
        m_dragData.beginMove = fromPos;
        m_dragData.ed.pos = fromPos;
    }
    m_dragData.mode = mode;

    PointF normalizedBegin = m_dragData.beginMove - m_dragData.elementOffset;
    PointF delta = toPos - normalizedBegin;
    PointF evtDelta = toPos - m_dragData.ed.pos;

    switch (mode) {
    case DragMode::BothXY:
    case DragMode::LassoList:
        break;
    case DragMode::OnlyX:
        delta.setY(m_dragData.ed.delta.y());
        evtDelta.setY(0.0);
        break;
    case DragMode::OnlyY:
        delta.setX(m_dragData.ed.delta.x());
        evtDelta.setX(0.0);
        break;
    }

    m_dragData.ed.lastPos = m_dragData.ed.pos;

    m_dragData.ed.hRaster = configuration()->isSnappedToGrid(framework::Orientation::Horizontal);
    m_dragData.ed.vRaster = configuration()->isSnappedToGrid(framework::Orientation::Vertical);
    m_dragData.ed.delta = delta;
    m_dragData.ed.moveDelta = delta - m_dragData.elementOffset;
    m_dragData.ed.evtDelta = evtDelta;
    m_dragData.ed.pos = toPos;
    m_dragData.ed.modifiers = keyboardModifier(QGuiApplication::keyboardModifiers());

    if (isTextEditingStarted()) {
        m_editData.pos = toPos;
        toTextBase(m_editData.element)->dragTo(m_editData);

        notifyAboutTextEditingChanged();
        return;
    }

    if (isGripEditStarted()) {
        m_dragData.ed.curGrip = m_editData.curGrip;
        m_dragData.ed.delta = m_dragData.ed.pos - m_dragData.ed.lastPos;
        m_dragData.ed.moveDelta = m_dragData.ed.delta - m_dragData.elementOffset;
        m_dragData.ed.addData(m_editData.getData(m_editData.element));
        m_editData.element->editDrag(m_dragData.ed);
    } else if (m_editData.element && !m_editData.element->hasGrips()) {
        m_dragData.ed.delta = evtDelta;
        m_editData.element->editDrag(m_dragData.ed);
    } else {
        for (auto& group : m_dragData.dragGroups) {
            score()->addRefresh(group->drag(m_dragData.ed));
        }
    }

    score()->update();

    if (isGripEditStarted()) {
        updateAnchorLines();
    } else {
        std::vector<LineF> anchorLines;
        for (const EngravingItem* e : selection()->elements()) {
            std::vector<LineF> elAnchorLines = e->dragAnchorLines();
            if (!elAnchorLines.empty()) {
                for (LineF& l : elAnchorLines) {
                    anchorLines.push_back(l);
                }
            }
        }
        setAnchorLines(anchorLines);
    }

    if (m_dragData.elements.size() == 0) {
        doDragLasso(toPos);
    }

    notifyAboutDragChanged();
}

void NotationInteraction::doEndDrag()
{
    if (isGripEditStarted()) {
        m_editData.element->endEditDrag(m_editData);
        m_editData.element->endEdit(m_editData);
    } else {
        for (auto& group : m_dragData.dragGroups) {
            group->endDrag(m_dragData.ed);
        }
        if (m_lasso && !m_lasso->isEmpty()) {
            endLasso();
        }
    }

    m_dragData.reset();
    setDropTarget(nullptr, false);
}

void NotationInteraction::endDrag()
{
    doEndDrag();
    apply();
    notifyAboutDragChanged();

    //    updateGrips();
    //    if (editData.element->normalModeEditBehavior() == EngravingItem::EditBehavior::Edit
    //        && score()->selection().element() == editData.element) {
    //        startEdit(/* editMode */ false);
    //    }
}

mu::async::Notification NotationInteraction::dragChanged() const
{
    return m_dragChanged;
}

bool NotationInteraction::isDragCopyStarted() const
{
    return m_drag != nullptr;
}

//! NOTE: Copied from ScoreView::cloneElement
void NotationInteraction::startDragCopy(const EngravingItem* element, QObject* dragSource)
{
    if (!element) {
        return;
    }

    if (element->isMeasure() || element->isNote() || element->isVBox()) {
        return;
    }

    if (isDragStarted()) {
        endDragCopy();
    }

    if (element->isSpannerSegment()) {
        element = toSpannerSegment(element)->spanner();
    }

    QMimeData* mimeData = new QMimeData();
    mimeData->setData(mu::engraving::mimeSymbolFormat, element->mimeData().toQByteArray());

    m_drag = new QDrag(dragSource);
    m_drag->setMimeData(mimeData);

    static QPixmap pixmap(2, 2); // null or 1x1 crashes on Linux under ChromeOS?!
    pixmap.fill(Qt::white);

    m_drag->setPixmap(pixmap);
    m_drag->exec(Qt::CopyAction);
}

void NotationInteraction::endDragCopy()
{
    delete m_drag;
    m_drag = nullptr;
}

//! NOTE Copied from ScoreView::dragEnterEvent
void NotationInteraction::startDrop(const QByteArray& edata)
{
    resetDropElement();

    mu::engraving::XmlReader e(edata);
    m_dropData.ed.dragOffset = QPointF();
    Fraction duration;      // dummy
    ElementType type = EngravingItem::readType(e, &m_dropData.ed.dragOffset, &duration);

    EngravingItem* el = engraving::Factory::createItem(type, score()->dummy());
    if (el) {
        if (type == ElementType::BAR_LINE || type == ElementType::ARPEGGIO || type == ElementType::BRACKET) {
            double spatium = score()->spatium();
            el->setHeight(spatium * 5);
        }
        m_dropData.ed.dropElement = el;
        m_dropData.ed.dropElement->setParent(0);
        m_dropData.ed.dropElement->read(e);
        m_dropData.ed.dropElement->layout();
    }
}

bool NotationInteraction::startDrop(const QUrl& url)
{
    if (url.scheme() != "file") {
        return false;
    }

    auto image = static_cast<mu::engraving::Image*>(Factory::createItem(mu::engraving::ElementType::IMAGE, score()->dummy()));
    if (!image->load(url.toLocalFile())) {
        return false;
    }

    resetDropElement();

    m_dropData.ed.dropElement = image;
    m_dropData.ed.dragOffset = QPointF();
    m_dropData.ed.dropElement->setParent(nullptr);
    m_dropData.ed.dropElement->layout();

    return true;
}

//! NOTE Copied from ScoreView::dragMoveEvent
bool NotationInteraction::isDropAccepted(const PointF& pos, Qt::KeyboardModifiers modifiers)
{
    if (!m_dropData.ed.dropElement) {
        return false;
    }

    m_dropData.ed.pos = pos;
    m_dropData.ed.modifiers = keyboardModifier(modifiers);

    switch (m_dropData.ed.dropElement->type()) {
    case ElementType::VOLTA:
    case ElementType::GRADUAL_TEMPO_CHANGE:
        return dragMeasureAnchorElement(pos);
    case ElementType::PEDAL:
    case ElementType::LET_RING:
    case ElementType::VIBRATO:
    case ElementType::PALM_MUTE:
    case ElementType::OTTAVA:
    case ElementType::TRILL:
    case ElementType::HAIRPIN:
    case ElementType::TEXTLINE:
        return dragTimeAnchorElement(pos);
    case ElementType::IMAGE:
    case ElementType::SYMBOL:
    case ElementType::FSYMBOL:
    case ElementType::DYNAMIC:
    case ElementType::KEYSIG:
    case ElementType::CLEF:
    case ElementType::TIMESIG:
    case ElementType::BAR_LINE:
    case ElementType::ARPEGGIO:
    case ElementType::BREATH:
    case ElementType::GLISSANDO:
    case ElementType::MEASURE_NUMBER:
    case ElementType::BRACKET:
    case ElementType::ARTICULATION:
    case ElementType::FERMATA:
    case ElementType::CHORDLINE:
    case ElementType::BEND:
    case ElementType::ACCIDENTAL:
    case ElementType::TEXT:
    case ElementType::FINGERING:
    case ElementType::TEMPO_TEXT:
    case ElementType::STAFF_TEXT:
    case ElementType::SYSTEM_TEXT:
    case ElementType::TRIPLET_FEEL:
    case ElementType::PLAYTECH_ANNOTATION:
    case ElementType::NOTEHEAD:
    case ElementType::TREMOLO:
    case ElementType::LAYOUT_BREAK:
    case ElementType::MARKER:
    case ElementType::STAFF_STATE:
    case ElementType::INSTRUMENT_CHANGE:
    case ElementType::REHEARSAL_MARK:
    case ElementType::JUMP:
    case ElementType::MEASURE_REPEAT:
    case ElementType::ACTION_ICON:
    case ElementType::CHORD:
    case ElementType::SPACER:
    case ElementType::SLUR:
    case ElementType::HARMONY:
    case ElementType::BAGPIPE_EMBELLISHMENT:
    case ElementType::AMBITUS:
    case ElementType::TREMOLOBAR:
    case ElementType::FIGURED_BASS:
    case ElementType::LYRICS:
    case ElementType::FRET_DIAGRAM:
    case ElementType::STAFFTYPE_CHANGE: {
        EngravingItem* e = dropTarget(m_dropData.ed);
        if (e) {
            if (!e->isMeasure()) {
                setDropTarget(e);
            }
            return true;
        } else {
            return false;
        }
    }
    break;
    default:
        break;
    }

    return false;
}

//! NOTE Copied from ScoreView::dropEvent
bool NotationInteraction::drop(const PointF& pos, Qt::KeyboardModifiers modifiers)
{
    if (!m_dropData.ed.dropElement) {
        return false;
    }

    IF_ASSERT_FAILED(m_dropData.ed.dropElement->score() == score()) {
        return false;
    }

    bool accepted = false;

    m_dropData.ed.pos       = pos;
    m_dropData.ed.modifiers = keyboardModifier(modifiers);
    m_dropData.ed.dropElement->styleChanged();

    bool systemStavesOnly = false;
    bool applyUserOffset = false;

    startEdit();
    score()->addRefresh(m_dropData.ed.dropElement->canvasBoundingRect());
    ElementType et = m_dropData.ed.dropElement->type();
    switch (et) {
    case ElementType::TEXTLINE:
        systemStavesOnly = m_dropData.ed.dropElement->systemFlag();
    // fall-thru
    case ElementType::VOLTA:
    case ElementType::GRADUAL_TEMPO_CHANGE:
        // voltas drop to system staves by default, or closest staff if Control is held
        systemStavesOnly = systemStavesOnly || !(m_dropData.ed.modifiers & Qt::ControlModifier);
    // fall-thru
    case ElementType::OTTAVA:
    case ElementType::TRILL:
    case ElementType::PEDAL:
    case ElementType::LET_RING:
    case ElementType::VIBRATO:
    case ElementType::PALM_MUTE:
    case ElementType::HAIRPIN:
    {
        mu::engraving::Spanner* spanner = ptr::checked_cast<mu::engraving::Spanner>(m_dropData.ed.dropElement);
        score()->cmdAddSpanner(spanner, pos, systemStavesOnly);
        score()->setUpdateAll();
        accepted = true;
    }
    break;
    case ElementType::SYMBOL:
    case ElementType::FSYMBOL:
    case ElementType::IMAGE:
        applyUserOffset = true;
    // fall-thru
    case ElementType::DYNAMIC:
    case ElementType::FRET_DIAGRAM:
    case ElementType::HARMONY:
    {
        EngravingItem* el = elementAt(pos);
        if (el == 0 || el->type() == ElementType::STAFF_LINES) {
            mu::engraving::staff_idx_t staffIdx;
            mu::engraving::Segment* seg;
            PointF offset;
            el = score()->pos2measure(pos, &staffIdx, 0, &seg, &offset);
            if (el && el->isMeasure()) {
                m_dropData.ed.dropElement->setTrack(staffIdx * mu::engraving::VOICES);
                m_dropData.ed.dropElement->setParent(seg);

                if (applyUserOffset) {
                    m_dropData.ed.dropElement->setOffset(offset);
                }

                score()->undoAddElement(m_dropData.ed.dropElement);
            } else {
                LOGD("cannot drop here");
                resetDropElement();
            }
        } else {
            score()->addRefresh(el->canvasBoundingRect());
            score()->addRefresh(m_dropData.ed.dropElement->canvasBoundingRect());

            if (!el->acceptDrop(m_dropData.ed)) {
                LOGD("drop %s onto %s not accepted", m_dropData.ed.dropElement->typeName(), el->typeName());
                break;
            }
            EngravingItem* dropElement = el->drop(m_dropData.ed);
            score()->addRefresh(el->canvasBoundingRect());
            if (dropElement) {
                doSelect({ dropElement }, SelectType::SINGLE);
                score()->addRefresh(dropElement->canvasBoundingRect());
            }
        }
    }
        accepted = true;
        break;
    case ElementType::HBOX:
    case ElementType::VBOX:
    case ElementType::KEYSIG:
    case ElementType::CLEF:
    case ElementType::TIMESIG:
    case ElementType::BAR_LINE:
    case ElementType::ARPEGGIO:
    case ElementType::BREATH:
    case ElementType::GLISSANDO:
    case ElementType::MEASURE_NUMBER:
    case ElementType::BRACKET:
    case ElementType::ARTICULATION:
    case ElementType::FERMATA:
    case ElementType::CHORDLINE:
    case ElementType::BEND:
    case ElementType::ACCIDENTAL:
    case ElementType::TEXT:
    case ElementType::FINGERING:
    case ElementType::TEMPO_TEXT:
    case ElementType::STAFF_TEXT:
    case ElementType::SYSTEM_TEXT:
    case ElementType::TRIPLET_FEEL:
    case ElementType::PLAYTECH_ANNOTATION:
    case ElementType::NOTEHEAD:
    case ElementType::TREMOLO:
    case ElementType::LAYOUT_BREAK:
    case ElementType::MARKER:
    case ElementType::STAFF_STATE:
    case ElementType::INSTRUMENT_CHANGE:
    case ElementType::REHEARSAL_MARK:
    case ElementType::JUMP:
    case ElementType::MEASURE_REPEAT:
    case ElementType::ACTION_ICON:
    case ElementType::NOTE:
    case ElementType::CHORD:
    case ElementType::SPACER:
    case ElementType::BAGPIPE_EMBELLISHMENT:
    case ElementType::AMBITUS:
    case ElementType::TREMOLOBAR:
    case ElementType::FIGURED_BASS:
    case ElementType::LYRICS:
    case ElementType::STAFFTYPE_CHANGE: {
        EngravingItem* el = dropTarget(m_dropData.ed);
        if (!el) {
            if (!dropCanvas(m_dropData.ed.dropElement)) {
                LOGD("cannot drop %s(%p) to canvas", m_dropData.ed.dropElement->typeName(), m_dropData.ed.dropElement);
                resetDropElement();
            }
            break;
        }
        score()->addRefresh(el->canvasBoundingRect());

        // TODO: HACK ALERT!
        if (el->isMeasure() && m_dropData.ed.dropElement->isLayoutBreak()) {
            Measure* m = toMeasure(el);
            if (m->isMMRest()) {
                el = m->mmRestLast();
            }
        }

        EngravingItem* dropElement = el->drop(m_dropData.ed);
        if (dropElement && dropElement->isInstrumentChange()) {
            if (!selectInstrument(toInstrumentChange(dropElement))) {
                rollback();
                accepted = true;
                break;
            }
        }
        score()->addRefresh(el->canvasBoundingRect());
        if (dropElement) {
            if (!score()->noteEntryMode()) {
                doSelect({ dropElement }, SelectType::SINGLE);
            }
            score()->addRefresh(dropElement->canvasBoundingRect());
        }
        accepted = true;
    }
    break;
    case ElementType::SLUR:
    {
        EngravingItem* el = dropTarget(m_dropData.ed);
        mu::engraving::Slur* dropElement = toSlur(m_dropData.ed.dropElement);
        if (toNote(el)->chord()) {
            doAddSlur(toNote(el)->chord(), nullptr, dropElement);
            accepted = true;
        }
    }
    break;
    default:
        resetDropElement();
        break;
    }
    m_dropData.ed.dropElement = nullptr;
    setDropTarget(nullptr);         // this also resets dropRectangle and dropAnchor
    apply();
    // update input cursor position (must be done after layout)
//    if (noteEntryMode()) {
//        moveCursor();
//    }
    if (accepted) {
        notifyAboutDropChanged();
    }

    return accepted;
}

bool NotationInteraction::selectInstrument(mu::engraving::InstrumentChange* instrumentChange)
{
    if (!instrumentChange) {
        return false;
    }

    RetVal<Instrument> selectedInstrument = selectInstrumentScenario()->selectInstrument();
    if (!selectedInstrument.ret) {
        return false;
    }

    instrumentChange->setInit(true);
    instrumentChange->setupInstrument(&selectedInstrument.val);

    return true;
}

//! NOTE Copied from Palette::applyPaletteElement
bool NotationInteraction::applyPaletteElement(mu::engraving::EngravingItem* element, Qt::KeyboardModifiers modifiers)
{
    IF_ASSERT_FAILED(element) {
        return false;
    }

    mu::engraving::Score* score = this->score();

    if (!score) {
        return false;
    }

    const mu::engraving::Selection sel = score->selection();   // make a copy of selection state before applying the operation.
    if (sel.isNone()) {
        return false;
    }

//#ifdef MSCORE_UNSTABLE
//    if (ScriptRecorder* rec = adapter()->getScriptRecorder()) {
//        if (modifiers == 0) {
//            rec->recordPaletteElement(element);
//        }
//    }
//#endif

    startEdit();

    if (sel.isList()) {
        ChordRest* cr1 = sel.firstChordRest();
        ChordRest* cr2 = sel.lastChordRest();
        bool addSingle = false;           // add a single line only
        if (cr1 && cr2 == cr1) {
            // one chordrest selected, ok to add line
            addSingle = true;
        } else if (sel.elements().size() == 2 && cr1 && cr2 && cr1 != cr2) {
            // two chordrests selected
            // must be on same staff in order to add line, except for slur
            if (element->isSlur() || cr1->staffIdx() == cr2->staffIdx()) {
                addSingle = true;
            }
        }

        auto isEntryDrumStaff = [score]() {
            const mu::engraving::InputState& is = score->inputState();
            const mu::engraving::Staff* staff = score->staff(is.track() / mu::engraving::VOICES);
            return staff ? staff->staffType(is.tick())->group() == mu::engraving::StaffGroup::PERCUSSION : false;
        };

        if (isEntryDrumStaff() && element->isChord()) {
            mu::engraving::InputState& is = score->inputState();
            EngravingItem* e = nullptr;
            if (!(modifiers & Qt::ShiftModifier)) {
                // shift+double-click: add note to "chord"
                // use input position rather than selection if possible
                // look for a cr in the voice predefined for the drum in the palette
                // back up if necessary
                // TODO: refactor this with similar code in putNote()
                if (is.segment()) {
                    mu::engraving::Segment* seg = is.segment();
                    while (seg) {
                        if (seg->element(is.track())) {
                            break;
                        }
                        seg = seg->prev(mu::engraving::SegmentType::ChordRest);
                    }
                    if (seg) {
                        is.setSegment(seg);
                    } else {
                        is.setSegment(is.segment()->measure()->first(mu::engraving::SegmentType::ChordRest));
                    }
                }
                score->expandVoice();
                e = is.cr();
            }
            if (!e) {
                e = sel.elements().front();
            }
            if (e) {
                // get note if selection was full chord
                if (e->isChord()) {
                    e = toChord(e)->upNote();
                }

                applyDropPaletteElement(score, e, element, modifiers, PointF(), true);
                // note has already been played (and what would play otherwise may be *next* input position)
                score->setPlayNote(false);
                score->setPlayChord(false);
                // continue in same track
                is.setTrack(e->track());
            } else {
                LOGD("nowhere to place drum note");
            }
        } else if (element->isLayoutBreak()) {
            mu::engraving::LayoutBreak* breakElement = toLayoutBreak(element);
            score->cmdToggleLayoutBreak(breakElement->layoutBreakType());
        } else if (element->isSlur() && addSingle) {
            doAddSlur(toSlur(element));
        } else if (element->isSLine() && !element->isGlissando() && addSingle) {
            mu::engraving::Segment* startSegment = cr1->segment();
            mu::engraving::Segment* endSegment = cr2->segment();
            if (element->type() == mu::engraving::ElementType::PEDAL && cr2 != cr1) {
                endSegment = endSegment->nextCR(cr2->track());
            }
            // TODO - handle cross-voice selections
            staff_idx_t idx = cr1->staffIdx();

            ByteArray a = element->mimeData();
//printf("<<%s>>\n", a.data());
            mu::engraving::XmlReader e(a);
            mu::engraving::Fraction duration;        // dummy
            PointF dragOffset;
            mu::engraving::ElementType type = mu::engraving::EngravingItem::readType(e, &dragOffset, &duration);
            mu::engraving::Spanner* spanner = static_cast<mu::engraving::Spanner*>(engraving::Factory::createItem(type, score->dummy()));
            spanner->read(e);
            spanner->styleChanged();
            score->cmdAddSpanner(spanner, idx, startSegment, endSegment);
        } else {
            for (EngravingItem* e : sel.elements()) {
                applyDropPaletteElement(score, e, element, modifiers);
            }
        }
    } else if (sel.isRange()) {
        if (element->type() == ElementType::BAR_LINE
            || element->type() == ElementType::MARKER
            || element->type() == ElementType::JUMP
            || element->type() == ElementType::SPACER
            || element->type() == ElementType::VBOX
            || element->type() == ElementType::HBOX
            || element->type() == ElementType::TBOX
            || element->type() == ElementType::MEASURE
            || element->type() == ElementType::BRACKET
            || element->type() == ElementType::STAFFTYPE_CHANGE
            || (element->type() == ElementType::ACTION_ICON
                && (toActionIcon(element)->actionType() == mu::engraving::ActionIconType::VFRAME
                    || toActionIcon(element)->actionType() == mu::engraving::ActionIconType::HFRAME
                    || toActionIcon(element)->actionType() == mu::engraving::ActionIconType::TFRAME
                    || toActionIcon(element)->actionType() == mu::engraving::ActionIconType::MEASURE
                    || toActionIcon(element)->actionType() == mu::engraving::ActionIconType::BRACKETS))) {
            Measure* last = sel.endSegment() ? sel.endSegment()->measure() : nullptr;
            for (Measure* m = sel.startSegment()->measure(); m; m = m->nextMeasureMM()) {
                RectF r = m->staffabbox(sel.staffStart());
                PointF pt(r.x() + r.width() * .5, r.y() + r.height() * .5);
                pt += m->system()->page()->pos();
                applyDropPaletteElement(score, m, element, modifiers, pt);
                if ((m == last) || (element->type() == ElementType::BRACKET)) {
                    break;
                }
            }
        } else if (element->type() == ElementType::LAYOUT_BREAK) {
            mu::engraving::LayoutBreak* breakElement = static_cast<mu::engraving::LayoutBreak*>(element);
            score->cmdToggleLayoutBreak(breakElement->layoutBreakType());
        } else if (element->isClef() || element->isKeySig() || element->isTimeSig()) {
            Measure* m1 = sel.startSegment()->measure();
            Measure* m2 = sel.endSegment() ? sel.endSegment()->measure() : nullptr;
            if (m2 == m1 && sel.startSegment()->rtick().isZero()) {
                m2 = nullptr;             // don't restore original if one full measure selected
            } else if (m2) {
                m2 = m2->nextMeasureMM();
            }
            // for clefs, apply to each staff separately
            // otherwise just apply to top staff
            staff_idx_t staffIdx1 = sel.staffStart();
            staff_idx_t staffIdx2 = element->type() == ElementType::CLEF ? sel.staffEnd() : staffIdx1 + 1;
            for (staff_idx_t i = staffIdx1; i < staffIdx2; ++i) {
                // for clefs, use mid-measure changes if appropriate
                EngravingItem* e1 = nullptr;
                EngravingItem* e2 = nullptr;
                // use mid-measure clef changes as appropriate
                if (element->type() == ElementType::CLEF) {
                    if (sel.startSegment()->isChordRestType() && sel.startSegment()->rtick().isNotZero()) {
                        ChordRest* cr = static_cast<ChordRest*>(sel.startSegment()->nextChordRest(i * mu::engraving::VOICES));
                        if (cr && cr->isChord()) {
                            e1 = static_cast<mu::engraving::Chord*>(cr)->upNote();
                        } else {
                            e1 = cr;
                        }
                    }
                    if (sel.endSegment() && sel.endSegment()->segmentType() == mu::engraving::SegmentType::ChordRest) {
                        ChordRest* cr = static_cast<ChordRest*>(sel.endSegment()->nextChordRest(i * mu::engraving::VOICES));
                        if (cr && cr->isChord()) {
                            e2 = static_cast<mu::engraving::Chord*>(cr)->upNote();
                        } else {
                            e2 = cr;
                        }
                    }
                }
                if (m2 || e2) {
                    // restore original clef/keysig/timesig
                    mu::engraving::Staff* staff = score->staff(i);
                    mu::engraving::Fraction tick1 = sel.startSegment()->tick();
                    mu::engraving::EngravingItem* oelement = nullptr;
                    switch (element->type()) {
                    case mu::engraving::ElementType::CLEF:
                    {
                        mu::engraving::Clef* oclef = engraving::Factory::createClef(score->dummy()->segment());
                        oclef->setClefType(staff->clef(tick1));
                        oelement = oclef;
                        break;
                    }
                    case mu::engraving::ElementType::KEYSIG:
                    {
                        mu::engraving::KeySig* okeysig = engraving::Factory::createKeySig(score->dummy()->segment());
                        okeysig->setKeySigEvent(staff->keySigEvent(tick1));
                        if (!score->styleB(mu::engraving::Sid::concertPitch) && !okeysig->isAtonal()) {
                            mu::engraving::Interval v = staff->part()->instrument(tick1)->transpose();
                            if (!v.isZero()) {
                                Key k = okeysig->key();
                                okeysig->setKey(transposeKey(k, v, okeysig->part()->preferSharpFlat()));
                            }
                        }
                        oelement = okeysig;
                        break;
                    }
                    case mu::engraving::ElementType::TIMESIG:
                    {
                        mu::engraving::TimeSig* otimesig = engraving::Factory::createTimeSig(score->dummy()->segment());
                        otimesig->setFrom(staff->timeSig(tick1));
                        oelement = otimesig;
                        break;
                    }
                    default:
                        break;
                    }
                    if (oelement) {
                        if (e2) {
                            applyDropPaletteElement(score, e2, oelement, modifiers);
                        } else {
                            RectF r = m2->staffabbox(i);
                            PointF pt(r.x() + r.width() * .5, r.y() + r.height() * .5);
                            pt += m2->system()->page()->pos();
                            applyDropPaletteElement(score, m2, oelement, modifiers, pt);
                        }
                        delete oelement;
                    }
                }
                // apply new clef/keysig/timesig
                if (e1) {
                    applyDropPaletteElement(score, e1, element, modifiers);
                } else {
                    RectF r = m1->staffabbox(i);
                    PointF pt(r.x() + r.width() * .5, r.y() + r.height() * .5);
                    pt += m1->system()->page()->pos();
                    applyDropPaletteElement(score, m1, element, modifiers, pt);
                }
            }
        } else if (element->isSlur()) {
            doAddSlur(toSlur(element));
        } else if (element->isSLine() && element->type() != ElementType::GLISSANDO) {
            mu::engraving::Segment* startSegment = sel.startSegment();
            mu::engraving::Segment* endSegment = sel.endSegment();
            bool firstStaffOnly = isSystemTextLine(element) && !(modifiers & Qt::ControlModifier);
            staff_idx_t startStaff = firstStaffOnly ? 0 : sel.staffStart();
            staff_idx_t endStaff   = firstStaffOnly ? 1 : sel.staffEnd();
            for (staff_idx_t i = startStaff; i < endStaff; ++i) {
                mu::engraving::Spanner* spanner = static_cast<mu::engraving::Spanner*>(element->clone());
                spanner->setScore(score);
                spanner->styleChanged();
                score->cmdAddSpanner(spanner, i, startSegment, endSegment);
            }
        } else if (element->isTextBase()) {
            mu::engraving::Segment* firstSegment = sel.startSegment();
            staff_idx_t firstStaffIndex = sel.staffStart();
            staff_idx_t lastStaffIndex = sel.staffEnd();

            // A text should only be added at the start of the selection
            // There shouldn't be a text at each element
            for (staff_idx_t staff = firstStaffIndex; staff < lastStaffIndex; staff++) {
                applyDropPaletteElement(score, firstSegment->firstElement(staff), element, modifiers);
            }
        } else {
            track_idx_t track1 = sel.staffStart() * mu::engraving::VOICES;
            track_idx_t track2 = sel.staffEnd() * mu::engraving::VOICES;
            mu::engraving::Segment* startSegment = sel.startSegment();
            mu::engraving::Segment* endSegment = sel.endSegment();       //keep it, it could change during the loop

            for (mu::engraving::Segment* s = startSegment; s && s != endSegment; s = s->next1()) {
                for (track_idx_t track = track1; track < track2; ++track) {
                    mu::engraving::EngravingItem* e = s->element(track);
                    if (e == 0 || !score->selectionFilter().canSelect(e)
                        || !score->selectionFilter().canSelectVoice(track)) {
                        continue;
                    }
                    if (e->isChord()) {
                        mu::engraving::Chord* chord = toChord(e);
                        for (mu::engraving::Note* n : chord->notes()) {
                            applyDropPaletteElement(score, n, element, modifiers);
                            if (!(element->isAccidental() || element->isNoteHead())) {             // only these need to apply to every note
                                break;
                            }
                        }
                    } else {
                        // do not apply articulation to barline in a range selection
                        if (!e->isBarLine() || !element->isArticulation()) {
                            applyDropPaletteElement(score, e, element, modifiers);
                        }
                    }
                }
                if (!element->placeMultiple()) {
                    break;
                }
            }
        }
    } else {
        LOGD("unknown selection state");
    }

    apply();

    setDropTarget(nullptr);

    return true;
}

//! NOTE Copied from Palette applyDrop
void NotationInteraction::applyDropPaletteElement(mu::engraving::Score* score, mu::engraving::EngravingItem* target,
                                                  mu::engraving::EngravingItem* e,
                                                  Qt::KeyboardModifiers modifiers,
                                                  PointF pt, bool pasteMode)
{
    if (!target) {
        return;
    }

    mu::engraving::EditData newData(&m_scoreCallbacks);
    mu::engraving::EditData* dropData = &newData;

    if (isTextEditingStarted()) {
        dropData = &m_editData;
    }

    dropData->pos         = pt.isNull() ? target->pagePos() : pt;
    dropData->dragOffset  = QPointF();
    dropData->modifiers   = keyboardModifier(modifiers);
    dropData->dropElement = e;

    if (target->acceptDrop(*dropData)) {
        // use same code path as drag&drop

        ByteArray a = e->mimeData();

        mu::engraving::XmlReader n(a);
        n.context()->setPasteMode(pasteMode);
        Fraction duration;      // dummy
        PointF dragOffset;
        ElementType type = EngravingItem::readType(n, &dragOffset, &duration);
        dropData->dropElement = engraving::Factory::createItem(type, score->dummy());

        dropData->dropElement->read(n);
        dropData->dropElement->styleChanged();       // update to local style

        mu::engraving::EngravingItem* el = target->drop(*dropData);
        if (el && el->isInstrumentChange()) {
            if (!selectInstrument(toInstrumentChange(el))) {
                rollback();
                return;
            }
        }

        if (el && !score->inputState().noteEntryMode()) {
            doSelect({ el }, mu::engraving::SelectType::SINGLE, 0);
        }
        dropData->dropElement = nullptr;

        m_notifyAboutDropChanged = true;
    }
}

//! NOTE Copied from ScoreView::cmdAddSlur
void NotationInteraction::doAddSlur(const mu::engraving::Slur* slurTemplate)
{
    startEdit();
    m_notifyAboutDropChanged = true;

    mu::engraving::ChordRest* firstChordRest = nullptr;
    mu::engraving::ChordRest* secondChordRest = nullptr;
    const auto& sel = score()->selection();
    auto el = sel.uniqueElements();

    if (sel.isRange()) {
        mu::engraving::track_idx_t startTrack = sel.staffStart() * mu::engraving::VOICES;
        mu::engraving::track_idx_t endTrack = sel.staffEnd() * mu::engraving::VOICES;
        for (mu::engraving::track_idx_t track = startTrack; track < endTrack; ++track) {
            firstChordRest = nullptr;
            secondChordRest = nullptr;
            for (mu::engraving::EngravingItem* e : el) {
                if (e->track() != track) {
                    continue;
                }
                if (e->isNote()) {
                    e = toNote(e)->chord();
                }
                if (!e->isChord()) {
                    continue;
                }
                mu::engraving::ChordRest* cr = mu::engraving::toChordRest(e);
                if (!firstChordRest || firstChordRest->tick() > cr->tick()) {
                    firstChordRest = cr;
                }
                if (!secondChordRest || secondChordRest->tick() < cr->tick()) {
                    secondChordRest = cr;
                }
            }

            if (firstChordRest && (firstChordRest != secondChordRest)) {
                doAddSlur(firstChordRest, secondChordRest, slurTemplate);
            }
        }
    } else if (sel.isSingle()) {
        if (sel.element()->isNote()) {
            doAddSlur(toNote(sel.element())->chord(), nullptr, slurTemplate);
        }
    } else {
        for (mu::engraving::EngravingItem* e : el) {
            if (e->isNote()) {
                e = mu::engraving::toNote(e)->chord();
            }
            if (!e->isChord()) {
                continue;
            }
            mu::engraving::ChordRest* cr = mu::engraving::toChordRest(e);
            if (!firstChordRest || cr->isBefore(firstChordRest)) {
                firstChordRest = cr;
            }
            if (!secondChordRest || secondChordRest->isBefore(cr)) {
                secondChordRest = cr;
            }
        }

        if (firstChordRest == secondChordRest) {
            secondChordRest = mu::engraving::nextChordRest(firstChordRest);
        }

        if (firstChordRest) {
            doAddSlur(firstChordRest, secondChordRest, slurTemplate);
        }
    }

    apply();
}

void NotationInteraction::doAddSlur(ChordRest* firstChordRest, ChordRest* secondChordRest, const mu::engraving::Slur* slurTemplate)
{
    mu::engraving::Slur* slur = firstChordRest->slur(secondChordRest);
    if (!slur || slur->slurDirection() != mu::engraving::DirectionV::AUTO) {
        slur = score()->addSlur(firstChordRest, secondChordRest, slurTemplate);
    }

    if (m_noteInput->isNoteInputMode()) {
        m_noteInput->addSlur(slur);
    } else if (!secondChordRest) {
        mu::engraving::SlurSegment* segment = slur->frontSegment();
        select({ segment }, SelectType::SINGLE);
        startEditGrip(segment, mu::engraving::Grip::END);
    }
}

bool NotationInteraction::scoreHasMeasure() const
{
    mu::engraving::Page* page = score()->pages().empty() ? nullptr : score()->pages().front();
    const std::vector<mu::engraving::System*>* systems = page ? &page->systems() : nullptr;
    if (systems == nullptr || systems->empty() || systems->front()->measures().empty()) {
        return false;
    }

    return true;
}

bool NotationInteraction::notesHaveActiculation(const std::vector<Note*>& notes, SymbolId articulationSymbolId) const
{
    for (Note* note: notes) {
        Chord* chord = note->chord();

        std::set<SymbolId> chordArticulations = chord->articulationSymbolIds();
        chordArticulations = mu::engraving::flipArticulations(chordArticulations, mu::engraving::PlacementV::ABOVE);
        chordArticulations = mu::engraving::splitArticulations(chordArticulations);

        if (chordArticulations.find(articulationSymbolId) == chordArticulations.end()) {
            return false;
        }
    }

    return true;
}

//! NOTE Copied from ScoreView::dragLeaveEvent
void NotationInteraction::endDrop()
{
    if (m_dropData.ed.dropElement) {
        score()->setUpdateAll();
        resetDropElement();
        score()->update();
    }
    setDropTarget(nullptr);
}

mu::async::Notification NotationInteraction::dropChanged() const
{
    return m_dropChanged;
}

//! NOTE Copied from ScoreView::dropCanvas
bool NotationInteraction::dropCanvas(EngravingItem* e)
{
    if (e->isActionIcon()) {
        switch (mu::engraving::toActionIcon(e)->actionType()) {
        case mu::engraving::ActionIconType::VFRAME:
            score()->insertMeasure(ElementType::VBOX);
            break;
        case mu::engraving::ActionIconType::HFRAME:
            score()->insertMeasure(ElementType::HBOX);
            break;
        case mu::engraving::ActionIconType::TFRAME:
            score()->insertMeasure(ElementType::TBOX);
            break;
        case mu::engraving::ActionIconType::FFRAME:
            score()->insertMeasure(ElementType::FBOX);
            break;
        case mu::engraving::ActionIconType::MEASURE:
            score()->insertMeasure(ElementType::MEASURE);
            break;
        default:
            return false;
        }
        delete e;
        return true;
    }
    return false;
}

//! NOTE Copied from ScoreView::getDropTarget
EngravingItem* NotationInteraction::dropTarget(mu::engraving::EditData& ed) const
{
    std::vector<EngravingItem*> el = elementsAt(ed.pos);
    for (EngravingItem* e : el) {
        if (e->isStaffLines()) {
            if (el.size() > 2) {          // is not first class drop target
                continue;
            }
            e = mu::engraving::toStaffLines(e)->measure();
        }
        if (e->acceptDrop(ed)) {
            return e;
        }
    }
    return nullptr;
}

//! NOTE Copied from ScoreView::dragMeasureAnchorElement
bool NotationInteraction::dragMeasureAnchorElement(const PointF& pos)
{
    mu::engraving::staff_idx_t staffIdx;
    mu::engraving::Segment* seg;
    mu::engraving::MeasureBase* mb = score()->pos2measure(pos, &staffIdx, 0, &seg, 0);
    if (!(m_dropData.ed.modifiers & Qt::ControlModifier)) {
        staffIdx = 0;
    }
    mu::engraving::track_idx_t track = staffIdx * mu::engraving::VOICES;

    if (mb && mb->isMeasure()) {
        mu::engraving::Measure* m = mu::engraving::toMeasure(mb);
        mu::engraving::System* s  = m->system();
        qreal y    = s->staff(staffIdx)->y() + s->pos().y() + s->page()->pos().y();
        RectF b(m->canvasBoundingRect());
        if (pos.x() >= (b.x() + b.width() * .5) && m != score()->lastMeasureMM()
            && m->nextMeasure()->system() == m->system()) {
            m = m->nextMeasure();
        }
        PointF anchor(m->canvasBoundingRect().x(), y);
        setAnchorLines({ LineF(pos, anchor) });
        m_dropData.ed.dropElement->score()->addRefresh(m_dropData.ed.dropElement->canvasBoundingRect());
        m_dropData.ed.dropElement->setTrack(track);
        m_dropData.ed.dropElement->score()->addRefresh(m_dropData.ed.dropElement->canvasBoundingRect());
        notifyAboutDragChanged();
        return true;
    }
    m_dropData.ed.dropElement->score()->addRefresh(m_dropData.ed.dropElement->canvasBoundingRect());
    setDropTarget(nullptr);
    return false;
}

//! NOTE Copied from ScoreView::dragTimeAnchorElement
bool NotationInteraction::dragTimeAnchorElement(const PointF& pos)
{
    mu::engraving::staff_idx_t staffIdx = 0;
    mu::engraving::Segment* seg = nullptr;
    mu::engraving::MeasureBase* mb = score()->pos2measure(pos, &staffIdx, 0, &seg, 0);
    mu::engraving::track_idx_t track = staffIdx * mu::engraving::VOICES;

    if (mb && mb->isMeasure() && seg->element(track)) {
        mu::engraving::Measure* m = mu::engraving::toMeasure(mb);
        mu::engraving::System* s  = m->system();
        qreal y    = s->staff(staffIdx)->y() + s->pos().y() + s->page()->pos().y();
        PointF anchor(seg->canvasBoundingRect().x(), y);
        setAnchorLines({ LineF(pos, anchor) });
        m_dropData.ed.dropElement->score()->addRefresh(m_dropData.ed.dropElement->canvasBoundingRect());
        m_dropData.ed.dropElement->setTrack(track);
        m_dropData.ed.dropElement->score()->addRefresh(m_dropData.ed.dropElement->canvasBoundingRect());
        notifyAboutDragChanged();
        return true;
    }

    m_dropData.ed.dropElement->score()->addRefresh(m_dropData.ed.dropElement->canvasBoundingRect());
    setDropTarget(nullptr);

    return false;
}

//! NOTE Copied from ScoreView::setDropTarget
void NotationInteraction::setDropTarget(const EngravingItem* item, bool notify)
{
    if (m_dropData.dropTarget != item) {
        if (m_dropData.dropTarget) {
            m_dropData.dropTarget->setDropTarget(false);
            m_dropData.dropTarget = nullptr;
        }

        m_dropData.dropTarget = item;
        if (m_dropData.dropTarget) {
            m_dropData.dropTarget->setDropTarget(true);
        }
    }

    resetAnchorLines();

    if (m_dropData.dropRect.isValid()) {
        m_dropData.dropRect = RectF();
    }

    if (notify) {
        notifyAboutDragChanged();
    }
}

//! NOTE: Copied from ScoreView::setDropRectangle
void NotationInteraction::setDropRect(const RectF& rect)
{
    if (m_dropData.dropRect == rect) {
        return;
    }

    m_dropData.dropRect = rect;

    if (rect.isValid()) {
        score()->addRefresh(rect);
    }

    if (m_dropData.dropTarget) {
        m_dropData.dropTarget->setDropTarget(false);
        score()->addRefresh(m_dropData.dropTarget->canvasBoundingRect());
        m_dropData.dropTarget = nullptr;
    } else if (!m_anchorLines.empty()) {
        RectF rf;
        rf.setTopLeft(m_anchorLines.front().p1());
        rf.setBottomRight(m_anchorLines.front().p2());
        score()->addRefresh(rf.normalized());
        resetAnchorLines();
    }

    notifyAboutDragChanged();
}

void NotationInteraction::resetDropElement()
{
    if (m_dropData.ed.dropElement) {
        delete m_dropData.ed.dropElement;
        m_dropData.ed.dropElement = nullptr;
    }
}

void NotationInteraction::setAnchorLines(const std::vector<LineF>& anchorList)
{
    m_anchorLines = anchorList;
}

void NotationInteraction::resetAnchorLines()
{
    m_anchorLines.clear();
}

double NotationInteraction::currentScaling(mu::draw::Painter* painter) const
{
    qreal guiScaling = configuration()->guiScaling();
    return painter->worldTransform().m11() / guiScaling;
}

void NotationInteraction::drawAnchorLines(mu::draw::Painter* painter)
{
    if (m_anchorLines.empty()) {
        return;
    }

    const auto dropAnchorColor = configuration()->anchorLineColor();
    mu::draw::Pen pen(dropAnchorColor, 2.0 / currentScaling(painter), mu::draw::PenStyle::DotLine);

    for (const LineF& anchor : m_anchorLines) {
        painter->setPen(pen);
        painter->drawLine(anchor);

        qreal d = 4.0 / currentScaling(painter);
        RectF rect(-d, -d, 2 * d, 2 * d);

        painter->setBrush(mu::draw::Brush(dropAnchorColor));
        painter->setNoPen();
        rect.moveCenter(anchor.p1());
        painter->drawEllipse(rect);
        rect.moveCenter(anchor.p2());
        painter->drawEllipse(rect);
    }
}

void NotationInteraction::drawTextEditMode(draw::Painter* painter)
{
    if (!isTextEditingStarted()) {
        return;
    }

    m_editData.element->drawEditMode(painter, m_editData, currentScaling(painter));
}

void NotationInteraction::drawSelectionRange(draw::Painter* painter)
{
    using namespace draw;
    if (!m_selection->isRange()) {
        return;
    }

    painter->setBrush(BrushStyle::NoBrush);

    QColor selectionColor = configuration()->selectionColor();
    qreal penWidth = 3.0 / currentScaling(painter);

    Pen pen;
    pen.setColor(selectionColor);
    pen.setWidthF(penWidth);
    pen.setStyle(PenStyle::SolidLine);
    painter->setPen(pen);

    std::vector<RectF> rangeArea = m_selection->range()->boundingArea();
    for (const RectF& rect: rangeArea) {
        PainterPath path;
        path.addRoundedRect(rect, 6, 6);

        QColor fillColor = selectionColor;
        fillColor.setAlpha(10);
        painter->fillPath(path, fillColor);
        painter->drawPath(path);
    }
}

void NotationInteraction::drawGripPoints(draw::Painter* painter)
{
    if (isDragStarted() && !isGripEditStarted()) {
        return;
    }

    mu::engraving::EngravingItem* editedElement = m_editData.element;
    int gripsCount = editedElement ? editedElement->gripsCount() : 0;

    if (gripsCount == 0) {
        return;
    }

    m_editData.grips = gripsCount;
    m_editData.grip.resize(m_editData.grips);

    constexpr qreal DEFAULT_GRIP_SIZE = 8;
    qreal scaling = currentScaling(painter);
    qreal gripSize = DEFAULT_GRIP_SIZE / scaling;
    RectF newRect(-gripSize / 2, -gripSize / 2, gripSize, gripSize);

    const EngravingItem* page = editedElement->findAncestor(ElementType::PAGE);
    PointF pageOffset = page ? page->pos() : editedElement->pos();

    for (RectF& gripRect: m_editData.grip) {
        gripRect = newRect.translated(pageOffset);
    }

    editedElement->updateGrips(m_editData);
    editedElement->drawEditMode(painter, m_editData, scaling);
}

ChordRest* activeCr(mu::engraving::Score* score)
{
    ChordRest* cr = score->selection().activeCR();
    if (!cr) {
        cr = score->selection().lastChordRest();
        if (!cr && score->noteEntryMode()) {
            cr = score->inputState().cr();
        }
    }
    return cr;
}

void NotationInteraction::expandSelection(ExpandSelectionMode mode)
{
    ChordRest* cr = activeCr(score());
    if (!cr) {
        return;
    }
    ChordRest* el = 0;
    switch (mode) {
    case ExpandSelectionMode::BeginSystem: {
        Measure* measure = cr->segment()->measure()->system()->firstMeasure();
        if (measure) {
            el = measure->first()->nextChordRest(cr->track());
        }
        break;
    }
    case ExpandSelectionMode::EndSystem: {
        Measure* measure = cr->segment()->measure()->system()->lastMeasure();
        if (measure) {
            el = measure->last()->nextChordRest(cr->track(), true);
        }
        break;
    }
    case ExpandSelectionMode::BeginScore: {
        Measure* measure = score()->firstMeasureMM();
        if (measure) {
            el = measure->first()->nextChordRest(cr->track());
        }
        break;
    }
    case ExpandSelectionMode::EndScore: {
        Measure* measure = score()->lastMeasureMM();
        if (measure) {
            el = measure->last()->nextChordRest(cr->track(), true);
        }
        break;
    }
    }

    if (el) {
        select({ el }, SelectType::RANGE, el->staffIdx());
    }
}

void NotationInteraction::addToSelection(MoveDirection d, MoveSelectionType type)
{
    ChordRest* cr = activeCr(score());
    if (!cr) {
        return;
    }
    ChordRest* el = 0;
    switch (type) {
    case MoveSelectionType::Chord:
        if (d == MoveDirection::Right) {
            el = mu::engraving::nextChordRest(cr, true);
        } else {
            el = mu::engraving::prevChordRest(cr, true);
        }
        break;
    case MoveSelectionType::Measure:
        if (d == MoveDirection::Right) {
            el = score()->nextMeasure(cr, true, true);
        } else {
            el = score()->prevMeasure(cr, true);
        }
        break;
    case MoveSelectionType::Track:
        if (d == MoveDirection::Up) {
            el = score()->upStaff(cr);
        } else {
            el = score()->downStaff(cr);
        }
    case MoveSelectionType::EngravingItem:
    case MoveSelectionType::Frame:
    case MoveSelectionType::System:
    case MoveSelectionType::String:
    case MoveSelectionType::Undefined:
        break;
    }

    if (el) {
        select({ el }, SelectType::RANGE, el->staffIdx());
        resetHitElementContext();
    }
}

bool NotationInteraction::moveSelectionAvailable(MoveSelectionType type) const
{
    if (type != MoveSelectionType::EngravingItem) {
        return !isElementEditStarted();
    }

    if (isGripEditStarted()) {
        return true;
    }

    return !isElementEditStarted();
}

void NotationInteraction::moveSelection(MoveDirection d, MoveSelectionType type)
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

    mu::engraving::EngravingItem* item = score()->move(cmd);
    resetHitElementContext();

    notifyAboutSelectionChangedIfNeed();
    showItem(item);

    if (noteInput()->isNoteInputMode()) {
        notifyAboutNoteInputStateChanged();
    }
}

void NotationInteraction::selectTopStaff()
{
    EngravingItem* el = score()->cmdTopStaff(activeCr(score()));
    if (score()->noteEntryMode()) {
        score()->inputState().moveInputPos(el);
    }

    if (el->type() == ElementType::CHORD) {
        el = mu::engraving::toChord(el)->upNote();
    }

    select({ el }, SelectType::SINGLE, 0);
    showItem(el);
    resetHitElementContext();
}

void NotationInteraction::selectEmptyTrailingMeasure()
{
    ChordRest* cr = activeCr(score());
    const Measure* ftm = score()->firstTrailingMeasure(cr ? &cr : nullptr);
    if (!ftm) {
        ftm = score()->lastMeasure();
    }
    if (ftm) {
        if (score()->styleB(mu::engraving::Sid::createMultiMeasureRests) && ftm->hasMMRest()) {
            ftm = ftm->mmRest1();
        }
        EngravingItem* el
            = !cr ? ftm->first()->nextChordRest(0, false) : ftm->first()->nextChordRest(mu::engraving::trackZeroVoice(cr->track()), false);
        score()->inputState().moveInputPos(el);
        select({ el }, SelectType::SINGLE);
        resetHitElementContext();
    }
}

static ChordRest* asChordRest(EngravingItem* e)
{
    if (e && e->isNote()) {
        return toNote(e)->chord();
    } else if (e && e->isChordRest()) {
        return toChordRest(e);
    }
    return nullptr;
}

void NotationInteraction::moveChordRestToStaff(MoveDirection dir)
{
    startEdit();

    for (EngravingItem* e: score()->selection().uniqueElements()) {
        ChordRest* cr = asChordRest(e);
        if (cr != nullptr) {
            if (dir == MoveDirection::Up) {
                score()->moveUp(cr);
            } else if (dir == MoveDirection::Down) {
                score()->moveDown(cr);
            }
        }
    }

    apply();
}

void NotationInteraction::swapChordRest(MoveDirection direction)
{
    ChordRest* cr = asChordRest(score()->getSelectedElement());
    if (!cr) {
        return;
    }
    QList<ChordRest*> crl;
    if (cr->links()) {
        for (auto* l : *cr->links()) {
            crl.append(toChordRest(l));
        }
    } else {
        crl.append(cr);
    }
    startEdit();
    for (ChordRest* cr1 : crl) {
        if (cr1->type() == ElementType::REST) {
            Measure* m = toRest(cr1)->measure();
            if (m && m->isMMRest()) {
                break;
            }
        }
        ChordRest* cr2;
        // ensures cr1 is the left chord, useful in SwapCR::flip()
        if (direction == MoveDirection::Left) {
            cr2 = cr1;
            cr1 = prevChordRest(cr2);
        } else {
            cr2 = nextChordRest(cr1);
        }
        if (cr1 && cr2 && cr1->measure() == cr2->measure() && !cr1->tuplet() && !cr2->tuplet()
            && cr1->durationType() == cr2->durationType() && cr1->ticks() == cr2->ticks()
            // if two chords belong to different two-note tremolos, abort
            && !(cr1->isChord() && toChord(cr1)->tremolo() && toChord(cr1)->tremolo()->twoNotes()
                 && cr2->isChord() && toChord(cr2)->tremolo() && toChord(cr2)->tremolo()->twoNotes()
                 && toChord(cr1)->tremolo() != toChord(cr2)->tremolo())) {
            score()->undo(new mu::engraving::SwapCR(cr1, cr2));
        }
    }
    apply();
}

void NotationInteraction::moveElementSelection(MoveDirection d)
{
    EngravingItem* el = score()->selection().element();
    if (!el && !score()->selection().elements().empty()) {
        el = score()->selection().elements().back();
    }

    bool isLeftDirection = MoveDirection::Left == d;

    if (!el) {
        ChordRest* cr = score()->selection().currentCR();
        if (cr) {
            if (cr->isChord()) {
                if (isLeftDirection) {
                    el = toChord(cr)->upNote();
                } else {
                    el = toChord(cr)->downNote();
                }
            } else if (cr->isRest()) {
                el = cr;
            }
            score()->select(el);
        }
    }

    EngravingItem* toEl = nullptr;

    if (el) {
        toEl = isLeftDirection ? score()->prevElement() : score()->nextElement();
    } else {
        toEl = isLeftDirection ? score()->lastElement() : score()->firstElement();
    }

    if (!toEl) {
        return;
    }

    if (isElementEditStarted()) {
        endEditElement();
    }

    select({ toEl }, SelectType::REPLACE);
    resetHitElementContext();
    showItem(toEl);

    if (toEl->type() == ElementType::NOTE || toEl->type() == ElementType::HARMONY) {
        score()->setPlayNote(true);
    }

    if (toEl->hasGrips()) {
        startEditGrip(toEl, toEl->defaultGrip());
    }
}

void NotationInteraction::moveStringSelection(MoveDirection d)
{
    mu::engraving::InputState& is = score()->inputState();
    mu::engraving::Staff* staff = score()->staff(is.track() / mu::engraving::VOICES);
    int instrStrgs = static_cast<int>(staff->part()->instrument(is.tick())->stringData()->strings());
    int delta = (staff->staffType(is.tick())->upsideDown() ? -1 : 1);

    if (MoveDirection::Up == d) {
        delta = -delta;
    }

    int strg = is.string() + delta;
    if (strg >= 0 && strg < instrStrgs && strg != is.string()) {
        is.setString(strg);
        notifyAboutNoteInputStateChanged();
    }
}

inline mu::engraving::DirectionV toDirection(MoveDirection d)
{
    return d == MoveDirection::Up ? mu::engraving::DirectionV::UP : mu::engraving::DirectionV::DOWN;
}

void NotationInteraction::movePitch(MoveDirection d, PitchMode mode)
{
    IF_ASSERT_FAILED(MoveDirection::Up == d || MoveDirection::Down == d) {
        return;
    }

    startEdit();

    if (score()->selection().element() && score()->selection().element()->isRest()) {
        score()->cmdMoveRest(toRest(score()->selection().element()), toDirection(d));
    } else {
        score()->upDown(MoveDirection::Up == d, mode);
    }

    apply();
}

void NotationInteraction::moveLyrics(MoveDirection d)
{
    EngravingItem* el = score()->selection().element();
    IF_ASSERT_FAILED(el && el->isLyrics()) {
        return;
    }
    startEdit();
    score()->cmdMoveLyrics(toLyrics(el), toDirection(d));
    apply();
}

void NotationInteraction::nudge(MoveDirection d, bool quickly)
{
    EngravingItem* el = score()->selection().element();
    IF_ASSERT_FAILED(el && (el->isTextBase() || el->isArticulation())) {
        return;
    }

    startEdit();

    qreal step = quickly ? mu::engraving::MScore::nudgeStep10 : mu::engraving::MScore::nudgeStep;
    step = step * el->spatium();

    switch (d) {
    case MoveDirection::Undefined:
        IF_ASSERT_FAILED(d != MoveDirection::Undefined) {
            return;
        }
        break;
    case MoveDirection::Left:
        el->undoChangeProperty(mu::engraving::Pid::OFFSET, el->offset() - PointF(step, 0.0), mu::engraving::PropertyFlags::UNSTYLED);
        break;
    case MoveDirection::Right:
        el->undoChangeProperty(mu::engraving::Pid::OFFSET, el->offset() + PointF(step, 0.0), mu::engraving::PropertyFlags::UNSTYLED);
        break;
    case MoveDirection::Up:
        el->undoChangeProperty(mu::engraving::Pid::OFFSET, el->offset() - PointF(0.0, step), mu::engraving::PropertyFlags::UNSTYLED);
        break;
    case MoveDirection::Down:
        el->undoChangeProperty(mu::engraving::Pid::OFFSET, el->offset() + PointF(0.0, step), mu::engraving::PropertyFlags::UNSTYLED);
        break;
    }

    apply();

    notifyAboutDragChanged();
}

bool NotationInteraction::isTextSelected() const
{
    EngravingItem* selectedElement = m_selection->element();
    if (!selectedElement) {
        return false;
    }

    if (!selectedElement->isTextBase()) {
        return false;
    }

    return true;
}

bool NotationInteraction::isTextEditingStarted() const
{
    return m_editData.element && m_editData.element->isTextBase();
}

bool NotationInteraction::textEditingAllowed(const EngravingItem* element) const
{
    return element && element->isEditable() && (element->isTextBase() || element->isTBox());
}

void NotationInteraction::startEditText(EngravingItem* element, const PointF& cursorPos)
{
    if (!element) {
        return;
    }

    if (isTextEditingStarted()) {
        mu::engraving::TextBase* textBase = mu::engraving::toTextBase(m_editData.element);
        m_editData.startMove = bindCursorPosToText(cursorPos, textBase);

        // double click on a textBase element that is being edited - select word
        textBase->multiClickSelect(m_editData, mu::engraving::MultiClick::Double);
        textBase->endHexState(m_editData);
        textBase->setPrimed(false);
    } else {
        m_editData.clear();

        if (element->isTBox()) {
            m_editData.element = toTBox(element)->text();
        } else {
            m_editData.element = element;
        }

        m_editData.startMove = bindCursorPosToText(cursorPos, m_editData.element);
        m_editData.element->startEdit(m_editData);
    }

    notifyAboutTextEditingStarted();
    notifyAboutTextEditingChanged();
}

//! NOTE: Copied from TextBase::inputTransition
void NotationInteraction::editText(QInputMethodEvent* event)
{
    if (!isTextEditingStarted()) {
        return;
    }

    mu::engraving::TextBase* text = mu::engraving::toTextBase(m_editData.element);
    mu::engraving::TextCursor* cursor = text->cursor();
    String& preeditString = m_editData.preeditString;

    // remove preedit string
    size_t n = preeditString.size();
    while (n--) {
        if (cursor->movePosition(mu::engraving::TextCursor::MoveOperation::Left)) {
            mu::engraving::TextBlock& curLine = cursor->curLine();
            curLine.remove(static_cast<int>(cursor->column()), cursor);
            text->triggerLayout();
            text->setTextInvalid();
        }
    }

    if (!event->commitString().isEmpty()) {
        score()->startCmd();
        text->insertText(m_editData, event->commitString());
        score()->endCmd();
        preeditString.clear();
    } else {
        preeditString = event->preeditString();

        if (!preeditString.isEmpty()) {
            cursor->updateCursorFormat();
            text->editInsertText(cursor, preeditString);
            text->setTextInvalid();
            text->layout1();
            score()->update();
        }
    }

    event->accept();
    notifyAboutTextEditingChanged();
}

bool NotationInteraction::needStartEditGrip(QKeyEvent* event) const
{
    if (!m_editData.element || !m_editData.element->hasGrips()) {
        return false;
    }

    if (isGripEditStarted()) {
        return false;
    }

    static const std::set<Qt::Key> arrows {
        Qt::Key_Left,
        Qt::Key_Right,
        Qt::Key_Up,
        Qt::Key_Down
    };

    return mu::contains(arrows, static_cast<Qt::Key>(event->key()));
}

bool NotationInteraction::handleKeyPress(QKeyEvent* event)
{
    if (event->modifiers() & Qt::KeyboardModifier::AltModifier) {
        return false;
    }

    if (m_editData.element->isTextBase()) {
        return false;
    }

    qreal vRaster = mu::engraving::MScore::vRaster();
    qreal hRaster = mu::engraving::MScore::hRaster();

    switch (event->key()) {
    case Qt::Key_Tab:
        if (!m_editData.element->hasGrips()) {
            return false;
        }

        m_editData.element->nextGrip(m_editData);

        return true;
    case Qt::Key_Backtab:
        if (!m_editData.element->hasGrips()) {
            return false;
        }

        m_editData.element->prevGrip(m_editData);

        return true;
    case Qt::Key_Left:
        m_editData.delta = QPointF(-nudgeDistance(m_editData, hRaster), 0);
        break;
    case Qt::Key_Right:
        m_editData.delta = QPointF(nudgeDistance(m_editData, hRaster), 0);
        break;
    case Qt::Key_Up:
        m_editData.delta = QPointF(0, -nudgeDistance(m_editData, vRaster));
        break;
    case Qt::Key_Down:
        m_editData.delta = QPointF(0, nudgeDistance(m_editData, vRaster));
        break;
    default:
        return false;
    }

    m_editData.evtDelta = m_editData.moveDelta = m_editData.delta;
    m_editData.hRaster = hRaster;
    m_editData.vRaster = vRaster;

    if (m_editData.curGrip != mu::engraving::Grip::NO_GRIP && int(m_editData.curGrip) < m_editData.grips) {
        m_editData.pos = m_editData.grip[int(m_editData.curGrip)].center() + m_editData.delta;
    }

    m_editData.element->startEditDrag(m_editData);
    m_editData.element->editDrag(m_editData);
    m_editData.element->endEditDrag(m_editData);

    return true;
}

void NotationInteraction::endEditText()
{
    IF_ASSERT_FAILED(m_editData.element) {
        return;
    }

    if (!isTextEditingStarted()) {
        return;
    }

    doEndEditElement();

    notifyAboutTextEditingEnded();
    notifyAboutTextEditingChanged();
    notifyAboutSelectionChangedIfNeed();
}

void NotationInteraction::changeTextCursorPosition(const PointF& newCursorPos)
{
    IF_ASSERT_FAILED(isTextEditingStarted() && m_editData.element) {
        return;
    }

    m_editData.startMove = bindCursorPosToText(newCursorPos, m_editData.element);

    mu::engraving::TextBase* textEl = mu::engraving::toTextBase(m_editData.element);

    textEl->mousePress(m_editData);
    if (m_editData.buttons == mu::engraving::MiddleButton) {
        #if defined(Q_OS_MAC) || defined(Q_OS_WIN)
        QClipboard::Mode mode = QClipboard::Clipboard;
        #else
        QClipboard::Mode mode = QClipboard::Selection;
        #endif
        QString txt = QGuiApplication::clipboard()->text(mode);
        textEl->paste(m_editData, txt);
    }

    notifyAboutTextEditingChanged();
}

const TextBase* NotationInteraction::editedText() const
{
    return mu::engraving::toTextBase(m_editData.element);
}

void NotationInteraction::undo()
{
    m_undoStack->undo(&m_editData);
}

void NotationInteraction::redo()
{
    m_undoStack->redo(&m_editData);
}

mu::async::Notification NotationInteraction::textEditingStarted() const
{
    return m_textEditingStarted;
}

mu::async::Notification NotationInteraction::textEditingChanged() const
{
    return m_textEditingChanged;
}

mu::async::Notification NotationInteraction::textEditingEnded() const
{
    return m_textEditingEnded;
}

mu::async::Channel<ScoreConfigType> NotationInteraction::scoreConfigChanged() const
{
    return m_scoreConfigChanged;
}

bool NotationInteraction::isGripEditStarted() const
{
    return m_editData.element && m_editData.curGrip != mu::engraving::Grip::NO_GRIP;
}

static int findGrip(const std::vector<mu::RectF>& grips, const mu::PointF& canvasPos)
{
    if (grips.empty()) {
        return -1;
    }
    qreal align = grips[0].width() / 2;
    for (size_t i = 0; i < grips.size(); ++i) {
        if (grips[i].adjusted(-align, -align, align, align).contains(canvasPos)) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

bool NotationInteraction::isHitGrip(const PointF& pos) const
{
    return selection()->element() && findGrip(m_editData.grip, pos) != -1;
}

void NotationInteraction::startEditGrip(const PointF& pos)
{
    int grip = findGrip(m_editData.grip, pos);
    if (grip == -1) {
        return;
    }
    startEditGrip(selection()->element(), mu::engraving::Grip(grip));
}

void NotationInteraction::startEditGrip(EngravingItem* element, mu::engraving::Grip grip)
{
    if (m_editData.element == element && m_editData.curGrip == grip) {
        return;
    }

    m_editData.element = element;
    m_editData.curGrip = grip;

    updateAnchorLines();
    m_editData.element->startEdit(m_editData);

    notifyAboutNotationChanged();
}

void NotationInteraction::endEditGrip()
{
    if (m_editData.curGrip == Grip::NO_GRIP) {
        return;
    }

    m_editData.curGrip = Grip::NO_GRIP;
    notifyAboutNotationChanged();
}

void NotationInteraction::updateAnchorLines()
{
    std::vector<LineF> lines;
    mu::engraving::Grip anchorLinesGrip = m_editData.curGrip
                                          == mu::engraving::Grip::NO_GRIP ? m_editData.element->defaultGrip() : m_editData.curGrip;
    std::vector<LineF> anchorLines = m_editData.element->gripAnchorLines(anchorLinesGrip);

    if (!anchorLines.empty()) {
        for (LineF& line : anchorLines) {
            if (line.p1() != line.p2()) {
                lines.push_back(line);
            }
        }
    }

    setAnchorLines(lines);
}

bool NotationInteraction::isElementEditStarted() const
{
    return m_editData.element != nullptr;
}

void NotationInteraction::startEditElement(EngravingItem* element)
{
    if (!element) {
        return;
    }

    if (isElementEditStarted()) {
        return;
    }

    if (element->isTextBase()) {
        startEditText(element);
    } else if (element->isEditable()) {
        element->startEdit(m_editData);
        m_editData.element = element;
    }
}

void NotationInteraction::changeEditElement(EngravingItem* newElement)
{
    IF_ASSERT_FAILED(newElement) {
        return;
    }

    if (m_editData.element == newElement) {
        return;
    }

    mu::engraving::Grip currentGrip = m_editData.curGrip;
    bool gripEditStarted = isGripEditStarted();

    doEndEditElement();

    if (gripEditStarted) {
        startEditGrip(newElement, currentGrip);
    } else {
        startEditElement(newElement);
    }
}

bool NotationInteraction::isEditAllowed(QKeyEvent* event)
{
    if (!m_editData.element) {
        return false;
    }

    mu::engraving::EditData editData = m_editData;
    editData.modifiers = keyboardModifier(event->modifiers());
    editData.key = event->key();
    editData.s = event->text();

    if (editData.element->isEditAllowed(editData)) {
        return true;
    }

    if (event->modifiers() & Qt::KeyboardModifier::AltModifier) {
        return false;
    }

    if (editData.element->isTextBase()) {
        return false;
    }

    static QSet<int> navigationKeys = {
        Qt::Key_Left,
        Qt::Key_Right,
        Qt::Key_Up,
        Qt::Key_Down
    };

    if (editData.element->hasGrips()) {
        navigationKeys += { Qt::Key_Tab, Qt::Key_Backtab };
    }

    return navigationKeys.contains(event->key());
}

void NotationInteraction::editElement(QKeyEvent* event)
{
    if (!m_editData.element) {
        return;
    }

    m_editData.modifiers = keyboardModifier(event->modifiers());

    if (isDragStarted()) {
        return; // ignore all key strokes while dragging
    }

    m_editData.key = event->key();
    m_editData.s = event->text();

    // Brackets may be deleted and replaced
    bool isBracket = m_editData.element->isBracket();
    const mu::engraving::System* system = nullptr;
    size_t bracketIndex = mu::nidx;

    if (isBracket) {
        const mu::engraving::Bracket* bracket = mu::engraving::toBracket(m_editData.element);
        system = bracket->system();

        if (system) {
            bracketIndex = mu::indexOf(system->brackets(), bracket);
        }
    }

    startEdit();

    if (needStartEditGrip(event)) {
        m_editData.curGrip = m_editData.element->defaultGrip();
    }

    bool handled = m_editData.element->edit(m_editData);
    if (!handled) {
        handled = handleKeyPress(event);
    }

    if (handled) {
        event->accept();

        if (isBracket && system && bracketIndex != mu::nidx) {
            mu::engraving::EngravingItem* bracket = system->brackets().at(bracketIndex);
            m_editData.element = bracket;
            select({ bracket }, SelectType::SINGLE);
        }

        apply();

        if (isGripEditStarted()) {
            updateAnchorLines();
        }
    } else {
        rollback();
    }

    if (isTextEditingStarted()) {
        notifyAboutTextEditingChanged();
    }
}

void NotationInteraction::endEditElement()
{
    if (!m_editData.element) {
        return;
    }

    if (isTextEditingStarted()) {
        endEditText();
        return;
    }

    if (isDragStarted()) {
        doEndDrag();
        rollback();
    }

    doEndEditElement();
    resetAnchorLines();

    notifyAboutNotationChanged();
}

void NotationInteraction::doEndEditElement()
{
    if (m_editData.element) {
        m_editData.element->endEdit(m_editData);
    }

    m_editData.clear();
}

void NotationInteraction::onElementDestroyed(EngravingItem* element)
{
    if (m_editData.element == element) {
        m_editData.element = nullptr;
    }

    if (m_hitElementContext.element == element) {
        m_hitElementContext.element = nullptr;
        m_hitElementContext.staff = nullptr;
    }
}

void NotationInteraction::splitSelectedMeasure()
{
    EngravingItem* selectedElement = m_selection->element();
    if (!selectedElement) {
        return;
    }

    if (!selectedElement->isNote() && !selectedElement->isRest()) {
        return;
    }

    if (selectedElement->isNote()) {
        selectedElement = dynamic_cast<Note*>(selectedElement)->chord();
    }

    ChordRest* chordRest = dynamic_cast<ChordRest*>(selectedElement);

    startEdit();
    score()->cmdSplitMeasure(chordRest);
    apply();
}

void NotationInteraction::joinSelectedMeasures()
{
    if (!m_selection->isRange()) {
        return;
    }

    INotationSelectionRange::MeasureRange measureRange = m_selection->range()->measureRange();

    startEdit();
    score()->cmdJoinMeasure(measureRange.startMeasure, measureRange.endMeasure);
    apply();
}

mu::Ret NotationInteraction::canAddBoxes() const
{
    if (selection()->isRange()) {
        return make_ok();
    }

    static const std::vector<ElementType> boxesTypes {
        ElementType::VBOX, ElementType::HBOX, ElementType::TBOX
    };

    for (const EngravingItem* element: selection()->elements()) {
        if (mu::engraving::toMeasure(element->findMeasure())) {
            return make_ok();
        }

        if (std::find(boxesTypes.cbegin(), boxesTypes.cend(), element->type()) != boxesTypes.cend()) {
            return make_ok();
        }
    }

    return make_ret(Err::MeasureIsNotSelected);
}

void NotationInteraction::addBoxes(BoxType boxType, int count, AddBoxesTarget target)
{
    int beforeBoxIndex = -1;

    switch (target) {
    case AddBoxesTarget::AfterSelection:
    case AddBoxesTarget::BeforeSelection: {
        if (selection()->isNone()) {
            return;
        }

        if (selection()->isRange()) {
            INotationSelectionRange::MeasureRange range = selection()->range()->measureRange();
            int startMeasureIndex = range.startMeasure ? range.startMeasure->index() : 0;
            int endMeasureIndex = range.endMeasure ? range.endMeasure->index() + 1 : 0;

            beforeBoxIndex = target == AddBoxesTarget::BeforeSelection
                             ? startMeasureIndex
                             : endMeasureIndex;
            break;
        }

        auto elements = selection()->elements();
        IF_ASSERT_FAILED(!elements.empty()) {
            // This would contradict the fact that selection()->isNone() == false at this point
            return;
        }

        for (mu::engraving::EngravingItem* item : elements) {
            mu::engraving::MeasureBase* itemMeasure = item->findMeasureBase();
            if (!itemMeasure) {
                continue;
            }

            int itemMeasureIndex = itemMeasure->index();
            if (itemMeasureIndex < 0) {
                continue;
            }

            if (target == AddBoxesTarget::BeforeSelection) {
                if (beforeBoxIndex < 0 || itemMeasureIndex < beforeBoxIndex) {
                    beforeBoxIndex = itemMeasureIndex;
                }
            } else {
                if (itemMeasureIndex + 1 > beforeBoxIndex) {
                    beforeBoxIndex = itemMeasureIndex + 1;
                }
            }
        }

        if (beforeBoxIndex < 0) {
            // No suitable element found
            return;
        }
    } break;
    case AddBoxesTarget::AtStartOfScore:
        beforeBoxIndex = score()->firstMeasure()->index();
        break;
    case AddBoxesTarget::AtEndOfScore:
        beforeBoxIndex = -1;
        break;
    }

    addBoxes(boxType, count, beforeBoxIndex);
}

void NotationInteraction::addBoxes(BoxType boxType, int count, int beforeBoxIndex)
{
    if (count < 1) {
        return;
    }

    auto boxTypeToElementType = [](BoxType boxType) {
        switch (boxType) {
        case BoxType::Horizontal: return mu::engraving::ElementType::HBOX;
        case BoxType::Vertical: return mu::engraving::ElementType::VBOX;
        case BoxType::Text: return mu::engraving::ElementType::TBOX;
        case BoxType::Measure: return mu::engraving::ElementType::MEASURE;
        case BoxType::Unknown: return mu::engraving::ElementType::INVALID;
        }

        return ElementType::INVALID;
    };

    mu::engraving::ElementType elementType = boxTypeToElementType(boxType);
    if (elementType == mu::engraving::ElementType::INVALID) {
        return;
    }

    mu::engraving::MeasureBase* beforeBox = beforeBoxIndex >= 0 ? score()->measure(beforeBoxIndex) : nullptr;

    startEdit();

    mu::engraving::Score::InsertMeasureOptions options;
    options.createEmptyMeasures = false;
    options.moveSignaturesClef = true;
    options.needDeselectAll = false;

    for (int i = 0; i < count; ++i) {
        score()->insertMeasure(elementType, beforeBox, options);
    }

    apply();

    int indexOfFirstAddedMeasure = beforeBoxIndex >= 0 ? beforeBoxIndex : score()->measures()->size() - count;
    doSelect({ score()->measure(indexOfFirstAddedMeasure) }, SelectType::REPLACE);

    // For other box types, it makes little sense to select them all
    if (boxType == BoxType::Measure) {
        doSelect({ score()->measure(indexOfFirstAddedMeasure + count - 1) }, SelectType::RANGE);
    }

    notifyAboutSelectionChangedIfNeed();
}

void NotationInteraction::copySelection()
{
    if (!selection()->canCopy()) {
        return;
    }

    if (isTextEditingStarted()) {
        m_editData.element->editCopy(m_editData);
        mu::engraving::TextEditData* ted = static_cast<mu::engraving::TextEditData*>(m_editData.getData(m_editData.element).get());
        if (!ted->selectedText.isEmpty()) {
            #if defined(Q_OS_MAC) || defined(Q_OS_WIN)
            QClipboard::Mode mode = QClipboard::Clipboard;
            #else
            QClipboard::Mode mode = QClipboard::Selection;
            #endif
            QGuiApplication::clipboard()->setText(ted->selectedText, mode);
        }
    } else {
        QMimeData* mimeData = selection()->mimeData();
        if (!mimeData) {
            return;
        }
        QApplication::clipboard()->setMimeData(mimeData);
    }
}

mu::Ret NotationInteraction::repeatSelection()
{
    const mu::engraving::Selection& selection = score()->selection();
    if (score()->noteEntryMode() && selection.isSingle()) {
        EngravingItem* el = selection.element();
        if (el && el->type() == ElementType::NOTE && !score()->inputState().endOfScore()) {
            startEdit();
            Chord* c = toNote(el)->chord();
            for (Note* note : c->notes()) {
                mu::engraving::NoteVal nval = note->noteVal();
                score()->addPitch(nval, note != c->notes()[0]);
            }
            apply();
        }
        return make_ok();
    }

    if (!selection.isRange()) {
        ChordRest* cr = score()->getSelectedChordRest();
        if (!cr) {
            return make_ret(Err::EmptySelection);
        }
        score()->select(cr, SelectType::RANGE);
    }

    Ret ret = m_selection->canCopy();
    if (!ret) {
        return ret;
    }

    mu::engraving::XmlReader xml(selection.mimeData());
    xml.context()->setPasteMode(true);
    track_idx_t dStaff = selection.staffStart();
    mu::engraving::Segment* endSegment = selection.endSegment();

    if (endSegment && endSegment->segmentType() != mu::engraving::SegmentType::ChordRest) {
        endSegment = endSegment->next1(mu::engraving::SegmentType::ChordRest);
    }
    if (endSegment && endSegment->element(dStaff * mu::engraving::VOICES)) {
        EngravingItem* e = endSegment->element(dStaff * mu::engraving::VOICES);
        if (e) {
            startEdit();
            ChordRest* cr = toChordRest(e);
            score()->pasteStaff(xml, cr->segment(), cr->staffIdx());
            apply();
        }
    }

    return ret;
}

void NotationInteraction::copyLyrics()
{
    QString text = score()->extractLyrics();
    QApplication::clipboard()->setText(text);
}

void NotationInteraction::pasteSelection(const Fraction& scale)
{
    startEdit();

    if (isTextEditingStarted()) {
        #if defined(Q_OS_MAC) || defined(Q_OS_WIN)
        QClipboard::Mode mode = QClipboard::Clipboard;
        #else
        QClipboard::Mode mode = QClipboard::Selection;
        #endif
        QString txt = QGuiApplication::clipboard()->text(mode);
        toTextBase(m_editData.element)->paste(m_editData, txt);
    } else {
        const QMimeData* mimeData = QApplication::clipboard()->mimeData();
        QMimeDataAdapter ma(mimeData);
        score()->cmdPaste(&ma, nullptr, scale);
    }
    apply();
}

void NotationInteraction::swapSelection()
{
    if (!selection()->canCopy()) {
        return;
    }

    mu::engraving::Selection& selection = score()->selection();
    QString mimeType = selection.mimeType();

    if (mimeType == mu::engraving::mimeStaffListFormat) { // determine size of clipboard selection
        const QMimeData* mimeData = this->selection()->mimeData();
        QByteArray data = mimeData ? mimeData->data(mu::engraving::mimeStaffListFormat) : QByteArray();
        mu::engraving::XmlReader reader(data);
        reader.readNextStartElement();

        Fraction tickLen = Fraction(0, 1);
        int stavesCount = 0;

        if (reader.name() == "StaffList") {
            tickLen = mu::engraving::Fraction::fromTicks(reader.intAttribute("len", 0));
            stavesCount = reader.intAttribute("staves", 0);
        }

        if (tickLen > mu::engraving::Fraction(0, 1)) { // attempt to extend selection to match clipboard size
            mu::engraving::Segment* segment = selection.startSegment();
            mu::engraving::Fraction startTick = selection.tickStart() + tickLen;
            mu::engraving::Segment* segmentAfter = score()->tick2leftSegment(startTick);

            size_t staffIndex = selection.staffStart() + stavesCount - 1;
            if (staffIndex >= score()->nstaves()) {
                staffIndex = score()->nstaves() - 1;
            }

            startTick = selection.tickStart();
            mu::engraving::Fraction endTick = startTick + tickLen;
            selection.extendRangeSelection(segment, segmentAfter, staffIndex, startTick, endTick);
            selection.update();
        }
    }

    QByteArray currentSelectionBackup = selection.mimeData().toQByteArray();
    pasteSelection();
    QMimeData* mimeData = new QMimeData();
    mimeData->setData(mimeType, currentSelectionBackup);
    QApplication::clipboard()->setMimeData(mimeData);
}

void NotationInteraction::deleteSelection()
{
    if (selection()->isNone()) {
        return;
    }

    startEdit();

    if (isTextEditingStarted()) {
        mu::engraving::TextBase* textBase = toTextBase(m_editData.element);
        if (!textBase->deleteSelectedText(m_editData)) {
            m_editData.key = Qt::Key_Backspace;
            m_editData.modifiers = {};
            textBase->edit(m_editData);
        }
    } else {
        doEndEditElement();
        resetGripEdit();
        score()->cmdDeleteSelection();
    }

    apply();
}

void NotationInteraction::flipSelection()
{
    if (selection()->isNone()) {
        return;
    }

    startEdit();
    score()->cmdFlip();
    apply();
}

void NotationInteraction::addTieToSelection()
{
    startEdit();
    score()->cmdToggleTie();
    apply();
}

void NotationInteraction::addTiedNoteToChord()
{
    startEdit();
    score()->cmdAddTie(true);
    apply();
}

void NotationInteraction::addSlurToSelection()
{
    if (selection()->isNone()) {
        return;
    }

    startEdit();
    doAddSlur();
    apply();
}

void NotationInteraction::addOttavaToSelection(OttavaType type)
{
    if (selection()->isNone()) {
        return;
    }

    startEdit();
    score()->cmdAddOttava(type);
    apply();
}

void NotationInteraction::addHairpinsToSelection(HairpinType type)
{
    if (selection()->isNone()) {
        return;
    }

    startEdit();
    std::vector<mu::engraving::Hairpin*> hairpins = score()->addHairpins(type);
    apply();

    if (!noteInput()->isNoteInputMode() && hairpins.size() == 1) {
        mu::engraving::LineSegment* segment = hairpins.front()->frontSegment();
        select({ segment });
        startEditGrip(segment, mu::engraving::Grip::END);
    }
}

void NotationInteraction::addAccidentalToSelection(AccidentalType type)
{
    if (selection()->isNone()) {
        return;
    }

    mu::engraving::EditData editData(&m_scoreCallbacks);

    startEdit();
    score()->toggleAccidental(type, editData);
    apply();
}

void NotationInteraction::putRestToSelection()
{
    mu::engraving::InputState& is = score()->inputState();
    if (!is.duration().isValid() || is.duration().isZero() || is.duration().isMeasure()) {
        is.setDuration(DurationType::V_QUARTER);
    }
    putRest(is.duration().type());
}

void NotationInteraction::putRest(DurationType duration)
{
    if (selection()->isNone()) {
        return;
    }

    startEdit();
    score()->cmdEnterRest(Duration(duration));
    apply();
}

void NotationInteraction::addBracketsToSelection(BracketsType type)
{
    if (selection()->isNone()) {
        return;
    }

    startEdit();

    switch (type) {
    case BracketsType::Brackets:
        score()->cmdAddBracket();
        break;
    case BracketsType::Braces:
        score()->cmdAddBraces();
        break;
    case BracketsType::Parentheses:
        score()->cmdAddParentheses();
        break;
    }

    apply();
}

void NotationInteraction::changeSelectedNotesArticulation(SymbolId articulationSymbolId)
{
    if (selection()->isNone()) {
        return;
    }

    std::vector<mu::engraving::Note*> notes = score()->selection().noteList();

    auto updateMode = notesHaveActiculation(notes, articulationSymbolId)
                      ? mu::engraving::ArticulationsUpdateMode::Remove : mu::engraving::ArticulationsUpdateMode::Insert;

    std::set<Chord*> chords;
    for (Note* note: notes) {
        Chord* chord = note->chord();
        if (chords.find(chord) == chords.end()) {
            chords.insert(chord);
        }
    }

    startEdit();
    for (Chord* chord: chords) {
        chord->updateArticulations({ articulationSymbolId }, updateMode);
    }
    apply();
}

void NotationInteraction::addGraceNotesToSelectedNotes(GraceNoteType type)
{
    if (selection()->isNone()) {
        return;
    }

    int denominator = 1;

    switch (type) {
    case GraceNoteType::GRACE4:
    case GraceNoteType::INVALID:
    case GraceNoteType::NORMAL:
        denominator = 1;
        break;
    case GraceNoteType::ACCIACCATURA:
    case GraceNoteType::APPOGGIATURA:
    case GraceNoteType::GRACE8_AFTER:
        denominator = 2;
        break;
    case GraceNoteType::GRACE16:
    case GraceNoteType::GRACE16_AFTER:
        denominator = 4;
        break;
    case GraceNoteType::GRACE32:
    case GraceNoteType::GRACE32_AFTER:
        denominator = 8;
        break;
    }

    startEdit();
    score()->cmdAddGrace(type, mu::engraving::Constants::division / denominator);
    apply();
}

bool NotationInteraction::canAddTupletToSelectedChordRests() const
{
    for (ChordRest* chordRest : score()->getSelectedChordRests()) {
        if (chordRest->isGrace()) {
            continue;
        }

        if (chordRest->durationType() < mu::engraving::TDuration(mu::engraving::DurationType::V_512TH)
            && chordRest->durationType() != mu::engraving::TDuration(mu::engraving::DurationType::V_MEASURE)) {
            return false;
        }
    }

    return true;
}

void NotationInteraction::addTupletToSelectedChordRests(const TupletOptions& options)
{
    if (selection()->isNone()) {
        return;
    }

    startEdit();

    for (ChordRest* chordRest : score()->getSelectedChordRests()) {
        if (!chordRest->isGrace()) {
            Fraction ratio = options.ratio;
            // prevent weird dotted tuplets when adding tuplets to dotted durations
            if (options.autoBaseLen) {
                ratio.setDenominator(chordRest->dots() ? 3 : 2);
                while (ratio.numerator() >= ratio.denominator() * 2) {
                    ratio.setDenominator(ratio.denominator() * 2);      // operator*= reduces, we don't want that here
                }
            }
            score()->addTuplet(chordRest, ratio, options.numberType, options.bracketType);
        }
    }

    apply();
}

void NotationInteraction::addBeamToSelectedChordRests(BeamMode mode)
{
    if (selection()->isNone()) {
        return;
    }

    startEdit();
    score()->cmdSetBeamMode(mode);
    apply();
}

void NotationInteraction::increaseDecreaseDuration(int steps, bool stepByDots)
{
    if (selection()->isNone()) {
        return;
    }

    startEdit();
    score()->cmdIncDecDuration(steps, stepByDots);
    apply();
}

bool NotationInteraction::toggleLayoutBreakAvailable() const
{
    return !selection()->isNone() && !isTextEditingStarted();
}

void NotationInteraction::toggleLayoutBreak(LayoutBreakType breakType)
{
    startEdit();
    score()->cmdToggleLayoutBreak(breakType);
    apply();
}

void NotationInteraction::setBreaksSpawnInterval(BreaksSpawnIntervalType intervalType, int interval)
{
    interval = intervalType == BreaksSpawnIntervalType::MeasuresInterval ? interval : 0;
    bool afterEachSystem = intervalType == BreaksSpawnIntervalType::AfterEachSystem;

    startEdit();
    score()->addRemoveBreaks(interval, afterEachSystem);
    apply();
}

bool NotationInteraction::transpose(const TransposeOptions& options)
{
    startEdit();

    bool ok = score()->transpose(options.mode, options.direction, options.key, options.interval,
                                 options.needTransposeKeys, options.needTransposeChordNames, options.needTransposeDoubleSharpsFlats);

    apply();

    return ok;
}

void NotationInteraction::swapVoices(int voiceIndex1, int voiceIndex2)
{
    if (selection()->isNone()) {
        return;
    }

    if (voiceIndex1 == voiceIndex2) {
        return;
    }

    if (!isVoiceIndexValid(voiceIndex1) || !isVoiceIndexValid(voiceIndex2)) {
        return;
    }

    startEdit();
    score()->cmdExchangeVoice(voiceIndex1, voiceIndex2);
    apply();
}

void NotationInteraction::addIntervalToSelectedNotes(int interval)
{
    if (!isNotesIntervalValid(interval)) {
        return;
    }

    std::vector<Note*> notes;

    if (score()->selection().isRange()) {
        for (const ChordRest* chordRest : score()->getSelectedChordRests()) {
            if (chordRest->isChord()) {
                const Chord* chord = toChord(chordRest);
                Note* note = interval > 0 ? chord->upNote() : chord->downNote();
                notes.push_back(note);
            }
        }
    } else {
        notes = score()->selection().noteList();
    }

    if (notes.empty()) {
        return;
    }

    startEdit();
    score()->addInterval(interval, notes);
    apply();
}

void NotationInteraction::addFret(int fretIndex)
{
    startEdit();
    score()->cmdAddFret(fretIndex);
    apply();
}

void NotationInteraction::changeSelectedNotesVoice(int voiceIndex)
{
    if (selection()->isNone()) {
        return;
    }

    if (!isVoiceIndexValid(voiceIndex)) {
        return;
    }

    startEdit();
    score()->changeSelectedNotesVoice(voiceIndex);
    apply();
}

void NotationInteraction::addAnchoredLineToSelectedNotes()
{
    if (selection()->isNone()) {
        return;
    }

    startEdit();
    score()->addNoteLine();
    apply();
}

void NotationInteraction::addTextToTopFrame(TextStyleType type)
{
    addText(type);
}

mu::Ret NotationInteraction::canAddTextToItem(TextStyleType type, const EngravingItem* item) const
{
    if (!item) {
        return false;
    }

    if (isVerticalBoxTextStyle(type)) {
        return item->isVBox();
    }

    if (type == TextStyleType::FRAME) {
        return item->isBox();
    }

    static const std::set<TextStyleType> needSelectNoteOrRestTypes {
        TextStyleType::SYSTEM,
        TextStyleType::STAFF,
        TextStyleType::EXPRESSION,
        TextStyleType::REHEARSAL_MARK,
        TextStyleType::INSTRUMENT_CHANGE,
        TextStyleType::FINGERING,
        TextStyleType::LH_GUITAR_FINGERING,
        TextStyleType::RH_GUITAR_FINGERING,
        TextStyleType::STRING_NUMBER,
        TextStyleType::STICKING,
        TextStyleType::HARMONY_A,
        TextStyleType::HARMONY_ROMAN,
        TextStyleType::HARMONY_NASHVILLE,
        TextStyleType::LYRICS_ODD,
        TextStyleType::TEMPO,
    };

    if (mu::contains(needSelectNoteOrRestTypes, type)) {
        static const std::set<ElementType> requiredElementTypes {
            ElementType::NOTE,
            ElementType::REST,
            ElementType::MEASURE_REPEAT,
            ElementType::CHORD,
        };

        bool isNoteOrRestSelected = mu::contains(requiredElementTypes, item->type());
        return isNoteOrRestSelected ? make_ok() : make_ret(Err::NoteOrRestIsNotSelected);
    }

    return make_ok();
}

void NotationInteraction::addTextToItem(TextStyleType type, EngravingItem* item)
{
    if (!scoreHasMeasure()) {
        LOGE() << "Need to create measure";
        return;
    }

    addText(type, item);
}

void NotationInteraction::addText(TextStyleType type, EngravingItem* item)
{
    if (m_noteInput->isNoteInputMode()) {
        m_noteInput->endNoteInput();
    }

    startEdit();
    mu::engraving::TextBase* textBox = score()->addText(type, item);

    if (!textBox) {
        rollback();
        return;
    }

    apply();
    startEditText(textBox);
}

mu::Ret NotationInteraction::canAddImageToItem(const EngravingItem* item) const
{
    return item && item->isMeasureBase();
}

void NotationInteraction::addImageToItem(const io::path_t& imagePath, EngravingItem* item)
{
    if (imagePath.empty() || !item) {
        return;
    }

    static std::map<io::path_t, ImageType> suffixToType {
        { "svg", ImageType::SVG },
        { "jpg", ImageType::RASTER },
        { "jpeg", ImageType::RASTER },
        { "png", ImageType::RASTER },
    };

    io::path_t suffix = io::suffix(imagePath);

    ImageType type = mu::value(suffixToType, suffix, ImageType::NONE);
    if (type == ImageType::NONE) {
        return;
    }

    Image* image = Factory::createImage(item);
    image->setImageType(type);

    if (!image->load(imagePath)) {
        delete image;
        return;
    }

    startEdit();
    score()->undoAddElement(image);
    apply();
}

mu::Ret NotationInteraction::canAddFiguredBass() const
{
    static const std::set<ElementType> requiredTypes {
        ElementType::NOTE,
        ElementType::FIGURED_BASS,
        ElementType::REST
    };

    bool isNoteOrRestSelected = elementsSelected(requiredTypes);
    return isNoteOrRestSelected ? make_ok() : make_ret(Err::NoteOrFiguredBassIsNotSelected);
}

void NotationInteraction::addFiguredBass()
{
    startEdit();
    mu::engraving::FiguredBass* figuredBass = score()->addFiguredBass();

    if (figuredBass) {
        apply();
        startEditText(figuredBass, PointF());
        notifyAboutSelectionChangedIfNeed();
    } else {
        rollback();
    }
}

void NotationInteraction::addStretch(qreal value)
{
    startEdit();
    score()->cmdAddStretch(value);
    apply();
}

void NotationInteraction::addTimeSignature(Measure* measure, staff_idx_t staffIndex, TimeSignature* timeSignature)
{
    startEdit();
    score()->cmdAddTimeSig(measure, staffIndex, timeSignature, true);
    apply();
}

void NotationInteraction::explodeSelectedStaff()
{
    if (!selection()->isRange()) {
        return;
    }

    startEdit();
    score()->cmdExplode();
    apply();
}

void NotationInteraction::implodeSelectedStaff()
{
    if (!selection()->isRange()) {
        return;
    }

    startEdit();
    score()->cmdImplode();
    apply();
}

void NotationInteraction::realizeSelectedChordSymbols(bool literal, Voicing voicing, HarmonyDurationType durationType)
{
    if (selection()->isNone()) {
        return;
    }

    startEdit();
    score()->cmdRealizeChordSymbols(literal, voicing, durationType);
    apply();
}

void NotationInteraction::removeSelectedMeasures()
{
    if (selection()->isNone()) {
        return;
    }

    mu::engraving::MeasureBase* firstMeasure = nullptr;
    mu::engraving::MeasureBase* lastMeasure = nullptr;

    if (selection()->isRange()) {
        INotationSelectionRange::MeasureRange measureRange = selection()->range()->measureRange();
        firstMeasure = measureRange.startMeasure;
        lastMeasure = measureRange.endMeasure;
    } else {
        auto elements = selection()->elements();
        if (elements.empty()) {
            return;
        }

        for (auto element : elements) {
            mu::engraving::MeasureBase* elementMeasure = element->findMeasureBase();

            if (!firstMeasure || firstMeasure->index() > elementMeasure->index()) {
                firstMeasure = elementMeasure;
            }

            if (!lastMeasure || lastMeasure->index() < elementMeasure->index()) {
                lastMeasure = elementMeasure;
            }
        }
    }

    doSelect({ firstMeasure }, SelectType::REPLACE);
    doSelect({ lastMeasure }, SelectType::RANGE);

    startEdit();
    score()->cmdTimeDelete();
    apply();
}

void NotationInteraction::removeSelectedRange()
{
    if (selection()->isNone()) {
        return;
    }

    startEdit();
    score()->cmdTimeDelete();
    apply();
}

void NotationInteraction::removeEmptyTrailingMeasures()
{
    startEdit();
    score()->cmdRemoveEmptyTrailingMeasures();
    apply();
}

void NotationInteraction::fillSelectionWithSlashes()
{
    if (selection()->isNone()) {
        return;
    }

    startEdit();
    score()->cmdSlashFill();
    apply();
}

void NotationInteraction::replaceSelectedNotesWithSlashes()
{
    if (selection()->isNone()) {
        return;
    }

    startEdit();
    score()->cmdSlashRhythm();
    apply();
}

void NotationInteraction::changeEnharmonicSpelling(bool both)
{
    startEdit();
    score()->changeEnharmonicSpelling(both);
    apply();
}

void NotationInteraction::spellPitches()
{
    startEdit();
    score()->spell();
    apply();
}

void NotationInteraction::regroupNotesAndRests()
{
    startEdit();
    score()->cmdResetNoteAndRestGroupings();
    apply();
}

void NotationInteraction::resequenceRehearsalMarks()
{
    startEdit();
    score()->cmdResequenceRehearsalMarks();
    apply();
}

void NotationInteraction::resetStretch()
{
    startEdit();
    score()->resetUserStretch();
    apply();
}

void NotationInteraction::resetTextStyleOverrides()
{
    startEdit();
    score()->cmdResetTextStyleOverrides();
    apply();
}

void NotationInteraction::resetBeamMode()
{
    startEdit();
    score()->cmdResetBeamMode();
    apply();
}

void NotationInteraction::resetShapesAndPosition()
{
    auto resetItem = [](EngravingItem* item) {
        item->reset();

        if (item->isSpanner()) {
            for (mu::engraving::SpannerSegment* spannerSegment : toSpanner(item)->spannerSegments()) {
                spannerSegment->reset();
            }
        }
    };

    startEdit();

    DEFER {
        apply();
    };

    if (selection()->element()) {
        resetItem(selection()->element());
        return;
    }

    for (EngravingItem* item : selection()->elements()) {
        resetItem(item);
    }
}

ScoreConfig NotationInteraction::scoreConfig() const
{
    ScoreConfig config;
    config.isShowInvisibleElements = score()->showInvisible();
    config.isShowUnprintableElements = score()->showUnprintable();
    config.isShowFrames = score()->showFrames();
    config.isShowPageMargins = score()->showPageborders();
    config.isMarkIrregularMeasures = score()->markIrregularMeasures();

    return config;
}

void NotationInteraction::setScoreConfig(ScoreConfig config)
{
    startEdit();
    score()->setShowInvisible(config.isShowInvisibleElements);
    score()->setShowUnprintable(config.isShowUnprintableElements);
    score()->setShowFrames(config.isShowFrames);
    score()->setShowPageborders(config.isShowPageMargins);
    score()->setMarkIrregularMeasures(config.isMarkIrregularMeasures);

    EngravingItem* selectedElement = selection()->element();
    if (selectedElement && !selectedElement->isInteractionAvailable()) {
        clearSelection();
    }

    apply();
}

bool NotationInteraction::needEndTextEditing(const std::vector<EngravingItem*>& newSelectedElements) const
{
    if (!isTextEditingStarted()) {
        return false;
    }

    if (newSelectedElements.empty()) {
        return false;
    }

    if (newSelectedElements.size() > 1) {
        return true;
    }

    return newSelectedElements.front() != m_editData.element;
}

void NotationInteraction::resetGripEdit()
{
    m_editData.grips = 0;
    m_editData.curGrip = mu::engraving::Grip::NO_GRIP;
    m_editData.grip.clear();

    resetAnchorLines();
}

void NotationInteraction::resetHitElementContext()
{
    setHitElementContext(HitElementContext());
}

bool NotationInteraction::elementsSelected(const std::set<ElementType>& elementsTypes) const
{
    const EngravingItem* element = selection()->element();
    return element && mu::contains(elementsTypes, element->type());
}

void NotationInteraction::navigateToLyrics(bool back, bool moveOnly, bool end)
{
    if (!m_editData.element || !m_editData.element->isLyrics()) {
        LOGW("nextLyric called with invalid current element");
        return;
    }
    mu::engraving::Lyrics* lyrics = toLyrics(m_editData.element);
    track_idx_t track = lyrics->track();
    mu::engraving::Segment* segment = lyrics->segment();
    int verse = lyrics->no();
    mu::engraving::PlacementV placement = lyrics->placement();
    mu::engraving::PropertyFlags pFlags = lyrics->propertyFlags(mu::engraving::Pid::PLACEMENT);

    mu::engraving::Segment* nextSegment = segment;
    if (back) {
        // search prev chord
        while ((nextSegment = nextSegment->prev1(mu::engraving::SegmentType::ChordRest))) {
            EngravingItem* el = nextSegment->element(track);
            if (el && el->isChord()) {
                break;
            }
        }
    } else {
        // search next chord
        while ((nextSegment = nextSegment->next1(mu::engraving::SegmentType::ChordRest))) {
            EngravingItem* el = nextSegment->element(track);
            if (el && el->isChord()) {
                break;
            }
        }
    }
    if (nextSegment == 0) {
        return;
    }

    endEditText();

    // look for the lyrics we are moving from; may be the current lyrics or a previous one
    // if we are skipping several chords with spaces
    mu::engraving::Lyrics* fromLyrics = 0;
    if (!back) {
        while (segment) {
            ChordRest* cr = toChordRest(segment->element(track));
            if (cr) {
                fromLyrics = cr->lyrics(verse, placement);
                if (fromLyrics) {
                    break;
                }
            }
            segment = segment->prev1(mu::engraving::SegmentType::ChordRest);
        }
    }

    ChordRest* cr = toChordRest(nextSegment->element(track));
    if (!cr) {
        LOGD("no next lyrics list: %s", nextSegment->element(track)->typeName());
        return;
    }
    mu::engraving::Lyrics* nextLyrics = cr->lyrics(verse, placement);

    bool newLyrics = false;
    if (!nextLyrics) {
        nextLyrics = Factory::createLyrics(cr);
        nextLyrics->setTrack(track);
        cr = toChordRest(nextSegment->element(track));
        nextLyrics->setParent(cr);
        nextLyrics->setNo(verse);
        nextLyrics->setPlacement(placement);
        nextLyrics->setPropertyFlags(mu::engraving::Pid::PLACEMENT, pFlags);
        nextLyrics->setSyllabic(mu::engraving::Lyrics::Syllabic::SINGLE);
        newLyrics = true;
    }

    score()->startCmd();
    if (fromLyrics && !moveOnly) {
        switch (nextLyrics->syllabic()) {
        // as we arrived at nextLyrics by a [Space], it can be the beginning
        // of a multi-syllable, but cannot have syllabic dashes before
        case mu::engraving::Lyrics::Syllabic::SINGLE:
        case mu::engraving::Lyrics::Syllabic::BEGIN:
            break;
        case mu::engraving::Lyrics::Syllabic::END:
            nextLyrics->undoChangeProperty(mu::engraving::Pid::SYLLABIC, int(mu::engraving::Lyrics::Syllabic::SINGLE));
            break;
        case mu::engraving::Lyrics::Syllabic::MIDDLE:
            nextLyrics->undoChangeProperty(mu::engraving::Pid::SYLLABIC, int(mu::engraving::Lyrics::Syllabic::BEGIN));
            break;
        }
        // as we moved away from fromLyrics by a [Space], it can be
        // the end of a multi-syllable, but cannot have syllabic dashes after
        switch (fromLyrics->syllabic()) {
        case mu::engraving::Lyrics::Syllabic::SINGLE:
        case mu::engraving::Lyrics::Syllabic::END:
            break;
        case mu::engraving::Lyrics::Syllabic::BEGIN:
            fromLyrics->undoChangeProperty(mu::engraving::Pid::SYLLABIC, int(mu::engraving::Lyrics::Syllabic::SINGLE));
            break;
        case mu::engraving::Lyrics::Syllabic::MIDDLE:
            fromLyrics->undoChangeProperty(mu::engraving::Pid::SYLLABIC, int(mu::engraving::Lyrics::Syllabic::END));
            break;
        }
        // for the same reason, it cannot have a melisma
        fromLyrics->undoChangeProperty(mu::engraving::Pid::LYRIC_TICKS, 0);
    }

    if (newLyrics) {
        score()->undoAddElement(nextLyrics);
    }
    score()->endCmd();
    score()->select(nextLyrics, SelectType::SINGLE, 0);
    score()->setLayoutAll();

    startEditText(nextLyrics, PointF());

    mu::engraving::TextCursor* cursor = nextLyrics->cursor();
    if (end) {
        nextLyrics->selectAll(cursor);
    } else {
        cursor->movePosition(mu::engraving::TextCursor::MoveOperation::End, mu::engraving::TextCursor::MoveMode::MoveAnchor);
        cursor->movePosition(mu::engraving::TextCursor::MoveOperation::Start, mu::engraving::TextCursor::MoveMode::KeepAnchor);
    }

    showItem(nextLyrics);
}

void NotationInteraction::navigateToLyrics(MoveDirection direction)
{
    navigateToLyrics(direction == MoveDirection::Left, true, false);
}

void NotationInteraction::navigateToNextSyllable()
{
    if (!m_editData.element || !m_editData.element->isLyrics()) {
        LOGW("nextSyllable called with invalid current element");
        return;
    }
    mu::engraving::Lyrics* lyrics = toLyrics(m_editData.element);
    track_idx_t track = lyrics->track();
    mu::engraving::Segment* segment = lyrics->segment();
    int verse = lyrics->no();
    mu::engraving::PlacementV placement = lyrics->placement();
    mu::engraving::PropertyFlags pFlags = lyrics->propertyFlags(mu::engraving::Pid::PLACEMENT);

    // search next chord
    mu::engraving::Segment* nextSegment = segment;
    while ((nextSegment = nextSegment->next1(mu::engraving::SegmentType::ChordRest))) {
        EngravingItem* el = nextSegment->element(track);
        if (el && el->isChord()) {
            break;
        }
    }
    if (nextSegment == 0) {
        return;
    }

    // look for the lyrics we are moving from; may be the current lyrics or a previous one
    // we are extending with several dashes
    mu::engraving::Lyrics* fromLyrics = 0;
    while (segment) {
        ChordRest* cr = toChordRest(segment->element(track));
        if (!cr) {
            segment = segment->prev1(mu::engraving::SegmentType::ChordRest);
            continue;
        }
        fromLyrics = cr->lyrics(verse, placement);
        if (fromLyrics) {
            break;
        }
        segment = segment->prev1(mu::engraving::SegmentType::ChordRest);
    }

    endEditText();

    score()->startCmd();
    ChordRest* cr = toChordRest(nextSegment->element(track));
    mu::engraving::Lyrics* toLyrics = cr->lyrics(verse, placement);
    bool newLyrics = (toLyrics == 0);
    if (!toLyrics) {
        toLyrics = Factory::createLyrics(cr);
        toLyrics->setTrack(track);
        toLyrics->setParent(cr);
        toLyrics->setNo(verse);
        toLyrics->setPlacement(placement);
        toLyrics->setPropertyFlags(mu::engraving::Pid::PLACEMENT, pFlags);
        toLyrics->setSyllabic(mu::engraving::Lyrics::Syllabic::END);
    } else {
        // as we arrived at toLyrics by a dash, it cannot be initial or isolated
        if (toLyrics->syllabic() == mu::engraving::Lyrics::Syllabic::BEGIN) {
            toLyrics->undoChangeProperty(mu::engraving::Pid::SYLLABIC, int(mu::engraving::Lyrics::Syllabic::MIDDLE));
        } else if (toLyrics->syllabic() == mu::engraving::Lyrics::Syllabic::SINGLE) {
            toLyrics->undoChangeProperty(mu::engraving::Pid::SYLLABIC, int(mu::engraving::Lyrics::Syllabic::END));
        }
    }

    if (fromLyrics) {
        // as we moved away from fromLyrics by a dash,
        // it can have syll. dashes before and after but cannot be isolated or terminal
        switch (fromLyrics->syllabic()) {
        case mu::engraving::Lyrics::Syllabic::BEGIN:
        case mu::engraving::Lyrics::Syllabic::MIDDLE:
            break;
        case mu::engraving::Lyrics::Syllabic::SINGLE:
            fromLyrics->undoChangeProperty(mu::engraving::Pid::SYLLABIC, int(mu::engraving::Lyrics::Syllabic::BEGIN));
            break;
        case mu::engraving::Lyrics::Syllabic::END:
            fromLyrics->undoChangeProperty(mu::engraving::Pid::SYLLABIC, int(mu::engraving::Lyrics::Syllabic::MIDDLE));
            break;
        }
        // for the same reason, it cannot have a melisma
        fromLyrics->undoChangeProperty(mu::engraving::Pid::LYRIC_TICKS, Fraction(0, 1));
    }

    if (newLyrics) {
        score()->undoAddElement(toLyrics);
    }

    score()->endCmd();
    score()->select(toLyrics, SelectType::SINGLE, 0);
    score()->setLayoutAll();

    startEditText(toLyrics, PointF());

    toLyrics->selectAll(toLyrics->cursor());
    showItem(toLyrics);
}

void NotationInteraction::navigateToLyricsVerse(MoveDirection direction)
{
    if (!m_editData.element || !m_editData.element->isLyrics()) {
        LOGW("nextLyricVerse called with invalid current element");
        return;
    }
    mu::engraving::Lyrics* lyrics = toLyrics(m_editData.element);
    engraving::track_idx_t track = lyrics->track();
    ChordRest* cr = lyrics->chordRest();
    int verse = lyrics->no();
    mu::engraving::PlacementV placement = lyrics->placement();
    mu::engraving::PropertyFlags pFlags = lyrics->propertyFlags(mu::engraving::Pid::PLACEMENT);

    if (direction == MoveDirection::Up) {
        if (verse == 0) {
            return;
        }
        --verse;
    } else {
        ++verse;
        if (verse > cr->lastVerse(placement)) {
            return;
        }
    }

    endEditText();

    lyrics = cr->lyrics(verse, placement);
    if (!lyrics) {
        lyrics = Factory::createLyrics(cr);
        lyrics->setTrack(track);
        lyrics->setParent(cr);
        lyrics->setNo(verse);
        lyrics->setPlacement(placement);
        lyrics->setPropertyFlags(mu::engraving::Pid::PLACEMENT, pFlags);
        score()->startCmd();
        score()->undoAddElement(lyrics);
        score()->endCmd();
    }

    score()->select(lyrics, SelectType::SINGLE, 0);
    startEditText(lyrics, PointF());

    lyrics = toLyrics(m_editData.element);

    score()->setLayoutAll();
    score()->update();

    lyrics->selectAll(lyrics->cursor());
    showItem(lyrics);
}

//! NOTE: Copied from ScoreView::harmonyBeatsTab
void NotationInteraction::navigateToNearHarmony(MoveDirection direction, bool nearNoteOrRest)
{
    mu::engraving::Harmony* harmony = editedHarmony();
    mu::engraving::Segment* segment = harmony ? toSegment(harmony->parent()) : nullptr;
    if (!segment) {
        LOGD("no segment");
        return;
    }

    Measure* measure = segment->measure();
    Fraction tick = segment->tick();
    engraving::track_idx_t track = harmony->track();
    bool backDirection = direction == MoveDirection::Left;

    if (backDirection && tick == measure->tick()) {
        // previous bar, if any
        measure = measure->prevMeasure();
        if (!measure) {
            LOGD("no previous measure");
            return;
        }
    }

    Fraction f = measure->ticks();
    int ticksPerBeat   = f.ticks()
                         / ((f.numerator() > 3 && (f.numerator() % 3) == 0 && f.denominator() > 4) ? f.numerator() / 3 : f.numerator());
    Fraction tickInBar = tick - measure->tick();
    Fraction newTick   = measure->tick()
                         + Fraction::fromTicks((
                                                   (tickInBar.ticks() + (backDirection ? -1 : ticksPerBeat)) / ticksPerBeat
                                                   )
                                               * ticksPerBeat);

    bool needAddSegment = false;

    // look for next/prev beat, note, rest or chord
    for (;;) {
        segment = backDirection ? segment->prev1(mu::engraving::SegmentType::ChordRest) : segment->next1(
            mu::engraving::SegmentType::ChordRest);

        if (!segment || (backDirection ? (segment->tick() < newTick) : (segment->tick() > newTick))) {
            // no segment or moved past the beat - create new segment
            if (!backDirection && newTick >= measure->tick() + f) {
                // next bar, if any
                measure = measure->nextMeasure();
                if (!measure) {
                    LOGD("no next measure");
                    return;
                }
            }

            segment = Factory::createSegment(measure, mu::engraving::SegmentType::ChordRest, newTick - measure->tick());
            if (!segment) {
                LOGD("no prev segment");
                return;
            }
            needAddSegment = true;
            break;
        }

        if (segment->tick() == newTick) {
            break;
        }

        if (nearNoteOrRest) {
            track_idx_t minTrack = (track / mu::engraving::VOICES) * mu::engraving::VOICES;
            track_idx_t maxTrack = minTrack + (mu::engraving::VOICES - 1);
            if (segment->hasAnnotationOrElement(ElementType::HARMONY, minTrack, maxTrack)) {
                break;
            }
        }
    }

    startEdit();

    if (needAddSegment) {
        score()->undoAddElement(segment);
    }

    mu::engraving::Harmony* nextHarmony = findHarmonyInSegment(segment, track, harmony->textStyleType());
    if (!nextHarmony) {
        nextHarmony = createHarmony(segment, track, harmony->harmonyType());
        score()->undoAddElement(nextHarmony);
    }

    apply();
    startEditText(nextHarmony);
    showItem(nextHarmony);
}

//! NOTE: Copied from ScoreView::harmonyTab
void NotationInteraction::navigateToHarmonyInNearMeasure(MoveDirection direction)
{
    mu::engraving::Harmony* harmony = editedHarmony();
    mu::engraving::Segment* segment = harmony ? toSegment(harmony->parent()) : nullptr;
    if (!segment) {
        LOGD("harmonyTicksTab: no segment");
        return;
    }

    // moving to next/prev measure
    Measure* measure = segment->measure();
    if (measure) {
        if (direction == MoveDirection::Left) {
            measure = measure->prevMeasure();
        } else {
            measure = measure->nextMeasure();
        }
    }

    if (!measure) {
        LOGD("no prev/next measure");
        return;
    }

    segment = measure->findSegment(mu::engraving::SegmentType::ChordRest, measure->tick());
    if (!segment) {
        LOGD("no ChordRest segment as measure");
        return;
    }

    track_idx_t track = harmony->track();

    mu::engraving::Harmony* nextHarmony = findHarmonyInSegment(segment, track, harmony->textStyleType());
    if (!nextHarmony) {
        nextHarmony = createHarmony(segment, track, harmony->harmonyType());

        startEdit();
        score()->undoAddElement(nextHarmony);
        apply();
    }

    startEditText(nextHarmony);
    showItem(nextHarmony);
}

//! NOTE: Copied from ScoreView::harmonyBeatsTab
void NotationInteraction::navigateToHarmony(const Fraction& ticks)
{
    mu::engraving::Harmony* harmony = editedHarmony();
    mu::engraving::Segment* segment = harmony ? toSegment(harmony->parent()) : nullptr;
    if (!segment) {
        LOGD("no segment");
        return;
    }

    Measure* measure = segment->measure();

    Fraction newTick   = segment->tick() + ticks;

    // find the measure containing the target tick
    while (newTick >= measure->tick() + measure->ticks()) {
        measure = measure->nextMeasure();
        if (!measure) {
            LOGD("no next measure");
            return;
        }
    }

    // look for a segment at this tick; if none, create one
    while (segment && segment->tick() < newTick) {
        segment = segment->next1(mu::engraving::SegmentType::ChordRest);
    }

    startEdit();

    if (!segment || segment->tick() > newTick) {      // no ChordRest segment at this tick
        segment = Factory::createSegment(measure, mu::engraving::SegmentType::ChordRest, newTick - measure->tick());
        score()->undoAddElement(segment);
    }

    engraving::track_idx_t track = harmony->track();

    mu::engraving::Harmony* nextHarmony = findHarmonyInSegment(segment, track, harmony->textStyleType());
    if (!nextHarmony) {
        nextHarmony = createHarmony(segment, track, harmony->harmonyType());
        score()->undoAddElement(nextHarmony);
    }

    apply();
    startEditText(nextHarmony);
    showItem(nextHarmony);
}

//! NOTE: Copied from ScoreView::figuredBassTab
void NotationInteraction::navigateToNearFiguredBass(MoveDirection direction)
{
    mu::engraving::FiguredBass* fb = mu::engraving::toFiguredBass(m_editData.element);
    mu::engraving::Segment* segm = fb->segment();
    track_idx_t track = fb->track();
    bool backDirection = direction == MoveDirection::Left;

    if (!segm) {
        LOGD("figuredBassTab: no segment");
        return;
    }

    // search next chord segment in same staff
    mu::engraving::Segment* nextSegm = backDirection ? segm->prev1(mu::engraving::SegmentType::ChordRest) : segm->next1(
        mu::engraving::SegmentType::ChordRest);
    track_idx_t minTrack = (track / mu::engraving::VOICES) * mu::engraving::VOICES;
    track_idx_t maxTrack = minTrack + (mu::engraving::VOICES - 1);

    while (nextSegm) { // look for a ChordRest in the compatible track range
        if (nextSegm->hasAnnotationOrElement(ElementType::FIGURED_BASS, minTrack, maxTrack)) {
            break;
        }
        nextSegm = backDirection ? nextSegm->prev1(mu::engraving::SegmentType::ChordRest) : nextSegm->next1(
            mu::engraving::SegmentType::ChordRest);
    }

    if (!nextSegm) {
        LOGD("figuredBassTab: no prev/next segment");
        return;
    }

    bool bNew = false;
    // add a (new) FB element, using chord duration as default duration
    mu::engraving::FiguredBass* fbNew = mu::engraving::FiguredBass::addFiguredBassToSegment(nextSegm, track, Fraction(0, 1), &bNew);
    if (bNew) {
        startEdit();
        score()->undoAddElement(fbNew);
        apply();
    }

    startEditText(fbNew);
    showItem(fbNew);
}

//! NOTE: Copied from ScoreView::figuredBassTab
void NotationInteraction::navigateToFiguredBassInNearMeasure(MoveDirection direction)
{
    mu::engraving::FiguredBass* fb = mu::engraving::toFiguredBass(m_editData.element);
    mu::engraving::Segment* segm = fb->segment();

    if (!segm) {
        LOGD("figuredBassTab: no segment");
        return;
    }

    // if moving to next/prev measure
    Measure* meas = segm->measure();
    if (meas) {
        if (direction == MoveDirection::Left) {
            meas = meas->prevMeasure();
        } else {
            meas = meas->nextMeasure();
        }
    }
    if (!meas) {
        LOGD("figuredBassTab: no prev/next measure");
        return;
    }
    // find initial ChordRest segment
    mu::engraving::Segment* nextSegm = meas->findSegment(mu::engraving::SegmentType::ChordRest, meas->tick());
    if (!nextSegm) {
        LOGD("figuredBassTab: no ChordRest segment at measure");
        return;
    }

    bool bNew = false;
    // add a (new) FB element, using chord duration as default duration
    mu::engraving::FiguredBass* fbNew = mu::engraving::FiguredBass::addFiguredBassToSegment(nextSegm, fb->track(), Fraction(0, 1), &bNew);
    if (bNew) {
        startEdit();
        score()->undoAddElement(fbNew);
        apply();
    }

    startEditText(fbNew);
    showItem(fbNew);
}

//! NOTE: Copied from ScoreView::figuredBassTicksTab
void NotationInteraction::navigateToFiguredBass(const Fraction& ticks)
{
    mu::engraving::FiguredBass* fb = mu::engraving::toFiguredBass(m_editData.element);
    track_idx_t track = fb->track();
    mu::engraving::Segment* segm = fb->segment();
    if (!segm) {
        LOGD("figuredBassTicksTab: no segment");
        return;
    }
    Measure* measure = segm->measure();

    Fraction nextSegTick   = segm->tick() + ticks;

    // find the measure containing the target tick
    while (nextSegTick >= measure->tick() + measure->ticks()) {
        measure = measure->nextMeasure();
        if (!measure) {
            LOGD("figuredBassTicksTab: no next measure");
            return;
        }
    }

    // look for a segment at this tick; if none, create one
    mu::engraving::Segment* nextSegm = segm;
    while (nextSegm && nextSegm->tick() < nextSegTick) {
        nextSegm = nextSegm->next1(mu::engraving::SegmentType::ChordRest);
    }

    bool needAddSegment = false;

    if (!nextSegm || nextSegm->tick() > nextSegTick) {      // no ChordRest segm at this tick
        nextSegm = Factory::createSegment(measure, mu::engraving::SegmentType::ChordRest, nextSegTick - measure->tick());
        if (!nextSegm) {
            LOGD("figuredBassTicksTab: no next segment");
            return;
        }
        needAddSegment = true;
    }

    startEdit();

    if (needAddSegment) {
        score()->undoAddElement(nextSegm);
    }

    bool bNew = false;
    mu::engraving::FiguredBass* fbNew = mu::engraving::FiguredBass::addFiguredBassToSegment(nextSegm, track, ticks, &bNew);
    if (bNew) {
        score()->undoAddElement(fbNew);
    }

    apply();
    startEditText(fbNew);
    showItem(fbNew);
}

//! NOTE: Copied from ScoreView::textTab
void NotationInteraction::navigateToNearText(MoveDirection direction)
{
    mu::engraving::EngravingItem* oe = m_editData.element;
    if (!oe || !oe->isTextBase()) {
        return;
    }

    mu::engraving::EngravingItem* op = dynamic_cast<mu::engraving::EngravingItem*>(oe->parent());
    if (!op || !(op->isSegment() || op->isNote())) {
        return;
    }

    TextBase* ot = mu::engraving::toTextBase(oe);
    mu::engraving::TextStyleType textStyleType = ot->textStyleType();
    ElementType type = ot->type();
    mu::engraving::staff_idx_t staffIdx = ot->staffIdx();
    bool back = direction == MoveDirection::Left;

    // get prev/next element now, as current element may be deleted if empty
    mu::engraving::EngravingItem* el = back ? score()->prevElement() : score()->nextElement();

    // find new note to add text to
    bool here = false;      // prevent infinite loop (relevant if navigation is allowed to wrap around end of score)
    while (el) {
        if (el->isNote()) {
            Note* n = mu::engraving::toNote(el);
            if (op->isNote() && n != op) {
                break;
            } else if (op->isSegment() && n->chord()->segment() != op) {
                break;
            } else if (here) {
                break;
            }
            here = true;
        } else if (el->isRest() && op->isSegment()) {
            // skip rests, but still check for infinite loop
            Rest* r = mu::engraving::toRest(el);
            if (r->segment() != op) {
            } else if (here) {
                break;
            }
            here = true;
        }
        // get prev/next note
        score()->select(el);
        mu::engraving::EngravingItem* el2 = back ? score()->prevElement() : score()->nextElement();
        // start/end of score reached
        if (el2 == el) {
            break;
        }
        el = el2;
    }

    if (!el || !el->isNote()) {
        // nothing found, exit cleanly
        if (op->selectable()) {
            select({ op });
        } else {
            clearSelection();
        }
        return;
    }

    Note* nn = mu::engraving::toNote(el);

    // go to note
    if (nn) {
        score()->select(nn, SelectType::SINGLE);
    }

    // get existing text to edit
    el = nullptr;
    if (op->isNote()) {
        // check element list of new note
        for (mu::engraving::EngravingItem* e : nn->el()) {
            if (e->type() != type) {
                continue;
            }
            TextBase* nt = mu::engraving::toTextBase(e);
            if (nt->textStyleType() == textStyleType) {
                el = e;
                break;
            }
        }
    } else if (op->isSegment()) {
        // check annotation list of new segment
        mu::engraving::Segment* ns = nn->chord()->segment();
        for (mu::engraving::EngravingItem* e : ns->annotations()) {
            if (e->staffIdx() != staffIdx || e->type() != type) {
                continue;
            }
            TextBase* nt = mu::engraving::toTextBase(e);
            if (nt->textStyleType() == textStyleType) {
                el = e;
                break;
            }
        }
    }

    if (el) {
        // edit existing text
        TextBase* text = dynamic_cast<TextBase*>(el);

        if (text) {
            startEditText(text);
        }
    } else {
        // add new text if no existing element to edit
        // TODO: for tempo text, mscore->addTempo() could be called
        // but it pre-fills the text
        // would be better to create empty tempo element
        if (type != ElementType::TEMPO_TEXT) {
            addTextToItem(textStyleType, selection()->element());
        }
    }
}

void NotationInteraction::addMelisma()
{
    if (!m_editData.element || !m_editData.element->isLyrics()) {
        LOGW("addMelisma called with invalid current element");
        return;
    }
    mu::engraving::Lyrics* lyrics = toLyrics(m_editData.element);
    track_idx_t track = lyrics->track();
    mu::engraving::Segment* segment = lyrics->segment();
    int verse = lyrics->no();
    mu::engraving::PlacementV placement = lyrics->placement();
    mu::engraving::PropertyFlags pFlags = lyrics->propertyFlags(mu::engraving::Pid::PLACEMENT);
    Fraction endTick = segment->tick(); // a previous melisma cannot extend beyond this point

    endEditText();

    // search next chord
    mu::engraving::Segment* nextSegment = segment;
    while ((nextSegment = nextSegment->next1(mu::engraving::SegmentType::ChordRest))) {
        EngravingItem* el = nextSegment->element(track);
        if (el && el->isChord()) {
            break;
        }
    }

    // look for the lyrics we are moving from; may be the current lyrics or a previous one
    // we are extending with several underscores
    mu::engraving::Lyrics* fromLyrics = 0;
    while (segment) {
        ChordRest* cr = toChordRest(segment->element(track));
        if (cr) {
            fromLyrics = cr->lyrics(verse, placement);
            if (fromLyrics) {
                break;
            }
        }
        segment = segment->prev1(mu::engraving::SegmentType::ChordRest);
        // if the segment has a rest in this track, stop going back
        EngravingItem* e = segment ? segment->element(track) : 0;
        if (e && !e->isChord()) {
            break;
        }
    }

    // one-chord melisma?
    // if still at melisma initial chord and there is a valid next chord (if not,
    // there will be no melisma anyway), set a temporary melisma duration
    if (fromLyrics == lyrics && nextSegment) {
        score()->startCmd();
        lyrics->undoChangeProperty(mu::engraving::Pid::LYRIC_TICKS, Fraction::fromTicks(mu::engraving::Lyrics::TEMP_MELISMA_TICKS));
        score()->setLayoutAll();
        score()->endCmd();
    }

    if (nextSegment == 0) {
        score()->startCmd();
        if (fromLyrics) {
            switch (fromLyrics->syllabic()) {
            case mu::engraving::Lyrics::Syllabic::SINGLE:
            case mu::engraving::Lyrics::Syllabic::END:
                break;
            default:
                fromLyrics->undoChangeProperty(mu::engraving::Pid::SYLLABIC, int(mu::engraving::Lyrics::Syllabic::END));
                break;
            }
            if (fromLyrics->segment()->tick() < endTick) {
                fromLyrics->undoChangeProperty(mu::engraving::Pid::LYRIC_TICKS, endTick - fromLyrics->segment()->tick());
            }
        }

        if (fromLyrics) {
            score()->select(fromLyrics, SelectType::SINGLE, 0);
        }
        score()->setLayoutAll();
        score()->endCmd();
        return;
    }

    // if a place for a new lyrics has been found, create a lyrics there

    score()->startCmd();
    ChordRest* cr = toChordRest(nextSegment->element(track));
    mu::engraving::Lyrics* toLyrics = cr->lyrics(verse, placement);
    bool newLyrics = (toLyrics == 0);
    if (!toLyrics) {
        toLyrics = Factory::createLyrics(cr);
        toLyrics->setTrack(track);
        toLyrics->setParent(cr);
        toLyrics->setNo(verse);
        toLyrics->setPlacement(placement);
        toLyrics->setPropertyFlags(mu::engraving::Pid::PLACEMENT, pFlags);
        toLyrics->setSyllabic(mu::engraving::Lyrics::Syllabic::SINGLE);
    }
    // as we arrived at toLyrics by an underscore, it cannot have syllabic dashes before
    else if (toLyrics->syllabic() == mu::engraving::Lyrics::Syllabic::MIDDLE) {
        toLyrics->undoChangeProperty(mu::engraving::Pid::SYLLABIC, int(mu::engraving::Lyrics::Syllabic::BEGIN));
    } else if (toLyrics->syllabic() == mu::engraving::Lyrics::Syllabic::END) {
        toLyrics->undoChangeProperty(mu::engraving::Pid::SYLLABIC, int(mu::engraving::Lyrics::Syllabic::SINGLE));
    }
    if (fromLyrics) {
        // as we moved away from fromLyrics by an underscore,
        // it can be isolated or terminal but cannot have dashes after
        switch (fromLyrics->syllabic()) {
        case mu::engraving::Lyrics::Syllabic::SINGLE:
        case mu::engraving::Lyrics::Syllabic::END:
            break;
        default:
            fromLyrics->undoChangeProperty(mu::engraving::Pid::SYLLABIC, int(mu::engraving::Lyrics::Syllabic::END));
            break;
        }
        // for the same reason, if it has a melisma, this cannot extend beyond toLyrics
        if (fromLyrics->segment()->tick() < endTick) {
            fromLyrics->undoChangeProperty(mu::engraving::Pid::LYRIC_TICKS, endTick - fromLyrics->segment()->tick());
        }
    }
    if (newLyrics) {
        score()->undoAddElement(toLyrics);
    }
    score()->endCmd();

    score()->select(toLyrics, SelectType::SINGLE, 0);
    startEditText(toLyrics, PointF());

    toLyrics->selectAll(toLyrics->cursor());
}

void NotationInteraction::addLyricsVerse()
{
    if (!m_editData.element || !m_editData.element->isLyrics()) {
        LOGW("nextLyricVerse called with invalid current element");
        return;
    }
    mu::engraving::Lyrics* lyrics = toLyrics(m_editData.element);

    endEditText();

    score()->startCmd();
    int newVerse;
    newVerse = lyrics->no() + 1;

    mu::engraving::Lyrics* oldLyrics = lyrics;
    lyrics = Factory::createLyrics(oldLyrics->chordRest());
    lyrics->setTrack(oldLyrics->track());
    lyrics->setParent(oldLyrics->chordRest());
    lyrics->setPlacement(oldLyrics->placement());
    lyrics->setPropertyFlags(mu::engraving::Pid::PLACEMENT, oldLyrics->propertyFlags(mu::engraving::Pid::PLACEMENT));
    lyrics->setNo(newVerse);

    score()->undoAddElement(lyrics);
    score()->endCmd();

    score()->select(lyrics, SelectType::SINGLE, 0);
    startEditText(lyrics, PointF());
}

mu::engraving::Harmony* NotationInteraction::editedHarmony() const
{
    Harmony* harmony = static_cast<Harmony*>(m_editData.element);
    if (!harmony) {
        return nullptr;
    }

    if (!harmony->parent() || !harmony->parent()->isSegment()) {
        LOGD("no segment parent");
        return nullptr;
    }

    return harmony;
}

mu::engraving::Harmony* NotationInteraction::findHarmonyInSegment(const mu::engraving::Segment* segment, track_idx_t track,
                                                                  mu::engraving::TextStyleType textStyleType) const
{
    for (mu::engraving::EngravingItem* e : segment->annotations()) {
        if (e->isHarmony() && e->track() == track && toHarmony(e)->textStyleType() == textStyleType) {
            return toHarmony(e);
        }
    }

    return nullptr;
}

mu::engraving::Harmony* NotationInteraction::createHarmony(mu::engraving::Segment* segment, track_idx_t track,
                                                           mu::engraving::HarmonyType type) const
{
    mu::engraving::Harmony* harmony = Factory::createHarmony(score()->dummy()->segment());
    harmony->setScore(score());
    harmony->setParent(segment);
    harmony->setTrack(track);
    harmony->setHarmonyType(type);

    return harmony;
}

void NotationInteraction::startEditText(mu::engraving::TextBase* text)
{
    doEndEditElement();
    select({ text }, SelectType::SINGLE);
    startEditText(text, PointF());
    text->cursor()->moveCursorToEnd();
}

bool NotationInteraction::needEndTextEdit() const
{
    if (isTextEditingStarted()) {
        const mu::engraving::TextBase* text = mu::engraving::toTextBase(m_editData.element);
        return !text || !text->cursor()->editing();
    }

    return false;
}

void NotationInteraction::toggleFontStyle(mu::engraving::FontStyle style)
{
    if (!m_editData.element || !m_editData.element->isTextBase()) {
        LOGW("toggleFontStyle called with invalid current element");
        return;
    }
    mu::engraving::TextBase* text = toTextBase(m_editData.element);
    int currentStyle = text->getProperty(mu::engraving::Pid::FONT_STYLE).toInt();
    score()->startCmd();
    text->undoChangeProperty(mu::engraving::Pid::FONT_STYLE, PropertyValue::fromValue(
                                 currentStyle ^ static_cast<int>(style)), mu::engraving::PropertyFlags::UNSTYLED);
    score()->endCmd();
    notifyAboutTextEditingChanged();
}

void NotationInteraction::toggleBold()
{
    toggleFontStyle(mu::engraving::FontStyle::Bold);
}

void NotationInteraction::toggleItalic()
{
    toggleFontStyle(mu::engraving::FontStyle::Italic);
}

void NotationInteraction::toggleUnderline()
{
    toggleFontStyle(mu::engraving::FontStyle::Underline);
}

void NotationInteraction::toggleStrike()
{
    toggleFontStyle(mu::engraving::FontStyle::Strike);
}

template<typename P>
void NotationInteraction::execute(void (mu::engraving::Score::* function)(P), P param)
{
    startEdit();
    (score()->*function)(param);
    apply();
}

void NotationInteraction::toggleArticulation(mu::engraving::SymId symId)
{
    execute(&mu::engraving::Score::toggleArticulation, symId);
}

void NotationInteraction::toggleAutoplace(bool all)
{
    execute(&mu::engraving::Score::cmdToggleAutoplace, all);
}

void NotationInteraction::insertClef(mu::engraving::ClefType clef)
{
    execute(&mu::engraving::Score::cmdInsertClef, clef);
}

void NotationInteraction::changeAccidental(mu::engraving::AccidentalType accidental)
{
    execute(&mu::engraving::Score::changeAccidental, accidental);
}

void NotationInteraction::transposeSemitone(int steps)
{
    execute(&mu::engraving::Score::transposeSemitone, steps);
}

void NotationInteraction::transposeDiatonicAlterations(mu::engraving::TransposeDirection direction)
{
    execute(&mu::engraving::Score::transposeDiatonicAlterations, direction);
}

void NotationInteraction::toggleGlobalOrLocalInsert()
{
    score()->inputState().setInsertMode(!score()->inputState().insertMode());
}

void NotationInteraction::getLocation()
{
    auto* e = score()->selection().element();
    if (!e) {
        // no current selection - restore lost selection
        e = score()->selection().currentCR();
        if (e && e->isChord()) {
            e = toChord(e)->upNote();
        }
    }
    if (!e) {
        e = score()->firstElement(false);
    }
    if (e) {
        if (e->type() == ElementType::NOTE || e->type() == ElementType::HARMONY) {
            score()->setPlayNote(true);
        }
        select({ e }, SelectType::SINGLE);
    }
}

void NotationInteraction::execute(void (mu::engraving::Score::* function)())
{
    startEdit();
    (score()->*function)();
    apply();
}

//! NOTE: Copied from ScoreView::adjustCanvasPosition
void NotationInteraction::showItem(const mu::engraving::EngravingItem* el, int staffIndex)
{
    if (!el) {
        return;
    }

    if (!configuration()->isAutomaticallyPanEnabled()) {
        return;
    }

    const mu::engraving::MeasureBase* m = nullptr;

    if (el->type() == ElementType::NOTE) {
        m = static_cast<const Note*>(el)->chord()->measure();
    } else if (el->type() == ElementType::REST) {
        m = static_cast<const Rest*>(el)->measure();
    } else if (el->type() == ElementType::CHORD) {
        m = static_cast<const Chord*>(el)->measure();
    } else if (el->type() == ElementType::SEGMENT) {
        m = static_cast<const mu::engraving::Segment*>(el)->measure();
    } else if (el->type() == ElementType::LYRICS) {
        m = static_cast<const mu::engraving::Lyrics*>(el)->measure();
    } else if ((el->type() == ElementType::HARMONY || el->type() == ElementType::FIGURED_BASS)
               && el->parent()->type() == ElementType::SEGMENT) {
        m = static_cast<const mu::engraving::Segment*>(el->parent())->measure();
    } else if (el->type() == ElementType::HARMONY && el->parent()->type() == ElementType::FRET_DIAGRAM
               && el->parent()->parent()->type() == ElementType::SEGMENT) {
        m = static_cast<const mu::engraving::Segment*>(el->parent()->parent())->measure();
    } else if (el->isMeasureBase()) {
        m = static_cast<const mu::engraving::MeasureBase*>(el);
    } else if (el->isSpannerSegment()) {
        EngravingItem* se = static_cast<const mu::engraving::SpannerSegment*>(el)->spanner()->startElement();
        m = static_cast<Measure*>(se->findMeasure());
    } else if (el->isSpanner()) {
        EngravingItem* se = static_cast<const mu::engraving::Spanner*>(el)->startElement();
        m = static_cast<Measure*>(se->findMeasure());
    } else {
        // attempt to find measure
        mu::engraving::EngravingObject* e = el->parent();
        while (e && !e->isMeasureBase()) {
            e = e->parent();
        }
        if (e) {
            m = toMeasureBase(e);
        } else {
            return;
        }
    }
    if (!m) {
        return;
    }

    mu::engraving::System* sys = m->system();
    if (!sys) {
        return;
    }

    RectF mRect(m->canvasBoundingRect());
    RectF sysRect = mRect;

    double _spatium    = score()->spatium();
    const qreal border = _spatium * 3;
    RectF showRect;
    if (staffIndex == -1) {
        showRect = RectF(mRect.x(), sysRect.y(), mRect.width(), sysRect.height())
                   .adjusted(-border, -border, border, border);
    } else {
        // find a box for the individual stave in a system
        RectF stave = RectF(sys->canvasBoundingRect().left(),
                            sys->staffCanvasYpage(staffIndex),
                            sys->width(),
                            sys->staff(staffIndex)->bbox().height());
        showRect = mRect.intersected(stave).adjusted(-border, -border, border, border);
    }

    ShowItemRequest request;
    request.item = el;
    request.showRect = showRect;

    m_showItemRequested.send(request);
}

mu::async::Channel<NotationInteraction::ShowItemRequest> NotationInteraction::showItemRequested() const
{
    return m_showItemRequested;
}

void NotationInteraction::setGetViewRectFunc(const std::function<RectF()>& func)
{
    static_cast<NotationNoteInput*>(m_noteInput.get())->setGetViewRectFunc(func);
}

namespace mu::notation {
EngravingItem* contextItem(INotationInteractionPtr interaction)
{
    EngravingItem* item = interaction->selection()->element();
    if (item == nullptr) {
        return interaction->hitElementContext().element;
    }
    return item;
}
}
