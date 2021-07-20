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

#include "ptrutils.h"

#include "libmscore/score.h"
#include "libmscore/page.h"
#include "libmscore/shadownote.h"
#include "libmscore/staff.h"
#include "libmscore/measure.h"
#include "libmscore/part.h"
#include "libmscore/drumset.h"
#include "libmscore/rest.h"
#include "libmscore/slur.h"
#include "libmscore/system.h"
#include "libmscore/chord.h"
#include "libmscore/elementgroup.h"
#include "libmscore/textframe.h"
#include "libmscore/figuredbass.h"
#include "libmscore/stafflines.h"
#include "libmscore/actionicon.h"
#include "libmscore/undo.h"
#include "libmscore/navigate.h"
#include "libmscore/keysig.h"
#include "libmscore/instrchange.h"
#include "libmscore/lasso.h"
#include "libmscore/textedit.h"
#include "libmscore/lyrics.h"

#include "masternotation.h"
#include "scorecallbacks.h"
#include "notationnoteinput.h"
#include "notationselection.h"

#include "instrumentsconverter.h"

#include "draw/pen.h"
#include "draw/painterpath.h"

using namespace mu::notation;

NotationInteraction::NotationInteraction(Notation* notation, INotationUndoStackPtr undoStack)
    : m_notation(notation), m_undoStack(undoStack), m_gripEditData(&m_scoreCallbacks),
    m_textEditData(&m_scoreCallbacks), m_lasso(new Ms::Lasso(notation->score()))
{
    m_noteInput = std::make_shared<NotationNoteInput>(notation, this, m_undoStack);
    m_selection = std::make_shared<NotationSelection>(notation);

    m_noteInput->stateChanged().onNotify(this, [this]() {
        if (!m_noteInput->isNoteInputMode()) {
            hideShadowNote();
        }
    });

    m_dragData.ed = Ms::EditData(&m_scoreCallbacks);
    m_dropData.ed = Ms::EditData(&m_scoreCallbacks);
    m_scoreCallbacks.setScore(notation->score());
}

NotationInteraction::~NotationInteraction()
{
    delete m_shadowNote;
}

void NotationInteraction::init()
{
    m_shadowNote = new Ms::ShadowNote(score());
    m_shadowNote->setVisible(false);
}

Ms::Score* NotationInteraction::score() const
{
    return m_notation->score();
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
        notifyAboutSelectionChanged();
    }
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

void NotationInteraction::notifyAboutSelectionChanged()
{
    m_selectionChanged.notify();
}

void NotationInteraction::paint(mu::draw::Painter* painter)
{
    m_shadowNote->draw(painter);

    drawAnchorLines(painter);
    drawTextEditMode(painter);
    drawSelectionRange(painter);
    drawGripPoints(painter);
    if (!m_lasso->bbox().isEmpty()) {
        m_lasso->draw(painter);
    }
}

INotationNoteInputPtr NotationInteraction::noteInput() const
{
    return m_noteInput;
}

void NotationInteraction::showShadowNote(const PointF& pos)
{
    const Ms::InputState& inputState = score()->inputState();
    Ms::Position position;
    if (!score()->getPosition(&position, pos, inputState.voice())) {
        m_shadowNote->setVisible(false);
        return;
    }

    Staff* staff = score()->staff(position.staffIdx);
    const Ms::Instrument* instr = staff->part()->instrument();

    Ms::Segment* segment = position.segment;
    qreal segmentSkylineTopY = 0;
    qreal segmentSkylineBottomY = 0;

    Ms::Segment* shadowNoteActualSegment = position.segment->prev1enabled();
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
    position.pos.rx() -= qMin(relX - score()->styleP(Ms::Sid::barNoteDistance) * mag, 0.0);

    Ms::NoteHead::Group noteheadGroup = Ms::NoteHead::Group::HEAD_NORMAL;
    Ms::NoteHead::Type noteHead = inputState.duration().headType();
    int line = position.line;

    if (instr->useDrumset()) {
        const Ms::Drumset* ds  = instr->drumset();
        int pitch = inputState.drumNote();
        if (pitch >= 0 && ds->isValid(pitch)) {
            line = ds->line(pitch);
            noteheadGroup = ds->noteHead(pitch);
        }
    }

    int voice = 0;
    if (inputState.drumNote() != -1 && inputState.drumset() && inputState.drumset()->isValid(inputState.drumNote())) {
        voice = inputState.drumset()->voice(inputState.drumNote());
    } else {
        voice = inputState.voice();
    }

    m_shadowNote->setVisible(true);
    m_shadowNote->setMag(mag);
    m_shadowNote->setTick(tick);
    m_shadowNote->setStaffIdx(position.staffIdx);
    m_shadowNote->setVoice(voice);
    m_shadowNote->setLineIndex(line);

    Ms::SymId symNotehead;
    Ms::TDuration duration(inputState.duration());

    if (inputState.rest()) {
        int yo = 0;
        Ms::Rest rest(Ms::gscore, duration.type());
        rest.setTicks(duration.fraction());
        symNotehead = rest.getSymbol(inputState.duration().type(), 0, staff->lines(position.segment->tick()), &yo);
        m_shadowNote->setState(symNotehead, duration, true, segmentSkylineTopY, segmentSkylineBottomY);
    } else {
        if (Ms::NoteHead::Group::HEAD_CUSTOM == noteheadGroup) {
            symNotehead = instr->drumset()->noteHeads(inputState.drumNote(), noteHead);
        } else {
            symNotehead = Note::noteHead(0, noteheadGroup, noteHead);
        }

        m_shadowNote->setState(symNotehead, duration, false, segmentSkylineTopY, segmentSkylineBottomY);
    }

    m_shadowNote->layout();
    m_shadowNote->setPos(position.pos);
}

void NotationInteraction::hideShadowNote()
{
    m_shadowNote->setVisible(false);
}

void NotationInteraction::toggleVisible()
{
    startEdit();

    // TODO: Update `score()->cmdToggleVisible()` and call that here?
    for (Element* el : selection()->elements()) {
        if (el->isBracket()) {
            continue;
        }
        el->undoChangeProperty(Ms::Pid::VISIBLE, !el->visible());
    }

    apply();

    notifyAboutNotationChanged();
}

Element* NotationInteraction::hitElement(const PointF& pos, float width) const
{
    QList<Ms::Element*> elements = hitElements(pos, width);
    if (elements.isEmpty()) {
        return nullptr;
    }

    return elements.first();
}

Staff* NotationInteraction::hitStaff(const PointF& pos) const
{
    return hitMeasure(pos).staff;
}

Ms::Page* NotationInteraction::point2page(const PointF& p) const
{
    if (score()->layoutMode() == Ms::LayoutMode::LINE) {
        return score()->pages().isEmpty() ? 0 : score()->pages().front();
    }
    foreach (Ms::Page* page, score()->pages()) {
        if (page->bbox().translated(page->pos()).contains(p)) {
            return page;
        }
    }
    return nullptr;
}

QList<Element*> NotationInteraction::elementsAt(const PointF& p) const
{
    QList<Element*> el;
    Ms::Page* page = point2page(p);
    if (page) {
        el = page->items(p - page->pos());
        std::sort(el.begin(), el.end(), NotationInteraction::elementIsLess);
    }
    return el;
}

Element* NotationInteraction::elementAt(const PointF& p) const
{
    QList<Element*> el = elementsAt(p);
    Element* e = el.value(0);
    if (e && e->isPage()) {
        e = el.value(1);
    }
    return e;
}

QList<Ms::Element*> NotationInteraction::hitElements(const PointF& p_in, float w) const
{
    Ms::Page* page = point2page(p_in);
    if (!page) {
        return QList<Ms::Element*>();
    }

    QList<Ms::Element*> ll;

    PointF p = p_in - page->pos();

    RectF r(p.x() - w, p.y() - w, 3.0 * w, 3.0 * w);

    QList<Ms::Element*> elements = page->items(r);
    //! TODO
    //    for (int i = 0; i < MAX_HEADERS; i++)
    //        if (score()->headerText(i) != nullptr)      // gives the ability to select the header
    //            el.push_back(score()->headerText(i));
    //    for (int i = 0; i < MAX_FOOTERS; i++)
    //        if (score()->footerText(i) != nullptr)      // gives the ability to select the footer
    //            el.push_back(score()->footerText(i));
    //! -------

    for (Ms::Element* element : elements) {
        element->itemDiscovered = 0;
        if (!element->selectable() || element->isPage()) {
            continue;
        }

        if (!element->isInteractionAvailable()) {
            continue;
        }

        if (element->contains(p)) {
            ll.append(element);
        }
    }

    int n = ll.size();
    if ((n == 0) || ((n == 1) && (ll[0]->isMeasure()))) {
        //
        // if no relevant element hit, look nearby
        //
        for (Ms::Element* element : elements) {
            if (element->isPage() || !element->selectable()) {
                continue;
            }

            if (!element->isInteractionAvailable()) {
                continue;
            }

            if (element->intersects(r)) {
                ll.append(element);
            }
        }
    }

    if (!ll.empty()) {
        std::sort(ll.begin(), ll.end(), NotationInteraction::elementIsLess);
    } else {
        Ms::Measure* measure = hitMeasure(p_in).measure;
        if (measure) {
            ll << measure;
        }
    }

    return ll;
}

NotationInteraction::HitMeasureData NotationInteraction::hitMeasure(const PointF& pos) const
{
    int staffIndex = -1;
    Ms::Segment* segment = nullptr;
    PointF offset;
    Measure* measure = score()->pos2measure(pos, &staffIndex, 0, &segment, &offset);

    HitMeasureData result;
    if (measure && measure->staffLines(staffIndex)->canvasBoundingRect().contains(pos)) {
        result.measure = measure;
        result.staff = score()->staff(staffIndex);
    }

    return result;
}

