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
#include "notationinteraction.h"

#include "log.h"

#include <memory>
#include <limits>
#include <QRectF>
#include <QPainter>
#include <QClipboard>
#include <QApplication>
#include <QKeyEvent>
#include <QMimeData>
#include <QDrag>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

#include "defer.h"
#include "ptrutils.h"
#include "containers.h"

#include "draw/painter.h"
#include "draw/types/painterpath.h"
#include "draw/types/pen.h"
#include "engraving/internal/qmimedataadapter.h"

#include "engraving/dom/actionicon.h"
#include "engraving/dom/anchors.h"
#include "engraving/dom/articulation.h"
#include "engraving/dom/barline.h"
#include "engraving/dom/box.h"
#include "engraving/dom/bracket.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/drumset.h"
#include "engraving/dom/dynamic.h"
#include "engraving/dom/elementgroup.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/figuredbass.h"
#include "engraving/dom/guitarbend.h"
#include "engraving/dom/image.h"
#include "engraving/dom/instrchange.h"
#include "engraving/dom/gradualtempochange.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/lasso.h"
#include "engraving/dom/layoutbreak.h"
#include "engraving/dom/linkedobjects.h"
#include "engraving/dom/lyrics.h"
#include "engraving/dom/marker.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/navigate.h"
#include "engraving/dom/page.h"
#include "engraving/dom/part.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/shadownote.h"
#include "engraving/dom/slur.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafflines.h"
#include "engraving/dom/stafftype.h"
#include "engraving/dom/stafftypechange.h"
#include "engraving/dom/system.h"
#include "engraving/dom/textedit.h"
#include "engraving/dom/tuplet.h"
#include "engraving/dom/undo.h"
#include "engraving/dom/utils.h"
#include "engraving/compat/dummyelement.h"
#include "engraving/dom/utils.h"

#include "engraving/rw/xmlreader.h"
#include "engraving/rw/rwregister.h"

#include "mscoreerrorscontroller.h"
#include "notationerrors.h"
#include "notation.h"
#include "notationnoteinput.h"
#include "notationselection.h"
#include "scorecallbacks.h"

#include "utilities/scorerangeutilities.h"

using namespace mu::notation;
using namespace mu::engraving;
using namespace muse;
using namespace muse::draw;
using namespace muse::io;

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
    if (cursorPos.isNull() || !text || !text->isTextBase()) {
        return PointF();
    }

    muse::RectF bbox = text->canvasBoundingRect();
    muse::PointF boundPos = muse::PointF(
        cursorPos.x() < bbox.left() ? bbox.left()
        : cursorPos.x() >= bbox.right() ? bbox.right() - 1 : cursorPos.x(),
        cursorPos.y() < bbox.top() ? bbox.top()
        : cursorPos.y() >= bbox.bottom() ? bbox.bottom() - 1 : cursorPos.y());

    return boundPos;
}

inline QString extractSyllable(const QString& text)
{
    QString _text = text;

    _text.replace(QRegularExpression("\r+"), "\n");
    _text.replace(QRegularExpression("\n+"), "\n");
    if (_text.startsWith(u"\n")) {
        _text.remove("\n");
    }

    int textPos = _text.indexOf(QRegularExpression("\\S"));
    if (textPos == -1) {
        return QString();
    }

    QRegularExpressionMatch match;
    int splitPos = _text.indexOf(QRegularExpression("(_| |-|\n)"), textPos, &match);
    if (splitPos == -1) {
        splitPos = _text.size();
    } else {
        splitPos += match.capturedLength();
    }

    return _text.mid(textPos, splitPos - textPos);
}

