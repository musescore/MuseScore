//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "notationviewinputcontroller.h"

#include <QMimeData>

#include "log.h"
#include "notationpaintview.h"
#include "scenes/common/commonscenetypes.h"

using namespace mu::scene::notation;
using namespace mu::domain::notation;
using namespace mu::actions;

static constexpr int PIXELSSTEPSFACTOR = 5;

NotationViewInputController::NotationViewInputController(IControlledView* view)
    : m_view(view)
{
}

void NotationViewInputController::wheelEvent(QWheelEvent* ev)
{
    QPoint pixelsScrolled = ev->pixelDelta();
    QPoint stepsScrolled  = ev->angleDelta();

    int dy = 0;
    qreal steps = 0.0;

    if (!pixelsScrolled.isNull()) {
        dy = pixelsScrolled.y();
        steps = static_cast<qreal>(dy) / static_cast<qreal>(PIXELSSTEPSFACTOR);
    } else if (!stepsScrolled.isNull()) {
        dy = (stepsScrolled.y() * qMax(2.0, m_view->height() / 10.0)) / QWheelEvent::DefaultDeltasPerStep;
        steps = static_cast<qreal>(stepsScrolled.y()) / static_cast<qreal>(QWheelEvent::DefaultDeltasPerStep);
    }

    Qt::KeyboardModifiers keyState = ev->modifiers();

    // Windows touch pad pinches also execute this
    if (keyState & Qt::ControlModifier) {
        m_view->zoomStep(steps, m_view->toLogical(ev->pos()));
    } else if (keyState & Qt::ShiftModifier) {
        m_view->scrollHorizontal(dy);
    } else {
        m_view->scrollVertical(dy);
    }
}

void NotationViewInputController::mousePressEvent(QMouseEvent* ev)
{
    QPoint logicPos = m_view->toLogical(ev->pos());
    Qt::KeyboardModifiers keyState = ev->modifiers();

    // note enter mode
    if (m_view->isNoteEnterMode()) {
        bool replace = keyState & Qt::ShiftModifier;
        bool insert = keyState & Qt::ControlModifier;
        dispatcher()->dispatch("put-note", ActionData::make_arg3<QPoint, bool, bool>(logicPos, replace, insert));
        return;
    }

    m_interactData.beginPoint = logicPos;
    m_interactData.hitElement = m_view->notationInteraction()->hitElement(logicPos, hitWidth());

    if (m_interactData.hitElement) {

        if (!m_interactData.hitElement->selected()) {
            SelectType st = SelectType::SINGLE;
            if (keyState == Qt::NoModifier) {
                st = SelectType::SINGLE;
            } else if (keyState & Qt::ShiftModifier) {
                st = SelectType::RANGE;
            } else if (keyState & Qt::ControlModifier) {
                st = SelectType::ADD;
            }

            m_view->notationInteraction()->select(m_interactData.hitElement, st);
        }
    } else {
        m_view->notationInteraction()->clearSelection();
    }

    if (m_view->notationInteraction()->isTextEditingStarted()) {
        if (!m_interactData.hitElement || !m_interactData.hitElement->isText()) {
            m_view->notationInteraction()->endEditText();
        } else {
            m_view->notationInteraction()->changeTextCursorPosition(m_interactData.beginPoint);
        }
    }
}

void NotationViewInputController::mouseMoveEvent(QMouseEvent* ev)
{
    if (m_view->isNoteEnterMode()) {
        return;
    }

    QPoint logicPos = m_view->toLogical(ev->pos());
    Qt::KeyboardModifiers keyState = ev->modifiers();

    // start some drag operations after a minimum of movement:
    bool isDrag = (logicPos - m_interactData.beginPoint).manhattanLength() > 4;
    if (!isDrag) {
        return;
    }

    // hit element
    if (m_interactData.hitElement && m_interactData.hitElement->isMovable()) {
        if (!m_view->notationInteraction()->isDragStarted()) {
            startDragElements(m_interactData.hitElement->type(), m_interactData.hitElement->offset());
        }

        DragMode mode = DragMode::BothXY;
        if (keyState & Qt::ShiftModifier) {
            mode = DragMode::OnlyY;
        } else if (keyState & Qt::ControlModifier) {
            mode = DragMode::OnlyX;
        }

        m_view->notationInteraction()->drag(m_interactData.beginPoint, logicPos, mode);
        return;
    }

    QPoint d = logicPos - m_interactData.beginPoint;
    int dx = d.x();
    int dy = d.y();

    if (dx == 0 && dy == 0) {
        return;
    }

    m_view->moveCanvas(dx, dy);
}