bool NotationInteraction::elementIsLess(const Ms::Element* e1, const Ms::Element* e2)
{
    if (!e1->selectable()) {
        return false;
    }
    if (!e2->selectable()) {
        return true;
    }
    if (e1->isNote() && e2->isStem()) {
        return true;
    }
    if (e2->isNote() && e1->isStem()) {
        return false;
    }
    if (e1->z() == e2->z()) {
        // same stacking order, prefer non-hidden elements
        if (e1->type() == e2->type()) {
            if (e1->type() == Ms::ElementType::NOTEDOT) {
                const Ms::NoteDot* n1 = static_cast<const Ms::NoteDot*>(e1);
                const Ms::NoteDot* n2 = static_cast<const Ms::NoteDot*>(e2);
                if (n1->note() && n1->note()->hidden()) {
                    return false;
                } else if (n2->note() && n2->note()->hidden()) {
                    return true;
                }
            } else if (e1->type() == Ms::ElementType::NOTE) {
                const Ms::Note* n1 = static_cast<const Ms::Note*>(e1);
                const Ms::Note* n2 = static_cast<const Ms::Note*>(e2);
                if (n1->hidden()) {
                    return false;
                } else if (n2->hidden()) {
                    return true;
                }
            }
        }
        // different types, or same type but nothing hidden - use track
        return e1->track() <= e2->track();
    }

    // default case, use stacking order
    return e1->z() <= e2->z();
}

const NotationInteraction::HitElementContext& NotationInteraction::hitElementContext() const
{
    return m_hitElementContext;
}

void NotationInteraction::setHitElementContext(const HitElementContext& context)
{
    m_hitElementContext = context;
}

void NotationInteraction::addChordToSelection(MoveDirection d)
{
    IF_ASSERT_FAILED(MoveDirection::Left == d || MoveDirection::Right == d) {
        return;
    }

    QString cmd;
    if (MoveDirection::Left == d) {
        cmd = "select-prev-chord";
    } else if (MoveDirection::Right == d) {
        cmd = "select-next-chord";
    }

    score()->selectMove(cmd);
    notifyAboutSelectionChanged();
}

void NotationInteraction::moveChordNoteSelection(MoveDirection d)
{
    IF_ASSERT_FAILED(MoveDirection::Up == d || MoveDirection::Down == d) {
        return;
    }

    Element* current = selection()->element();
    if (!current || !(current->isNote() || current->isRest())) {
        return;
    }

    Element* chordElem;
    if (d == MoveDirection::Up) {
        chordElem = score()->upAlt(current);
    } else {
        chordElem = score()->downAlt(current);
    }

    if (chordElem == current) {
        return;
    }

    score()->select(chordElem, SelectType::SINGLE, chordElem->staffIdx());

    notifyAboutSelectionChanged();
}

void NotationInteraction::doSelect(const std::vector<Element*>& elements, SelectType type, int staffIndex)
{
    if (needEndTextEditing(elements)) {
        endEditText();
    }

    updateGripEdit(elements);

    for (Element* element: elements) {
        score()->select(element, type, staffIndex);
    }
}

void NotationInteraction::select(const std::vector<Element*>& elements, SelectType type, int staffIndex)
{
    doSelect(elements, type, staffIndex);
    notifyAboutSelectionChanged();
}

void NotationInteraction::selectAll()
{
    if (isTextEditingStarted()) {
        auto textBase = toTextBase(m_textEditData.element);
        textBase->selectAll(textBase->cursorFromEditData(m_textEditData));
    } else {
        score()->cmdSelectAll();
    }

    notifyAboutSelectionChanged();
}

void NotationInteraction::selectSection()
{
    score()->cmdSelectSection();

    notifyAboutSelectionChanged();
}

void NotationInteraction::selectFirstElement()
{
    Element* element = score()->firstElement();
    score()->select(element, SelectType::SINGLE, element->staffIdx());

    notifyAboutSelectionChanged();
}

void NotationInteraction::selectLastElement()
{
    Element* element = score()->lastElement();
    score()->select(element, SelectType::SINGLE, element->staffIdx());

    notifyAboutSelectionChanged();
}

INotationSelectionPtr NotationInteraction::selection() const
{
    return m_selection;
}

void NotationInteraction::clearSelection()
{
    if (selection()->isNone()) {
        return;
    }

    score()->deselectAll();

    notifyAboutSelectionChanged();

    setHitElementContext(HitElementContext());
}

mu::async::Notification NotationInteraction::selectionChanged() const
{
    return m_selectionChanged;
}

bool NotationInteraction::isDragStarted() const
{
    return m_dragData.dragGroups.size() > 0 || !m_lasso->bbox().isEmpty();
}

void NotationInteraction::DragData::reset()
{
    beginMove = QPointF();
    elementOffset = QPointF();
    ed = Ms::EditData(ed.view());
    dragGroups.clear();
}

void NotationInteraction::startDrag(const std::vector<Element*>& elems,
                                    const PointF& eoffset,
                                    const IsDraggable& isDraggable)
{
    m_dragData.reset();
    m_dragData.elements = elems;
    m_dragData.elementOffset = eoffset;

    for (Element* e : m_dragData.elements) {
        if (!isDraggable(e)) {
            continue;
        }

        std::unique_ptr<Ms::ElementGroup> g = e->getDragGroup(isDraggable);
        if (g && g->enabled()) {
            m_dragData.dragGroups.push_back(std::move(g));
        }
    }

    startEdit();

    if (isGripEditStarted()) {
        m_gripEditData.element->startEditDrag(m_gripEditData);
        return;
    }

    for (auto& group : m_dragData.dragGroups) {
        group->startDrag(m_dragData.ed);
    }
}

void NotationInteraction::doDragLasso(const PointF& pt)
{
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
    m_dragData.ed.hRaster = false;    //mscore->hRaster();
    m_dragData.ed.vRaster = false;    //mscore->vRaster();
    m_dragData.ed.delta   = delta;
    m_dragData.ed.moveDelta = delta - m_dragData.elementOffset;
    m_dragData.ed.evtDelta = evtDelta;
    m_dragData.ed.pos     = toPos;

    if (isTextEditingStarted()) {
        m_textEditData.pos = toPos;
        toTextBase(m_textEditData.element)->dragTo(m_textEditData);

        notifyAboutTextEditingChanged();
        return;
    }

    if (isGripEditStarted()) {
        m_dragData.ed.curGrip = m_gripEditData.curGrip;
        m_dragData.ed.delta = m_dragData.ed.pos - m_dragData.ed.lastPos;
        m_dragData.ed.moveDelta = m_dragData.ed.delta - m_dragData.elementOffset;
        m_dragData.ed.addData(m_gripEditData.getData(m_gripEditData.element));
        m_gripEditData.element->editDrag(m_dragData.ed);
    } else {
        for (auto& group : m_dragData.dragGroups) {
            score()->addRefresh(group->drag(m_dragData.ed));
        }
    }

    score()->update();

    std::vector<LineF> anchorLines;
    for (const Element* e : m_dragData.elements) {
        QVector<LineF> elAnchorLines = e->dragAnchorLines();
        const Ms::Element* page = e->findAncestor(ElementType::PAGE);
        const PointF pageOffset((page ? page : e)->pos());

        if (!elAnchorLines.isEmpty()) {
            for (LineF& l : elAnchorLines) {
                l.translate(pageOffset);
                anchorLines.push_back(l);
            }
        }
    }

    setAnchorLines(anchorLines);

    if (m_dragData.elements.size() == 0) {
        doDragLasso(toPos);
    }

    notifyAboutDragChanged();

    //    Element* e = score()->getSelectedElement();
    //    if (e) {
    //        if (score()->playNote()) {
    //            mscore->play(e);
    //            score()->setPlayNote(false);
    //        }
    //    }
    //    updateGrips();
    //    score()->update();
}

void NotationInteraction::endDrag()
{
    if (isGripEditStarted()) {
        m_gripEditData.element->endEditDrag(m_gripEditData);
    } else {
        for (auto& group : m_dragData.dragGroups) {
            group->endDrag(m_dragData.ed);
        }
        if (!m_lasso->bbox().isEmpty()) {
            endLasso();
        }
    }

    m_dragData.reset();
    resetAnchorLines();
    apply();
    notifyAboutDragChanged();
    //    updateGrips();
    //    if (editData.element->normalModeEditBehavior() == Element::EditBehavior::Edit
    //        && score()->selection().element() == editData.element) {
    //        startEdit(/* editMode */ false);
    //    }
}

mu::async::Notification NotationInteraction::dragChanged() const
{
    return m_dragChanged;
}