NotationInteraction::NotationInteraction(Notation* notation, INotationUndoStackPtr undoStack)
    : muse::Injectable(notation->iocContext()), m_notation(notation), m_undoStack(undoStack), m_editData(&m_scoreCallbacks)
{
    m_noteInput = std::make_shared<NotationNoteInput>(notation, this, m_undoStack, iocContext());
    m_selection = std::make_shared<NotationSelection>(notation);

    m_noteInput->stateChanged().onNotify(this, [this]() {
        if (!m_noteInput->isNoteInputMode()) {
            hideShadowNote();
        }
    });

    m_undoStack->undoRedoNotification().onNotify(this, [this]() {
        endEditElement();
    });

    m_undoStack->stackChanged().onNotify(this, [this]() {
        notifyAboutSelectionChangedIfNeed();
        notifyAboutNoteInputStateChanged();
    });

    m_dragData.ed = mu::engraving::EditData(&m_scoreCallbacks);

    m_scoreCallbacks.setNotationInteraction(this);

    m_notation->scoreInited().onNotify(this, [this]() {
        onScoreInited();
    });

    m_notation->viewModeChanged().onNotify(this, [this]() {
        onViewModeChanged();
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

void NotationInteraction::onViewModeChanged()
{
    if (!score()) {
        return;
    }

    if (!score()->isLayoutMode(LayoutMode::LINE) && !score()->isLayoutMode(LayoutMode::HORIZONTAL_FIXED)) {
        return;
    }

    // VBoxes are not included in horizontal layouts - deselect them (and their contents) when switching to horizontal mode...
    const std::vector<EngravingItem*> sel = selection()->elements();
    for (EngravingItem* item : sel) {
        if (!item->findAncestor(ElementType::VBOX)) {
            continue;
        }
        score()->deselect(item);
        if (item == m_editData.element) {
            endEditElement();
        }
    }
}

void NotationInteraction::startEdit(const muse::TranslatableString& actionName)
{
    m_notifyAboutDropChanged = false;

    m_undoStack->prepareChanges(actionName);
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

void NotationInteraction::notifyAboutTextEditingEnded(TextBase* text)
{
    m_textEditingEnded.send(text);
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

void NotationInteraction::paint(Painter* painter)
{
    if (shouldDrawInputPreview()) {
        drawInputPreview(painter);
    }

    score()->renderer()->drawItem(score()->shadowNote(), painter);

    drawAnchorLines(painter);
    drawTextEditMode(painter);
    drawSelectionRange(painter);
    drawGripPoints(painter);
    drawLasso(painter);
    drawDrop(painter);
}

INotationNoteInputPtr NotationInteraction::noteInput() const
{
    return m_noteInput;
}

bool NotationInteraction::showShadowNote(const PointF& pos)
{
    const mu::engraving::InputState& inputState = score()->inputState();
    mu::engraving::ShadowNote& shadowNote = *score()->shadowNote();

    mu::engraving::Position position;
    if (!score()->getPosition(&position, pos, inputState.voice())) {
        shadowNote.setVisible(false);
        return false;
    }

    ShadowNoteParams params;
    params.duration = inputState.duration();
    params.accidentalType = inputState.accidentalType();
    params.articulationIds = inputState.articulationIds();

    showShadowNoteAtPosition(shadowNote, params, position);
    return true;
}

void NotationInteraction::showShadowNoteAtPosition(ShadowNote& shadowNote, const ShadowNoteParams& params, Position& position)
{
    const mu::engraving::InputState& inputState = score()->inputState();
    const Staff* staff = score()->staff(position.staffIdx);
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
    position.pos.rx() -= qMin(relX - score()->style().styleMM(mu::engraving::Sid::barNoteDistance) * mag, 0.0);

    mu::engraving::NoteHeadGroup noteheadGroup = mu::engraving::NoteHeadGroup::HEAD_NORMAL;
    mu::engraving::NoteHeadType noteHead = params.duration.headType();
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
    shadowNote.mutldata()->setMag(mag);
    shadowNote.setTick(tick);
    shadowNote.setStaffIdx(position.staffIdx);
    shadowNote.setVoice(voice);
    shadowNote.setLineIndex(line);

    Color color = configuration()->noteInputPreviewColor();

    if (color.isValid() && color != configuration()->selectionColor()) {
        shadowNote.setColor(color);
    } else {
        shadowNote.setColor(configuration()->selectionColor(voice));
    }

    mu::engraving::SymId symNotehead;

    if (inputState.rest()) {
        mu::engraving::Rest* rest = mu::engraving::Factory::createRest(
            mu::engraving::gpaletteScore->dummy()->segment(), params.duration.type());
        rest->setTicks(params.duration.fraction());
        symNotehead = rest->getSymbol(params.duration.type(), 0, staff->lines(position.segment->tick()));
        shadowNote.setState(symNotehead, params.duration, true, segmentSkylineTopY, segmentSkylineBottomY);
        delete rest;
    } else {
        if (mu::engraving::NoteHeadGroup::HEAD_CUSTOM == noteheadGroup) {
            symNotehead = instr->drumset()->noteHeads(inputState.drumNote(), noteHead);
        } else {
            symNotehead = Note::noteHead(0, noteheadGroup, noteHead);
        }

        shadowNote.setState(symNotehead, params.duration, false, segmentSkylineTopY, segmentSkylineBottomY,
                            params.accidentalType, params.articulationIds);
    }

    score()->renderer()->layoutItem(&shadowNote);

    shadowNote.setPos(position.pos);
}

void NotationInteraction::hideShadowNote()
{
    score()->shadowNote()->setVisible(false);
}

RectF NotationInteraction::shadowNoteRect() const
{
    const ShadowNote* note = score()->shadowNote();
    if (!note) {
        return RectF();
    }

    RectF rect = note->canvasBoundingRect();

    double penWidth = note->style().styleMM(Sid::stemWidth);
    if (note->ledgerLinesVisible()) {
        double w = note->style().styleMM(Sid::ledgerLineWidth);
        penWidth = std::max(penWidth, w);
    }

    penWidth *= note->mag();
    rect.adjust(-penWidth, -penWidth, penWidth, penWidth);

    return rect;
}

void NotationInteraction::toggleVisible()
{
    startEdit(TranslatableString("undoableAction", "Toggle visible"));
    score()->cmdToggleVisible();
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

mu::engraving::Page* NotationInteraction::point2page(const PointF& p, bool useNearestPage) const
{
    if (score()->linearMode()) {
        return score()->pages().empty() ? nullptr : score()->pages().front();
    }

    double shortestDistance = std::numeric_limits<double>::max();
    Page* nearestPage = nullptr;
    for (mu::engraving::Page* page : score()->pages()) {
        auto pageRect = page->ldata()->bbox().translated(page->pos());
        if (pageRect.contains(p)) {
            return page;
        } else if (useNearestPage && pageRect.distanceTo(p) < shortestDistance) {
            shortestDistance = pageRect.distanceTo(p);
            nearestPage = page;
        }
    }
    return nearestPage;
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

std::vector<EngravingItem*> NotationInteraction::hitElements(const PointF& pos, float width) const
{
    mu::engraving::Page* page = point2page(pos);
    if (!page) {
        return {};
    }

    std::vector<EngravingItem*> hitElements;

    PointF posOnPage = pos - page->pos();

    if (isTextEditingStarted()) {
        auto editW = width * 2;
        RectF editHitRect(posOnPage.x() - editW, posOnPage.y() - editW, 2.0 * editW, 2.0 * editW);
        if (m_editData.element->intersects(editHitRect)) {
            hitElements.push_back(m_editData.element);
            return hitElements;
        }
    }

    RectF hitRect(posOnPage.x() - width, posOnPage.y() - width, 3.0 * width, 3.0 * width);

    std::vector<EngravingItem*> potentiallyHitElements = page->items(hitRect);

    for (int i = 0; i < engraving::MAX_HEADERS; ++i) {
        if (score()->headerText(i) != nullptr) { // gives the ability to select the header
            potentiallyHitElements.push_back(score()->headerText(i));
        }
    }

    for (int i = 0; i < engraving::MAX_FOOTERS; ++i) {
        if (score()->footerText(i) != nullptr) { // gives the ability to select the footer
            potentiallyHitElements.push_back(score()->footerText(i));
        }
    }

    auto canHitElement = [](const EngravingItem* element) {
        if (!element->selectable() || element->isPage()) {
            return false;
        }

        if (element->isSoundFlag()) {
            return !toSoundFlag(element)->shouldHide();
        } else if (!element->isInteractionAvailable()) {
            return false;
        }

        return true;
    };

    for (EngravingItem* element : potentiallyHitElements) {
        element->itemDiscovered = 0;

        if (!canHitElement(element)) {
            continue;
        }

        if (element->hitShapeContains(posOnPage)) {
            hitElements.push_back(element);
        }
    }

    if (hitElements.empty() || (hitElements.size() == 1 && hitElements.front()->isMeasure())) {
        //
        // if no relevant element hit, look nearby
        //
        for (EngravingItem* element : potentiallyHitElements) {
            if (!canHitElement(element)) {
                continue;
            }

            if (element->hitShapeIntersects(hitRect)) {
                hitElements.push_back(element);
            }
        }
    }

    if (!hitElements.empty()) {
        std::sort(hitElements.begin(), hitElements.end(), NotationInteraction::elementIsLess);
    } else {
        Measure* measure = hitMeasure(pos).measure;
        if (measure) {
            hitElements.push_back(measure);
        }
    }

    return hitElements;
}

NotationInteraction::HitMeasureData NotationInteraction::hitMeasure(const PointF& pos) const
{
    mu::engraving::staff_idx_t staffIndex = muse::nidx;
    mu::engraving::Segment* segment = nullptr;
    PointF offset;
    Measure* measure = score()->pos2measure(pos, &staffIndex, 0, &segment, &offset);

    HitMeasureData result;
    if (measure && measure->staffLines(staffIndex)->canvasHitShape().contains(pos)) {
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
    if (e1->isImage() && e2->isBox()) {
        return false;
    }
    if (e1->isBox() && e2->isImage()) {
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
    } else if (needEndElementEditing(elements)) {
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

    score()->select(elements, type, staffIndex);
}

void NotationInteraction::selectElementsWithSameTypeOnSegment(mu::engraving::ElementType elementType, mu::engraving::Segment* segment)
{
    TRACEFUNC;

    IF_ASSERT_FAILED(segment) {
        return;
    }

    score()->deselectAll();

    std::vector<EngravingItem*> elementsToSelect;

    for (size_t staffIdx = 0; staffIdx < score()->nstaves(); ++staffIdx) {
        EngravingItem* element = segment->element(staffIdx * mu::engraving::VOICES);
        if (element && element->type() == elementType) {
            elementsToSelect.push_back(element);
        }
    }

    score()->select(elementsToSelect, SelectType::ADD);
}

void NotationInteraction::selectAndStartEditIfNeeded(EngravingItem* element)
{
    if (element->isSpanner() && !toSpanner(element)->segmentsEmpty()) {
        SpannerSegment* frontSeg = toSpanner(element)->frontSegment();
        doSelect({ frontSeg }, SelectType::SINGLE);
        if (frontSeg->needStartEditingAfterSelecting()) {
            startEditElement(frontSeg, false);
        }
    } else {
        doSelect({ element }, SelectType::SINGLE);
        if (element->needStartEditingAfterSelecting()) {
            startEditElement(element, false);
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
        showItem(element);
    }
}

void NotationInteraction::selectLastElement()
{
    if (EngravingItem* element = score()->lastElement()) {
        select({ element }, SelectType::SINGLE, element->staffIdx());
        showItem(element);
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

muse::async::Notification NotationInteraction::selectionChanged() const
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
            Spanner* s = toSpanner(e);
            draggable = !s->segmentsEmpty() && isDraggable(s->frontSegment());
        }

        if (!draggable) {
            continue;
        }

        std::unique_ptr<mu::engraving::ElementGroup> g = e->getDragGroup(isDraggable);
        if (g && g->enabled()) {
            m_dragData.dragGroups.push_back(std::move(g));
        }
    }

    startEdit(TranslatableString("undoableAction", "Drag element"));

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
    score()->lassoSelect(m_lasso->ldata()->bbox());
    score()->update();
}

void NotationInteraction::endLasso()
{
    if (!m_lasso) {
        return;
    }

    score()->addRefresh(m_lasso->canvasBoundingRect());
    m_lasso->setbbox(RectF());
    score()->lassoSelectEnd();
    score()->update();
}

void NotationInteraction::drag(const PointF& fromPos, const PointF& toPos, DragMode mode)
{
    if (m_dragData.beginMove.isNull()) {
        m_dragData.beginMove = fromPos;
        m_dragData.ed.pos = fromPos;
    }

    PointF normalizedBegin = m_dragData.beginMove - m_dragData.elementOffset;
    PointF delta = toPos - normalizedBegin;
    PointF evtDelta = toPos - m_dragData.ed.pos;
    PointF moveDelta = delta - m_dragData.elementOffset;

    bool constrainDirection = !(isGripEditStarted() && m_editData.element && m_editData.element->isBarLine());
    if (constrainDirection) {
        switch (mode) {
        case DragMode::BothXY:
            break;
        case DragMode::OnlyX:
            delta.setY(m_dragData.ed.delta.y());
            moveDelta.setY(m_dragData.ed.moveDelta.y());
            evtDelta.setY(0.0);
            break;
        case DragMode::OnlyY:
            delta.setX(m_dragData.ed.delta.x());
            moveDelta.setX(m_dragData.ed.moveDelta.x());
            evtDelta.setX(0.0);
            break;
        }
    }

    m_dragData.ed.lastPos = m_dragData.ed.pos;

    m_dragData.ed.hRaster = configuration()->isSnappedToGrid(muse::Orientation::Horizontal);
    m_dragData.ed.vRaster = configuration()->isSnappedToGrid(muse::Orientation::Vertical);
    m_dragData.ed.delta = delta;
    m_dragData.ed.moveDelta = moveDelta;
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
        m_dragData.ed.delta = evtDelta;
        m_dragData.ed.moveDelta = m_dragData.ed.delta - m_dragData.elementOffset;
        m_dragData.ed.addData(m_editData.getData(m_editData.element));
        m_editData.element->editDrag(m_dragData.ed);

        if (m_editData.element->isDynamic()) {
            // When the dynamic has no left grip, the right grip will have index zero, a.k.a. Grip::LEFT.
            // TODO: refactor all code that works with Grips, so that this is not necessary
            Dynamic* dynamic = toDynamic(m_editData.element);
            bool isLeftGrip = dynamic->hasLeftGrip() ? m_editData.curGrip == Grip::LEFT : false;
            addHairpinOnGripDrag(toDynamic(m_editData.element), isLeftGrip);
        }
    } else {
        if (m_editData.element) {
            m_editData.element->editDrag(m_dragData.ed);
        }
        for (auto& group : m_dragData.dragGroups) {
            score()->addRefresh(group->drag(m_dragData.ed));
        }
    }

    score()->update();

    if (isGripEditStarted()) {
        if (m_editData.element->isDynamic() && !m_editData.isStartEndGrip()) {
            updateDragAnchorLines();
        } else {
            updateGripAnchorLines();
        }
    } else {
        updateDragAnchorLines();
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

    MScoreErrorsController(iocContext()).checkAndShowMScoreError();

    //    updateGrips();
    //    if (editData.element->normalModeEditBehavior() == EngravingItem::EditBehavior::Edit
    //        && score()->selection().element() == editData.element) {
    //        startEdit(/* editMode */ false);
    //    }
}

muse::async::Notification NotationInteraction::dragChanged() const
{
    return m_dragChanged;
}

bool NotationInteraction::isOutgoingDragElementAllowed(const EngravingItem* element) const
{
    if (!element) {
        return false;
    }

    switch (element->type()) {
    case ElementType::MEASURE:
    case ElementType::NOTE:
    case ElementType::VBOX:
    case ElementType::HBOX:
    case ElementType::TBOX:
    case ElementType::FBOX:
    // TODO: Bends can't be copy-dragged until corresponding SingleLayout::layout and SingleDraw::draw methods have been implemented
    case ElementType::GUITAR_BEND:
    case ElementType::GUITAR_BEND_SEGMENT:
    case ElementType::GUITAR_BEND_HOLD:
    case ElementType::GUITAR_BEND_HOLD_SEGMENT:
    case ElementType::GUITAR_BEND_TEXT:
        return false;
    default: return true;
    }
}

//! NOTE: Copied from ScoreView::cloneElement
void NotationInteraction::startOutgoingDragElement(const EngravingItem* element, QObject* dragSource)
{
    if (isDragStarted()) {
        endOutgoingDrag();
    }

    if (element->isSpannerSegment()) {
        element = toSpannerSegment(element)->spanner();
    }

    QMimeData* mimeData = new QMimeData();
    mimeData->setData(mu::engraving::mimeSymbolFormat, element->mimeData().toQByteArray());

    m_outgoingDrag = new QDrag(dragSource);
    m_outgoingDrag->setMimeData(mimeData);

    QObject::connect(m_outgoingDrag, &QDrag::destroyed, [this]() {
        m_outgoingDrag = nullptr;
    });

    const qreal adjustedRatio = 0.4;
    const RectF bbox = element->ldata()->bbox();
    const qreal width = bbox.width();
    const qreal height = bbox.height();

    QSize pixmapSize = QSize(width * adjustedRatio, height * adjustedRatio);
    QPixmap pixmap(pixmapSize);
    pixmap.fill(Qt::transparent);

    QPainter qp(&pixmap);
    const qreal dpi = qp.device()->logicalDpiX();

    Painter p(&qp, "prepareDragCopyElement");
    p.setAntialiasing(true);

    mu::engraving::MScore::pixelRatio = mu::engraving::DPI / dpi;
    p.translate(qAbs(bbox.x() * adjustedRatio), qAbs(bbox.y() * adjustedRatio));
    p.scale(adjustedRatio, adjustedRatio);
    engravingRenderer()->drawItem(element, &p);

    m_outgoingDrag->setPixmap(pixmap);

    m_outgoingDrag->exec(Qt::CopyAction);
}

void NotationInteraction::startOutgoingDragRange(QObject* dragSource)
{
    if (isDragStarted()) {
        endOutgoingDrag();
    }

    if (!selection()->isRange() || !selection()->canCopy()) {
        return;
    }

    QMimeData* mimeData = selection()->mimeData();
    if (!mimeData) {
        return;
    }

    m_outgoingDrag = new QDrag(dragSource);
    m_outgoingDrag->setMimeData(mimeData);

    QObject::connect(m_outgoingDrag, &QDrag::destroyed, [this]() {
        m_outgoingDrag = nullptr;
    });

    QPixmap pixmap(1, 1);
    pixmap.fill(Qt::transparent);
    m_outgoingDrag->setPixmap(pixmap);

    m_outgoingDrag->exec(Qt::MoveAction | Qt::CopyAction);
}

bool NotationInteraction::isOutgoingDragStarted() const
{
    return m_outgoingDrag != nullptr;
}

void NotationInteraction::endOutgoingDrag()
{
    if (m_outgoingDrag) {
        delete m_outgoingDrag;
        m_outgoingDrag = nullptr;
    }
}

//! NOTE Copied from ScoreView::dragEnterEvent
bool NotationInteraction::startDropSingle(const QByteArray& edata)
{
    resetDropData();

    m_dropData.elementDropData = ElementDropData();
    ElementDropData& edd = m_dropData.elementDropData.value();
    edd.ed = mu::engraving::EditData(&m_scoreCallbacks);

    mu::engraving::XmlReader e(edata);
    edd.ed.dragOffset = QPointF();
    Fraction duration;      // dummy
    ElementType type = EngravingItem::readType(e, &edd.ed.dragOffset, &duration);

    EngravingItem* el = engraving::Factory::createItem(type, score()->dummy());
    if (el) {
        if (type == ElementType::BAR_LINE || type == ElementType::BRACKET) {
            double spatium = score()->style().spatium();
            el->setHeight(spatium * 5);
        }
        edd.ed.dropElement = el;

        rw::RWRegister::reader()->readItem(edd.ed.dropElement, e);

        engravingRenderer()->layoutItem(edd.ed.dropElement);
        return true;
    }

    resetDropData();
    return false;
}

bool NotationInteraction::startDropRange(const QByteArray& data)
{
    resetDropData();

    m_dropData.rangeDropData = RangeDropData();
    RangeDropData& rdd = m_dropData.rangeDropData.value();

    mu::engraving::XmlReader reader(data);
    while (reader.readNextStartElement()) {
        if (reader.name() == "StaffList") {
            rdd.sourceTick = Fraction::fromString(reader.attribute("tick"));
            rdd.tickLength = Fraction::fromString(reader.attribute("len"));
            rdd.sourceStaffIdx = static_cast<staff_idx_t>(reader.intAttribute("staff", -1));
            rdd.numStaves = reader.intAttribute("staves", 0);
            break;
        }
    }

    if (rdd.tickLength.isZero() || rdd.numStaves == 0) {
        resetDropData();
        return false;
    }

    return true;
}

bool NotationInteraction::startDropImage(const QUrl& url)
{
    if (url.scheme() != "file") {
        return false;
    }

    auto image = static_cast<mu::engraving::Image*>(Factory::createItem(mu::engraving::ElementType::IMAGE, score()->dummy()));
    if (!image->load(url.toLocalFile())) {
        return false;
    }

    resetDropData();

    m_dropData.elementDropData = ElementDropData();
    ElementDropData& edd = m_dropData.elementDropData.value();
    edd.ed = mu::engraving::EditData(&m_scoreCallbacks);

    edd.ed.dropElement = image;
    edd.ed.dragOffset = QPointF();
    edd.ed.dropElement->setParent(nullptr);

    engravingRenderer()->layoutItem(edd.ed.dropElement);

    return true;
}

//! NOTE Copied from ScoreView::dragMoveEvent
bool NotationInteraction::isDropSingleAccepted(const PointF& pos, Qt::KeyboardModifiers modifiers)
{
    if (!m_dropData.elementDropData.has_value()) {
        return false;
    }

    ElementDropData& edd = m_dropData.elementDropData.value();

    EngravingItem* dropElem = edd.ed.dropElement;
    if (!dropElem) {
        return false;
    }

    switch (dropElem->type()) {
    case ElementType::BRACKET:
    case ElementType::GRADUAL_TEMPO_CHANGE:
    case ElementType::JUMP:
    case ElementType::KEYSIG:
    case ElementType::LAYOUT_BREAK:
    case ElementType::MARKER:
    case ElementType::MEASURE_REPEAT:
    case ElementType::MEASURE_NUMBER:
    case ElementType::TIMESIG:
    case ElementType::SPACER:
    case ElementType::STAFFTYPE_CHANGE:
    case ElementType::STRING_TUNINGS:
    case ElementType::VOLTA: {
        edd.ed.modifiers = keyboardModifier(modifiers);
        return prepareDropMeasureAnchorElement(pos);
    }
    case ElementType::PEDAL:
    case ElementType::LET_RING:
    case ElementType::VIBRATO:
    case ElementType::PALM_MUTE:
    case ElementType::OTTAVA:
    case ElementType::TRILL:
    case ElementType::HAIRPIN:
    case ElementType::TEXTLINE: {
        edd.ed.modifiers = keyboardModifier(modifiers);
        return prepareDropTimeAnchorElement(pos);
    }
    case ElementType::IMAGE:
    case ElementType::SYMBOL:
    case ElementType::FSYMBOL:
    case ElementType::DYNAMIC:
    case ElementType::CLEF:
    case ElementType::BAR_LINE:
    case ElementType::ARPEGGIO:
    case ElementType::BREATH:
    case ElementType::GLISSANDO:
    case ElementType::ARTICULATION:
    case ElementType::FERMATA:
    case ElementType::CHORDLINE:
    case ElementType::BEND:
    case ElementType::ACCIDENTAL:
    case ElementType::TEXT:
    case ElementType::FINGERING:
    case ElementType::TEMPO_TEXT:
    case ElementType::ORNAMENT:
    case ElementType::EXPRESSION:
    case ElementType::STAFF_TEXT:
    case ElementType::SYSTEM_TEXT:
    case ElementType::TRIPLET_FEEL:
    case ElementType::PLAYTECH_ANNOTATION:
    case ElementType::CAPO:
    case ElementType::NOTEHEAD:
    case ElementType::TREMOLO_SINGLECHORD:
    case ElementType::TREMOLO_TWOCHORD:
    case ElementType::STAFF_STATE:
    case ElementType::INSTRUMENT_CHANGE:
    case ElementType::REHEARSAL_MARK:
    case ElementType::CHORD:
    case ElementType::SLUR:
    case ElementType::HARMONY:
    case ElementType::BAGPIPE_EMBELLISHMENT:
    case ElementType::AMBITUS:
    case ElementType::TREMOLOBAR:
    case ElementType::FIGURED_BASS:
    case ElementType::LYRICS:
    case ElementType::FRET_DIAGRAM:
    case ElementType::HARP_DIAGRAM: {
        return prepareDropStandardElement(pos, modifiers);
    }
    //! NOTE: See Measure::acceptDrop
    case ElementType::ACTION_ICON: {
        switch (toActionIcon(dropElem)->actionType()) {
        case ActionIconType::VFRAME:
        case ActionIconType::HFRAME:
        case ActionIconType::TFRAME:
        case ActionIconType::FFRAME:
        case ActionIconType::MEASURE:
        case ActionIconType::SYSTEM_LOCK:
        case ActionIconType::STAFF_TYPE_CHANGE: {
            edd.ed.modifiers = keyboardModifier(modifiers);
            return prepareDropMeasureAnchorElement(pos);
        }
        // Other action icons (e.g parenthesis) can be dragged normally
        default: return prepareDropStandardElement(pos, modifiers);
        }
    }
    default:
        break;
    }

    return false;
}

bool NotationInteraction::isDropRangeAccepted(const PointF& pos)
{
    IF_ASSERT_FAILED(m_dropData.rangeDropData.has_value()) {
        return false;
    }

    RangeDropData& rdd = m_dropData.rangeDropData.value();

    staff_idx_t staffIdx = muse::nidx;
    Segment* segment = nullptr;
    static constexpr double spacingFactor = 0.5;
    static constexpr bool useTimeAnchors = true;

    score()->dragPosition(pos, &staffIdx, &segment, spacingFactor, useTimeAnchors);

    if (staffIdx == muse::nidx || !segment) {
        return false;
    }

    rdd.targetSegment = segment;
    rdd.targetStaffIdx = staffIdx;

    const Segment* endSegment = score()->tick2rightSegment(segment->tick() + rdd.tickLength,
                                                           true,
                                                           Segment::CHORD_REST_OR_TIME_TICK_TYPE);

    if (endSegment && !endSegment->enabled()) {
        endSegment = endSegment->next1MMenabled();
    }

    if (!endSegment) {
        endSegment = score()->lastSegmentMM();
    }

    if (!endSegment) {
        return false;
    }

    const staff_idx_t endStaffIdx = staffIdx + rdd.numStaves;

    rdd.dropRects = ScoreRangeUtilities::boundingArea(score(),
                                                      segment, endSegment,
                                                      staffIdx, endStaffIdx);

    notifyAboutDragChanged();

    return true;
}

//! NOTE Copied from ScoreView::dropEvent
bool NotationInteraction::dropSingle(const PointF& pos, Qt::KeyboardModifiers modifiers)
{
    if (!m_dropData.elementDropData.has_value()) {
        return false;
    }

    ElementDropData& edd = m_dropData.elementDropData.value();

    if (!edd.ed.dropElement) {
        return false;
    }

    IF_ASSERT_FAILED(edd.ed.dropElement->score() == score()) {
        return false;
    }

    bool accepted = false;

    // If the drop position hasn't been set already through isDropAccepted's helper methods, use the mouse position
    if (edd.ed.pos.isNull()) {
        edd.ed.pos = pos;
    }

    edd.ed.modifiers = keyboardModifier(modifiers);
    edd.ed.dropElement->styleChanged();

    bool systemStavesOnly = false;
    bool applyUserOffset = false;

    startEdit(TranslatableString("undoableAction", "Drop element"));
    score()->addRefresh(edd.ed.dropElement->canvasBoundingRect());
    ElementType et = edd.ed.dropElement->type();
    switch (et) {
    case ElementType::TEXTLINE:
        systemStavesOnly = edd.ed.dropElement->systemFlag();
        [[fallthrough]];
    case ElementType::VOLTA:
    case ElementType::GRADUAL_TEMPO_CHANGE:
        // voltas drop to system staves by default, or closest staff if Control is held
        systemStavesOnly = systemStavesOnly || !(edd.ed.modifiers & Qt::ControlModifier);
        [[fallthrough]];
    case ElementType::OTTAVA:
    case ElementType::TRILL:
    case ElementType::PEDAL:
    case ElementType::LET_RING:
    case ElementType::VIBRATO:
    case ElementType::PALM_MUTE:
    case ElementType::HAIRPIN:
    {
        mu::engraving::Spanner* spanner = ptr::checked_cast<mu::engraving::Spanner>(edd.ed.dropElement);
        score()->cmdAddSpanner(spanner, pos, systemStavesOnly);
        score()->setUpdateAll();
        accepted = true;
    }
    break;
    case ElementType::SYMBOL:
    case ElementType::FSYMBOL:
    case ElementType::IMAGE:
        applyUserOffset = true;
        [[fallthrough]];
    case ElementType::DYNAMIC:
    case ElementType::FRET_DIAGRAM:
    case ElementType::HARMONY:
        accepted = doDropTextBaseAndSymbols(pos, applyUserOffset);
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
    case ElementType::ORNAMENT:
    case ElementType::EXPRESSION:
    case ElementType::STAFF_TEXT:
    case ElementType::SYSTEM_TEXT:
    case ElementType::TRIPLET_FEEL:
    case ElementType::PLAYTECH_ANNOTATION:
    case ElementType::CAPO:
    case ElementType::STRING_TUNINGS:
    case ElementType::NOTEHEAD:
    case ElementType::TREMOLO_SINGLECHORD:
    case ElementType::TREMOLO_TWOCHORD:
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
    case ElementType::HARP_DIAGRAM:
        accepted = doDropStandard();
        break;
    case ElementType::STAFFTYPE_CHANGE: {
        EngravingItem* el = dropTarget(edd.ed);
        if (el->isMeasure()) {
            Measure* m = toMeasure(el);
            System* s = m->system();
            double y = pos.y() - s->canvasPos().y();
            staff_idx_t staffIndex = s->searchStaff(y);
            StaffTypeChange* stc = toStaffTypeChange(edd.ed.dropElement);
            score()->cmdAddStaffTypeChange(toMeasure(el), staffIndex, stc);
        }
    }
    break;
    case ElementType::SLUR:
    {
        EngravingItem* el = dropTarget(edd.ed);
        mu::engraving::Slur* dropElement = toSlur(edd.ed.dropElement);
        if (toNote(el)->chord()) {
            doAddSlur(toNote(el)->chord(), nullptr, dropElement);
            accepted = true;
        }
    }
    break;
    default:
        resetDropData();
        break;
    }

    edd.ed.dropElement = nullptr;
    edd.ed.pos = PointF();
    edd.ed.modifiers = {};

    setDropTarget(nullptr); // this also resets dropRectangle and dropAnchor
    apply();

    if (accepted) {
        notifyAboutDropChanged();
    }

    MScoreErrorsController(iocContext()).checkAndShowMScoreError();

    return accepted;
}

//! NOTE: Helper method for NotationInteraction::drop. Handles drop logic for majority of elements (returns "accepted")
bool NotationInteraction::doDropStandard()
{
    if (!m_dropData.elementDropData.has_value()) {
        return false;
    }

    ElementDropData& edd = m_dropData.elementDropData.value();

    EngravingItem* el = edd.dropTarget ? edd.dropTarget : dropTarget(edd.ed);
    if (!el) {
        if (!dropCanvas(edd.ed.dropElement)) {
            LOGD("cannot drop %s(%p) to canvas", edd.ed.dropElement->typeName(), edd.ed.dropElement);
            resetDropData();
        }
        return false;
    }
    score()->addRefresh(el->canvasBoundingRect());

    // TODO: HACK ALERT!
    if (el->isMeasure() && edd.ed.dropElement->isLayoutBreak()) {
        Measure* m = toMeasure(el);
        if (m->isMMRest()) {
            el = m->mmRestLast();
        }
    }

    EngravingItem* dropElement = el->drop(edd.ed);

    if (dropElement && dropElement->isInstrumentChange()) {
        if (!selectInstrument(toInstrumentChange(dropElement))) {
            rollback();
            return true; // Really?
        }
    }

    score()->addRefresh(el->canvasBoundingRect());
    if (dropElement) {
        if (!score()->noteEntryMode()) {
            selectAndStartEditIfNeeded(dropElement);
        }
        score()->addRefresh(dropElement->canvasBoundingRect());
    }
    return true;
}

//! NOTE: Helper method for NotationInteraction::drop. Handles drop logic for text base items & symbols (returns "accepted")
bool NotationInteraction::doDropTextBaseAndSymbols(const PointF& pos, bool applyUserOffset)
{
    IF_ASSERT_FAILED(m_dropData.elementDropData.has_value()) {
        return false;
    }

    ElementDropData& edd = m_dropData.elementDropData.value();

    EngravingItem* el = edd.dropTarget ? edd.dropTarget : elementAt(pos);
    if (el == 0 || el->type() == ElementType::STAFF_LINES) {
        mu::engraving::staff_idx_t staffIdx;
        mu::engraving::Segment* seg;
        PointF offset;
        el = score()->pos2measure(pos, &staffIdx, 0, &seg, &offset);
        if (el && el->isMeasure()) {
            edd.ed.dropElement->setTrack(staffIdx * mu::engraving::VOICES);
            edd.ed.dropElement->setParent(seg);

            if (applyUserOffset) {
                edd.ed.dropElement->setOffset(offset);
            }

            score()->undoAddElement(edd.ed.dropElement);
        } else {
            LOGD("cannot drop here");
            resetDropData();
        }
    } else {
        score()->addRefresh(el->canvasBoundingRect());
        score()->addRefresh(edd.ed.dropElement->canvasBoundingRect());

        if (!el->acceptDrop(edd.ed)) {
            LOGD("drop %s onto %s not accepted", edd.ed.dropElement->typeName(), el->typeName());
            return false;
        }
        edd.ed.pos = pos;
        EngravingItem* dropElement = el->drop(edd.ed);
        score()->addRefresh(el->canvasBoundingRect());
        if (dropElement) {
            selectAndStartEditIfNeeded(dropElement);
            score()->addRefresh(dropElement->canvasBoundingRect());
        }
    }

    return true;
}

bool NotationInteraction::dropRange(const QByteArray& data, const PointF& pos, bool deleteSourceMaterial)
{
    IF_ASSERT_FAILED(m_dropData.rangeDropData.has_value()) {
        return false;
    }

    RangeDropData& rdd = m_dropData.rangeDropData.value();

    if (rdd.tickLength.isZero() || rdd.numStaves == 0) {
        return false;
    }

    staff_idx_t staffIdx = muse::nidx;
    Segment* segment = nullptr;
    static constexpr double spacingFactor = 0.5;
    static constexpr bool useTimeAnchors = true;

    score()->dragPosition(pos, &staffIdx, &segment, spacingFactor, useTimeAnchors);

    if (staffIdx == muse::nidx || !segment) {
        return false;
    }

    rdd.targetSegment = segment;
    rdd.targetStaffIdx = staffIdx;

    startEdit(deleteSourceMaterial
              ? TranslatableString("undoableAction", "Move range")
              : TranslatableString("undoableAction", "Copy range"));

    if (deleteSourceMaterial && rdd.sourceStaffIdx != muse::nidx) {
        Segment* sourceStartSegment = score()->tick2leftSegmentMM(rdd.sourceTick);
        if (sourceStartSegment && !sourceStartSegment->enabled()) {
            sourceStartSegment = sourceStartSegment->next1MMenabled();
        }

        Segment* sourceEndSegment = score()->tick2rightSegment(rdd.sourceTick + rdd.tickLength,
                                                               true,
                                                               Segment::CHORD_REST_OR_TIME_TICK_TYPE);
        if (sourceEndSegment && !sourceEndSegment->enabled()) {
            sourceEndSegment = sourceEndSegment->next1MMenabled();
        }
        if (!sourceEndSegment) {
            sourceEndSegment = score()->lastSegmentMM();
        }

        if (sourceStartSegment && sourceEndSegment) {
            score()->deleteRange(sourceStartSegment, sourceEndSegment,
                                 engraving::staff2track(rdd.sourceStaffIdx),
                                 engraving::staff2track(rdd.sourceStaffIdx + rdd.numStaves),
                                 score()->selectionFilter());
        }
    }

    XmlReader e(data);
    score()->pasteStaff(e, segment, staffIdx);

    apply();
    endDrop();

    MScoreErrorsController(iocContext()).checkAndShowMScoreError();

    return true;
}

bool NotationInteraction::selectInstrument(mu::engraving::InstrumentChange* instrumentChange)
{
    if (!instrumentChange) {
        return false;
    }

    RetVal<InstrumentTemplate> templ = selectInstrumentScenario()->selectInstrument();
    if (!templ.ret) {
        return false;
    }

    Instrument newInstrument = Instrument::fromTemplate(&templ.val);
    instrumentChange->setInit(true);
    instrumentChange->setupInstrument(&newInstrument);

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

    startEdit(TranslatableString("undoableAction", "Apply palette element"));

    const bool isMeasureAnchoredElement = element->type() == ElementType::MARKER
                                          || element->type() == ElementType::JUMP
                                          || element->type() == ElementType::SPACER
                                          || element->type() == ElementType::VBOX
                                          || element->type() == ElementType::HBOX
                                          || element->type() == ElementType::TBOX
                                          || element->type() == ElementType::MEASURE
                                          || element->type() == ElementType::BRACKET
                                          || (element->type() == ElementType::ACTION_ICON
                                              && (toActionIcon(element)->actionType() == mu::engraving::ActionIconType::VFRAME
                                                  || toActionIcon(element)->actionType() == mu::engraving::ActionIconType::HFRAME
                                                  || toActionIcon(element)->actionType() == mu::engraving::ActionIconType::TFRAME
                                                  || toActionIcon(element)->actionType() == mu::engraving::ActionIconType::STAFF_TYPE_CHANGE
                                                  || toActionIcon(element)->actionType() == mu::engraving::ActionIconType::MEASURE
                                                  || toActionIcon(element)->actionType() == mu::engraving::ActionIconType::BRACKETS));

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

        bool elementIsStandardBend = element->isActionIcon() && toActionIcon(element)->actionType() == ActionIconType::STANDARD_BEND;
        bool elementIsNoteLine = element->isActionIcon() && toActionIcon(element)->actionType() == ActionIconType::NOTE_ANCHORED_LINE;
        bool isLineNoteToNote = (element->isGlissando() || elementIsStandardBend || elementIsNoteLine)
                                && sel.isList() && sel.elements().size() == 2
                                && sel.elements()[0]->isNote() && sel.elements()[1]->isNote()
                                && sel.elements()[1]->tick() != sel.elements()[0]->tick();

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
        } else if (element->isActionIcon() && toActionIcon(element)->actionType() == ActionIconType::SYSTEM_LOCK) {
            score->cmdApplyLockToSelection();
        } else if (element->isSlur() && addSingle) {
            doAddSlur(toSlur(element));
        } else if (element->isSLine() && !element->isGlissando() && !element->isGuitarBend() && addSingle) {
            mu::engraving::Segment* startSegment = cr1->segment();
            mu::engraving::Segment* endSegment = cr2->segment();
            if (element->type() == mu::engraving::ElementType::PEDAL && cr2 != cr1) {
                endSegment = endSegment->nextCR(cr2->track());
            } else {
                // Ensure that list-selection results in the same endSegment as range selection
                endSegment = cr2->nextSegmentAfterCR(SegmentType::ChordRest | SegmentType::EndBarLine | SegmentType::Clef);
            }
            mu::engraving::Spanner* spanner = static_cast<mu::engraving::Spanner*>(element->clone());
            spanner->setScore(score);
            spanner->styleChanged();
            if (spanner->isHairpin()) {
                score->addHairpin(toHairpin(spanner), cr1, cr2);
                if (!spanner->segmentsEmpty() && !score->noteEntryMode()) {
                    SpannerSegment* frontSegment = spanner->frontSegment();
                    score->select(frontSegment);
                    startEditElement(frontSegment);
                }
            } else {
                bool firstStaffOnly = isSystemTextLine(element) && !(modifiers & Qt::ControlModifier);
                staff_idx_t targetStaff = firstStaffOnly ? 0 : cr1->staffIdx();
                score->cmdAddSpanner(spanner, targetStaff, startSegment, endSegment, modifiers & Qt::ControlModifier);
            }
            if (spanner->hasVoiceAssignmentProperties()) {
                spanner->setInitialTrackAndVoiceAssignment(cr1->track(), modifiers & ControlModifier);
            } else if (spanner->isVoiceSpecific()) {
                spanner->setTrack(cr1->track());
            }
        } else if (element->isArticulationFamily() && sel.elements().size() == 1) {
            // understand adding an articulation to another articulation as adding it to the chord it's attached to
            EngravingItem* e = sel.elements().front();
            if (e->isArticulationFamily()) {
                if (Chord* c = toChord(toArticulation(e)->explicitParent())) {
                    applyDropPaletteElement(score, c->notes().front(), element, modifiers);
                }
            } else {
                applyDropPaletteElement(score, e, element, modifiers);
            }
        } else if (isLineNoteToNote) {
            applyLineNoteToNote(score, toNote(sel.elements()[0]), toNote(sel.elements()[1]), element);
        } else if ((element->isClef() || element->isTimeSig() || element->isKeySig()) && score->noteEntryMode()) {
            // in note input mode place clef / time sig / key sig before cursor
            EngravingItem* e = score->inputState().cr();
            if (!e) {
                e = sel.elements().front();
            } else if (e->isChord()) {
                e = toChord(e)->notes().front();
            }
            applyDropPaletteElement(score, e, element, modifiers);
        } else if (isMeasureAnchoredElement) {
            // we add the following measure-based items to each measure containing selected items
            std::vector<Measure*> measuresWithSelectedContent;
            for (EngravingItem* e : sel.elements()) {
                Measure* m = e->findMeasure();
                if (!m) {
                    continue;
                }
                if (element->type() == ElementType::MARKER && e->isBarLine()
                    && toBarLine(e)->segment()->segmentType() != SegmentType::BeginBarLine
                    && toBarLine(e)->segment()->segmentType() != SegmentType::StartRepeatBarLine) {
                    // exception: markers are anchored to the start of a measure,
                    // so when the user selects an end barline we take the next measure
                    m = m->nextMeasureMM() ? m->nextMeasureMM() : m;
                }
                if (muse::contains(measuresWithSelectedContent, m)) {
                    continue;
                }
                measuresWithSelectedContent.push_back(m);
                const RectF r = m->staffPageBoundingRect(e->staff()->idx());
                const PointF pt = r.center() + m->system()->page()->pos();
                applyDropPaletteElement(score, m, element, modifiers, pt);
                if (element->type() == ElementType::BRACKET) {
                    break;
                }
            }
        } else {
            for (EngravingItem* e : sel.elements()) {
                applyDropPaletteElement(score, e, element, modifiers);
            }
        }
    } else if (sel.isRange()) {
        if (element->type() == ElementType::BAR_LINE || isMeasureAnchoredElement) {
            Measure* last = sel.endSegment() ? sel.endSegment()->measure() : nullptr;
            for (Measure* m = sel.startSegment()->measure(); m; m = m->nextMeasureMM()) {
                const RectF r = m->staffPageBoundingRect(sel.staffStart());
                const PointF pt = r.center() + m->system()->page()->pos();
                applyDropPaletteElement(score, m, element, modifiers, pt);
                if ((m == last) || (element->type() == ElementType::BRACKET)) {
                    break;
                }
            }
        } else if (element->isStaffTypeChange()) {
            Measure* measure = sel.startSegment() ? sel.startSegment()->measure() : nullptr;
            if (measure) {
                ByteArray a = element->mimeData();

                for (staff_idx_t i = sel.staffStart(); i < sel.staffEnd(); ++i) {
                    mu::engraving::XmlReader n(a);
                    StaffTypeChange* stc = engraving::Factory::createStaffTypeChange(measure);
                    rw::RWRegister::reader()->readItem(stc, n);
                    stc->styleChanged(); // update to local style

                    score->cmdAddStaffTypeChange(measure, i, stc);
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
                    // restore clef/keysig/timesig that was in effect at end of selection
                    mu::engraving::Staff* staff = score->staff(i);
                    mu::engraving::Fraction tick2 = sel.endSegment()->tick();
                    mu::engraving::EngravingItem* oelement = nullptr;
                    switch (element->type()) {
                    case mu::engraving::ElementType::CLEF:
                    {
                        mu::engraving::Clef* oclef = engraving::Factory::createClef(score->dummy()->segment());
                        oclef->setClefType(staff->clef(tick2));
                        oelement = oclef;
                        break;
                    }
                    case mu::engraving::ElementType::KEYSIG:
                    {
                        mu::engraving::KeySig* okeysig = engraving::Factory::createKeySig(score->dummy()->segment());
                        okeysig->setKeySigEvent(staff->keySigEvent(tick2));
                        Key ck = okeysig->concertKey();
                        okeysig->setKey(ck);
                        oelement = okeysig;
                        break;
                    }
                    case mu::engraving::ElementType::TIMESIG:
                    {
                        mu::engraving::TimeSig* otimesig = engraving::Factory::createTimeSig(score->dummy()->segment());
                        otimesig->setFrom(staff->timeSig(tick2));
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
                            RectF r = m2->staffPageBoundingRect(i);
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
                    RectF r = m1->staffPageBoundingRect(i);
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
                score->cmdAddSpanner(spanner, i, startSegment, endSegment, modifiers & Qt::ControlModifier);
                if (spanner->hasVoiceAssignmentProperties()) {
                    spanner->setInitialTrackAndVoiceAssignment(staff2track(i), modifiers & ControlModifier);
                }
                selectAndStartEditIfNeeded(spanner);
            }
        } else if (element->isTextBase() && !element->isFingering() && !element->isSticking()) {
            mu::engraving::Segment* firstSegment = sel.startSegment();
            staff_idx_t firstStaffIndex = sel.staffStart();
            staff_idx_t lastStaffIndex = sel.staffEnd();

            // A text should only be added at the start of the selection
            // There shouldn't be a text at each element
            if (element->systemFlag()) {
                applyDropPaletteElement(score, firstSegment->firstElementForNavigation(0), element, modifiers);
            } else {
                for (staff_idx_t staff = firstStaffIndex; staff < lastStaffIndex; staff++) {
                    applyDropPaletteElement(score, firstSegment->firstElementForNavigation(staff), element, modifiers);
                }
            }
        } else if (element->isActionIcon()
                   && (toActionIcon(element)->actionType() == ActionIconType::STANDARD_BEND
                       || toActionIcon(element)->actionType() == ActionIconType::PRE_BEND
                       || toActionIcon(element)->actionType() == ActionIconType::GRACE_NOTE_BEND
                       || toActionIcon(element)->actionType() == ActionIconType::SLIGHT_BEND)) {
            // Insertion of bend may alter the segment list, so collect the original list here and loop on this
            std::vector<Segment*> segList;
            for (Segment* seg = sel.startSegment(); seg && seg != sel.endSegment(); seg = seg->next1()) {
                if (seg->isChordRestType()) {
                    segList.push_back(seg);
                }
            }
            track_idx_t track1 = sel.staffStart() * mu::engraving::VOICES;
            track_idx_t track2 = sel.staffEnd() * mu::engraving::VOICES;
            for (Segment* seg : segList) {
                for (track_idx_t track = track1; track < track2; ++track) {
                    EngravingItem* item = seg->elementAt(track);
                    if (!item || !item->isChord()) {
                        continue;
                    }
                    for (Note* note : toChord(item)->notes()) {
                        applyDropPaletteElement(score, note, element, modifiers);
                    }
                }
            }
        } else if (element->isActionIcon() && toActionIcon(element)->actionType() == ActionIconType::SYSTEM_LOCK) {
            score->cmdApplyLockToSelection();
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
                            if (!(element->isAccidental() || element->isNoteHead()
                                  || element->isGlissando() || element->isChordLine())) { // only these need to apply to every note
                                break;
                            }
                        }
                    } else {
                        // do not apply articulation to barline in a range selection
                        if (!e->isBarLine() || !element->isArticulationFamily()) {
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

    m_notifyAboutDropChanged = true;

    score->setSelectionChanged(true);

    apply();

    setDropTarget(nullptr);

    MScoreErrorsController(iocContext()).checkAndShowMScoreError();

    return true;
}

//! NOTE Copied from Palette applyDrop
void NotationInteraction::applyDropPaletteElement(mu::engraving::Score* score, mu::engraving::EngravingItem* target,
                                                  mu::engraving::EngravingItem* e,
                                                  Qt::KeyboardModifiers modifiers,
                                                  PointF pt, bool pasteMode)
{
    UNUSED(pasteMode);

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

        Fraction duration;      // dummy
        PointF dragOffset;
        ElementType type = EngravingItem::readType(n, &dragOffset, &duration);
        dropData->dropElement = engraving::Factory::createItem(type, score->dummy());
        rw::RWRegister::reader()->readItem(dropData->dropElement, n);
        dropData->dropElement->styleChanged();       // update to local style

        EngravingItem* el = target->drop(*dropData);

        if (el && el->isInstrumentChange()) {
            if (!selectInstrument(toInstrumentChange(el))) {
                rollback();
                return;
            }
        }

        if (el && el->hasVoiceAssignmentProperties()) {
            // If target has voice assignment properties, dropped element takes those and discards the default
            if (!target->hasVoiceAssignmentProperties()) {
                el->setInitialTrackAndVoiceAssignment(target->track(), modifiers & ControlModifier);
            }
        }

        if (el && !score->inputState().noteEntryMode()) {
            selectAndStartEditIfNeeded(el);
        }

        dropData->dropElement = nullptr;

        m_notifyAboutDropChanged = true;
    }
}

void NotationInteraction::applyLineNoteToNote(Score* score, Note* note1, Note* note2, EngravingItem* line)
{
    mu::engraving::EditData newData(&m_scoreCallbacks);
    mu::engraving::EditData* dropData = &newData;

    Chord* chord1 = note1->chord();
    Chord* chord2 = note2->chord();
    bool swapNotes = chord2->tick() < chord1->tick() || chord2->graceIndex() > chord1->graceIndex()
                     || (chord2->isGraceBefore() && chord2->parent() == chord1)
                     || (chord1->isGraceAfter() && chord1->parent() == chord2);

    if (swapNotes) {
        std::swap(note1, note2);
    }

    if (line->isActionIcon() && toActionIcon(line)->actionType() == ActionIconType::STANDARD_BEND) {
        score->addGuitarBend(GuitarBendType::BEND, note1, note2);
    } else if (line->isActionIcon() && toActionIcon(line)->actionType() == ActionIconType::NOTE_ANCHORED_LINE) {
        score->addNoteLine();
    } else if (line->isSLine()) {
        SLine* dropLine = ((SLine*)line->clone());
        dropData->dropElement = dropLine;
        if (note1->acceptDrop(*dropData)) {
            dropLine->setEndElement(note2);
            dropLine->styleChanged();

            mu::engraving::EngravingItem* el = note1->drop(*dropData);
            if (el && !score->inputState().noteEntryMode()) {
                doSelect({ el }, mu::engraving::SelectType::SINGLE, 0);
            }

            dropData->dropElement = nullptr;

            m_notifyAboutDropChanged = true;
        }
    }
}

//! NOTE Copied from ScoreView::cmdAddSlur
void NotationInteraction::doAddSlur(const Slur* slurTemplate)
{
    startEdit(TranslatableString("undoableAction", "Add slur"));
    m_notifyAboutDropChanged = true;

    ChordRest* firstChordRest = nullptr;
    ChordRest* secondChordRest = nullptr;
    const auto& sel = score()->selection();
    auto el = sel.uniqueElements();

    if (sel.isRange()) {
        track_idx_t startTrack = sel.staffStart() * VOICES;
        track_idx_t endTrack = sel.staffEnd() * VOICES;
        for (track_idx_t track = startTrack; track < endTrack; ++track) {
            firstChordRest = nullptr;
            secondChordRest = nullptr;
            for (EngravingItem* e : el) {
                if (e->track() != track) {
                    continue;
                }
                if (e->isNote()) {
                    e = toNote(e)->chord();
                }
                if (!e->isChord()) {
                    continue;
                }
                ChordRest* cr = toChordRest(e);
                if (!firstChordRest || firstChordRest->tick() > cr->tick()) {
                    firstChordRest = cr;
                }
                if (!secondChordRest || secondChordRest->tick() < cr->tick()) {
                    secondChordRest = cr;
                }
            }

            bool firstCrTrill = firstChordRest && firstChordRest->isChord() && toChord(firstChordRest)->isTrillCueNote();
            bool secondCrTrill = secondChordRest && secondChordRest->isChord() && toChord(secondChordRest)->isTrillCueNote();

            if (firstChordRest && (firstChordRest != secondChordRest)
                && !(firstCrTrill || secondCrTrill)) {
                doAddSlur(firstChordRest, secondChordRest, slurTemplate);
            }
        }
    } else if (sel.isSingle()) {
        if (sel.element()->isNote() && !toNote(sel.element())->isTrillCueNote()) {
            doAddSlur(toNote(sel.element())->chord(), nullptr, slurTemplate);
        }
    } else {
        EngravingItem* firstItem = nullptr;
        EngravingItem* secondItem = nullptr;
        for (EngravingItem* e : el) {
            if (e->isNote()) {
                e = toNote(e)->chord();
            }
            if (!e->isChord() && !e->isBarLine()) {
                continue;
            }

            if (!firstItem || e->isBefore(firstItem)) {
                firstItem = e;
            }
            if (!secondItem || secondItem->isBefore(e)) {
                secondItem = e;
            }
        }

        if (firstChordRest == secondItem && (!firstItem || firstItem->isChordRest())) {
            ChordRestNavigateOptions options;
            options.disableOverRepeats = true;
            secondItem = nextChordRest(toChordRest(firstItem), options);
        }

        bool firstCrTrill = firstItem && firstItem->isChord() && toChord(firstItem)->isTrillCueNote();
        bool secondCrTrill = secondItem && secondItem->isChord() && toChord(secondItem)->isTrillCueNote();

        if (firstItem && !(firstCrTrill || secondCrTrill)) {
            doAddSlur(firstItem, secondItem, slurTemplate);
        }
    }

    apply();
}

void NotationInteraction::doAddSlur(EngravingItem* firstItem, EngravingItem* secondItem, const Slur* slurTemplate)
{
    ChordRest* firstChordRest = nullptr;
    ChordRest* secondChordRest = nullptr;

    if (firstItem && secondItem && (firstItem->isBarLine() != secondItem->isBarLine())) {
        const bool outgoing = firstItem->isChordRest();
        const BarLine* bl = outgoing ? toBarLine(secondItem) : toBarLine(firstItem);

        // Check the barline is the start of a repeat section
        const Segment* adjacentCrSeg
            = outgoing ? bl->segment()->prev1(SegmentType::ChordRest) : bl->segment()->next1(SegmentType::ChordRest);
        ChordRest* adjacentCr = nullptr;
        for (track_idx_t track = 0; track < score()->ntracks(); track++) {
            EngravingItem* adjacentItem = adjacentCrSeg->element(track);
            if (!adjacentItem || !adjacentItem->isChordRest()) {
                continue;
            }
            adjacentCr = toChordRest(adjacentItem);
            break;
        }

        if (!adjacentCr || (outgoing && !adjacentCr->hasFollowingJumpItem()) || (!outgoing && !adjacentCr->hasPrecedingJumpItem())) {
            return;
        }

        Slur* partialSlur = slurTemplate ? slurTemplate->clone() : Factory::createSlur(score()->dummy());
        if (outgoing) {
            partialSlur->undoSetOutgoing(true);
            firstChordRest = toChordRest(firstItem);
            secondChordRest = toChordRest(adjacentCr);
        } else {
            partialSlur->undoSetIncoming(true);
            firstChordRest = toChordRest(adjacentCr);
            secondChordRest = toChordRest(secondItem);
        }
        slurTemplate = partialSlur;
    } else {
        firstChordRest = toChordRest(firstItem);
        secondChordRest = toChordRest(secondItem);
    }

    Slur* slur = firstChordRest->slur(secondChordRest);
    if (!slur || slur->slurDirection() != DirectionV::AUTO) {
        slur = score()->addSlur(firstChordRest, secondChordRest, slurTemplate);
    }

    if (m_noteInput->isNoteInputMode()) {
        m_noteInput->addSlur(slur);
    } else if (!secondItem) {
        SlurSegment* segment = slur->frontSegment();
        select({ segment }, SelectType::SINGLE);
        startEditGrip(segment, Grip::END);
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
    score()->setUpdateAll();
    setDropTarget(nullptr);
    resetDropData();
    score()->update();
}

muse::async::Notification NotationInteraction::dropChanged() const
{
    return m_dropChanged;
}

//! NOTE Copied from ScoreView::dropCanvas
bool NotationInteraction::dropCanvas(EngravingItem* e)
{
    if (e->isActionIcon()) {
        switch (mu::engraving::toActionIcon(e)->actionType()) {
        case mu::engraving::ActionIconType::VFRAME
            : score()->insertBox(ElementType::VBOX);
            break;
        case mu::engraving::ActionIconType::HFRAME:
            score()->insertBox(ElementType::HBOX);
            break;
        case mu::engraving::ActionIconType::TFRAME:
            score()->insertBox(ElementType::TBOX);
            break;
        case mu::engraving::ActionIconType::FFRAME:
            score()->insertBox(ElementType::FBOX);
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
    mu::engraving::Measure* fallbackMeasure = nullptr;
    for (EngravingItem* e : el) {
        if (e->isStaffLines()) {
            fallbackMeasure = mu::engraving::toStaffLines(e)->measure();
            continue;
        }
        if (e->acceptDrop(ed)) {
            return e;
        }
    }
    if (fallbackMeasure && fallbackMeasure->acceptDrop(ed)) {
        return fallbackMeasure;
    }
    return nullptr;
}

bool NotationInteraction::prepareDropStandardElement(const PointF& pos, Qt::KeyboardModifiers modifiers)
{
    IF_ASSERT_FAILED(m_dropData.elementDropData.has_value()) {
        return false;
    }

    ElementDropData& edd = m_dropData.elementDropData.value();

    Page* page = point2page(pos, true);
    if (!page) {
        return false;
    }

    // Update the tree when starting a new drag (ed.pos hasn't been set yet), if the modifier changed, or when crossing a page
    if (edd.ed.pos.isNull() || edd.ed.modifiers != modifiers || page != m_currentDropPage) {
        m_currentDropPage = page;
        edd.ed.modifiers = keyboardModifier(modifiers);

        // Where "m" is the number of page elements and "n" is the number of elements accepting a drop...
        // O(log m) operation
        m_droppableTree.initialize(page->pageBoundingRect(), static_cast<int>(page->elements().size()));

        // O(m log n) operation
        for (EngravingItem* elem : page->elements()) {
            if (elem->acceptDrop(edd.ed)) {
                m_droppableTree.insert(elem);
            }
        }
    }

    PointF posInPage(pos.x() - page->pos().x(), pos.y() - page->pos().y());
    // O(log n) operation
    EngravingItem* targetElem = m_droppableTree.nearestNeighbor(posInPage);
    EngravingItem* dropElem = edd.ed.dropElement;

    if (!targetElem || !dropElem) {
        return false;
    }

    // Clefs are a special case somewhere between a measure anchored and standard element (hence why
    // we handle it here instead of dragMeasureAnchorElement). They can be applied to individual items
    // but we should switch targets to the whole measure if the target element is the first in the measure...
    if (dropElem->isClef() && targetElem->rtick() == Fraction(0, 1)) {
        Measure* targetMeasure = targetElem->findMeasure();
        if (targetMeasure && dropElem->type() != targetElem->type()) {
            setDropTarget(targetMeasure, true);

            RectF measureRect = targetMeasure->staffPageBoundingRect(targetElem->staffIdx());
            measureRect.adjust(page->x(), page->y(), page->x(), page->y());
            edd.ed.pos = measureRect.center();
            setAnchorLines({ LineF(pos, measureRect.topLeft()) });

            return targetMeasure->acceptDrop(edd.ed);
        }
    }

    setDropTarget(targetElem, true);
    edd.ed.pos = targetElem->canvasBoundingRect().center();
    setAnchorLines({ LineF(pos, targetElem->canvasBoundingRect().center()) });

    return true;
}

//! NOTE Copied from ScoreView::dragMeasureAnchorElement
bool NotationInteraction::prepareDropMeasureAnchorElement(const PointF& pos)
{
    IF_ASSERT_FAILED(m_dropData.elementDropData.has_value()) {
        return false;
    }

    ElementDropData& edd = m_dropData.elementDropData.value();

    EngravingItem* dropElem = edd.ed.dropElement;
    Page* page = point2page(pos);
    if (!dropElem || !page) {
        return false;
    }

    mu::engraving::staff_idx_t staffIdx;
    mu::engraving::MeasureBase* mb = score()->pos2measure(pos, &staffIdx, 0, nullptr, 0);

    //! NOTE: Should match Measure::acceptDrop
    switch (dropElem->type()) {
    case ElementType::STAFFTYPE_CHANGE:
    case ElementType::VOLTA:
    case ElementType::GRADUAL_TEMPO_CHANGE:
    case ElementType::KEYSIG:
    case ElementType::TIMESIG: {
        // If ctrl is pressed, break and target a specific staff
        if (edd.ed.modifiers & Qt::ControlModifier) {
            break;
        }
    }
    // fall through
    case ElementType::JUMP:
    case ElementType::LAYOUT_BREAK:
    case ElementType::MARKER:
    case ElementType::MEASURE_LIST:
    case ElementType::MEASURE_NUMBER:
    case ElementType::STAFF_LIST:
        // Target all staves
        staffIdx = 0;
    // fall through
    default: break;
    }

    // Apart from STAFF_TYPE_CHANGE, measure anchored action icons are applied to all staves
    if (dropElem->isActionIcon() && toActionIcon(dropElem)->actionType() != ActionIconType::STAFF_TYPE_CHANGE) {
        staffIdx = 0;
    }

    if (mb && mb->isMeasure()) {
        mu::engraving::Measure* targetMeasure = mu::engraving::toMeasure(mb);
        setDropTarget(targetMeasure, true);

        RectF measureRect = targetMeasure->staffPageBoundingRect(staffIdx);
        measureRect.adjust(page->x(), page->y(), page->x(), page->y());
        edd.ed.pos = measureRect.center();

        const bool dropAccepted = targetMeasure->acceptDrop(edd.ed);
        if (dropAccepted) {
            setAnchorLines({ LineF(pos, measureRect.topLeft()) });
        }

        return dropAccepted;
    }
    dropElem->score()->addRefresh(dropElem->canvasBoundingRect());
    setDropTarget(nullptr);
    return false;
}

//! NOTE Copied from ScoreView::dragTimeAnchorElement
bool NotationInteraction::prepareDropTimeAnchorElement(const PointF& pos)
{
    IF_ASSERT_FAILED(m_dropData.elementDropData.has_value()) {
        return false;
    }

    ElementDropData& edd = m_dropData.elementDropData.value();

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
        edd.ed.dropElement->score()->addRefresh(edd.ed.dropElement->canvasBoundingRect());
        edd.ed.dropElement->setTrack(track);
        edd.ed.dropElement->score()->addRefresh(edd.ed.dropElement->canvasBoundingRect());
        notifyAboutDragChanged();
        return true;
    }

    edd.ed.dropElement->score()->addRefresh(edd.ed.dropElement->canvasBoundingRect());
    setDropTarget(nullptr);

    return false;
}

//! NOTE Copied from ScoreView::setDropTarget
void NotationInteraction::setDropTarget(EngravingItem* item, bool notify)
{
    if (!m_dropData.elementDropData.has_value()) {
        return;
    }

    ElementDropData& edd = m_dropData.elementDropData.value();

    if (edd.dropTarget != item) {
        if (edd.dropTarget) {
            edd.dropTarget->setDropTarget(false);
            edd.dropTarget = nullptr;
        }

        edd.dropTarget = item;
        if (edd.dropTarget) {
            edd.dropTarget->setDropTarget(true);
        }
    }

    resetAnchorLines();

    if (edd.dropRect.isValid()) {
        edd.dropRect = RectF();
    }

    if (notify) {
        notifyAboutDragChanged();
    }
}

//! NOTE: Copied from ScoreView::setDropRectangle
void NotationInteraction::setDropRect(const RectF& rect)
{
    if (!m_dropData.elementDropData.has_value()) {
        return;
    }

    ElementDropData& edd = m_dropData.elementDropData.value();

    if (edd.dropRect == rect) {
        return;
    }

    edd.dropRect = rect;

    if (rect.isValid()) {
        score()->addRefresh(rect);
    }

    if (edd.dropTarget) {
        edd.dropTarget->setDropTarget(false);
        score()->addRefresh(edd.dropTarget->canvasBoundingRect());
        edd.dropTarget = nullptr;
    } else if (!m_anchorLines.empty()) {
        RectF rf;
        rf.setTopLeft(m_anchorLines.front().p1());
        rf.setBottomRight(m_anchorLines.front().p2());
        score()->addRefresh(rf.normalized());
        resetAnchorLines();
    }

    notifyAboutDragChanged();
}

void NotationInteraction::resetDropData()
{
    if (m_dropData.elementDropData.has_value()) {
        delete m_dropData.elementDropData->ed.dropElement;
    }

    m_dropData = {};
}

void NotationInteraction::setAnchorLines(const std::vector<LineF>& anchorList)
{
    m_anchorLines = anchorList;
}

void NotationInteraction::resetAnchorLines()
{
    m_anchorLines.clear();
}

double NotationInteraction::currentScaling(Painter* painter) const
{
    qreal guiScaling = configuration()->guiScaling();
    return painter->worldTransform().m11() / guiScaling;
}

std::vector<Position> NotationInteraction::inputPositions() const
{
    std::vector<Position> result;

    const InputState& is = score()->inputState();
    if (!is.isValid()) {
        return result;
    }

    const staff_idx_t staffIdx = is.staffIdx();
    const Staff* staff = score()->staff(staffIdx);
    const System* system = is.segment()->system();
    const SysStaff* sysStaff = system ? system->staff(staffIdx) : nullptr;
    const Measure* measure = is.segment()->measure();

    if (!staff || !sysStaff || !measure) {
        return result;
    }

    const Fraction tick = is.tick();
    const PointF measurePos = measure->canvasPos();
    const double lineDist = staff->staffType(tick)->lineDistance().val()
                            * (staff->isTabStaff(tick) ? 1 : .5)
                            * staff->staffMag(tick)
                            * score()->style().spatium();

    Position pos;
    pos.segment = is.segment();
    pos.staffIdx = staffIdx;

    for (const NoteVal& nval : is.notes()) {
        pos.line = mu::engraving::noteValToLine(nval, staff, tick);
        const double y = sysStaff->y() + pos.line * lineDist;
        pos.pos = PointF(is.segment()->x(), y) + measurePos;

        result.push_back(pos);
    }

    std::sort(result.begin(), result.end(), [](const Position& p1, const Position& p2) {
        return p1.line < p2.line;
    });

    return result;
}

bool NotationInteraction::shouldDrawInputPreview() const
{
    return m_noteInput->isNoteInputMode() && m_noteInput->usingNoteInputMethod(NoteInputMethod::BY_DURATION);
}

void NotationInteraction::drawInputPreview(Painter* painter)
{
    std::vector<Position> positions = inputPositions();
    if (positions.empty()) {
        return;
    }

    std::vector<ShadowNote*> notes;
    notes.reserve(positions.size());

    const InputState& is = score()->inputState();

    ShadowNoteParams params;
    params.duration = is.rest() ? is.duration() : TDuration();

    for (Position& pos : positions) {
        ShadowNote* note = new ShadowNote(score());
        showShadowNoteAtPosition(*note, params, pos);
        notes.push_back(note);
    }

    const bool isUp = !is.rest() && notes.front()->computeUp();
    bool isLeft = isUp;
    int prevLine = INT_MAX;

    auto correctNotePositionIfNeed = [&](ShadowNote* note) {
        if (positions.size() == 1) {
            return;
        }

        const bool conflict = std::abs(prevLine - note->lineIndex()) < 2;
        prevLine = note->lineIndex();

        if (conflict || (isUp != isLeft)) {
            isLeft = !isLeft;
        }

        if (isUp == isLeft) {
            return;
        }

        const RectF noteheadBbox = note->symBbox(note->noteheadSymbol());

        if (isLeft) {
            note->move(PointF(-noteheadBbox.width(), 0.));
        } else {
            note->move(PointF(noteheadBbox.width(), 0.));
        }
    };

    if (isUp) {
        for (auto it = notes.rbegin(); it != notes.rend(); ++it) {
            correctNotePositionIfNeed(*it);
            score()->renderer()->drawItem(*it, painter);
        }
    } else {
        for (auto it = notes.begin(); it != notes.end(); ++it) {
            correctNotePositionIfNeed(*it);
            score()->renderer()->drawItem(*it, painter);
        }
    }

    DeleteAll(notes);
}

void NotationInteraction::drawAnchorLines(Painter* painter)
{
    if (m_anchorLines.empty()) {
        return;
    }

    const auto dropAnchorColor = score()->configuration()->formattingColor();
    Pen pen(dropAnchorColor, 2.0 / currentScaling(painter), PenStyle::DotLine);

    for (const LineF& anchor : m_anchorLines) {
        painter->setPen(pen);
        painter->drawLine(anchor);

        qreal d = 4.0 / currentScaling(painter);
        RectF rect(-d, -d, 2 * d, 2 * d);

        painter->setBrush(Brush(dropAnchorColor));
        painter->setNoPen();
        rect.moveCenter(anchor.p1());
        painter->drawEllipse(rect);
        rect.moveCenter(anchor.p2());
        painter->drawEllipse(rect);
    }
}

void NotationInteraction::drawTextEditMode(muse::draw::Painter* painter)
{
    if (!isTextEditingStarted()) {
        return;
    }

    m_editData.element->drawEditMode(painter, m_editData, currentScaling(painter));
}

void NotationInteraction::drawSelectionRange(muse::draw::Painter* painter)
{
    using namespace muse::draw;
    if (!m_selection->isRange()) {
        return;
    }

    painter->setBrush(BrushStyle::NoBrush);

    Color selectionColor = configuration()->selectionColor();
    double penWidth = 3.0 / currentScaling(painter);
    double minPenWidth = 0.20 * m_selection->range()->measureRange().startMeasure->spatium();
    penWidth = std::max(penWidth, minPenWidth);

    Pen pen;
    pen.setColor(selectionColor);
    pen.setWidthF(penWidth);
    pen.setStyle(PenStyle::SolidLine);
    painter->setPen(pen);

    std::vector<RectF> rangeArea = m_selection->range()->boundingArea();
    for (const RectF& rect: rangeArea) {
        PainterPath path;
        path.addRoundedRect(rect, 4, 4);

        Color fillColor = selectionColor;
        fillColor.setAlpha(10);
        painter->fillPath(path, fillColor);
        painter->drawPath(path);
    }
}

void NotationInteraction::drawGripPoints(muse::draw::Painter* painter)
{
    if (isDragStarted() && !isGripEditStarted()) {
        return;
    }

    mu::engraving::EngravingItem* editedElement = m_editData.element;

    if (editedElement && editedElement->isDynamic()) {
        toDynamic(editedElement)->findAdjacentHairpins();
    }

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

void NotationInteraction::drawLasso(muse::draw::Painter* painter)
{
    if (!m_lasso || m_lasso->isEmpty()) {
        return;
    }

    score()->renderer()->drawItem(m_lasso, painter);
}

void NotationInteraction::drawDrop(muse::draw::Painter* painter)
{
    if (m_dropData.elementDropData.has_value()) {
        const ElementDropData& edd = m_dropData.elementDropData.value();

        if (edd.dropRect.isValid()) {
            painter->fillRect(edd.dropRect, configuration()->dropRectColor());
        }
    }

    if (m_dropData.rangeDropData.has_value()) {
        const RangeDropData& rdd = m_dropData.rangeDropData.value();

        Color selectionColor = configuration()->selectionColor();
        double penWidth = 3.0 / currentScaling(painter);
        double minPenWidth = 0.20 * score()->style().spatium();
        penWidth = std::max(penWidth, minPenWidth);

        Pen pen;
        pen.setColor(selectionColor);
        pen.setWidthF(penWidth);
        pen.setStyle(PenStyle::SolidLine);
        painter->setPen(pen);

        for (const RectF& rect: rdd.dropRects) {
            PainterPath path;
            path.addRoundedRect(rect, 4, 4);

            Color fillColor = selectionColor;
            fillColor.setAlpha(10);
            painter->fillPath(path, fillColor);
            painter->drawPath(path);
        }
    }
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
    case MoveSelectionType::Chord: {
        ChordRestNavigateOptions options;
        options.skipGrace = true;
        if (d == MoveDirection::Right) {
            el = mu::engraving::nextChordRest(cr, options);
        } else {
            el = mu::engraving::prevChordRest(cr, options);
        }
        break;
    }
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
        showItem(el);
        resetHitElementContext();
    }
}

bool NotationInteraction::moveSelectionAvailable(MoveSelectionType type) const
{
    if (type != MoveSelectionType::EngravingItem) {
        return !isElementEditStarted();
    }

    EngravingItem* el = score()->selection().element();
    const std::set<ElementType> allowedTextEditTypes = {
        ElementType::STAFF_TEXT,
        ElementType::SYSTEM_TEXT,
        ElementType::EXPRESSION,
        ElementType::REHEARSAL_MARK
    };
    if (isTextEditingStarted() && el && allowedTextEditTypes.find(el->type()) != allowedTextEditTypes.end()) {
        return true;
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
        if (score()->style().styleB(mu::engraving::Sid::createMultiMeasureRests) && ftm->hasMMRest()) {
            ftm = ftm->coveringMMRestOrThis();
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
    startEdit(TranslatableString("undoableAction", "Move chord/rest to staff"));

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
    startEdit(TranslatableString("undoableAction", "Move chord/rest"));
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
            && !(cr1->isChord() && toChord(cr1)->tremoloTwoChord()
                 && cr2->isChord() && toChord(cr2)->tremoloTwoChord()
                 && toChord(cr1)->tremoloTwoChord() != toChord(cr2)->tremoloTwoChord())) {
            score()->undo(new mu::engraving::SwapCR(cr1, cr2));
        }
    }
    apply();
}

void NotationInteraction::toggleSnapToPrevious()
{
    bool newSnapValue = false;

    // Collect items to toggle...
    std::unordered_set<Hairpin*> hairpins;
    for (EngravingItem* e : score()->selection().elements()) {
        if (e->isHairpinSegment()) {
            Hairpin* h = toHairpinSegment(e)->hairpin();
            // If any item in the selection has a false snapping value, then we should
            // toggle all to true (handles mixed/indeterminate state)
            if (!h->snapToItemBefore() && !newSnapValue) {
                newSnapValue = true;
            }
            hairpins.emplace(h);
        }
    }
    if (hairpins.empty()) {
        return;
    }

    // Do toggle...
    startEdit(TranslatableString("undoableAction", "Toggle snap to previous"));
    for (Hairpin* h : hairpins) {
        if (h->snapToItemBefore() == newSnapValue) {
            continue;
        }
        h->undoChangeProperty(Pid::SNAP_BEFORE, PropertyValue(newSnapValue));
    }
    apply();
}

void NotationInteraction::toggleSnapToNext()
{
    bool newSnapValue = false;

    // Collect items to toggle...
    std::unordered_set<Hairpin*> hairpins;
    std::unordered_set<GradualTempoChange*> gradualTempoChanges;
    for (EngravingItem* e : score()->selection().elements()) {
        if (e->isHairpinSegment()) {
            Hairpin* h = toHairpinSegment(e)->hairpin();
            // If any item in the selection has a false snapping value, then we should
            // toggle all to true (handles mixed/indeterminate state)
            if (!h->snapToItemAfter() && !newSnapValue) {
                newSnapValue = true;
            }
            hairpins.emplace(h);
        }
        if (e->isGradualTempoChangeSegment()) {
            GradualTempoChange* gtc = toGradualTempoChangeSegment(e)->tempoChange();
            // Same here (see above)
            if (!gtc->snapToItemAfter() && !newSnapValue) {
                newSnapValue = true;
            }
            gradualTempoChanges.emplace(gtc);
        }
    }
    if (hairpins.empty() && gradualTempoChanges.empty()) {
        return;
    }

    // Do toggle...
    startEdit(TranslatableString("undoableAction", "Toggle snap to next"));
    for (Hairpin* h : hairpins) {
        if (h->snapToItemAfter() == newSnapValue) {
            continue;
        }
        h->undoChangeProperty(Pid::SNAP_AFTER, PropertyValue(newSnapValue));
    }
    for (GradualTempoChange* gtc : gradualTempoChanges) {
        if (gtc->snapToItemAfter() == newSnapValue) {
            continue;
        }
        gtc->undoChangeProperty(Pid::SNAP_AFTER, PropertyValue(newSnapValue));
    }
    apply();
}

void NotationInteraction::moveElementSelection(MoveDirection d)
{
    EngravingItem* el = score()->selection().element();
    if (!el && !score()->selection().elements().empty()) {
        el = score()->selection().elements().back();
    }

    if (isTextEditingStarted() && el && el->isTextBase()) {
        navigateToNearText(d);
        return;
    }

    const bool isLeftDirection = MoveDirection::Left == d;
    const bool isHorizontalLayout = score()->isLayoutMode(LayoutMode::LINE) || score()->isLayoutMode(LayoutMode::HORIZONTAL_FIXED);

    // VBoxes are not included in horizontal layouts - skip over them (and their contents) when moving selections...
    const auto nextNonVBox = [this, isLeftDirection](EngravingItem* currElem) -> EngravingItem* {
        while (const EngravingItem* vBox = currElem->findAncestor(ElementType::VBOX)) {
            currElem = isLeftDirection ? toVBox(vBox)->prevMM() : toVBox(vBox)->nextMM();
            if (currElem && currElem->isMeasure()) {
                const ChordRest* cr = score()->selection().currentCR();
                const staff_idx_t si = cr ? cr->staffIdx() : 0;
                Measure* mb = toMeasure(currElem);
                currElem = isLeftDirection ? mb->prevElementStaff(si, currElem) : mb->nextElementStaff(si, currElem);
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
            toEl = isLeftDirection ? score()->lastElement() : score()->firstElement();
            if (isHorizontalLayout) {
                toEl = nextNonVBox(toEl);
            }
        }
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

    startEdit(TranslatableString("undoableAction", "Change pitch"));

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
    startEdit(TranslatableString("undoableAction", "Move lyrics"));
    score()->cmdMoveLyrics(toLyrics(el), toDirection(d));
    apply();
}

void NotationInteraction::nudge(MoveDirection d, bool quickly)
{
    EngravingItem* el = score()->selection().element();
    IF_ASSERT_FAILED(el && (el->isTextBase() || el->isArticulationFamily())) {
        return;
    }

    startEdit(TranslatableString("undoableAction", "Nudge element"));

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

void NotationInteraction::nudgeAnchors(MoveDirection d)
{
    IF_ASSERT_FAILED(m_editData.element) {
        return;
    }

    startEdit(TranslatableString("undoableAction", "Nudge"));
    qreal vRaster = mu::engraving::MScore::vRaster();
    qreal hRaster = mu::engraving::MScore::hRaster();

    switch (d) {
    case MoveDirection::Left:
        m_editData.delta = QPointF(-nudgeDistance(m_editData, hRaster), 0);
        break;
    case MoveDirection::Right:
        m_editData.delta = QPointF(nudgeDistance(m_editData, hRaster), 0);
        break;
    case MoveDirection::Up:
        m_editData.delta = QPointF(0, -nudgeDistance(m_editData, vRaster));
        break;
    case MoveDirection::Down:
        m_editData.delta = QPointF(0, nudgeDistance(m_editData, vRaster));
        break;
    default:
        rollback();
        return;
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

    apply();
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
    return m_editData.element && m_editData.element->isTextBase() && m_editData.editTextualProperties;
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

    m_editData.clear();

    if (element->isTBox()) {
        m_editData.element = toTBox(element)->text();
    } else {
        m_editData.element = element;
    }

    m_editData.editTextualProperties = true;
    m_editData.startMove = bindCursorPosToText(cursorPos, m_editData.element);
    m_editData.element->startEdit(m_editData);

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
        score()->startCmd(TranslatableString("undoableAction", "Edit text"));
        text->insertText(m_editData, event->commitString());
        score()->endCmd();
        preeditString.clear();
    } else {
        preeditString = event->preeditString();

        if (!preeditString.isEmpty()) {
            cursor->updateCursorFormat();
            text->editInsertText(cursor, preeditString);
            text->setTextInvalid();
            score()->renderer()->layoutText1(text);
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

    return muse::contains(arrows, static_cast<Qt::Key>(event->key()));
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
    if (!isTextEditingStarted()) {
        return;
    }

    EngravingItem* editedElement = m_editData.element;
    doEndEditElement();

    if (editedElement) {
        notifyAboutTextEditingEnded(toTextBase(editedElement));
    }

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
        QString txt = QGuiApplication::clipboard()->text();
        textEl->paste(m_editData, txt);
    }

    notifyAboutTextEditingChanged();
}

void NotationInteraction::selectText(mu::engraving::SelectTextType type)
{
    if (!isTextEditingStarted()) {
        return;
    }

    mu::engraving::TextBase* text = mu::engraving::toTextBase(m_editData.element);
    text->select(m_editData, type);
    text->endHexState(m_editData);
    text->setPrimed(false);

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

void NotationInteraction::undoRedoToIndex(size_t idx)
{
    m_undoStack->undoRedoToIndex(idx, &m_editData);
}

muse::async::Notification NotationInteraction::textEditingStarted() const
{
    return m_textEditingStarted;
}

muse::async::Notification NotationInteraction::textEditingChanged() const
{
    return m_textEditingChanged;
}

muse::async::Channel<TextBase*> NotationInteraction::textEditingEnded() const
{
    return m_textEditingEnded;
}

muse::async::Channel<ScoreConfigType> NotationInteraction::scoreConfigChanged() const
{
    return m_scoreConfigChanged;
}

bool NotationInteraction::isGripEditStarted() const
{
    return m_editData.element && m_editData.curGrip != mu::engraving::Grip::NO_GRIP;
}

static int findGrip(const std::vector<muse::RectF>& grips, const muse::PointF& canvasPos)
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
    m_editData.editTextualProperties = false;

    updateGripAnchorLines();
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

void NotationInteraction::updateGripAnchorLines()
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

void NotationInteraction::updateDragAnchorLines()
{
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

bool NotationInteraction::isElementEditStarted() const
{
    return m_editData.element != nullptr;
}

void NotationInteraction::startEditElement(EngravingItem* element, bool editTextualProperties)
{
    if (!element) {
        return;
    }

    if (isElementEditStarted()) {
        return;
    }

    if (element->isTextBase() && editTextualProperties) {
        startEditText(element);
    } else if (element->isEditable()) {
        m_editData.editTextualProperties = false;
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

    bool isShiftRelease = event->type() == QKeyEvent::Type::KeyRelease;
    if (isShiftRelease) {
        m_editData.isKeyRelease = true;
        resetAnchorLines();
    } else {
        m_editData.isKeyRelease = false;
    }

    // Brackets may be deleted and replaced
    bool isBracket = m_editData.element->isBracket();
    const mu::engraving::System* system = nullptr;
    size_t bracketIndex = muse::nidx;

    if (isBracket) {
        const mu::engraving::Bracket* bracket = mu::engraving::toBracket(m_editData.element);
        system = bracket->system();

        if (system) {
            bracketIndex = muse::indexOf(system->brackets(), bracket);
        }
    } else if (m_editData.element->isHarmony()) {
        if (isTextEditingStarted() && (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)) {
            endEditText();
            return;
        }
    }

    //: Means: an editing operation triggered by a keystroke
    startEdit(TranslatableString("undoableAction", "Keystroke edit"));

    if (needStartEditGrip(event)) {
        m_editData.curGrip = m_editData.element->defaultGrip();
    }

    bool handled = m_editData.element->edit(m_editData);
    if (!handled) {
        handled = handleKeyPress(event);
    }

    if (handled) {
        event->accept();

        if (isBracket && system && bracketIndex != muse::nidx) {
            mu::engraving::EngravingItem* bracket = system->brackets().at(bracketIndex);
            m_editData.element = bracket;
            select({ bracket }, SelectType::SINGLE);
        }

        apply();

        if (!isShiftRelease) {
            if (isGripEditStarted()) {
                if (m_editData.element->isDynamic() && !m_editData.isStartEndGrip()) {
                    updateDragAnchorLines();
                } else {
                    updateGripAnchorLines();
                }
            } else if (isElementEditStarted() && !m_editData.editTextualProperties) {
                updateDragAnchorLines();
            }
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

    startEdit(TranslatableString("undoableAction", "Split measure"));
    score()->cmdSplitMeasure(chordRest);
    apply();

    MScoreErrorsController(iocContext()).checkAndShowMScoreError();
}

void NotationInteraction::joinSelectedMeasures()
{
    if (!m_selection->isRange()) {
        return;
    }

    INotationSelectionRange::MeasureRange measureRange = m_selection->range()->measureRange();

    startEdit(TranslatableString("undoableAction", "Join measures"));
    score()->cmdJoinMeasure(measureRange.startMeasure, measureRange.endMeasure);
    apply();

    MScoreErrorsController(iocContext()).checkAndShowMScoreError();
}

Ret NotationInteraction::canAddBoxes() const
{
    if (selection()->isRange()) {
        return muse::make_ok();
    }

    static const ElementTypeSet BOX_TYPES {
        ElementType::VBOX, ElementType::HBOX, ElementType::TBOX
    };

    for (const EngravingItem* element: selection()->elements()) {
        if (mu::engraving::toMeasure(element->findMeasure())) {
            return muse::make_ok();
        }

        if (muse::contains(BOX_TYPES, element->type())) {
            return muse::make_ok();
        }
    }

    return make_ret(Err::MeasureIsNotSelected);
}

void NotationInteraction::addBoxes(BoxType boxType, int count, AddBoxesTarget target)
{
    int beforeBoxIndex = -1;
    bool moveSignaturesClefs = (target != AddBoxesTarget::AfterSelection);
    const EngravingItem* selectedItem = nullptr;
    const MeasureBase* selectedItemMeasure = nullptr;

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

        const std::vector<EngravingItem*>& elements = selection()->elements();
        IF_ASSERT_FAILED(!elements.empty()) {
            // This would contradict the fact that selection()->isNone() == false at this point
            return;
        }

        for (const EngravingItem* item : elements) {
            const MeasureBase* itemMeasure = item->findMeasureBase();
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
                    selectedItem = item;
                    selectedItemMeasure = itemMeasure;
                }
            } else {
                if (itemMeasureIndex + 1 > beforeBoxIndex) {
                    beforeBoxIndex = itemMeasureIndex + 1;
                    selectedItem = item;
                    selectedItemMeasure = itemMeasure;
                }
            }
        }

        // special cases for "between measures elements"
        if (selectedItem && selectedItemMeasure) { // null check
            ElementType selectedItemType = selectedItem->type();
            if (selectedItemType == ElementType::CLEF || selectedItemType == ElementType::BAR_LINE
                || selectedItemType == ElementType::TIMESIG || selectedItemType == ElementType::KEYSIG) {
                Fraction itemTick = selectedItem->tick();
                Fraction measureTick = selectedItemMeasure->tick();
                Fraction measureLastTick = measureTick + selectedItemMeasure->ticks();

                if (itemTick == measureTick) {
                    if (target == AddBoxesTarget::AfterSelection) {
                        beforeBoxIndex -= 1;
                    }
                    moveSignaturesClefs = (target == AddBoxesTarget::AfterSelection);
                } else if (itemTick == measureLastTick) {
                    if (target == AddBoxesTarget::BeforeSelection) {
                        beforeBoxIndex += 1;
                    }
                    moveSignaturesClefs = (target == AddBoxesTarget::AfterSelection);
                }
            }
        }

        if (beforeBoxIndex < 0) {
            // No suitable element found
            return;
        }
    } break;
    case AddBoxesTarget::AtStartOfScore: {
        Measure* firstMeasure = score()->firstMeasure();
        beforeBoxIndex = firstMeasure ? firstMeasure->index() : -1;
    } break;
    case AddBoxesTarget::AtEndOfScore:
        beforeBoxIndex = -1;
        break;
    }

    addBoxes(boxType, count, beforeBoxIndex, moveSignaturesClefs);
}

void NotationInteraction::addBoxes(BoxType boxType, int count, int beforeBoxIndex, bool moveSignaturesClef)
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

    auto boxTypeDescription = [count](BoxType boxType) -> TranslatableString {
        switch (boxType) {
        case BoxType::Horizontal: return TranslatableString("undoableAction", "Add horizontal frame");
        case BoxType::Vertical: return TranslatableString("undoableAction", "Add vertical frame");
        case BoxType::Text: return TranslatableString("undoableAction", "Add text frame");
        case BoxType::Measure: return TranslatableString("undoableAction", "Add %n measure(s)", nullptr, count);
        case BoxType::Unknown: break;
        }

        return {};
    };

    startEdit(boxTypeDescription(boxType));

    mu::engraving::MeasureBase* beforeBox = beforeBoxIndex >= 0 ? score()->measure(beforeBoxIndex) : nullptr;

    mu::engraving::Score::InsertMeasureOptions options;
    options.createEmptyMeasures = false;
    options.moveSignaturesClef = moveSignaturesClef;
    options.needDeselectAll = false;

    for (int i = 0; i < count; ++i) {
        score()->insertMeasure(elementType, beforeBox, options);
    }

    apply();

    int indexOfFirstAddedMeasure = beforeBoxIndex >= 0 ? beforeBoxIndex : score()->measures()->size() - count;
    MeasureBase* firstAddedMeasure = score()->measure(indexOfFirstAddedMeasure);
    doSelect({ firstAddedMeasure }, SelectType::REPLACE);
    showItem(firstAddedMeasure);

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
            QMimeData* mimeData = new QMimeData();
            mimeData->setData(TextEditData::mimeRichTextFormat, ted->selectedText.toQString().toUtf8());
            mimeData->setText(ted->selectedPlainText);
            QGuiApplication::clipboard()->setMimeData(mimeData);
        }
    } else {
        QMimeData* mimeData = selection()->mimeData();
        if (!mimeData) {
            return;
        }
        QApplication::clipboard()->setMimeData(mimeData);
    }
}

Ret NotationInteraction::repeatSelection()
{
    const mu::engraving::Selection& selection = score()->selection();
    if (score()->noteEntryMode() && selection.isSingle()) {
        EngravingItem* el = selection.element();
        if (el && el->type() == ElementType::NOTE && !score()->inputState().endOfScore()) {
            startEdit(TranslatableString("undoableAction", "Repeat selection"));
            Chord* c = toNote(el)->chord();
            for (Note* note : c->notes()) {
                mu::engraving::NoteVal nval = note->noteVal();
                score()->addPitch(nval, note != c->notes()[0]);
            }
            apply();
        }
        return muse::make_ok();
    }

    if (!selection.isRange()) {
        ChordRest* cr = score()->getSelectedChordRest();
        if (!cr) {
            return make_ret(Err::NoteOrRestIsNotSelected);
        }
        score()->select(cr, SelectType::RANGE);
    }

    Ret ret = m_selection->canCopy();
    if (!ret) {
        return ret;
    }

    mu::engraving::XmlReader xml(selection.mimeData());
    track_idx_t dStaff = selection.staffStart();
    mu::engraving::Segment* endSegment = selection.endSegment();

    if (endSegment && endSegment->segmentType() != mu::engraving::SegmentType::ChordRest) {
        endSegment = endSegment->next1(mu::engraving::SegmentType::ChordRest);
    }
    if (endSegment && endSegment->element(dStaff * mu::engraving::VOICES)) {
        EngravingItem* e = endSegment->element(dStaff * mu::engraving::VOICES);
        if (e) {
            startEdit(TranslatableString("undoableAction", "Repeat selection"));
            ChordRest* cr = toChordRest(e);
            score()->pasteStaff(xml, cr->segment(), cr->staffIdx());
            apply();

            showItem(cr);
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
    startEdit(TranslatableString("undoableAction", "Paste"));

    if (isTextEditingStarted()) {
        const QMimeData* mimeData = QApplication::clipboard()->mimeData();
        if (mimeData->hasFormat(TextEditData::mimeRichTextFormat)) {
            const QString txt = QString::fromUtf8(mimeData->data(TextEditData::mimeRichTextFormat));
            toTextBase(m_editData.element)->paste(m_editData, txt);
        } else {
            QString clipboardText = mimeData->text();
            QString textForPaste = clipboardText;
            if ((!clipboardText.startsWith('<') || !clipboardText.contains('>')) && m_editData.element->isLyrics()) {
                textForPaste = extractSyllable(clipboardText);
            }

            toTextBase(m_editData.element)->paste(m_editData, textForPaste);

            if (!textForPaste.isEmpty() && m_editData.element->isLyrics()) {
                if (textForPaste.endsWith('-')) {
                    navigateToNextSyllable();
                } else if (textForPaste.endsWith('_')) {
                    addMelisma();
                } else {
                    navigateToLyrics(false, false, false);
                }

                QString textForNextPaste = clipboardText.remove(0, clipboardText.indexOf(textForPaste) + textForPaste.size());
                QGuiApplication::clipboard()->setText(textForNextPaste);
            }
        }
    } else {
        const QMimeData* mimeData = QApplication::clipboard()->mimeData();
        QMimeDataAdapter ma(mimeData);
        score()->cmdPaste(&ma, nullptr, scale);
    }

    apply();

    if (EngravingItem* element = selection()->element()) {
        selectAndStartEditIfNeeded(element);
    }

    MScoreErrorsController(iocContext()).checkAndShowMScoreError();
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
            tickLen = mu::engraving::Fraction::fromString(reader.attribute("len"));
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

    startEdit(TranslatableString("undoableAction", "Delete"));

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

    MScoreErrorsController(iocContext()).checkAndShowMScoreError();
    apply();
    resetHitElementContext();
}

void NotationInteraction::flipSelection()
{
    if (selection()->isNone()) {
        return;
    }

    startEdit(TranslatableString("undoableAction", "Flip direction"));
    score()->cmdFlip();
    apply();
}

void NotationInteraction::addTieToSelection()
{
    // Calls `startEdit` internally
    Tie* newTie = score()->cmdToggleTie();

    notifyAboutNotationChanged();

    if (newTie && newTie->tieJumpPoints() && newTie->tieJumpPoints()->size() > 1) {
        selectAndStartEditIfNeeded(newTie);
    }
}

void NotationInteraction::addLaissezVibToSelection()
{
    // Calls `startEdit` internally
    score()->cmdToggleLaissezVib();

    notifyAboutNotationChanged();
}

void NotationInteraction::addTiedNoteToChord()
{
    // Calls `startEdit` internally
    score()->cmdAddTie(true);

    notifyAboutNotationChanged();
}

void NotationInteraction::addSlurToSelection()
{
    if (selection()->isNone()) {
        return;
    }

    // Calls `startEdit` internally
    doAddSlur();
}

void NotationInteraction::addOttavaToSelection(OttavaType type)
{
    if (selection()->isNone()) {
        return;
    }

    startEdit(TranslatableString("undoableAction", "Add ottava"));
    score()->cmdAddOttava(type);
    apply();
}

void NotationInteraction::addHairpinOnGripDrag(Dynamic* dynamic, bool isLeftGrip)
{
    startEdit(TranslatableString("undoableAction", "Add hairpin"));

    const PointF pos = m_dragData.ed.pos;
    Hairpin* hairpin = score()->addHairpinToDynamicOnGripDrag(dynamic, isLeftGrip, pos);

    if (!hairpin) {
        rollback();
        return;
    }

    apply();

    // Reset grip offset to zero after drawing the hairpin
    dynamic->resetRightDragOffset();

    IF_ASSERT_FAILED(!hairpin->segmentsEmpty()) {
        return;
    }

    if (isLeftGrip) {
        LineSegment* segment = hairpin->frontSegment();
        select({ segment });
        startEditGrip(segment, Grip::START);
    } else {
        LineSegment* segment = hairpin->backSegment();
        select({ segment });
        startEditGrip(segment, Grip::END);
    }
}

void NotationInteraction::addHairpinsToSelection(HairpinType type)
{
    if (selection()->isNone()) {
        return;
    }

    startEdit(TranslatableString("undoableAction", "Add hairpin"));
    std::vector<mu::engraving::Hairpin*> hairpins = score()->addHairpins(type);
    apply();

    if (!noteInput()->isNoteInputMode() && hairpins.size() == 1) {
        mu::engraving::LineSegment* segment = hairpins.front()->frontSegment();
        select({ segment });
        startEditGrip(segment, mu::engraving::Grip::END);
    }
}

void NotationInteraction::putRestToSelection()
{
    mu::engraving::InputState& is = score()->inputState();
    if (!is.duration().isValid() || is.duration().isZero() || is.duration().isMeasure()) {
        is.setDuration(DurationType::V_QUARTER);
    }

    if (!m_noteInput->isNoteInputMode()) {
        m_noteInput->startNoteInput(configuration()->defaultNoteInputMethod());
    }

    if (is.usingNoteEntryMethod(NoteEntryMethod::BY_DURATION) || is.usingNoteEntryMethod(NoteEntryMethod::RHYTHM)) {
        m_noteInput->padNote(Pad::REST);
    } else {
        putRest(is.duration());
    }
}

void NotationInteraction::putRest(Duration duration)
{
    if (selection()->isNone()) {
        return;
    }

    // Calls `startEdit` internally
    score()->cmdEnterRest(duration);

    notifyAboutNotationChanged();
}

void NotationInteraction::addBracketsToSelection(BracketsType type)
{
    if (selection()->isNone()) {
        return;
    }

    switch (type) {
    case BracketsType::Brackets:
        startEdit(TranslatableString("undoableAction", "Add brackets"));
        score()->cmdAddBracket();
        apply();
        break;
    case BracketsType::Braces:
        startEdit(TranslatableString("undoableAction", "Add braces"));
        score()->cmdAddBraces();
        apply();
        break;
    case BracketsType::Parentheses:
        startEdit(TranslatableString("undoableAction", "Add parentheses"));
        score()->cmdAddParentheses();
        apply();
        break;
    }
}

void NotationInteraction::toggleAccidentalForSelection(AccidentalType type)
{
    if (selection()->isNone()) {
        return;
    }

    bool accidentalAlreadyAdded = false;
    for (const EngravingItem* item : score()->selection().elements()) {
        if (!item->isNote()) {
            continue;
        }

        if (toNote(item)->accidentalType() != type) {
            accidentalAlreadyAdded = false;
            break;
        }

        accidentalAlreadyAdded = true;
    }

    startEdit(TranslatableString("undoableAction", "Toggle accidental"));

    if (accidentalAlreadyAdded) {
        score()->changeAccidental(AccidentalType::NONE);
    } else {
        score()->changeAccidental(type);
    }

    apply();
}

void NotationInteraction::toggleArticulationForSelection(SymbolId articulationSymbolId)
{
    if (selection()->isNone()) {
        return;
    }

    std::vector<mu::engraving::Note*> notes = score()->selection().noteList();
    if (notes.empty()) {
        // no notes, but maybe they have an articulation selected. we should use that chord
        EngravingItem* e = score()->selection().element();
        if (e && e->isArticulationFamily()) {
            Chord* c = toChord(toArticulation(e)->explicitParent());
            if (c) {
                notes.insert(notes.begin(), c->notes().begin(), c->notes().end());
            }
        }
    }

    auto updateMode = notesHaveActiculation(notes, articulationSymbolId)
                      ? mu::engraving::ArticulationsUpdateMode::Remove : mu::engraving::ArticulationsUpdateMode::Insert;

    std::set<Chord*> chords;
    for (Note* note: notes) {
        if (note->isTrillCueNote()) {
            return;
        }
        Chord* chord = note->chord();
        if (chords.find(chord) == chords.end()) {
            chords.insert(chord);
        }
    }

    startEdit(TranslatableString("undoableAction", "Toggle articulation"));
    for (Chord* chord: chords) {
        chord->updateArticulations({ articulationSymbolId }, updateMode);
    }
    apply();
}

void NotationInteraction::toggleDotsForSelection(Pad dots)
{
    IF_ASSERT_FAILED(dots >= Pad::DOT && dots <= Pad::DOT4) {
        return;
    }

    if (selection()->isNone()) {
        return;
    }

    startEdit(TranslatableString("undoableAction", "Toggle augmentation dots"));
    score()->padToggle(dots, true /*toggleForSelectionOnly*/);
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

    startEdit(TranslatableString("undoableAction", "Add grace note"));
    score()->cmdAddGrace(type, mu::engraving::Constants::DIVISION / denominator);
    apply();
}

bool NotationInteraction::canAddTupletToSelectedChordRests() const
{
    for (ChordRest* chordRest : score()->getSelectedChordRests()) {
        if (chordRest->isGrace() || (chordRest->isChord() && toChord(chordRest)->isTrillCueNote())) {
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

    startEdit(TranslatableString("undoableAction", "Add tuplet"));

    for (ChordRest* chordRest : score()->getSelectedChordRests()) {
        if (!chordRest->isGrace() && !(chordRest->isChord() && toChord(chordRest)->isTrillCueNote())) {
            Fraction ratio = options.ratio;
            if (options.autoBaseLen) {
                ratio.setDenominator(Tuplet::computeTupletDenominator(ratio.numerator(), chordRest->ticks()));
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

    startEdit(TranslatableString("undoableAction", "Set beam type"));
    score()->cmdSetBeamMode(mode);
    apply();
}

void NotationInteraction::increaseDecreaseDuration(int steps, bool stepByDots)
{
    if (selection()->isNone()) {
        return;
    }

    startEdit(steps >= 0
              ? TranslatableString("undoableAction", "Increase duration")
              : TranslatableString("undoableAction", "Decrease duration"));
    score()->cmdIncDecDuration(steps, stepByDots);
    apply();
}

bool NotationInteraction::toggleLayoutBreakAvailable() const
{
    return !selection()->isNone() && !isTextEditingStarted();
}

void NotationInteraction::toggleLayoutBreak(LayoutBreakType breakType)
{
    startEdit(TranslatableString("undoableAction", "Toggle layout break"));
    score()->cmdToggleLayoutBreak(breakType);
    apply();
}

void NotationInteraction::moveMeasureToPrevSystem()
{
    startEdit(TranslatableString("undoableAction", "Move measure to previous system"));
    score()->cmdMoveMeasureToPrevSystem();
    apply();
}

void NotationInteraction::moveMeasureToNextSystem()
{
    startEdit(TranslatableString("undoableAction", "Move measure to next system"));
    score()->cmdMoveMeasureToNextSystem();
    apply();
}

void NotationInteraction::toggleSystemLock()
{
    startEdit(TranslatableString("undoableAction", "Lock/unlock selected system(s)"));
    score()->cmdToggleSystemLock();
    apply();
}

void NotationInteraction::toggleScoreLock()
{
    startEdit(TranslatableString("undoableAction", "Lock/unlock all systems"));
    score()->cmdToggleScoreLock();
    apply();
}

void NotationInteraction::makeIntoSystem()
{
    startEdit(TranslatableString("undoableAction", "Create system from selection"));
    score()->cmdMakeIntoSystem();
    apply();
}

void NotationInteraction::applySystemLock()
{
    startEdit(TranslatableString("undoableAction", "Apply system lock to selection"));
    score()->cmdApplyLockToSelection();
    apply();
}

void NotationInteraction::addRemoveSystemLocks(AddRemoveSystemLockType intervalType, int interval)
{
    interval = intervalType == AddRemoveSystemLockType::MeasuresInterval ? interval : 0;
    bool afterEachSystem = intervalType == AddRemoveSystemLockType::AfterEachSystem;

    startEdit(TranslatableString("undoableAction", "Measures per system"));
    score()->addRemoveSystemLocks(interval, afterEachSystem);
    apply();
}

bool NotationInteraction::transpose(const TransposeOptions& options)
{
    startEdit(TranslatableString("undoableAction", "Transposition"));

    bool ok = score()->transpose(options.mode, options.direction, options.key, options.interval,
                                 options.needTransposeKeys, options.needTransposeChordNames, options.needTransposeDoubleSharpsFlats);

    apply();

    return ok;
}

void NotationInteraction::swapVoices(voice_idx_t voiceIndex1, voice_idx_t voiceIndex2)
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

    startEdit(TranslatableString("undoableAction", "Swap voices"));
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
        MScore::setError(MsError::NO_NOTE_SELECTED);
        MScoreErrorsController(iocContext()).checkAndShowMScoreError();
        return;
    }

    startEdit(TranslatableString("undoableAction", "Add interval"));
    score()->addInterval(interval, notes);
    apply();
}

void NotationInteraction::addFret(int fretIndex)
{
    startEdit(TranslatableString("undoableAction", "Enter note at fret %1").arg(fretIndex));
    score()->cmdAddFret(fretIndex);
    apply();
}

void NotationInteraction::changeSelectedElementsVoice(voice_idx_t voiceIndex)
{
    if (selection()->isNone()) {
        return;
    }

    if (!isVoiceIndexValid(voiceIndex)) {
        return;
    }

    startEdit(TranslatableString("undoableAction", "Change voice"));
    score()->changeSelectedElementsVoice(voiceIndex);
    apply();
}

void NotationInteraction::changeSelectedElementsVoiceAssignment(VoiceAssignment voiceAssignment)
{
    if (selection()->isNone()) {
        return;
    }

    startEdit(TranslatableString("undoableAction", "Change voice assignment"));
    score()->changeSelectedElementsVoiceAssignment(voiceAssignment);
    apply();
}

void NotationInteraction::addAnchoredLineToSelectedNotes()
{
    if (selection()->isNone()) {
        return;
    }

    startEdit(TranslatableString("undoableAction", "Add note anchored line"));
    score()->addNoteLine();
    apply();
}

void NotationInteraction::addTextToTopFrame(TextStyleType type)
{
    addText(type);
}

Ret NotationInteraction::canAddTextToItem(TextStyleType type, const EngravingItem* item) const
{
    if (isVerticalBoxTextStyle(type)) {
        return item && item->isVBox();
    }

    if (type == TextStyleType::FRAME) {
        return item && item->isBox() ? muse::make_ok() : make_ret(Err::EmptySelection);
    }

    static const std::set<TextStyleType> needSelectNoteOrRestTypes {
        TextStyleType::SYSTEM,
        TextStyleType::STAFF,
        TextStyleType::DYNAMICS,
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

    if (muse::contains(needSelectNoteOrRestTypes, type)) {
        static const std::set<ElementType> requiredElementTypes {
            ElementType::NOTE,
            ElementType::REST,
            ElementType::MMREST,
            ElementType::MEASURE_REPEAT,
            ElementType::CHORD,
        };

        bool isNoteOrRestSelected = item && muse::contains(requiredElementTypes, item->type());
        return isNoteOrRestSelected ? muse::make_ok() : make_ret(Err::NoteOrRestIsNotSelected);
    }

    return muse::make_ok();
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

    startEdit(TranslatableString("undoableAction", "Add text"));
    mu::engraving::TextBase* text = score()->addText(type, item);

    if (!text) {
        rollback();
        return;
    }

    if (text->isInstrumentChange()) {
        if (!selectInstrument(toInstrumentChange(text))) {
            rollback();
            return;
        }
    }

    if (text->hasVoiceAssignmentProperties()) {
        text->setInitialTrackAndVoiceAssignment(item->track(), false);
    }

    apply();
    showItem(text);

    if (!text->isInstrumentChange()) {
        startEditText(text);
    }

    if (text->isRehearsalMark() || text->isTempoText()) {
        text->cursor()->selectWord();
    }
}

Ret NotationInteraction::canAddImageToItem(const EngravingItem* item) const
{
    return item && item->isMeasureBase();
}

void NotationInteraction::addImageToItem(const muse::io::path_t& imagePath, EngravingItem* item)
{
    if (imagePath.empty() || !item) {
        return;
    }

    static const std::map<muse::io::path_t, ImageType> suffixToType {
        { "svg", ImageType::SVG },
        { "svgz", ImageType::SVG },
        { "jpg", ImageType::RASTER },
        { "jpeg", ImageType::RASTER },
        { "png", ImageType::RASTER },
        { "bmp", ImageType::RASTER },
        { "tif", ImageType::RASTER },
        { "tiff", ImageType::RASTER },
    };

    muse::io::path_t suffix = io::suffix(imagePath);

    ImageType type = muse::value(suffixToType, suffix, ImageType::NONE);
    if (type == ImageType::NONE) {
        return;
    }

    Image* image = Factory::createImage(item);
    image->setImageType(type);

    if (!image->load(imagePath)) {
        delete image;
        return;
    }

    startEdit(TranslatableString("undoableAction", "Add image"));
    score()->undoAddElement(image);
    apply();
}

Ret NotationInteraction::canAddFiguredBass() const
{
    static const std::set<ElementType> requiredTypes {
        ElementType::NOTE,
        ElementType::FIGURED_BASS,
        ElementType::REST
    };

    bool isNoteOrRestSelected = elementsSelected(requiredTypes);
    return isNoteOrRestSelected ? muse::make_ok() : make_ret(Err::NoteOrFiguredBassIsNotSelected);
}

void NotationInteraction::addFiguredBass()
{
    startEdit(TranslatableString("undoableAction", "Add figured bass"));
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
    startEdit(value >= 0
              ? TranslatableString("undoableAction", "Increase layout stretch")
              : TranslatableString("undoableAction", "Decrease layout stretch"));
    score()->cmdAddStretch(value);
    apply();
}

Measure* NotationInteraction::selectedMeasure() const
{
    INotationInteraction::HitElementContext context = hitElementContext();
    mu::engraving::Measure* measure = context.element && context.element->isMeasure() ? mu::engraving::toMeasure(context.element) : nullptr;

    if (!measure) {
        INotationSelectionPtr selection = this->selection();
        if (selection->isRange()) {
            measure = selection->range()->measureRange().endMeasure;
        } else if (selection->element()) {
            measure = selection->element()->findMeasure();
        }
    }
    return measure;
}

void NotationInteraction::addTimeSignature(Measure* measure, staff_idx_t staffIndex, TimeSignature* timeSignature)
{
    startEdit(TranslatableString("undoableAction", "Add time signature"));
    score()->cmdAddTimeSig(measure, staffIndex, timeSignature, true);
    apply();
}

void NotationInteraction::explodeSelectedStaff()
{
    if (!selection()->isRange()) {
        return;
    }

    startEdit(TranslatableString("undoableAction", "Explode"));
    score()->cmdExplode();
    apply();
}

void NotationInteraction::implodeSelectedStaff()
{
    if (!selection()->isRange()) {
        return;
    }

    startEdit(TranslatableString("undoableAction", "Implode"));
    score()->cmdImplode();
    apply();
}

void NotationInteraction::realizeSelectedChordSymbols(bool literal, Voicing voicing, HarmonyDurationType durationType)
{
    if (selection()->isNone()) {
        return;
    }

    startEdit(TranslatableString("undoableAction", "Realize chord symbols"));
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
        const std::vector<EngravingItem*>& elements = selection()->elements();
        if (elements.empty()) {
            return;
        }

        for (EngravingItem* element : elements) {
            mu::engraving::MeasureBase* elementMeasure = element->findMeasureBase();

            if (!firstMeasure || firstMeasure->index() > elementMeasure->index()) {
                firstMeasure = elementMeasure;
            }

            if (!lastMeasure || lastMeasure->index() < elementMeasure->index()) {
                lastMeasure = elementMeasure;
            }
        }
    }

    IF_ASSERT_FAILED(firstMeasure && lastMeasure) {
        return;
    }

    doSelect({ firstMeasure }, SelectType::REPLACE);
    doSelect({ lastMeasure }, SelectType::RANGE);

    int numDeletedMeasures = 1 + lastMeasure->measureIndex() - firstMeasure->measureIndex();

    startEdit(TranslatableString("undoableAction", "Delete %n measure(s)", nullptr, numDeletedMeasures));
    score()->cmdTimeDelete();
    apply();
}

void NotationInteraction::removeSelectedRange()
{
    if (selection()->isNone()) {
        return;
    }

    startEdit(TranslatableString("undoableAction", "Delete range"));
    score()->cmdTimeDelete();
    apply();
}

void NotationInteraction::removeEmptyTrailingMeasures()
{
    startEdit(TranslatableString("undoableAction", "Remove empty trailing measures"));
    score()->cmdRemoveEmptyTrailingMeasures();
    apply();
}

void NotationInteraction::fillSelectionWithSlashes()
{
    if (selection()->isNone()) {
        return;
    }

    startEdit(TranslatableString("undoableAction", "Fill with slashes"));
    score()->cmdSlashFill();
    apply();
}

void NotationInteraction::replaceSelectedNotesWithSlashes()
{
    if (selection()->isNone()) {
        return;
    }

    startEdit(TranslatableString("undoableAction", "Toggle rhythmic slash notation"));
    score()->cmdSlashRhythm();
    apply();
}

void NotationInteraction::changeEnharmonicSpelling(bool both)
{
    startEdit(TranslatableString("undoableAction", "Change enharmonic spelling"));
    score()->changeEnharmonicSpelling(both);
    apply();
}

void NotationInteraction::spellPitches()
{
    startEdit(TranslatableString("undoableAction", "Respell pitches"));
    score()->spell();
    apply();
}

void NotationInteraction::regroupNotesAndRests()
{
    startEdit(TranslatableString("undoableAction", "Regroup rhythms"));
    score()->cmdResetNoteAndRestGroupings();
    apply();
}

void NotationInteraction::resequenceRehearsalMarks()
{
    startEdit(TranslatableString("undoableAction", "Resequence rehearsal marks"));
    score()->cmdResequenceRehearsalMarks();
    apply();
}

void NotationInteraction::resetStretch()
{
    startEdit(TranslatableString("undoableAction", "Reset layout stretch"));
    score()->resetUserStretch();
    apply();
}

void NotationInteraction::resetTextStyleOverrides()
{
    startEdit(TranslatableString("undoableAction", "Reset text style overrides"));
    score()->cmdResetTextStyleOverrides();
    apply();
}

void NotationInteraction::resetBeamMode()
{
    startEdit(TranslatableString("undoableAction", "Reset beams"));
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

    startEdit(TranslatableString("undoableAction", "Reset shapes and positions"));

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

void NotationInteraction::resetToDefaultLayout()
{
    TRACEFUNC;

    startEdit(TranslatableString("undoableAction", "Reset to default layout"));
    score()->cmdResetToDefaultLayout();
    apply();
}

ScoreConfig NotationInteraction::scoreConfig() const
{
    ScoreConfig config;
    config.isShowInvisibleElements = score()->isShowInvisible();
    config.isShowUnprintableElements = score()->showUnprintable();
    config.isShowFrames = score()->showFrames();
    config.isShowPageMargins = score()->showPageborders();
    config.isShowSoundFlags = score()->showSoundFlags();
    config.isMarkIrregularMeasures = score()->markIrregularMeasures();

    return config;
}

void NotationInteraction::setScoreConfig(const ScoreConfig& config)
{
    if (scoreConfig() == config) {
        return;
    }

    // TODO: refactor together with `NotationActionController::toggleScoreConfig`
    // to be able to give a more precise name
    startEdit(TranslatableString("undoableAction", "Set score view settings"));
    score()->setShowInvisible(config.isShowInvisibleElements);
    score()->setShowUnprintable(config.isShowUnprintableElements);
    score()->setShowFrames(config.isShowFrames);
    score()->setShowPageborders(config.isShowPageMargins);
    score()->setShowSoundFlags(config.isShowSoundFlags);
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

    if (m_editData.element && m_editData.element->isStaffText()) {
        EngravingItem* element = newSelectedElements.front();
        if (element && element->isSoundFlag() && element->parentItem() == m_editData.element) {
            return false;
        }
    }

    return newSelectedElements.front() != m_editData.element;
}

bool NotationInteraction::needEndElementEditing(const std::vector<EngravingItem*>& newSelectedElements) const
{
    if (!isElementEditStarted()) {
        return false;
    }

    if (newSelectedElements.size() != 1) {
        return true;
    }

    if (m_editData.element && m_editData.element->isStaffText()) {
        EngravingItem* element = newSelectedElements.front();
        if (element && element->isSoundFlag() && element->parentItem() == m_editData.element) {
            return false;
        }
    }

    return newSelectedElements.front() != score()->selection().element();
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
    return element && muse::contains(elementsTypes, element->type());
}

//! NOTE: Copied from ScoreView::lyricsTab
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
    mu::engraving::FontStyle fStyle = lyrics->fontStyle();
    mu::engraving::PropertyFlags fFlags = lyrics->propertyFlags(mu::engraving::Pid::FONT_STYLE);

    mu::engraving::Segment* nextSegment = segment;
    if (back) {
        // search prev chord
        while ((nextSegment = nextSegment->prev1(mu::engraving::SegmentType::ChordRest))) {
            EngravingItem* el = nextSegment->element(track);
            if (!el) {
                continue;
            }
            if (el->isChord()) {
                break;
            } else if (el->isRest()) {
                mu::engraving::Lyrics* nextLyrics = toChordRest(el)->lyrics(verse, placement);
                if (nextLyrics) {
                    break;
                }
            }
        }
    } else {
        // search next chord
        while ((nextSegment = nextSegment->next1(mu::engraving::SegmentType::ChordRest))) {
            EngravingItem* el = nextSegment->element(track);
            if (!el) {
                continue;
            }
            if (el->isChord()) {
                break;
            } else if (el->isRest()) {
                mu::engraving::Lyrics* nextLyrics = toChordRest(el)->lyrics(verse, placement);
                if (nextLyrics) {
                    break;
                }
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
        const mu::engraving::TextStyleType styleType(nextLyrics->isEven() ? TextStyleType::LYRICS_EVEN : TextStyleType::LYRICS_ODD);
        nextLyrics->setTextStyleType(styleType);

        nextLyrics->setPlacement(placement);
        nextLyrics->setPropertyFlags(mu::engraving::Pid::PLACEMENT, pFlags);
        nextLyrics->setSyllabic(mu::engraving::LyricsSyllabic::SINGLE);
        nextLyrics->setFontStyle(fStyle);
        nextLyrics->setPropertyFlags(mu::engraving::Pid::FONT_STYLE, fFlags);
        newLyrics = true;
    }

    score()->startCmd(TranslatableString("undoableAction", "Navigate to lyrics"));
    if (fromLyrics && !moveOnly) {
        switch (nextLyrics->syllabic()) {
        // as we arrived at nextLyrics by a [Space], it can be the beginning
        // of a multi-syllable, but cannot have syllabic dashes before
        case mu::engraving::LyricsSyllabic::SINGLE:
        case mu::engraving::LyricsSyllabic::BEGIN:
            break;
        case mu::engraving::LyricsSyllabic::END:
            nextLyrics->undoChangeProperty(mu::engraving::Pid::SYLLABIC, int(mu::engraving::LyricsSyllabic::SINGLE));
            break;
        case mu::engraving::LyricsSyllabic::MIDDLE:
            nextLyrics->undoChangeProperty(mu::engraving::Pid::SYLLABIC, int(mu::engraving::LyricsSyllabic::BEGIN));
            break;
        }
        // as we moved away from fromLyrics by a [Space], it can be
        // the end of a multi-syllable, but cannot have syllabic dashes after
        switch (fromLyrics->syllabic()) {
        case mu::engraving::LyricsSyllabic::SINGLE:
        case mu::engraving::LyricsSyllabic::END:
            break;
        case mu::engraving::LyricsSyllabic::BEGIN:
            fromLyrics->undoChangeProperty(mu::engraving::Pid::SYLLABIC, int(mu::engraving::LyricsSyllabic::SINGLE));
            break;
        case mu::engraving::LyricsSyllabic::MIDDLE:
            fromLyrics->undoChangeProperty(mu::engraving::Pid::SYLLABIC, int(mu::engraving::LyricsSyllabic::END));
            break;
        }
        // for the same reason, it cannot have a melisma
        if (fromLyrics->separator() && !fromLyrics->separator()->isEndMelisma()) {
            fromLyrics->undoChangeProperty(mu::engraving::Pid::LYRIC_TICKS, Fraction::fromTicks(0));
        }
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
    } else if (!newLyrics) {
        cursor->movePosition(mu::engraving::TextCursor::MoveOperation::End, mu::engraving::TextCursor::MoveMode::MoveAnchor);
        cursor->movePosition(mu::engraving::TextCursor::MoveOperation::Start, mu::engraving::TextCursor::MoveMode::KeepAnchor);
    }

    showItem(nextLyrics);
}

void NotationInteraction::navigateToLyrics(MoveDirection direction, bool moveOnly)
{
    navigateToLyrics(direction == MoveDirection::Left, moveOnly, false);
}

//! NOTE: Copied from ScoreView::lyricsTab
void NotationInteraction::navigateToNextSyllable()
{
    if (!m_editData.element || !m_editData.element->isLyrics()) {
        LOGW("nextSyllable called with invalid current element");
        return;
    }
    Lyrics* lyrics = toLyrics(m_editData.element);
    ChordRest* initialCR = lyrics->chordRest();
    const bool hasPrecedingRepeat = initialCR->hasPrecedingJumpItem();
    const bool hasFollowingRepeat = initialCR->hasFollowingJumpItem();
    track_idx_t track = lyrics->track();
    track_idx_t toLyricTrack = track;
    Segment* segment = lyrics->segment();
    int verse = lyrics->no();
    PlacementV placement = lyrics->placement();
    PropertyFlags pFlags = lyrics->propertyFlags(Pid::PLACEMENT);
    FontStyle fStyle = lyrics->fontStyle();
    PropertyFlags fFlags = lyrics->propertyFlags(Pid::FONT_STYLE);

    // search next chord
    Segment* nextSegment = segment;
    while ((nextSegment = nextSegment->next1(SegmentType::ChordRest))) {
        EngravingItem* el = nextSegment->element(track);
        if (!el || !el->isChord()) {
            const track_idx_t strack = track2staff(track) * VOICES;
            const track_idx_t etrack = strack + VOICES;
            for (track_idx_t t = strack; t < etrack; ++t) {
                el = nextSegment->element(t);
                if (el && el->isChord() && toChord(el)->lyrics(verse, placement)) {
                    toLyricTrack = t;
                    break;
                }
            }
        }

        if (el && el->isChord()) {
            break;
        }
    }

    if (!segmentsAreAdjacentInRepeatStructure(segment, nextSegment)) {
        nextSegment = nullptr;
    }

    if (!nextSegment && !hasFollowingRepeat) {
        return;
    }

    endEditText();

    // look for the lyrics we are moving from; may be the current lyrics or a previous one
    // we are extending with several dashes
    Lyrics* fromLyrics = 0;
    while (segment) {
        ChordRest* cr = toChordRest(segment->element(track));
        if (!cr) {
            segment = segment->prev1(SegmentType::ChordRest);
            continue;
        }
        fromLyrics = cr->lyrics(verse, placement);
        if (fromLyrics) {
            break;
        }
        segment = segment->prev1(SegmentType::ChordRest);
    }

    if (!nextSegment && hasFollowingRepeat && fromLyrics) {
        // Allow dash with no end syllable if there is a repeat
        score()->startCmd(TranslatableString("undoableAction", "Navigate to next syllable"));
        switch (fromLyrics->syllabic()) {
        case LyricsSyllabic::BEGIN:
        case LyricsSyllabic::MIDDLE:
            break;
        case LyricsSyllabic::SINGLE:
            fromLyrics->undoChangeProperty(Pid::SYLLABIC, int(LyricsSyllabic::BEGIN));
            break;
        case LyricsSyllabic::END:
            fromLyrics->undoChangeProperty(Pid::SYLLABIC, int(LyricsSyllabic::MIDDLE));
            break;
        }
        fromLyrics->undoChangeProperty(Pid::LYRIC_TICKS, Fraction(0, 1));

        score()->endCmd();
        score()->setLayoutAll();

        return;
    }

    score()->startCmd(TranslatableString("undoableAction", "Navigate to next syllable"));
    ChordRest* cr = nextSegment ? toChordRest(nextSegment->element(toLyricTrack)) : nullptr;
    Lyrics* toLyrics = cr ? cr->lyrics(verse, placement) : nullptr;

    if (!toLyrics && !cr) {
        return;
    }

    // If no lyrics in current track, check others
    if (!toLyrics) {
        const track_idx_t strack = track2staff(track) * VOICES;
        const track_idx_t etrack = strack + VOICES;
        for (track_idx_t t = strack; t < etrack; ++t) {
            if (t == track) {
                continue;
            }
            cr = toChordRest(nextSegment->element(t));
            if (cr) {
                toLyrics = cr->lyrics(verse, placement);
                if (toLyrics) {
                    break;
                }
            }
        }
    }

    // This will be a partial dash at the start of a measure after a repeat
    // We don't want toLyrics
    toLyrics = hasPrecedingRepeat && !fromLyrics ? nullptr : toLyrics;

    // Make sure we end up with either the cr of toLyrics or cr on correct track
    cr = !toLyrics ? toChordRest(nextSegment->element(track)) : cr;

    // Disallow dashes between non-adjacent repeat sections eg. 1st volta -> 2nd volta
    // Instead, try to add partial dashes
    if (cr && fromLyrics) {
        Measure* toLyricsMeasure = cr->measure();
        Measure* fromLyricsMeasure = fromLyrics->measure();

        if (toLyricsMeasure != fromLyricsMeasure && fromLyricsMeasure->lastChordRest(track)->hasFollowingJumpItem()) {
            const std::vector<Measure*> previousRepeats = findPreviousRepeatMeasures(toLyricsMeasure);
            const bool inPrecedingRepeatSeg = muse::contains(previousRepeats, fromLyricsMeasure);
            if (!previousRepeats.empty() && !inPrecedingRepeatSeg) {
                fromLyrics = nullptr;
            }
        }
    }

    PartialLyricsLine* prevPartialLyricsLine = nullptr;

    for (auto sp : score()->spannerMap().findOverlapping(initialCR->tick().ticks(), initialCR->tick().ticks())) {
        if (!sp.value->isPartialLyricsLine() || sp.value->track() != track) {
            continue;
        }
        PartialLyricsLine* partialLine = toPartialLyricsLine(sp.value);
        if (partialLine->isEndMelisma() || partialLine->no() != lyrics->no() || partialLine->placement() != lyrics->placement()) {
            continue;
        }
        prevPartialLyricsLine = partialLine;
        break;
    }

    bool newLyrics = (toLyrics == 0);
    if (!toLyrics || hasPrecedingRepeat) {
        // Don't advance cursor if we are after a repeat, there is no partial dash present and we are inputting a dash
        ChordRest* toLyricsChord = hasPrecedingRepeat && !prevPartialLyricsLine && lyrics->xmlText().empty() ? initialCR : cr;

        toLyrics = Factory::createLyrics(toLyricsChord);
        toLyrics->setTrack(track);
        toLyrics->setParent(toLyricsChord);

        toLyrics->setNo(verse);
        const TextStyleType styleType(toLyrics->isEven() ? TextStyleType::LYRICS_EVEN : TextStyleType::LYRICS_ODD);
        toLyrics->setTextStyleType(styleType);

        toLyrics->setPlacement(placement);
        toLyrics->setPropertyFlags(Pid::PLACEMENT, pFlags);
        toLyrics->setSyllabic(LyricsSyllabic::END);
        toLyrics->setFontStyle(fStyle);
        toLyrics->setPropertyFlags(Pid::FONT_STYLE, fFlags);
    } else {
        // as we arrived at toLyrics by a dash, it cannot be initial or isolated
        if (toLyrics->syllabic() == LyricsSyllabic::BEGIN) {
            toLyrics->undoChangeProperty(Pid::SYLLABIC, int(LyricsSyllabic::MIDDLE));
        } else if (toLyrics->syllabic() == LyricsSyllabic::SINGLE) {
            toLyrics->undoChangeProperty(Pid::SYLLABIC, int(LyricsSyllabic::END));
        }
    }

    if (fromLyrics) {
        // as we moved away from fromLyrics by a dash,
        // it can have syll. dashes before and after but cannot be isolated or terminal
        switch (fromLyrics->syllabic()) {
        case LyricsSyllabic::BEGIN:
        case LyricsSyllabic::MIDDLE:
            break;
        case LyricsSyllabic::SINGLE:
            fromLyrics->undoChangeProperty(Pid::SYLLABIC, int(LyricsSyllabic::BEGIN));
            break;
        case LyricsSyllabic::END:
            fromLyrics->undoChangeProperty(Pid::SYLLABIC, int(LyricsSyllabic::MIDDLE));
            break;
        }
        // for the same reason, it cannot have a melisma
        fromLyrics->undoChangeProperty(Pid::LYRIC_TICKS, Fraction(0, 1));
    } else if (hasPrecedingRepeat && !prevPartialLyricsLine) {
        // No from lyrics - create incoming partial dash
        PartialLyricsLine* dash = Factory::createPartialLyricsLine(score()->dummy());
        dash->setIsEndMelisma(false);
        dash->setNo(verse);
        dash->setPlacement(lyrics->placement());
        dash->setTick(initialCR->tick());
        dash->setTicks(hasPrecedingRepeat ? Fraction(0, 1) : initialCR->ticks());
        dash->setTrack(initialCR->track());
        dash->setTrack2(initialCR->track());

        score()->undoAddElement(dash);
    } else if (prevPartialLyricsLine) {
        const Fraction tickDiff = cr->tick() - prevPartialLyricsLine->tick2();
        prevPartialLyricsLine->undoMoveEnd(tickDiff);
        prevPartialLyricsLine->triggerLayout();
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

//! NOTE: Copied from ScoreView::lyricsUpDown
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
    mu::engraving::FontStyle fStyle = lyrics->fontStyle();
    mu::engraving::PropertyFlags fFlags = lyrics->propertyFlags(mu::engraving::Pid::FONT_STYLE);

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
        const mu::engraving::TextStyleType styleType(lyrics->isEven() ? TextStyleType::LYRICS_EVEN : TextStyleType::LYRICS_ODD);
        lyrics->setTextStyleType(styleType);

        lyrics->setPlacement(placement);
        lyrics->setPropertyFlags(mu::engraving::Pid::PLACEMENT, pFlags);
        lyrics->setFontStyle(fStyle);
        lyrics->setPropertyFlags(mu::engraving::Pid::FONT_STYLE, fFlags);

        score()->startCmd(TranslatableString("undoableAction", "Navigate to verse"));
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

            segment = Factory::createSegment(measure, mu::engraving::SegmentType::TimeTick, newTick - measure->tick());
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

    startEdit(TranslatableString("undoableAction", "Navigate to next chord symbol"));

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

        startEdit(TranslatableString("undoableAction", "Navigate to next chord symbol"));
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

    startEdit(TranslatableString("undoableAction", "Navigate to chord symbol"));

    if (!segment || segment->tick() > newTick) {      // no ChordRest segment at this tick
        segment = EditTimeTickAnchors::createTimeTickAnchor(measure, newTick - measure->tick(), harmony->staffIdx())->segment();
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
        startEdit(TranslatableString("undoableAction", "Navigate to next figured bass"));
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
        startEdit(TranslatableString("undoableAction", "Navigate to next figured bass"));
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

    doEndEditElement();

    // look for a segment at this tick; if none, create one
    mu::engraving::Segment* nextSegm = segm;
    while (nextSegm && nextSegm->tick() < nextSegTick) {
        nextSegm = nextSegm->next1(mu::engraving::SegmentType::ChordRest);
    }

    if (!nextSegm || nextSegm->tick() > nextSegTick) {      // no ChordRest segm at this tick
        nextSegm = EditTimeTickAnchors::createTimeTickAnchor(measure, nextSegTick - measure->tick(), fb->staffIdx())->segment();
        if (!nextSegm) {
            LOGD("figuredBassTicksTab: no next segment");
            return;
        }
    }

    startEdit(TranslatableString("undoableAction", "Navigate to figured bass"));

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
        LOGD("navigateToNearText: parent not note or segment.");
        return;
    }

    TextBase* ot = mu::engraving::toTextBase(oe);
    mu::engraving::TextStyleType textStyleType = ot->textStyleType();
    ElementType type = ot->type();
    mu::engraving::staff_idx_t staffIdx = ot->staffIdx();
    bool back = direction == MoveDirection::Left;
    int curTrack = static_cast<int>(oe->track());
    int minTrack = (curTrack / mu::engraving::VOICES) * mu::engraving::VOICES;
    int maxTrack = minTrack + mu::engraving::VOICES - 1;

    mu::engraving::EngravingItem* el = nullptr;

    if (op->isNote()) {
        // go to next/prev note in same chord, or go to next/prev chord, which may be in another voice
        Note* origNote = toNote(op);
        Chord* ch = origNote->chord();
        const std::vector<Note*>& notes = ch->notes();

        // first, try going to prev/next note in the current chord
        if (origNote != (back ? notes.back() : notes.front())) {
            auto it = std::find(notes.begin(), notes.end(), origNote);
            if (it != notes.end()) {
                el = back ? *std::next(it) : *std::prev(it);
            }
        }

        // next, try going to next/prev grace note chord in the same group as the current
        const std::vector<Chord*> chordList = ch->allGraceChordsOfMainChord();

        if (!el && ch != (back ? chordList.front() : chordList.back())) {
            auto it = std::find(chordList.begin(), chordList.end(), ch);
            if (it != chordList.end()) {
                if (back) {
                    const Chord* targetChord = *std::prev(it);
                    el = targetChord->notes().front();
                } else {
                    const Chord* targetChord = *std::next(it);
                    el = targetChord->notes().back();
                }
            }
        }

        // next, try going to prev/next chord in another voice
        if (!el) {
            Segment* seg = ch->segment();
            if (!seg) {
                LOGD("navigateToNearText: no segment");
                return;
            }
            int sTrack = back ? curTrack - 1 : curTrack + 1;
            int eTrack = back ? minTrack : maxTrack;
            int inc = back ? -1 : 1;
            for (int track = sTrack; back ? (track >= eTrack) : (track <= eTrack); track += inc) {
                EngravingItem* e = seg->element(track);
                if (e && e->isChord()) {
                    const std::vector<Chord*> targetChordList = toChord(e)->allGraceChordsOfMainChord();
                    if (back) {
                        Chord* targetChord = targetChordList.back();
                        el = targetChord->notes().front();
                    } else {
                        Chord* targetChord = targetChordList.front();
                        el = targetChord->notes().back();
                    }
                    break;
                }
            }
        }

        // finally, try going to chord in prev/next segments
        if (!el) {
            Segment* seg = ch->segment();
            seg = back ? seg->prev1(SegmentType::ChordRest) : seg->next1(SegmentType::ChordRest);
            int sTrack = back ? maxTrack : minTrack;
            int eTrack = back ? minTrack : maxTrack;
            int inc = back ? -1 : 1;
            while (seg) {
                for (int track = sTrack; back ? (track >= eTrack) : (track <= eTrack); track += inc) {
                    EngravingItem* e = seg->element(track);
                    if (e && e->isChord()) {
                        const std::vector<Chord*> targetChordList = toChord(e)->allGraceChordsOfMainChord();
                        if (back) {
                            Chord* targetChord = targetChordList.back();
                            el = targetChord->notes().front();
                        } else {
                            Chord* targetChord = targetChordList.front();
                            el = targetChord->notes().back();
                        }
                        break;
                    }
                }

                if (el) {
                    break;
                }

                seg = back ? seg->prev1(SegmentType::ChordRest) : seg->next1(SegmentType::ChordRest);
            }
        }
    } else if (op->isSegment()) {
        Segment* seg = toSegment(op);
        seg = back ? seg->prev1(SegmentType::ChordRest) : seg->next1(SegmentType::ChordRest);

        // go to first segment with a chord or an existing text
        while (seg) {
            for (int track = minTrack; track <= maxTrack; ++track) {
                EngravingItem* e = seg->element(track);
                if (e && e->isChord()) {
                    el = e;
                    break;
                }
            }
            if (el) {
                break;
            }

            // this segment only contains rests, check for existing text
            for (EngravingItem* e : seg->annotations()) {
                if (e->staffIdx() != staffIdx || e->type() != type) {
                    continue;
                }
                TextBase* nt = mu::engraving::toTextBase(e);
                if (nt->textStyleType() == textStyleType) {
                    el = seg->firstElement(staffIdx);
                    break;
                }
            }
            if (el) {
                break;
            }

            seg = back ? seg->prev1(SegmentType::ChordRest) : seg->next1(SegmentType::ChordRest);
        }
    }

    if (!el) {
        return;
    }

    // get existing text to edit
    EngravingItem* textEl = nullptr;
    if (op->isNote()) {
        if (!el->isNote()) {
            LOGD("navigateToNearText: new element is not Note.");
            return;
        }
        // check element list of new note
        for (mu::engraving::EngravingItem* e : toNote(el)->el()) {
            if (e->type() != type) {
                continue;
            }
            TextBase* nt = mu::engraving::toTextBase(e);
            if (nt->textStyleType() == textStyleType) {
                textEl = e;
                break;
            }
        }
    } else if (op->isSegment()) {
        if (!el->isChordRest()) {
            LOGD("navigateToNearText: new element is not ChordRest.");
            return;
        }
        // check annotation list of new segment
        mu::engraving::Segment* ns = toChordRest(el)->segment();
        for (mu::engraving::EngravingItem* e : ns->annotations()) {
            if (e->staffIdx() != staffIdx || e->type() != type) {
                continue;
            }
            TextBase* nt = mu::engraving::toTextBase(e);
            if (nt->textStyleType() == textStyleType) {
                textEl = e;
                break;
            }
        }
    }

    if (textEl) {
        // edit existing text
        TextBase* text = dynamic_cast<TextBase*>(textEl);

        if (text) {
            startEditText(text);
            text->selectAll(text->cursor());
            showItem(text);
        }
    } else {
        // add new text if no existing element to edit
        // TODO: for tempo text, mscore->addTempo() could be called
        // but it pre-fills the text
        // would be better to create empty tempo element
        if (type != ElementType::TEMPO_TEXT) {
            addTextToItem(textStyleType, el);
        }
    }
}

//! NOTE: Copied from ScoreView::lyricsUnderscore
void NotationInteraction::addMelisma()
{
    if (!m_editData.element || !m_editData.element->isLyrics()) {
        LOGW("addMelisma called with invalid current element");
        return;
    }
    Lyrics* lyrics = toLyrics(m_editData.element);
    ChordRest* initialCR = lyrics->chordRest();
    const bool hasPrecedingRepeat = initialCR->hasPrecedingJumpItem();
    track_idx_t track = lyrics->track();
    Segment* segment = lyrics->segment();
    int verse = lyrics->no();
    PlacementV placement = lyrics->placement();
    PropertyFlags pFlags = lyrics->propertyFlags(Pid::PLACEMENT);
    FontStyle fStyle = lyrics->fontStyle();
    PropertyFlags fFlags = lyrics->propertyFlags(Pid::FONT_STYLE);
    Fraction endTick = segment->tick(); // a previous melisma cannot extend beyond this point
    endEditText();

    // search next chord
    Segment* nextSegment = segment;
    while ((nextSegment = nextSegment->next1(SegmentType::ChordRest))) {
        EngravingItem* el = nextSegment->element(track);
        if (el && el->isChord()) {
            break;
        }
    }

    if (!segmentsAreAdjacentInRepeatStructure(segment, nextSegment)) {
        nextSegment = nullptr;
    }

    // look for the lyrics we are moving from; may be the current lyrics or a previous one
    // we are extending with several underscores
    Lyrics* fromLyrics = nullptr;
    PartialLyricsLine* prevPartialLyricsLine = nullptr;
    while (segment) {
        ChordRest* cr = toChordRest(segment->element(track));
        if (cr) {
            fromLyrics = cr->lyrics(verse, placement);
            if (fromLyrics) {
                break;
            }
            // Check if there is a partial melisma to extend
            auto spanners = score()->spannerMap().findOverlapping(cr->tick().ticks(), cr->endTick().ticks());
            for (auto& spanner : spanners) {
                if (!spanner.value->isPartialLyricsLine() || spanner.value->staffIdx() != cr->staffIdx()) {
                    continue;
                }
                PartialLyricsLine* lyricsLine = toPartialLyricsLine(spanner.value);
                if (lyricsLine->no() != verse || lyricsLine->placement() != placement || !lyricsLine->isEndMelisma()) {
                    continue;
                }

                prevPartialLyricsLine = lyricsLine;
                break;
            }

            if (prevPartialLyricsLine) {
                break;
            }
        }
        segment = segment->prev1(SegmentType::ChordRest);
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
        score()->startCmd(TranslatableString("undoableAction", "Enter lyrics extension line"));
        lyrics->undoChangeProperty(Pid::LYRIC_TICKS, Lyrics::TEMP_MELISMA_TICKS);
        score()->setLayoutAll();
        score()->endCmd();
    }

    if (nextSegment == 0) {
        score()->startCmd(TranslatableString("undoableAction", "Enter lyrics extension line"));
        if (fromLyrics) {
            switch (fromLyrics->syllabic()) {
            case LyricsSyllabic::SINGLE:
            case LyricsSyllabic::END:
                break;
            default:
                fromLyrics->undoChangeProperty(Pid::SYLLABIC, int(LyricsSyllabic::END));
                break;
            }
            if (fromLyrics->segment()->tick() < endTick
                || (endTick + initialCR->actualTicks() == segment->measure()->endTick() && initialCR->hasFollowingJumpItem())) {
                Fraction ticks = std::max(endTick - fromLyrics->segment()->tick(), Lyrics::TEMP_MELISMA_TICKS);
                fromLyrics->undoChangeProperty(Pid::LYRIC_TICKS, ticks);
            }
        }

        if (prevPartialLyricsLine) {
            const Fraction tickDiff = (segment->tick() + segment->ticks()) - prevPartialLyricsLine->tick2();
            prevPartialLyricsLine->undoMoveEnd(tickDiff);
        }

        if (fromLyrics) {
            score()->select(fromLyrics, SelectType::SINGLE, 0);
        }
        score()->setLayoutAll();
        score()->endCmd();
        return;
    }

    // if a place for a new lyrics has been found, create a lyrics there
    ChordRest* nextCR = toChordRest(nextSegment->element(track));

    // Disallow melisma lines between non-adjacent repeat sections eg. 1st volta -> 2nd volta
    // Instead, try to add partial melisma line
    if (nextCR && fromLyrics) {
        Measure* toLyricsMeasure = nextCR->measure();
        Measure* fromLyricsMeasure = fromLyrics->measure();

        if (toLyricsMeasure != fromLyricsMeasure && fromLyricsMeasure->lastChordRest(track)->hasFollowingJumpItem()) {
            const std::vector<Measure*> previousRepeats = findPreviousRepeatMeasures(toLyricsMeasure);
            const bool inPrecedingRepeatSeg = muse::contains(previousRepeats, fromLyricsMeasure);
            if (!previousRepeats.empty() && !inPrecedingRepeatSeg) {
                fromLyrics = nullptr;
            }
        }
    }

    score()->startCmd(TranslatableString("undoableAction", "Enter lyrics extension line"));
    Lyrics* toLyrics = nextCR->lyrics(verse, placement);
    bool newLyrics = (toLyrics == nullptr);
    if (!toLyrics) {
        toLyrics = Factory::createLyrics(nextCR);
        toLyrics->setTrack(track);
        toLyrics->setParent(nextCR);

        toLyrics->setNo(verse);
        const TextStyleType styleType(toLyrics->isEven() ? TextStyleType::LYRICS_EVEN : TextStyleType::LYRICS_ODD);
        toLyrics->setTextStyleType(styleType);

        toLyrics->setPlacement(placement);
        toLyrics->setPropertyFlags(Pid::PLACEMENT, pFlags);
        toLyrics->setSyllabic(LyricsSyllabic::SINGLE);
        toLyrics->setFontStyle(fStyle);
        toLyrics->setPropertyFlags(Pid::FONT_STYLE, fFlags);
    }
    // as we arrived at toLyrics by an underscore, it cannot have syllabic dashes before
    else if (toLyrics->syllabic() == LyricsSyllabic::MIDDLE) {
        toLyrics->undoChangeProperty(Pid::SYLLABIC, int(LyricsSyllabic::BEGIN));
    } else if (toLyrics->syllabic() == LyricsSyllabic::END) {
        toLyrics->undoChangeProperty(Pid::SYLLABIC, int(LyricsSyllabic::SINGLE));
    }

    if (fromLyrics) {
        // as we moved away from fromLyrics by an underscore,
        // it can be isolated or terminal but cannot have dashes after
        switch (fromLyrics->syllabic()) {
        case LyricsSyllabic::SINGLE:
        case LyricsSyllabic::END:
            break;
        default:
            fromLyrics->undoChangeProperty(Pid::SYLLABIC, int(LyricsSyllabic::END));
            break;
        }
        // for the same reason, if it has a melisma, this cannot extend beyond toLyrics
        if (fromLyrics->segment()->tick() < endTick) {
            fromLyrics->undoChangeProperty(Pid::LYRIC_TICKS, endTick - fromLyrics->segment()->tick());
        }
    } else if (hasPrecedingRepeat && !prevPartialLyricsLine) {
        // No from lyrics - create incoming partial melisma
        PartialLyricsLine* melisma = Factory::createPartialLyricsLine(score()->dummy());
        melisma->setIsEndMelisma(true);
        melisma->setNo(verse);
        melisma->setPlacement(lyrics->placement());
        melisma->setTick(initialCR->tick());
        melisma->setTicks(initialCR->ticks());
        melisma->setTrack(initialCR->track());
        melisma->setTrack2(initialCR->track());

        score()->undoAddElement(melisma);
    } else if (prevPartialLyricsLine) {
        const Fraction tickDiff = nextCR->tick() - prevPartialLyricsLine->tick2();
        prevPartialLyricsLine->undoMoveEnd(tickDiff);
        prevPartialLyricsLine->triggerLayout();
    }
    if (newLyrics) {
        score()->undoAddElement(toLyrics);
    }
    score()->endCmd();

    score()->select(toLyrics, SelectType::SINGLE, 0);
    startEditText(toLyrics, PointF());

    toLyrics->selectAll(toLyrics->cursor());
}

//! NOTE: Copied from ScoreView::lyricsReturn
void NotationInteraction::addLyricsVerse()
{
    if (!m_editData.element || !m_editData.element->isLyrics()) {
        LOGW("nextLyricVerse called with invalid current element");
        return;
    }
    mu::engraving::Lyrics* oldLyrics = toLyrics(m_editData.element);
    mu::engraving::FontStyle fStyle = oldLyrics->fontStyle();
    mu::engraving::PropertyFlags fFlags = oldLyrics->propertyFlags(mu::engraving::Pid::FONT_STYLE);

    endEditText();

    score()->startCmd(TranslatableString("undoableAction", "Add lyrics verse"));
    int newVerse = oldLyrics->no() + 1;

    mu::engraving::Lyrics* lyrics = Factory::createLyrics(oldLyrics->chordRest());
    lyrics->setTrack(oldLyrics->track());
    lyrics->setParent(oldLyrics->chordRest());
    lyrics->setPlacement(oldLyrics->placement());
    lyrics->setPropertyFlags(mu::engraving::Pid::PLACEMENT, oldLyrics->propertyFlags(mu::engraving::Pid::PLACEMENT));

    lyrics->setNo(newVerse);
    const mu::engraving::TextStyleType styleType(lyrics->isEven() ? TextStyleType::LYRICS_EVEN : TextStyleType::LYRICS_ODD);
    lyrics->setTextStyleType(styleType);

    lyrics->setFontStyle(fStyle);
    lyrics->setPropertyFlags(mu::engraving::Pid::FONT_STYLE, fFlags);

    score()->undoAddElement(lyrics);
    score()->endCmd();

    score()->select(lyrics, SelectType::SINGLE, 0);
    startEditText(lyrics, PointF());
}

Ret NotationInteraction::canAddGuitarBend() const
{
    Score* score = this->score();
    bool canAdd = score && score->selection().noteList().size() > 0;

    return canAdd ? muse::make_ok() : make_ret(Err::NoteIsNotSelected);
}

void NotationInteraction::addGuitarBend(GuitarBendType bendType)
{
    Score* score = this->score();
    if (!score) {
        return;
    }

    const Selection& selection = score->selection();
    if (selection.isNone()) {
        return;
    }

    const std::vector<Note*>& noteList = selection.noteList();
    if (noteList.empty()) {
        return;
    }

    startEdit(TranslatableString("undoableAction", "Enter guitar bend"));

    Note* startNote = nullptr;
    Note* endNote = nullptr;
    bool noteToNote = false;
    if (selection.isList() && noteList.size() == 2) {
        startNote = noteList.front();
        endNote = noteList.back();
        if (endNote->tick() > startNote->tick()) {
            noteToNote = true;
        }
    }

    mu::engraving::GuitarBend* guitarBend = nullptr;
    if (noteToNote) {
        guitarBend = score->addGuitarBend(bendType, startNote, endNote);
    } else {
        for (Note* note : noteList) {
            // (will select the last one)
            guitarBend = score->addGuitarBend(bendType, note, nullptr);
        }
    }

    if (guitarBend) {
        apply();
        select({ guitarBend });
    } else {
        rollback();
    }
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

    //! NOTE: Copied from ScoreView::cmdAddText
    Measure* measure = text->findMeasure();
    if (measure && measure->hasMMRest() && text->links()) {
        Measure* mmRest = measure->mmRest();
        for (EngravingObject* link : *text->links()) {
            TextBase* linkedText = toTextBase(link);
            if (text != linkedText && linkedText->findMeasure() == mmRest) {
                text = linkedText;
                break;
            }
        }
    }

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
    score()->startCmd(TranslatableString("undoableAction", "Format text"));
    text->undoChangeProperty(mu::engraving::Pid::FONT_STYLE, PropertyValue::fromValue(
                                 currentStyle ^ static_cast<int>(style)), mu::engraving::PropertyFlags::UNSTYLED);
    score()->endCmd();
    notifyAboutTextEditingChanged();
}

void NotationInteraction::toggleVerticalAlignment(VerticalAlignment align)
{
    if (!m_editData.element || !m_editData.element->isTextBase()) {
        LOGW("toggleVerticalAlignment called with invalid current element");
        return;
    }
    mu::engraving::TextBase* text = toTextBase(m_editData.element);
    int ialign = static_cast<int>(align);
    int currentAlign = text->getProperty(mu::engraving::Pid::TEXT_SCRIPT_ALIGN).toInt();

    TranslatableString actionName = [align]() {
        switch (align) {
        case VerticalAlignment::AlignSubScript:
            return TranslatableString("undoableAction", "Toggle subscript");
        case VerticalAlignment::AlignSuperScript:
            return TranslatableString("undoableAction", "Toggle superscript");
        default:
            return TranslatableString("undoableAction", "Toggle subscript/superscript");
        }
    }();

    score()->startCmd(actionName);
    text->undoChangeProperty(mu::engraving::Pid::TEXT_SCRIPT_ALIGN, PropertyValue::fromValue(
                                 (currentAlign == ialign) ? static_cast<int>(VerticalAlignment::AlignNormal) : ialign),
                             mu::engraving::PropertyFlags::UNSTYLED);
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

void NotationInteraction::toggleSubScript()
{
    toggleVerticalAlignment(VerticalAlignment::AlignSubScript);
}

void NotationInteraction::toggleSuperScript()
{
    toggleVerticalAlignment(VerticalAlignment::AlignSuperScript);
}

template<typename P>
void NotationInteraction::execute(void (mu::engraving::Score::* function)(P), P param, const TranslatableString& actionName)
{
    startEdit(actionName);
    (score()->*function)(param);
    apply();
}

void NotationInteraction::toggleArticulation(mu::engraving::SymId symId)
{
    execute(&mu::engraving::Score::toggleArticulation, symId, TranslatableString("undoableAction", "Toggle articulation"));
}

void NotationInteraction::toggleOrnament(mu::engraving::SymId symId)
{
    execute(&mu::engraving::Score::toggleOrnament, symId, TranslatableString("undoableAction", "Toggle ornament"));
}

void NotationInteraction::toggleAutoplace(bool all)
{
    execute(&mu::engraving::Score::cmdToggleAutoplace, all, TranslatableString("undoableAction", "Toggle automatic placement"));
}

bool NotationInteraction::canInsertClef(ClefType type) const
{
    const Score* score = this->score();
    return score && score->canInsertClef(type);
}

void NotationInteraction::insertClef(ClefType type)
{
    execute(&mu::engraving::Score::cmdInsertClef, type, TranslatableString("undoableAction", "Add clef"));
}

void NotationInteraction::changeAccidental(mu::engraving::AccidentalType accidental)
{
    execute(&mu::engraving::Score::changeAccidental, accidental, TranslatableString("undoableAction", "Add accidental"));
}

void NotationInteraction::transposeSemitone(int steps)
{
    execute(&mu::engraving::Score::transposeSemitone, steps, TranslatableString("undoableAction", "Transpose semitone"));
}

void NotationInteraction::transposeDiatonicAlterations(mu::engraving::TransposeDirection direction)
{
    execute(&mu::engraving::Score::transposeDiatonicAlterations, direction,
            TranslatableString("undoableAction", "Transpose diatonically"));
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
        showItem(e);
    }
}

void NotationInteraction::execute(void (mu::engraving::Score::* function)(), const TranslatableString& actionName)
{
    startEdit(actionName);
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
    } else if (el->isPage()) {
        const mu::engraving::Page* p = static_cast<const mu::engraving::Page*>(el);
        mu::engraving::System* s = !p->systems().empty() ? p->systems().front() : nullptr;
        m = s && !s->measures().empty() ? s->measures().front() : nullptr;
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

    double _spatium    = score()->style().spatium();
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

muse::async::Channel<NotationInteraction::ShowItemRequest> NotationInteraction::showItemRequested() const
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
    EngravingItem* item = nullptr;
    const INotationSelectionPtr sel = interaction->selection();

    if (sel->isRange()) {
        const INotationSelectionRangePtr range = sel->range();
        item = range->rangeStartSegment()->firstElementForNavigation(range->startStaffIndex());
    } else {
        item = sel->element();
    }

    if (item == nullptr) {
        return interaction->hitElementContext().element;
    }
    return item;
}
}