void NotationViewInputController::startDragElements(ElementType etype, const QPointF& eoffset)
{
    std::vector<Element*> els = m_view->notationInteraction()->selection()->elements();
    IF_ASSERT_FAILED(els.size() > 0) {
        return;
    }

    const bool isFilterType = m_view->notationInteraction()->selection()->isRange();
    const auto isDraggable = [isFilterType, etype](const Element* e) {
                                 return e && e->selected() && (!isFilterType || etype == e->type());
                             };

    m_view->notationInteraction()->startDrag(els, eoffset, isDraggable);
}

void NotationViewInputController::mouseReleaseEvent(QMouseEvent*)
{
    if (m_view->notationInteraction()->isDragStarted()) {
        m_view->notationInteraction()->endDrag();
        return;
    }
}

void NotationViewInputController::mouseDoubleClickEvent(QMouseEvent* ev)
{
    Element* element = m_view->notationInteraction()->selection()->element();

    if (!element) {
        return;
    }

    if (element->isTextBase()) {
        m_view->notationInteraction()->startEditText(element, m_view->toLogical(ev->pos()));
    }
}

void NotationViewInputController::hoverMoveEvent(QHoverEvent* ev)
{
    if (m_view->isNoteEnterMode()) {
        QPoint pos = m_view->toLogical(ev->pos());
        m_view->showShadowNote(pos);
    }
}

void NotationViewInputController::keyPressEvent(QKeyEvent* event)
{
    if (m_view->notationInteraction()->isTextEditingStarted()) {
        m_view->notationInteraction()->editText(event);
    }
}

void NotationViewInputController::dragEnterEvent(QDragEnterEvent* ev)
{
    //LOGI() << "ev: " << ev;

    const QMimeData* dta = ev->mimeData();
    IF_ASSERT_FAILED(dta) {
        return;
    }

    if (dta->hasFormat(MIME_SYMBOL_FORMAT)) {
        if (ev->possibleActions() & Qt::CopyAction) {
            ev->setDropAction(Qt::CopyAction);
        }

        if (ev->dropAction() == Qt::CopyAction) {
            ev->accept();
        }

        QByteArray edata = dta->data(MIME_SYMBOL_FORMAT);
        m_view->notationInteraction()->startDrop(edata);

        return;
    }

    ev->ignore();
}

void NotationViewInputController::dragMoveEvent(QDragMoveEvent* ev)
{
    //LOGI() << "ev: " << ev;

    const QMimeData* dta = ev->mimeData();
    IF_ASSERT_FAILED(dta) {
        return;
    }

    if (dta->hasFormat(MIME_SYMBOL_FORMAT)
        || dta->hasFormat(MIME_SYMBOLLIST_FORMAT)
        || dta->hasFormat(MIME_STAFFLLIST_FORMAT)) {
        if (ev->possibleActions() & Qt::CopyAction) {
            ev->setDropAction(Qt::CopyAction);
        }
    }

    QPointF pos = m_view->toLogical(ev->pos());
    Qt::KeyboardModifiers modifiers = ev->keyboardModifiers();

    bool isAccepted = m_view->notationInteraction()->isDropAccepted(pos, modifiers);
    if (isAccepted) {
        ev->setAccepted(isAccepted);
    } else {
        ev->ignore();
    }
}

void NotationViewInputController::dragLeaveEvent(QDragLeaveEvent*)
{
    //LOGI() << "ev: " << ev;
    m_view->notationInteraction()->endDrop();
}

void NotationViewInputController::dropEvent(QDropEvent* ev)
{
    //LOGI() << "ev: " << ev;

    QPointF pos = m_view->toLogical(ev->pos());
    Qt::KeyboardModifiers modifiers = ev->keyboardModifiers();

    bool isAccepted = m_view->notationInteraction()->drop(pos, modifiers);
    if (isAccepted) {
        ev->acceptProposedAction();
    } else {
        ev->ignore();
    }
}

float NotationViewInputController::hitWidth() const
{
    return configuration()->selectionProximity() * 0.5 / m_view->scale();
}