//! NOTE Copied from ScoreView::dragEnterEvent
void NotationInteraction::startDrop(const QByteArray& edata)
{
    if (m_dropData.ed.dropElement) {
        delete m_dropData.ed.dropElement;
        m_dropData.ed.dropElement = nullptr;
    }

    Ms::XmlReader e(edata);
    m_dropData.ed.dragOffset = QPointF();
    Fraction duration;      // dummy
    ElementType type = Element::readType(e, &m_dropData.ed.dragOffset, &duration);

    Element* el = Element::create(type, score());
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

//! NOTE Copied from ScoreView::dragMoveEvent
bool NotationInteraction::isDropAccepted(const PointF& pos, Qt::KeyboardModifiers modifiers)
{
    if (!m_dropData.ed.dropElement) {
        return false;
    }

    m_dropData.ed.pos = pos;
    m_dropData.ed.modifiers = modifiers;

    switch (m_dropData.ed.dropElement->type()) {
    case ElementType::VOLTA:
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
        Element* e = dropTarget(m_dropData.ed);
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
    m_dropData.ed.modifiers = modifiers;

    bool firstStaffOnly = false;
    bool applyUserOffset = false;
    //bool triggerSpannerDropApplyTour = m_dropData.ed.dropElement->isSpanner();
    m_dropData.ed.dropElement->styleChanged();
    startEdit();
    score()->addRefresh(m_dropData.ed.dropElement->canvasBoundingRect());
    switch (m_dropData.ed.dropElement->type()) {
    case ElementType::TEXTLINE:
        firstStaffOnly = m_dropData.ed.dropElement->systemFlag();
    // fall-thru
    case ElementType::VOLTA:
        // voltas drop to first staff by default, or closest staff if Control is held
        firstStaffOnly = firstStaffOnly || !(m_dropData.ed.modifiers & Qt::ControlModifier);
    // fall-thru
    case ElementType::OTTAVA:
    case ElementType::TRILL:
    case ElementType::PEDAL:
    case ElementType::LET_RING:
    case ElementType::VIBRATO:
    case ElementType::PALM_MUTE:
    case ElementType::HAIRPIN:
    {
        Ms::Spanner* spanner = ptr::checked_cast<Ms::Spanner>(m_dropData.ed.dropElement);
        score()->cmdAddSpanner(spanner, pos, firstStaffOnly);
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
        Element* el = elementAt(pos);
        if (el == 0 || el->type() == ElementType::STAFF_LINES) {
            int staffIdx;
            Ms::Segment* seg;
            PointF offset;
            el = score()->pos2measure(pos, &staffIdx, 0, &seg, &offset);
            if (el && el->isMeasure()) {
                m_dropData.ed.dropElement->setTrack(staffIdx * VOICES);
                if (m_dropData.ed.dropElement->isImage()) {
                    m_dropData.ed.dropElement->setParent(el);
                    offset = pos - el->canvasPos();
                } else {
                    m_dropData.ed.dropElement->setParent(seg);
                }
                if (applyUserOffset) {
                    m_dropData.ed.dropElement->setOffset(offset);
                }
                score()->undoAddElement(m_dropData.ed.dropElement);
            } else {
                qDebug("cannot drop here");
                delete m_dropData.ed.dropElement;
                m_dropData.ed.dropElement = nullptr;
            }
        } else {
            score()->addRefresh(el->canvasBoundingRect());
            score()->addRefresh(m_dropData.ed.dropElement->canvasBoundingRect());

            if (!el->acceptDrop(m_dropData.ed)) {
                qDebug("drop %s onto %s not accepted", m_dropData.ed.dropElement->name(), el->name());
                break;
            }
            Element* dropElement = el->drop(m_dropData.ed);
            score()->addRefresh(el->canvasBoundingRect());
            if (dropElement) {
                score()->select(dropElement, SelectType::SINGLE, 0);
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
    case ElementType::SLUR:
    case ElementType::BAGPIPE_EMBELLISHMENT:
    case ElementType::AMBITUS:
    case ElementType::TREMOLOBAR:
    case ElementType::FIGURED_BASS:
    case ElementType::LYRICS:
    case ElementType::STAFFTYPE_CHANGE: {
        Element* el = dropTarget(m_dropData.ed);
        if (!el) {
            if (!dropCanvas(m_dropData.ed.dropElement)) {
                qDebug("cannot drop %s(%p) to canvas", m_dropData.ed.dropElement->name(), m_dropData.ed.dropElement);
                delete m_dropData.ed.dropElement;
                m_dropData.ed.dropElement = nullptr;
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

        Element* dropElement = el->drop(m_dropData.ed);
        if (dropElement && dropElement->isInstrumentChange()) {
            selectInstrument(toInstrumentChange(dropElement));
        }
        score()->addRefresh(el->canvasBoundingRect());
        if (dropElement) {
            if (!score()->noteEntryMode()) {
                score()->select(dropElement, SelectType::SINGLE, 0);
            }
            score()->addRefresh(dropElement->canvasBoundingRect());
        }
        accepted = true;
    }
    break;
    default:
        delete m_dropData.ed.dropElement;
        break;
    }
    m_dropData.ed.dropElement = nullptr;
    setDropTarget(nullptr);         // this also resets dropRectangle and dropAnchor
    apply();
    // update input cursor position (must be done after layout)
//    if (noteEntryMode()) {
//        moveCursor();
//    }
//    if (triggerSpannerDropApplyTour) {
//        TourHandler::startTour("spanner-drop-apply");
//    }
    if (accepted) {
        notifyAboutDropChanged();
    }
    return accepted;
}

void NotationInteraction::selectInstrument(Ms::InstrumentChange* instrumentChange)
{
    if (!instrumentChange) {
        return;
    }

    RetVal<Val> retVal = interactive()->open("musescore://instruments/select?canSelectMultipleInstruments=false");
    if (!retVal.ret) {
        return;
    }

    Instrument selectedIstrument = retVal.val.toQVariant().value<Instrument>();
    if (!selectedIstrument.isValid()) {
        return;
    }

    Ms::Instrument instrument = InstrumentsConverter::convertInstrument(selectedIstrument);

    instrumentChange->setInit(true);
    instrumentChange->setupInstrument(&instrument);
}

//! NOTE Copied from Palette::applyPaletteElement
bool NotationInteraction::applyPaletteElement(Ms::Element* element, Qt::KeyboardModifiers modifiers)
{
    IF_ASSERT_FAILED(element) {
        return false;
    }

    Ms::Score* score = this->score();

    if (!score) {
        return false;
    }

    const Ms::Selection sel = score->selection();   // make a copy of selection state before applying the operation.
    if (sel.isNone()) {
        return false;
    }

//--    if (element->isSpanner()) {
//--        TourHandler::startTour("spanner-drop-apply");
//--    }

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
            const Ms::InputState& is = score->inputState();
            Ms::Staff* staff = score->staff(is.track() / VOICES);
            return staff->staffType(is.tick())->group() == Ms::StaffGroup::PERCUSSION;
        };

        if (isEntryDrumStaff() && element->isChord()) {
            Ms::InputState& is = score->inputState();
            Element* e = nullptr;
            if (!(modifiers & Qt::ShiftModifier)) {
                // shift+double-click: add note to "chord"
                // use input position rather than selection if possible
                // look for a cr in the voice predefined for the drum in the palette
                // back up if necessary
                // TODO: refactor this with similar code in putNote()
                if (is.segment()) {
                    Ms::Segment* seg = is.segment();
                    while (seg) {
                        if (seg->element(is.track())) {
                            break;
                        }
                        seg = seg->prev(Ms::SegmentType::ChordRest);
                    }
                    if (seg) {
                        is.setSegment(seg);
                    } else {
                        is.setSegment(is.segment()->measure()->first(Ms::SegmentType::ChordRest));
                    }
                }
                score->expandVoice();
                e = is.cr();
            }
            if (!e) {
                e = sel.elements().first();
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
                qDebug("nowhere to place drum note");
            }
        } else if (element->isLayoutBreak()) {
            Ms::LayoutBreak* breakElement = toLayoutBreak(element);
            score->cmdToggleLayoutBreak(breakElement->layoutBreakType());
        } else if (element->isSlur() && addSingle) {
            doAddSlur(toSlur(element));
        } else if (element->isSLine() && !element->isGlissando() && addSingle) {
            Ms::Segment* startSegment = cr1->segment();
            Ms::Segment* endSegment = cr2->segment();
            if (element->type() == Ms::ElementType::PEDAL && cr2 != cr1) {
                endSegment = endSegment->nextCR(cr2->track());
            }
            // TODO - handle cross-voice selections
            int idx = cr1->staffIdx();

            QByteArray a = element->mimeData(PointF());
//printf("<<%s>>\n", a.data());
            Ms::XmlReader e(a);
            Ms::Fraction duration;        // dummy
            PointF dragOffset;
            Ms::ElementType type = Ms::Element::readType(e, &dragOffset, &duration);
            Ms::Spanner* spanner = static_cast<Ms::Spanner*>(Ms::Element::create(type, score));
            spanner->read(e);
            spanner->styleChanged();
            score->cmdAddSpanner(spanner, idx, startSegment, endSegment);
        } else {
            for (Element* e : sel.elements()) {
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
                && (toActionIcon(element)->actionType() == Ms::ActionIconType::VFRAME
                    || toActionIcon(element)->actionType() == Ms::ActionIconType::HFRAME
                    || toActionIcon(element)->actionType() == Ms::ActionIconType::TFRAME
                    || toActionIcon(element)->actionType() == Ms::ActionIconType::MEASURE
                    || toActionIcon(element)->actionType() == Ms::ActionIconType::BRACKETS))) {
            Measure* last = sel.endSegment() ? sel.endSegment()->measure() : nullptr;
            for (Measure* m = sel.startSegment()->measure(); m; m = m->nextMeasureMM()) {
                RectF r = m->staffabbox(sel.staffStart());
                PointF pt(r.x() + r.width() * .5, r.y() + r.height() * .5);
                pt += m->system()->page()->pos();
                applyDropPaletteElement(score, m, element, modifiers, pt);
                if (m == last) {
                    break;
                }
            }
        } else if (element->type() == ElementType::LAYOUT_BREAK) {
            Ms::LayoutBreak* breakElement = static_cast<Ms::LayoutBreak*>(element);
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
            int staffIdx1 = sel.staffStart();
            int staffIdx2 = element->type() == ElementType::CLEF ? sel.staffEnd() : staffIdx1 + 1;
            for (int i = staffIdx1; i < staffIdx2; ++i) {
                // for clefs, use mid-measure changes if appropriate
                Element* e1 = nullptr;
                Element* e2 = nullptr;
                // use mid-measure clef changes as appropriate
                if (element->type() == ElementType::CLEF) {
                    if (sel.startSegment()->isChordRestType() && sel.startSegment()->rtick().isNotZero()) {
                        ChordRest* cr = static_cast<ChordRest*>(sel.startSegment()->nextChordRest(i * VOICES));
                        if (cr && cr->isChord()) {
                            e1 = static_cast<Ms::Chord*>(cr)->upNote();
                        } else {
                            e1 = cr;
                        }
                    }
                    if (sel.endSegment() && sel.endSegment()->segmentType() == Ms::SegmentType::ChordRest) {
                        ChordRest* cr = static_cast<ChordRest*>(sel.endSegment()->nextChordRest(i * VOICES));
                        if (cr && cr->isChord()) {
                            e2 = static_cast<Ms::Chord*>(cr)->upNote();
                        } else {
                            e2 = cr;
                        }
                    }
                }
                if (m2 || e2) {
                    // restore original clef/keysig/timesig
                    Ms::Staff* staff = score->staff(i);
                    Ms::Fraction tick1 = sel.startSegment()->tick();
                    Ms::Element* oelement = nullptr;
                    switch (element->type()) {
                    case Ms::ElementType::CLEF:
                    {
                        Ms::Clef* oclef = new Ms::Clef(score);
                        oclef->setClefType(staff->clef(tick1));
                        oelement = oclef;
                        break;
                    }
                    case Ms::ElementType::KEYSIG:
                    {
                        Ms::KeySig* okeysig = new Ms::KeySig(score);
                        okeysig->setKeySigEvent(staff->keySigEvent(tick1));
                        if (!score->styleB(Ms::Sid::concertPitch) && !okeysig->isCustom() && !okeysig->isAtonal()) {
                            Ms::Interval v = staff->part()->instrument(tick1)->transpose();
                            if (!v.isZero()) {
                                Key k = okeysig->key();
                                okeysig->setKey(transposeKey(k, v, okeysig->part()->preferSharpFlat()));
                            }
                        }
                        oelement = okeysig;
                        break;
                    }
                    case Ms::ElementType::TIMESIG:
                    {
                        Ms::TimeSig* otimesig = new Ms::TimeSig(score);
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
            Ms::Segment* startSegment = sel.startSegment();
            Ms::Segment* endSegment = sel.endSegment();
            bool firstStaffOnly = element->isVolta() && !(modifiers & Qt::ControlModifier);
            int startStaff = firstStaffOnly ? 0 : sel.staffStart();
            int endStaff   = firstStaffOnly ? 1 : sel.staffEnd();
            for (int i = startStaff; i < endStaff; ++i) {
                Ms::Spanner* spanner = static_cast<Ms::Spanner*>(element->clone());
                spanner->setScore(score);
                spanner->styleChanged();
                score->cmdAddSpanner(spanner, i, startSegment, endSegment);
            }
        } else {
            int track1 = sel.staffStart() * VOICES;
            int track2 = sel.staffEnd() * VOICES;
            Ms::Segment* startSegment = sel.startSegment();
            Ms::Segment* endSegment = sel.endSegment();       //keep it, it could change during the loop

            for (Ms::Segment* s = startSegment; s && s != endSegment; s = s->next1()) {
                for (int track = track1; track < track2; ++track) {
                    Ms::Element* e = s->element(track);
                    if (e == 0 || !score->selectionFilter().canSelect(e)
                        || !score->selectionFilter().canSelectVoice(track)) {
                        continue;
                    }
                    if (e->isChord()) {
                        Ms::Chord* chord = toChord(e);
                        for (Ms::Note* n : chord->notes()) {
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
        qDebug("unknown selection state");
    }

    apply();

    setDropTarget(nullptr);

    return true;
}

//! NOTE Copied from Palette applyDrop
void NotationInteraction::applyDropPaletteElement(Ms::Score* score, Ms::Element* target, Ms::Element* e,
                                                  Qt::KeyboardModifiers modifiers,
                                                  PointF pt, bool pasteMode)
{
    Ms::EditData newData(&m_scoreCallbacks);
    Ms::EditData* dropData = &newData;

    if (isTextEditingStarted()) {
        dropData = &m_textEditData;
    }

    dropData->pos         = pt.isNull() ? target->pagePos() : pt;
    dropData->dragOffset  = QPointF();
    dropData->modifiers   = modifiers;
    dropData->dropElement = e;

    if (target->acceptDrop(*dropData)) {
        // use same code path as drag&drop

        QByteArray a = e->mimeData(PointF());

        Ms::XmlReader n(a);
        n.setPasteMode(pasteMode);
        Fraction duration;      // dummy
        PointF dragOffset;
        ElementType type = Element::readType(n, &dragOffset, &duration);
        dropData->dropElement = Element::create(type, score);

        dropData->dropElement->read(n);
        dropData->dropElement->styleChanged();       // update to local style

        Ms::Element* el = target->drop(*dropData);
        if (el && el->isInstrumentChange()) {
            selectInstrument(toInstrumentChange(el));
        }

        if (el && !score->inputState().noteEntryMode()) {
            doSelect({ el }, Ms::SelectType::SINGLE, 0);
        }
        dropData->dropElement = nullptr;

        m_notifyAboutDropChanged = true;
    }
}

//! NOTE Copied from ScoreView::cmdAddSlur
void NotationInteraction::doAddSlur(const Ms::Slur* slurTemplate)
{
    startEdit();

    Ms::ChordRest* firstChordRest = nullptr;
    Ms::ChordRest* secondChordRest = nullptr;
    const auto& sel = score()->selection();
    auto el = sel.uniqueElements();

    if (sel.isRange()) {
        int startTrack = sel.staffStart() * VOICES;
        int endTrack = sel.staffEnd() * VOICES;
        for (int track = startTrack; track < endTrack; ++track) {
            firstChordRest = nullptr;
            secondChordRest = nullptr;
            for (Ms::Element* e : el) {
                if (e->track() != track) {
                    continue;
                }
                if (e->isNote()) {
                    e = toNote(e)->chord();
                }
                if (!e->isChord()) {
                    continue;
                }
                Ms::ChordRest* cr = Ms::toChordRest(e);
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
    } else {
        for (Ms::Element* e : el) {
            if (e->isNote()) {
                e = Ms::toNote(e)->chord();
            }
            if (!e->isChord()) {
                continue;
            }
            Ms::ChordRest* cr = Ms::toChordRest(e);
            if (!firstChordRest || cr->isBefore(firstChordRest)) {
                firstChordRest = cr;
            }
            if (!secondChordRest || secondChordRest->isBefore(cr)) {
                secondChordRest = cr;
            }
        }

        if (firstChordRest == secondChordRest) {
            secondChordRest = Ms::nextChordRest(firstChordRest);
        }

        if (firstChordRest) {
            doAddSlur(firstChordRest, secondChordRest, slurTemplate);
        }
    }

    apply();
}

void NotationInteraction::doAddSlur(ChordRest* firstChordRest, ChordRest* secondChordRest, const Ms::Slur* slurTemplate)
{
    Ms::Slur* slur = firstChordRest->slur(secondChordRest);
    if (slur) {
        score()->removeElement(slur);
        return;
    }

    slur = score()->addSlur(firstChordRest, secondChordRest, slurTemplate);

    if (m_noteInput->isNoteInputMode()) {
        m_noteInput->addSlur(slur);
    } else if (!secondChordRest) {
        NOT_IMPLEMENTED;
        //startEditMode(ss);
    }
}

bool NotationInteraction::scoreHasMeasure() const
{
    Ms::Page* page = score()->pages().isEmpty() ? nullptr : score()->pages().front();
    const QList<Ms::System*>* systems = page ? &page->systems() : nullptr;
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
        chordArticulations = Ms::flipArticulations(chordArticulations, Ms::Placement::ABOVE);
        chordArticulations = Ms::splitArticulations(chordArticulations);

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
        delete m_dropData.ed.dropElement;
        m_dropData.ed.dropElement = nullptr;
        score()->update();
    }
    setDropTarget(nullptr);
}

mu::async::Notification NotationInteraction::dropChanged() const
{
    return m_dropChanged;
}

//! NOTE Copied from ScoreView::dropCanvas
bool NotationInteraction::dropCanvas(Element* e)
{
    if (e->isActionIcon()) {
        switch (Ms::toActionIcon(e)->actionType()) {
        case Ms::ActionIconType::VFRAME:
            score()->insertMeasure(ElementType::VBOX, 0);
            break;
        case Ms::ActionIconType::HFRAME:
            score()->insertMeasure(ElementType::HBOX, 0);
            break;
        case Ms::ActionIconType::TFRAME:
            score()->insertMeasure(ElementType::TBOX, 0);
            break;
        case Ms::ActionIconType::FFRAME:
            score()->insertMeasure(ElementType::FBOX, 0);
            break;
        case Ms::ActionIconType::MEASURE:
            score()->insertMeasure(ElementType::MEASURE, 0);
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
Element* NotationInteraction::dropTarget(Ms::EditData& ed) const
{
    QList<Element*> el = elementsAt(ed.pos);
    for (Element* e : el) {
        if (e->isStaffLines()) {
            if (el.size() > 2) {          // is not first class drop target
                continue;
            }
            e = Ms::toStaffLines(e)->measure();
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
    int staffIdx;
    Ms::Segment* seg;
    Ms::MeasureBase* mb = score()->pos2measure(pos, &staffIdx, 0, &seg, 0);
    if (!(m_dropData.ed.modifiers & Qt::ControlModifier)) {
        staffIdx = 0;
    }
    int track = staffIdx * VOICES;

    if (mb && mb->isMeasure()) {
        Ms::Measure* m = Ms::toMeasure(mb);
        Ms::System* s  = m->system();
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
        m_notifyAboutDropChanged = true;
        return true;
    }
    m_dropData.ed.dropElement->score()->addRefresh(m_dropData.ed.dropElement->canvasBoundingRect());
    setDropTarget(nullptr);
    return false;
}

//! NOTE Copied from ScoreView::dragTimeAnchorElement
bool NotationInteraction::dragTimeAnchorElement(const PointF& pos)
{
    int staffIdx;
    Ms::Segment* seg;
    Ms::MeasureBase* mb = score()->pos2measure(pos, &staffIdx, 0, &seg, 0);
    int track  = staffIdx * VOICES;

    if (mb && mb->isMeasure() && seg->element(track)) {
        Ms::Measure* m = Ms::toMeasure(mb);
        Ms::System* s  = m->system();
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
void NotationInteraction::setDropTarget(Element* el)
{
    if (m_dropData.dropTarget != el) {
        if (m_dropData.dropTarget) {
            m_dropData.dropTarget->setDropTarget(false);
            m_dropData.dropTarget = nullptr;
        }

        m_dropData.dropTarget = el;
        if (m_dropData.dropTarget) {
            m_dropData.dropTarget->setDropTarget(true);
        }
    }

    m_anchorLines.clear();

    //! TODO
    //    if (dropRectangle.isValid()) {
    //        dropRectangle = QRectF();
    //    }
    //! ---

    notifyAboutDragChanged();
}

void NotationInteraction::setAnchorLines(const std::vector<LineF>& anchorList)
{
    m_anchorLines = anchorList;
}

void NotationInteraction::resetAnchorLines()
{
    m_anchorLines.clear();
}

void NotationInteraction::drawAnchorLines(mu::draw::Painter* painter)
{
    if (m_anchorLines.empty()) {
        return;
    }

    const auto dropAnchorColor = configuration()->anchorLineColor();
    mu::draw::Pen pen(dropAnchorColor, 2.0 / painter->worldTransform().m11(), mu::draw::PenStyle::DotLine);

    for (const LineF& anchor : m_anchorLines) {
        painter->setPen(pen);
        painter->drawLine(anchor);

        qreal d = 4.0 / painter->worldTransform().m11();
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

    m_textEditData.element->drawEditMode(painter, m_textEditData);
}

void NotationInteraction::drawSelectionRange(draw::Painter* painter)
{
    using namespace draw;
    if (!m_selection->isRange()) {
        return;
    }

    painter->setBrush(BrushStyle::NoBrush);

    QColor selectionColor = configuration()->selectionColor();
    qreal penWidth = 3.0 / painter->worldTransform().m11();

    Pen pen;
    pen.setColor(selectionColor);
    pen.setWidthF(penWidth);
    pen.setStyle(PenStyle::SolidLine);
    painter->setPen(pen);

    std::vector<RectF> rangeArea = m_selection->range()->boundingArea();
    for (const RectF& rect: rangeArea) {
        mu::PainterPath path;
        path.addRoundedRect(rect, 6, 6);

        QColor fillColor = selectionColor;
        fillColor.setAlpha(10);
        painter->fillPath(path, fillColor);
        painter->drawPath(path);
    }
}

void NotationInteraction::drawGripPoints(draw::Painter* painter)
{
    if (!selection()->element() || !m_gripEditData.element) {
        return;
    }

    m_gripEditData.grip.resize(m_gripEditData.grips);

    constexpr qreal DEFAULT_GRIP_SIZE = 8;

    qreal gripWidth = DEFAULT_GRIP_SIZE / painter->worldTransform().m11();
    qreal gripHeight = DEFAULT_GRIP_SIZE / painter->worldTransform().m22();
    RectF newRect(-gripWidth / 2, -gripHeight / 2, gripWidth, gripHeight);

    Element* page = m_gripEditData.element->findAncestor(ElementType::PAGE);
    PointF pageOffset = page ? page->pos() : m_gripEditData.element->pos();

    for (RectF& gripRect: m_gripEditData.grip) {
        gripRect = newRect.translated(pageOffset);
    }

    m_gripEditData.element->updateGrips(m_gripEditData);
    m_gripEditData.element->drawEditMode(painter, m_gripEditData);
}

void NotationInteraction::moveSelection(MoveDirection d, MoveSelectionType type)
{
    IF_ASSERT_FAILED(MoveDirection::Left == d || MoveDirection::Right == d) {
        return;
    }

    IF_ASSERT_FAILED(MoveSelectionType::Undefined != type) {
        return;
    }

    if (MoveSelectionType::Element == type) {
        moveElementSelection(d);
        return;
    }

    //! NOTE Previously, the `Score::move` method directly expected commands (actions)
    //! Now the `Notation` provides only notation management methods,
    //! and interpretation of actions is the responsibility of `NotationActionController`

    auto typeToString = [](MoveSelectionType type) {
        switch (type) {
        case MoveSelectionType::Undefined: return QString();
        case MoveSelectionType::Element:   return QString();
        case MoveSelectionType::Chord:     return QString("chord");
        case MoveSelectionType::Measure:   return QString("measure");
        case MoveSelectionType::Track:     return QString("track");
        }
        return QString();
    };

    QString cmd;
    if (MoveDirection::Left == d) {
        cmd = "prev-";
    }

    if (MoveDirection::Right == d) {
        cmd = "next-";
    }

    cmd += typeToString(type);

    score()->move(cmd);
    notifyAboutSelectionChanged();
}

void NotationInteraction::moveElementSelection(MoveDirection d)
{
    Element* el = score()->selection().element();
    if (!el && !score()->selection().elements().isEmpty()) {
        el = score()->selection().elements().last();
    }

    if (!el) {
        ChordRest* cr = score()->selection().currentCR();
        if (cr) {
            if (cr->isChord()) {
                if (MoveDirection::Left == d) {
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

    Element* toEl = nullptr;
    if (el) {
        toEl = (MoveDirection::Left == d) ? score()->prevElement() : score()->nextElement();
    } else {
        toEl = (MoveDirection::Left == d) ? score()->lastElement() : score()->firstElement();
    }

    if (toEl) {
        score()->select(toEl, SelectType::SINGLE, 0);
        if (toEl->type() == ElementType::NOTE || toEl->type() == ElementType::HARMONY) {
            score()->setPlayNote(true);
        }
    }

    notifyAboutSelectionChanged();
}

void NotationInteraction::movePitch(MoveDirection d, PitchMode mode)
{
    IF_ASSERT_FAILED(MoveDirection::Up == d || MoveDirection::Down == d) {
        return;
    }

    QList<Note*> el = score()->selection().uniqueNotes();
    IF_ASSERT_FAILED(!el.isEmpty()) {
        return;
    }

    startEdit();

    bool isUp = MoveDirection::Up == d;
    score()->upDown(isUp, mode);

    apply();

    notifyAboutDragChanged();
}

void NotationInteraction::moveText(MoveDirection d, bool quickly)
{
    Element* el = score()->selection().element();
    IF_ASSERT_FAILED(el && el->isTextBase()) {
        return;
    }

    startEdit();

    qreal step = quickly ? Ms::MScore::nudgeStep10 : Ms::MScore::nudgeStep;
    step = step * el->spatium();

    switch (d) {
    case MoveDirection::Undefined:
        IF_ASSERT_FAILED(d != MoveDirection::Undefined) {
            return;
        }
        break;
    case MoveDirection::Left:
        el->undoChangeProperty(Ms::Pid::OFFSET, el->offset() - PointF(step, 0.0), Ms::PropertyFlags::UNSTYLED);
        break;
    case MoveDirection::Right:
        el->undoChangeProperty(Ms::Pid::OFFSET, el->offset() + PointF(step, 0.0), Ms::PropertyFlags::UNSTYLED);
        break;
    case MoveDirection::Up:
        el->undoChangeProperty(Ms::Pid::OFFSET, el->offset() - PointF(0.0, step), Ms::PropertyFlags::UNSTYLED);
        break;
    case MoveDirection::Down:
        el->undoChangeProperty(Ms::Pid::OFFSET, el->offset() + PointF(0.0, step), Ms::PropertyFlags::UNSTYLED);
        break;
    }

    apply();

    notifyAboutDragChanged();
}

bool NotationInteraction::isTextEditingStarted() const
{
    return m_textEditData.element && m_textEditData.element->isTextBase();
}

void NotationInteraction::startEditText(Element* element, const PointF& cursorPos)
{
    if (!element || !element->isEditable() || !element->isTextBase()) {
        qDebug("The element cannot be edited");
        return;
    }

    m_textEditData.startMove = cursorPos;

    if (isTextEditingStarted()) {
        // double click on a textBase element that is being edited - select word
        Ms::TextBase* textBase = Ms::toTextBase(m_textEditData.element);
        textBase->multiClickSelect(m_textEditData, Ms::MultiClick::Double);
        textBase->endHexState(m_textEditData);
        textBase->setPrimed(false);
    } else {
        m_textEditData.clearData();

        m_textEditData.element = element;

        if (m_textEditData.element->isTBox()) {
            m_textEditData.element = toTBox(m_textEditData.element)->text();
        }

        element->startEdit(m_textEditData);
    }

    notifyAboutTextEditingStarted();
    notifyAboutTextEditingChanged();
}

void NotationInteraction::editText(QKeyEvent* event)
{
    bool wasEditingText = m_textEditData.element != nullptr;
    if (!wasEditingText && selection()->element()) {
        m_textEditData.element = selection()->element();
    }

    if (!m_textEditData.element) {
        return;
    }

    m_textEditData.key = event->key();
    m_textEditData.modifiers = event->modifiers();
    m_textEditData.s = event->text();
    startEdit();
    if (m_textEditData.element->edit(m_textEditData)) {
        event->accept();
        apply();
    } else {
        m_undoStack->rollbackChanges();
    }
    if (!wasEditingText) {
        m_textEditData.element = nullptr;
    }
    if (isTextEditingStarted()) {
        notifyAboutTextEditingChanged();
    }
}

void NotationInteraction::endEditText()
{
    IF_ASSERT_FAILED(m_textEditData.element) {
        return;
    }

    if (!isTextEditingStarted()) {
        return;
    }

    m_textEditData.element->endEdit(m_textEditData);
    m_textEditData.element = nullptr;
    m_textEditData.clearData();

    notifyAboutTextEditingChanged();
}

void NotationInteraction::changeTextCursorPosition(const PointF& newCursorPos)
{
    IF_ASSERT_FAILED(isTextEditingStarted() && m_textEditData.element) {
        return;
    }

    m_textEditData.startMove = newCursorPos;

    Ms::TextBase* textEl = Ms::toTextBase(m_textEditData.element);

    textEl->mousePress(m_textEditData);
    if (m_textEditData.buttons == Qt::MiddleButton) {
        #if defined(Q_OS_MAC) || defined(Q_OS_WIN)
        QClipboard::Mode mode = QClipboard::Clipboard;
        #else
        QClipboard::Mode mode = QClipboard::Selection;
        #endif
        QString txt = QGuiApplication::clipboard()->text(mode);
        textEl->paste(m_textEditData, txt);
    }

    notifyAboutTextEditingChanged();
}

const TextBase* NotationInteraction::editedText() const
{
    return Ms::toTextBase(m_textEditData.element);
}

void NotationInteraction::undo()
{
    m_undoStack->undo(&m_textEditData);
}

void NotationInteraction::redo()
{
    m_undoStack->redo(&m_textEditData);
}

mu::async::Notification NotationInteraction::textEditingStarted() const
{
    return m_textEditingStarted;
}

mu::async::Notification NotationInteraction::textEditingChanged() const
{
    return m_textEditingChanged;
}

mu::async::Channel<ScoreConfigType> NotationInteraction::scoreConfigChanged() const
{
    return m_scoreConfigChanged;
}

bool NotationInteraction::isGripEditStarted() const
{
    return m_gripEditData.element && m_gripEditData.curGrip != Ms::Grip::NO_GRIP;
}

bool NotationInteraction::isHitGrip(const PointF& pos) const
{
    if (!selection()->element() || m_gripEditData.grip.empty()) {
        return false;
    }

    qreal align = m_gripEditData.grip[0].width() / 2;

    for (int i = 0; i < m_gripEditData.grips; ++i) {
        if (m_gripEditData.grip[i].adjusted(-align, -align, align, align).contains(pos)) {
            return true;
        }
    }

    return false;
}

void NotationInteraction::startEditGrip(const PointF& pos)
{
    if (m_gripEditData.grip.size() == 0) {
        return;
    }

    const qreal align = m_gripEditData.grip[0].width() / 2;
    for (int i = 0; i < m_gripEditData.grips; ++i) {
        if (!m_gripEditData.grip[i].adjusted(-align, -align, align, align).contains(pos)) {
            continue;
        }

        m_gripEditData.curGrip = Ms::Grip(i);

        std::vector<LineF> lines;
        QVector<LineF> anchorLines = m_gripEditData.element->gripAnchorLines(m_gripEditData.curGrip);

        Element* page = m_gripEditData.element->findAncestor(ElementType::PAGE);
        const PointF pageOffset((page ? page : m_gripEditData.element)->pos());
        if (!anchorLines.isEmpty()) {
            for (LineF& line : anchorLines) {
                line.translate(pageOffset);
                lines.push_back(line);
            }
        }

        setAnchorLines(lines);

        m_gripEditData.element->startEdit(m_gripEditData);

        notifyAboutNotationChanged();
        return;
    }
}

void NotationInteraction::endEditGrip()
{
    if (!m_gripEditData.element) {
        return;
    }

    m_gripEditData.curGrip = Ms::Grip::NO_GRIP;

    resetAnchorLines();
    notifyAboutNotationChanged();
}

void NotationInteraction::splitSelectedMeasure()
{
    Element* selectedElement = m_selection->element();
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

    notifyAboutSelectionChanged();
}

void NotationInteraction::joinSelectedMeasures()
{
    if (!m_selection->isRange()) {
        return;
    }

    Measure* measureStart = score()->crMeasure(m_selection->range()->startMeasureIndex() - 1);
    Measure* measureEnd = score()->crMeasure(m_selection->range()->endMeasureIndex() - 1);

    startEdit();
    score()->cmdJoinMeasure(measureStart, measureEnd);
    apply();

    notifyAboutSelectionChanged();
}

void NotationInteraction::addBoxes(BoxType boxType, int count, int beforeBoxIndex)
{
    auto boxTypeToElementType = [](BoxType boxType) {
        switch (boxType) {
        case BoxType::Horizontal: return Ms::ElementType::HBOX;
        case BoxType::Vertical: return Ms::ElementType::VBOX;
        case BoxType::Text: return Ms::ElementType::TBOX;
        case BoxType::Measure: return Ms::ElementType::MEASURE;
        case BoxType::Unknown: return Ms::ElementType::INVALID;
        }

        return ElementType::INVALID;
    };

    Ms::ElementType elementType = boxTypeToElementType(boxType);
    Ms::MeasureBase* beforeBox = beforeBoxIndex >= 0 ? score()->measure(beforeBoxIndex) : nullptr;

    startEdit();
    for (int i = 0; i < count; ++i) {
        constexpr bool createEmptyMeasures = false;
        constexpr bool moveSignaturesClef = true;
        constexpr bool needDeselectAll = false;

        score()->insertMeasure(elementType, beforeBox, createEmptyMeasures, moveSignaturesClef, needDeselectAll);
    }
    apply();

    notifyAboutNotationChanged();
}

void NotationInteraction::copySelection()
{
    if (!selection()->canCopy()) {
        return;
    }

    if (isTextEditingStarted()) {
        m_textEditData.element->editCopy(m_textEditData);
        Ms::TextEditData* ted = static_cast<Ms::TextEditData*>(m_textEditData.getData(m_textEditData.element));
        if (!ted->selectedText.isEmpty()) {
            QGuiApplication::clipboard()->setText(ted->selectedText, QClipboard::Clipboard);
        }
    } else {
        QMimeData* mimeData = selection()->mimeData();
        if (!mimeData) {
            return;
        }
        QApplication::clipboard()->setMimeData(mimeData);
    }
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
        toTextBase(m_textEditData.element)->paste(m_textEditData, txt);
    } else {
        const QMimeData* mimeData = QApplication::clipboard()->mimeData();
        score()->cmdPaste(mimeData, nullptr, scale);
    }
    apply();

    notifyAboutSelectionChanged();
}

void NotationInteraction::swapSelection()
{
    if (!selection()->canCopy()) {
        return;
    }

    Ms::Selection& selection = score()->selection();
    QString mimeType = selection.mimeType();

    if (mimeType == Ms::mimeStaffListFormat) { // determine size of clipboard selection
        const QMimeData* mimeData = this->selection()->mimeData();
        QByteArray data = mimeData ? mimeData->data(Ms::mimeStaffListFormat) : QByteArray();
        Ms::XmlReader reader(data);
        reader.readNextStartElement();

        Fraction tickLen = Fraction(0, 1);
        int stavesCount = 0;

        if (reader.name() == "StaffList") {
            tickLen = Ms::Fraction::fromTicks(reader.intAttribute("len", 0));
            stavesCount = reader.intAttribute("staves", 0);
        }

        if (tickLen > Ms::Fraction(0, 1)) { // attempt to extend selection to match clipboard size
            Ms::Segment* segment = selection.startSegment();
            Ms::Fraction startTick = selection.tickStart() + tickLen;
            Ms::Segment* segmentAfter = score()->tick2leftSegment(startTick);

            int staffIndex = selection.staffStart() + stavesCount - 1;
            if (staffIndex >= score()->nstaves()) {
                staffIndex = score()->nstaves() - 1;
            }

            startTick = selection.tickStart();
            Ms::Fraction endTick = startTick + tickLen;
            selection.extendRangeSelection(segment, segmentAfter, staffIndex, startTick, endTick);
            selection.update();
        }
    }

    QByteArray currentSelectionBackup(selection.mimeData());
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
        auto textBase = toTextBase(m_textEditData.element);
        if (!textBase->deleteSelectedText(m_textEditData)) {
            m_textEditData.key = Qt::Key_Backspace;
            m_textEditData.modifiers = {};
            textBase->edit(m_textEditData);
        }
    } else {
        score()->cmdDeleteSelection();
    }
    apply();

    notifyAboutSelectionChanged();
}

void NotationInteraction::flipSelection()
{
    if (selection()->isNone()) {
        return;
    }

    startEdit();
    score()->cmdFlip();
    apply();

    notifyAboutSelectionChanged();
}

void NotationInteraction::addTieToSelection()
{
    startEdit();
    score()->cmdToggleTie();
    apply();

    notifyAboutSelectionChanged();
}

void NotationInteraction::addTiedNoteToChord()
{
    startEdit();
    score()->cmdAddTie(true);
    apply();

    notifyAboutSelectionChanged();
}

void NotationInteraction::addSlurToSelection()
{
    if (selection()->isNone()) {
        return;
    }

    startEdit();
    doAddSlur();
    apply();

    notifyAboutSelectionChanged();
}

void NotationInteraction::addOttavaToSelection(OttavaType type)
{
    if (selection()->isNone()) {
        return;
    }

    startEdit();
    score()->cmdAddOttava(type);
    apply();

    notifyAboutSelectionChanged();
}

void NotationInteraction::addHairpinToSelection(HairpinType type)
{
    if (selection()->isNone()) {
        return;
    }

    startEdit();
    score()->addHairpin(type);
    apply();

    notifyAboutSelectionChanged();
}

void NotationInteraction::addAccidentalToSelection(AccidentalType type)
{
    if (selection()->isNone()) {
        return;
    }

    Ms::EditData editData(&m_scoreCallbacks);

    startEdit();
    score()->toggleAccidental(type, editData);
    apply();

    notifyAboutSelectionChanged();
}

void NotationInteraction::putRestToSelection()
{
    Ms::InputState& is = score()->inputState();
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

    notifyAboutSelectionChanged();
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

    notifyAboutNotationChanged();
}

void NotationInteraction::changeSelectedNotesArticulation(SymbolId articulationSymbolId)
{
    if (selection()->isNone()) {
        return;
    }

    std::vector<Ms::Note*> notes = score()->selection().noteList();

    auto updateMode = notesHaveActiculation(notes, articulationSymbolId)
                      ? Ms::ArticulationsUpdateMode::Remove : Ms::ArticulationsUpdateMode::Insert;

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

    notifyAboutSelectionChanged();
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
    score()->cmdAddGrace(type, Ms::MScore::division / denominator);
    apply();

    notifyAboutNotationChanged();
}

void NotationInteraction::addTupletToSelectedChordRests(const TupletOptions& options)
{
    if (selection()->isNone()) {
        return;
    }

    startEdit();
    for (ChordRest* chordRest : score()->getSelectedChordRests()) {
        if (!chordRest->isGrace()) {
            score()->addTuplet(chordRest, options.ratio, options.numberType, options.bracketType);
        }
    }
    apply();

    notifyAboutSelectionChanged();
}

void NotationInteraction::addBeamToSelectedChordRests(BeamMode mode)
{
    if (selection()->isNone()) {
        return;
    }

    startEdit();
    score()->cmdSetBeamMode(mode);
    apply();

    notifyAboutNotationChanged();
}

void NotationInteraction::increaseDecreaseDuration(int steps, bool stepByDots)
{
    if (selection()->isNone()) {
        return;
    }

    startEdit();
    score()->cmdIncDecDuration(steps, stepByDots);
    apply();

    notifyAboutNotationChanged();
}

void NotationInteraction::toggleLayoutBreak(LayoutBreakType breakType)
{
    startEdit();
    score()->cmdToggleLayoutBreak(breakType);
    apply();

    notifyAboutNotationChanged();
}

void NotationInteraction::setBreaksSpawnInterval(BreaksSpawnIntervalType intervalType, int interval)
{
    interval = intervalType == BreaksSpawnIntervalType::MeasuresInterval ? interval : 0;
    bool afterEachSystem = intervalType == BreaksSpawnIntervalType::AfterEachSystem;

    startEdit();
    score()->addRemoveBreaks(interval, afterEachSystem);
    apply();

    notifyAboutNotationChanged();
}

bool NotationInteraction::transpose(const TransposeOptions& options)
{
    startEdit();

    bool ok = score()->transpose(options.mode, options.direction, options.key, options.interval,
                                 options.needTransposeKeys, options.needTransposeChordNames, options.needTransposeDoubleSharpsFlats);

    apply();

    notifyAboutNotationChanged();

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

    notifyAboutNotationChanged();
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

    notifyAboutSelectionChanged();
}

void NotationInteraction::addFret(int fretIndex)
{
    startEdit();
    score()->cmdAddFret(fretIndex);
    apply();

    notifyAboutSelectionChanged();
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

    notifyAboutSelectionChanged();
}

void NotationInteraction::addAnchoredLineToSelectedNotes()
{
    if (selection()->isNone()) {
        return;
    }

    startEdit();
    score()->addNoteLine();
    apply();

    notifyAboutSelectionChanged();
}

void NotationInteraction::addText(TextType type)
{
    if (!scoreHasMeasure()) {
        LOGE() << "Need to create measure";
        return;
    }

    if (m_noteInput->isNoteInputMode()) {
        m_noteInput->endNoteInput();
    }

    startEdit();
    Ms::TextBase* textBox = score()->addText(type);
    apply();

    if (textBox) {
        doSelect({ textBox }, SelectType::SINGLE, 0);
        startEditText(textBox, PointF());
    }

    notifyAboutSelectionChanged();
}

void NotationInteraction::addFiguredBass()
{
    Ms::FiguredBass* figuredBass = score()->addFiguredBass();

    if (figuredBass) {
        startEditText(figuredBass, PointF());
    }

    notifyAboutSelectionChanged();
}

void NotationInteraction::addStretch(qreal value)
{
    startEdit();
    score()->cmdAddStretch(value);
    apply();

    notifyAboutNotationChanged();
}

void NotationInteraction::addTimeSignature(Measure* measure, int staffIndex, TimeSignature* timeSignature)
{
    startEdit();
    score()->cmdAddTimeSig(measure, staffIndex, timeSignature, true);
    apply();

    notifyAboutNotationChanged();
}

void NotationInteraction::explodeSelectedStaff()
{
    if (!selection()->isRange()) {
        return;
    }

    startEdit();
    score()->cmdExplode();
    apply();

    notifyAboutNotationChanged();
}

void NotationInteraction::implodeSelectedStaff()
{
    if (!selection()->isRange()) {
        return;
    }

    startEdit();
    score()->cmdImplode();
    apply();

    notifyAboutNotationChanged();
}

void NotationInteraction::realizeSelectedChordSymbols()
{
    if (selection()->isNone()) {
        return;
    }

    startEdit();
    score()->cmdRealizeChordSymbols();
    apply();

    notifyAboutNotationChanged();
}

void NotationInteraction::removeSelectedRange()
{
    if (selection()->isNone()) {
        return;
    }

    startEdit();
    score()->cmdTimeDelete();
    apply();

    notifyAboutNotationChanged();
}

void NotationInteraction::removeEmptyTrailingMeasures()
{
    startEdit();
    score()->cmdRemoveEmptyTrailingMeasures();
    apply();

    notifyAboutNotationChanged();
}

void NotationInteraction::fillSelectionWithSlashes()
{
    if (selection()->isNone()) {
        return;
    }

    startEdit();
    score()->cmdSlashFill();
    apply();

    notifyAboutNotationChanged();
}

void NotationInteraction::replaceSelectedNotesWithSlashes()
{
    if (selection()->isNone()) {
        return;
    }

    startEdit();
    score()->cmdSlashRhythm();
    apply();

    notifyAboutNotationChanged();
}

void NotationInteraction::spellPitches()
{
    startEdit();
    score()->spell();
    apply();

    notifyAboutNotationChanged();
}

void NotationInteraction::regroupNotesAndRests()
{
    startEdit();
    score()->cmdResetNoteAndRestGroupings();
    apply();

    notifyAboutNotationChanged();
}

void NotationInteraction::resequenceRehearsalMarks()
{
    startEdit();
    score()->cmdResequenceRehearsalMarks();
    apply();

    notifyAboutNotationChanged();
}

void NotationInteraction::unrollRepeats()
{
    if (!score()->masterScore()) {
        return;
    }

    startEdit();
    score()->masterScore()->unrollRepeats();
    apply();

    notifyAboutNotationChanged();
}

void NotationInteraction::resetToDefault(ResettableValueType type)
{
    switch (type) {
    case ResettableValueType::Stretch:
        resetStretch();
        break;
    case ResettableValueType::BeamMode:
        resetBeamMode();
        break;
    case ResettableValueType::ShapesAndPosition:
        resetShapesAndPosition();
        break;
    case ResettableValueType::TextStyleOverriders:
        resetTextStyleOverrides();
        break;
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

    Element* selectedElement = selection()->element();
    if (selectedElement && !selectedElement->isInteractionAvailable()) {
        clearSelection();
    }

    apply();

    notifyAboutNotationChanged();
}

bool NotationInteraction::needEndTextEditing(const std::vector<Element*>& newSelectedElements) const
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

    return newSelectedElements.front() != m_textEditData.element;
}

void NotationInteraction::updateGripEdit(const std::vector<Element*>& elements)
{
    if (elements.size() > 1) {
        resetGripEdit();
        return;
    }

    Element* element = elements.front();
    if (element->gripsCount() <= 0) {
        resetGripEdit();
        return;
    }

    m_gripEditData.grips = element->gripsCount();
    m_gripEditData.curGrip = Ms::Grip::NO_GRIP;
    m_gripEditData.element = element;
    m_gripEditData.grip.resize(m_gripEditData.grips);

    m_gripEditData.element->startEdit(m_gripEditData);
    m_gripEditData.element->updateGrips(m_gripEditData);

    resetAnchorLines();
}

void NotationInteraction::resetGripEdit()
{
    m_gripEditData.grips = 0;
    m_gripEditData.curGrip = Ms::Grip::NO_GRIP;
    m_gripEditData.element = nullptr;
    m_gripEditData.grip.clear();

    resetAnchorLines();
}

void NotationInteraction::resetStretch()
{
    startEdit();
    score()->resetUserStretch();
    apply();

    notifyAboutNotationChanged();
}

void NotationInteraction::resetTextStyleOverrides()
{
    startEdit();
    score()->cmdResetTextStyleOverrides();
    apply();

    notifyAboutNotationChanged();
}

void NotationInteraction::resetBeamMode()
{
    startEdit();
    score()->cmdResetBeamMode();
    apply();

    notifyAboutNotationChanged();
}

void NotationInteraction::resetShapesAndPosition()
{
    startEdit();

    if (selection()->element()) {
        clearSelection();
        return;
    }

    for (Element* element : selection()->elements()) {
        element->reset();

        if (!element->isSpanner()) {
            continue;
        }

        Ms::Spanner* spanner = toSpanner(element);
        for (Ms::SpannerSegment* spannerSegment : spanner->spannerSegments()) {
            spannerSegment->reset();
        }
    }

    apply();

    notifyAboutNotationChanged();
}

void NotationInteraction::nextLyrics(bool back, bool moveOnly, bool end)
{
    if (!m_textEditData.element || !m_textEditData.element->isLyrics()) {
        qWarning("nextLyric called with invalid current element");
        return;
    }
    Ms::Lyrics* lyrics = toLyrics(m_textEditData.element);
    int track = lyrics->track();
    Ms::Segment* segment = lyrics->segment();
    int verse = lyrics->no();
    Ms::Placement placement = lyrics->placement();
    Ms::PropertyFlags pFlags = lyrics->propertyFlags(Ms::Pid::PLACEMENT);

    Ms::Segment* nextSegment = segment;
    if (back) {
        // search prev chord
        while ((nextSegment = nextSegment->prev1(Ms::SegmentType::ChordRest))) {
            Element* el = nextSegment->element(track);
            if (el && el->isChord()) {
                break;
            }
        }
    } else {
        // search next chord
        while ((nextSegment = nextSegment->next1(Ms::SegmentType::ChordRest))) {
            Element* el = nextSegment->element(track);
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
    Ms::Lyrics* fromLyrics = 0;
    if (!back) {
        while (segment) {
            ChordRest* cr = toChordRest(segment->element(track));
            if (cr) {
                fromLyrics = cr->lyrics(verse, placement);
                if (fromLyrics) {
                    break;
                }
            }
            segment = segment->prev1(Ms::SegmentType::ChordRest);
        }
    }

    ChordRest* cr = toChordRest(nextSegment->element(track));
    if (!cr) {
        qDebug("no next lyrics list: %s", nextSegment->element(track)->name());
        return;
    }
    Ms::Lyrics* nextLyrics = cr->lyrics(verse, placement);

    bool newLyrics = false;
    if (!nextLyrics) {
        nextLyrics = new Ms::Lyrics(score());
        nextLyrics->setTrack(track);
        cr = toChordRest(nextSegment->element(track));
        nextLyrics->setParent(cr);
        nextLyrics->setNo(verse);
        nextLyrics->setPlacement(placement);
        nextLyrics->setPropertyFlags(Ms::Pid::PLACEMENT, pFlags);
        nextLyrics->setSyllabic(Ms::Lyrics::Syllabic::SINGLE);
        newLyrics = true;
    }

    score()->startCmd();
    if (fromLyrics && !moveOnly) {
        switch (nextLyrics->syllabic()) {
        // as we arrived at nextLyrics by a [Space], it can be the beginning
        // of a multi-syllable, but cannot have syllabic dashes before
        case Ms::Lyrics::Syllabic::SINGLE:
        case Ms::Lyrics::Syllabic::BEGIN:
            break;
        case Ms::Lyrics::Syllabic::END:
            nextLyrics->undoChangeProperty(Ms::Pid::SYLLABIC, int(Ms::Lyrics::Syllabic::SINGLE));
            break;
        case Ms::Lyrics::Syllabic::MIDDLE:
            nextLyrics->undoChangeProperty(Ms::Pid::SYLLABIC, int(Ms::Lyrics::Syllabic::BEGIN));
            break;
        }
        // as we moved away from fromLyrics by a [Space], it can be
        // the end of a multi-syllable, but cannot have syllabic dashes after
        switch (fromLyrics->syllabic()) {
        case Ms::Lyrics::Syllabic::SINGLE:
        case Ms::Lyrics::Syllabic::END:
            break;
        case Ms::Lyrics::Syllabic::BEGIN:
            fromLyrics->undoChangeProperty(Ms::Pid::SYLLABIC, int(Ms::Lyrics::Syllabic::SINGLE));
            break;
        case Ms::Lyrics::Syllabic::MIDDLE:
            fromLyrics->undoChangeProperty(Ms::Pid::SYLLABIC, int(Ms::Lyrics::Syllabic::END));
            break;
        }
        // for the same reason, it cannot have a melisma
        fromLyrics->undoChangeProperty(Ms::Pid::LYRIC_TICKS, 0);
    }

    if (newLyrics) {
        score()->undoAddElement(nextLyrics);
    }
    score()->endCmd();
    score()->select(nextLyrics, SelectType::SINGLE, 0);
    score()->setLayoutAll();

    startEditText(nextLyrics, PointF());

    Ms::TextCursor* cursor = nextLyrics->cursor();
    if (end) {
        nextLyrics->selectAll(cursor);
    } else {
        cursor->movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
        cursor->movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
    }
}

void NotationInteraction::nextSyllable()
{
    if (!m_textEditData.element || !m_textEditData.element->isLyrics()) {
        qWarning("nextSyllable called with invalid current element");
        return;
    }
    Ms::Lyrics* lyrics = toLyrics(m_textEditData.element);
    int track = lyrics->track();
    Ms::Segment* segment = lyrics->segment();
    int verse = lyrics->no();
    Ms::Placement placement = lyrics->placement();
    Ms::PropertyFlags pFlags = lyrics->propertyFlags(Ms::Pid::PLACEMENT);

    // search next chord
    Ms::Segment* nextSegment = segment;
    while ((nextSegment = nextSegment->next1(Ms::SegmentType::ChordRest))) {
        Element* el = nextSegment->element(track);
        if (el && el->isChord()) {
            break;
        }
    }
    if (nextSegment == 0) {
        return;
    }

    // look for the lyrics we are moving from; may be the current lyrics or a previous one
    // we are extending with several dashes
    Ms::Lyrics* fromLyrics = 0;
    while (segment) {
        ChordRest* cr = toChordRest(segment->element(track));
        if (!cr) {
            segment = segment->prev1(Ms::SegmentType::ChordRest);
            continue;
        }
        fromLyrics = cr->lyrics(verse, placement);
        if (fromLyrics) {
            break;
        }
        segment = segment->prev1(Ms::SegmentType::ChordRest);
    }

    endEditText();

    score()->startCmd();
    ChordRest* cr = toChordRest(nextSegment->element(track));
    Ms::Lyrics* toLyrics = cr->lyrics(verse, placement);
    bool newLyrics = (toLyrics == 0);
    if (!toLyrics) {
        toLyrics = new Ms::Lyrics(score());
        toLyrics->setTrack(track);
        toLyrics->setParent(nextSegment->element(track));
        toLyrics->setNo(verse);
        toLyrics->setPlacement(placement);
        toLyrics->setPropertyFlags(Ms::Pid::PLACEMENT, pFlags);
        toLyrics->setSyllabic(Ms::Lyrics::Syllabic::END);
    } else {
        // as we arrived at toLyrics by a dash, it cannot be initial or isolated
        if (toLyrics->syllabic() == Ms::Lyrics::Syllabic::BEGIN) {
            toLyrics->undoChangeProperty(Ms::Pid::SYLLABIC, int(Ms::Lyrics::Syllabic::MIDDLE));
        } else if (toLyrics->syllabic() == Ms::Lyrics::Syllabic::SINGLE) {
            toLyrics->undoChangeProperty(Ms::Pid::SYLLABIC, int(Ms::Lyrics::Syllabic::END));
        }
    }

    if (fromLyrics) {
        // as we moved away from fromLyrics by a dash,
        // it can have syll. dashes before and after but cannot be isolated or terminal
        switch (fromLyrics->syllabic()) {
        case Ms::Lyrics::Syllabic::BEGIN:
        case Ms::Lyrics::Syllabic::MIDDLE:
            break;
        case Ms::Lyrics::Syllabic::SINGLE:
            fromLyrics->undoChangeProperty(Ms::Pid::SYLLABIC, int(Ms::Lyrics::Syllabic::BEGIN));
            break;
        case Ms::Lyrics::Syllabic::END:
            fromLyrics->undoChangeProperty(Ms::Pid::SYLLABIC, int(Ms::Lyrics::Syllabic::MIDDLE));
            break;
        }
        // for the same reason, it cannot have a melisma
        fromLyrics->undoChangeProperty(Ms::Pid::LYRIC_TICKS, 0);
    }

    if (newLyrics) {
        score()->undoAddElement(toLyrics);
    }

    score()->endCmd();
    score()->select(toLyrics, SelectType::SINGLE, 0);
    score()->setLayoutAll();

    startEditText(toLyrics, PointF());

    toLyrics->selectAll(toLyrics->cursor());
}

void NotationInteraction::nextLyricsVerse(bool back)
{
    if (!m_textEditData.element || !m_textEditData.element->isLyrics()) {
        qWarning("nextLyricVerse called with invalid current element");
        return;
    }
    Ms::Lyrics* lyrics = toLyrics(m_textEditData.element);
    int track = lyrics->track();
    ChordRest* cr = lyrics->chordRest();
    int verse = lyrics->no();
    Ms::Placement placement = lyrics->placement();
    Ms::PropertyFlags pFlags = lyrics->propertyFlags(Ms::Pid::PLACEMENT);

    if (back) {
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
        lyrics = new Ms::Lyrics(score());
        lyrics->setTrack(track);
        lyrics->setParent(cr);
        lyrics->setNo(verse);
        lyrics->setPlacement(placement);
        lyrics->setPropertyFlags(Ms::Pid::PLACEMENT, pFlags);
        score()->startCmd();
        score()->undoAddElement(lyrics);
        score()->endCmd();
    }

    score()->select(lyrics, SelectType::SINGLE, 0);
    startEditText(lyrics, PointF());

    lyrics = toLyrics(m_textEditData.element);

    score()->setLayoutAll();
    score()->update();

    lyrics->selectAll(lyrics->cursor());
}

void NotationInteraction::addMelisma()
{
    if (!m_textEditData.element || !m_textEditData.element->isLyrics()) {
        qWarning("addMelisma called with invalid current element");
        return;
    }
    Ms::Lyrics* lyrics = toLyrics(m_textEditData.element);
    int track = lyrics->track();
    Ms::Segment* segment = lyrics->segment();
    int verse = lyrics->no();
    Ms::Placement placement = lyrics->placement();
    Ms::PropertyFlags pFlags = lyrics->propertyFlags(Ms::Pid::PLACEMENT);
    Fraction endTick = segment->tick(); // a previous melisma cannot extend beyond this point

    endEditText();

    // search next chord
    Ms::Segment* nextSegment = segment;
    while ((nextSegment = nextSegment->next1(Ms::SegmentType::ChordRest))) {
        Element* el = nextSegment->element(track);
        if (el && el->isChord()) {
            break;
        }
    }

    // look for the lyrics we are moving from; may be the current lyrics or a previous one
    // we are extending with several underscores
    Ms::Lyrics* fromLyrics = 0;
    while (segment) {
        ChordRest* cr = toChordRest(segment->element(track));
        if (cr) {
            fromLyrics = cr->lyrics(verse, placement);
            if (fromLyrics) {
                break;
            }
        }
        segment = segment->prev1(Ms::SegmentType::ChordRest);
        // if the segment has a rest in this track, stop going back
        Element* e = segment ? segment->element(track) : 0;
        if (e && !e->isChord()) {
            break;
        }
    }

    // one-chord melisma?
    // if still at melisma initial chord and there is a valid next chord (if not,
    // there will be no melisma anyway), set a temporary melisma duration
    if (fromLyrics == lyrics && nextSegment) {
        score()->startCmd();
        lyrics->undoChangeProperty(Ms::Pid::LYRIC_TICKS, Fraction::fromTicks(Ms::Lyrics::TEMP_MELISMA_TICKS));
        score()->setLayoutAll();
        score()->endCmd();
    }

    if (nextSegment == 0) {
        score()->startCmd();
        if (fromLyrics) {
            switch (fromLyrics->syllabic()) {
            case Ms::Lyrics::Syllabic::SINGLE:
            case Ms::Lyrics::Syllabic::END:
                break;
            default:
                fromLyrics->undoChangeProperty(Ms::Pid::SYLLABIC, int(Ms::Lyrics::Syllabic::END));
                break;
            }
            if (fromLyrics->segment()->tick() < endTick) {
                fromLyrics->undoChangeProperty(Ms::Pid::LYRIC_TICKS, endTick - fromLyrics->segment()->tick());
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
    Ms::Lyrics* toLyrics = cr->lyrics(verse, placement);
    bool newLyrics = (toLyrics == 0);
    if (!toLyrics) {
        toLyrics = new Ms::Lyrics(score());
        toLyrics->setTrack(track);
        toLyrics->setParent(nextSegment->element(track));
        toLyrics->setNo(verse);
        toLyrics->setPlacement(placement);
        toLyrics->setPropertyFlags(Ms::Pid::PLACEMENT, pFlags);
        toLyrics->setSyllabic(Ms::Lyrics::Syllabic::SINGLE);
    }
    // as we arrived at toLyrics by an underscore, it cannot have syllabic dashes before
    else if (toLyrics->syllabic() == Ms::Lyrics::Syllabic::MIDDLE) {
        toLyrics->undoChangeProperty(Ms::Pid::SYLLABIC, int(Ms::Lyrics::Syllabic::BEGIN));
    } else if (toLyrics->syllabic() == Ms::Lyrics::Syllabic::END) {
        toLyrics->undoChangeProperty(Ms::Pid::SYLLABIC, int(Ms::Lyrics::Syllabic::SINGLE));
    }
    if (fromLyrics) {
        // as we moved away from fromLyrics by an underscore,
        // it can be isolated or terminal but cannot have dashes after
        switch (fromLyrics->syllabic()) {
        case Ms::Lyrics::Syllabic::SINGLE:
        case Ms::Lyrics::Syllabic::END:
            break;
        default:
            fromLyrics->undoChangeProperty(Ms::Pid::SYLLABIC, int(Ms::Lyrics::Syllabic::END));
            break;
        }
        // for the same reason, if it has a melisma, this cannot extend beyond toLyrics
        if (fromLyrics->segment()->tick() < endTick) {
            fromLyrics->undoChangeProperty(Ms::Pid::LYRIC_TICKS, endTick - fromLyrics->segment()->tick());
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
    if (!m_textEditData.element || !m_textEditData.element->isLyrics()) {
        qWarning("nextLyricVerse called with invalid current element");
        return;
    }
    Ms::Lyrics* lyrics = toLyrics(m_textEditData.element);

    endEditText();

    score()->startCmd();
    int newVerse;
    newVerse = lyrics->no() + 1;

    Ms::Lyrics* oldLyrics = lyrics;
    lyrics = new Ms::Lyrics(score());
    lyrics->setTrack(oldLyrics->track());
    lyrics->setParent(oldLyrics->segment()->element(oldLyrics->track()));
    lyrics->setPlacement(oldLyrics->placement());
    lyrics->setPropertyFlags(Ms::Pid::PLACEMENT, oldLyrics->propertyFlags(Ms::Pid::PLACEMENT));
    lyrics->setNo(newVerse);

    score()->undoAddElement(lyrics);
    score()->endCmd();

    score()->select(lyrics, SelectType::SINGLE, 0);
    startEditText(lyrics, PointF());
}
