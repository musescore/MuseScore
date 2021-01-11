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
#include <QtMath>

#include "log.h"
#include "notationpaintview.h"
#include "commonscene/commonscenetypes.h"

using namespace mu::notation;
using namespace mu::actions;
using namespace mu::commonscene;

static constexpr int PIXELSSTEPSFACTOR = 5;

NotationViewInputController::NotationViewInputController(IControlledView* view)
    : m_view(view)
{
    m_possibleZoomsPercentage = {
        5, 10, 15, 25, 50, 75, 100, 150, 200, 400, 800, 1600
    };

    if (dispatcher()) {
        dispatcher()->reg(this, "zoomin", this, &NotationViewInputController::zoomIn);
        dispatcher()->reg(this, "zoomout", this, &NotationViewInputController::zoomOut);
        dispatcher()->reg(this, "zoom-page-width", this, &NotationViewInputController::zoomToPageWidth);
        dispatcher()->reg(this, "zoom100", [this]() { setZoom(100); });

        dispatcher()->reg(this, "view-mode-page", [this]() {
            setViewMode(ViewMode::PAGE);
        });

        dispatcher()->reg(this, "view-mode-continuous", [this]() {
            setViewMode(ViewMode::LINE);
        });

        dispatcher()->reg(this, "view-mode-single", [this]() {
            setViewMode(ViewMode::SYSTEM);
        });
    }

    setZoom(configuration()->currentZoom().val);
}

void NotationViewInputController::setReadonly(bool readonly)
{
    m_readonly = readonly;
}

INotationPtr NotationViewInputController::currentNotation() const
{
    return globalContext()->currentNotation();
}

void NotationViewInputController::zoomIn()
{
    int maxIndex = m_possibleZoomsPercentage.size() > 0 ? m_possibleZoomsPercentage.size() - 1 : 0;
    int currentIndex = std::min(currentZoomIndex() + 1, maxIndex);

    int zoom = m_possibleZoomsPercentage[currentIndex];

    setZoom(zoom);
}

void NotationViewInputController::zoomOut()
{
    int currentIndex = std::max(currentZoomIndex() - 1, 0);

    int zoom = m_possibleZoomsPercentage[currentIndex];

    setZoom(zoom);
}

void NotationViewInputController::zoomToPageWidth()
{
    NOT_IMPLEMENTED;
}

int NotationViewInputController::currentZoomIndex() const
{
    for (int index = 0; index < m_possibleZoomsPercentage.size(); ++index) {
        if (m_possibleZoomsPercentage[index] >= currentZoomPercentage()) {
            return index;
        }
    }

    return m_possibleZoomsPercentage.isEmpty() ? 0 : m_possibleZoomsPercentage.size() - 1;
}

int NotationViewInputController::currentZoomPercentage() const
{
    return m_view->currentScaling() * 100.0 / notationScaling();
}

qreal NotationViewInputController::notationScaling() const
{
    return configuration()->notationScaling();
}

void NotationViewInputController::setZoom(int zoomPercentage, const QPoint& pos)
{
    int minZoom = m_possibleZoomsPercentage.first();
    int maxZoom = m_possibleZoomsPercentage.last();
    int correctedZoom = qBound(minZoom, zoomPercentage, maxZoom);

    if (!m_readonly) {
        configuration()->setCurrentZoom(correctedZoom);
    }

    qreal scaling = static_cast<qreal>(zoomPercentage) / 100.0 * notationScaling();
    m_view->scale(scaling, pos);
}

void NotationViewInputController::setViewMode(const ViewMode& viewMode)
{
    auto notation = currentNotation();
    if (!notation) {
        return;
    }

    notation->setViewMode(viewMode);
}

void NotationViewInputController::wheelEvent(QWheelEvent* event)
{
    QPoint pixelsScrolled = event->pixelDelta();
    QPoint stepsScrolled = event->angleDelta();

    int dy = 0;
    qreal steps = 0.0;

    if (!pixelsScrolled.isNull()) {
        dy = pixelsScrolled.y();
        steps = static_cast<qreal>(dy) / static_cast<qreal>(PIXELSSTEPSFACTOR);
    } else if (!stepsScrolled.isNull()) {
        dy = (stepsScrolled.y() * qMax(2.0, m_view->height() / 10.0)) / QWheelEvent::DefaultDeltasPerStep;
        steps = static_cast<qreal>(stepsScrolled.y()) / static_cast<qreal>(QWheelEvent::DefaultDeltasPerStep);
    }

    Qt::KeyboardModifiers keyState = event->modifiers();

    // Windows touch pad pinches also execute this
    if (keyState & Qt::ControlModifier) {
        int zoom = currentZoomPercentage() * qPow(1.1, steps);
        setZoom(zoom, event->position().toPoint());
    } else if (keyState & Qt::ShiftModifier) {
        m_view->moveCanvasHorizontal(dy);
    } else {
        m_view->moveCanvasVertical(dy);
    }
}

void NotationViewInputController::mousePressEvent(QMouseEvent* event)
{
    QPoint logicPos = m_view->toLogical(event->pos());
    Qt::KeyboardModifiers keyState = event->modifiers();

    // note enter mode
    if (m_view->isNoteEnterMode()) {
        bool replace = keyState & Qt::ShiftModifier;
        bool insert = keyState & Qt::ControlModifier;
        dispatcher()->dispatch("put-note", ActionData::make_arg3<QPoint, bool, bool>(logicPos, replace, insert));
        return;
    }

    m_interactData.beginPoint = logicPos;

    if (!m_readonly) {
        m_interactData.hitElement = m_view->notationInteraction()->hitElement(logicPos, hitWidth());
        m_interactData.hitStaffIndex = m_view->notationInteraction()->hitStaffIndex(logicPos);
    }

    if (playbackController()->isPlaying()) {
        if (m_interactData.hitElement) {
            m_view->notationPlayback()->setPlayPositionByElement(m_interactData.hitElement);
        }
        return;
    }

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

            m_view->notationInteraction()->select({ m_interactData.hitElement }, st, m_interactData.hitStaffIndex);
        }
    } else {
        m_view->notationInteraction()->clearSelection();
    }

    if (event->button() == Qt::MouseButton::RightButton) {
        ElementType type = selectionType();
        m_view->showContextMenu(type, event->pos());
    }

    if (m_interactData.hitElement) {
        playbackController()->playElementOnClick(m_interactData.hitElement);
    }

    if (m_view->notationInteraction()->isTextEditingStarted()) {
        if (!m_interactData.hitElement || !m_interactData.hitElement->isText()) {
            m_view->notationInteraction()->endEditText();
        } else {
            m_view->notationInteraction()->changeTextCursorPosition(m_interactData.beginPoint);
        }
    }
}

bool NotationViewInputController::isDragAllowed() const
{
    if (m_view->isNoteEnterMode()) {
        return false;
    }

    if (playbackController()->isPlaying()) {
        return false;
    }

    return true;
}

void NotationViewInputController::mouseMoveEvent(QMouseEvent* event)
{
    if (!isDragAllowed()) {
        return;
    }

    QPoint logicPos = m_view->toLogical(event->pos());
    Qt::KeyboardModifiers keyState = event->modifiers();

    // start some drag operations after a minimum of movement:
    bool isDrag = (logicPos - m_interactData.beginPoint).manhattanLength() > 4;
    if (!isDrag) {
        return;
    }

    // drag element
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

    // move canvas

    QPoint d = logicPos - m_interactData.beginPoint;
    int dx = d.x();
    int dy = d.y();

    if (dx == 0 && dy == 0) {
        return;
    }

    m_view->moveCanvas(dx, dy);
}

void NotationViewInputController::startDragElements(ElementType elemetsType, const QPointF& elementsOffset)
{
    std::vector<Element*> elements = m_view->notationInteraction()->selection()->elements();
    IF_ASSERT_FAILED(elements.size() > 0) {
        return;
    }

    const bool isFilterType = m_view->notationInteraction()->selection()->isRange();
    const auto isDraggable = [isFilterType, elemetsType](const Element* element) {
        return element && element->selected() && (!isFilterType || elemetsType == element->type());
    };

    m_view->notationInteraction()->startDrag(elements, elementsOffset, isDraggable);
}

void NotationViewInputController::mouseReleaseEvent(QMouseEvent*)
{
    if (m_view->notationInteraction()->isDragStarted()) {
        m_view->notationInteraction()->endDrag();
    }
}

void NotationViewInputController::mouseDoubleClickEvent(QMouseEvent* event)
{
    Element* element = m_view->notationInteraction()->selection()->element();

    if (!element) {
        return;
    }

    if (element->isTextBase()) {
        m_view->notationInteraction()->startEditText(element, m_view->toLogical(event->pos()));
    }
}

void NotationViewInputController::hoverMoveEvent(QHoverEvent* event)
{
    if (m_view->isNoteEnterMode()) {
        QPoint pos = m_view->toLogical(event->pos());
        m_view->showShadowNote(pos);
    }
}

void NotationViewInputController::keyReleaseEvent(QKeyEvent* event)
{
    if (m_view->notationInteraction()->isTextEditingStarted()) {
        m_view->notationInteraction()->editText(event);
    }
}

void NotationViewInputController::dragEnterEvent(QDragEnterEvent* event)
{
    const QMimeData* mimeData = event->mimeData();
    IF_ASSERT_FAILED(mimeData) {
        return;
    }

    if (mimeData->hasFormat(mu::commonscene::MIME_SYMBOL_FORMAT)) {
        if (event->possibleActions() & Qt::CopyAction) {
            event->setDropAction(Qt::CopyAction);
        }

        if (event->dropAction() == Qt::CopyAction) {
            event->accept();
        }

        QByteArray edata = mimeData->data(MIME_SYMBOL_FORMAT);
        m_view->notationInteraction()->startDrop(edata);

        return;
    }

    event->ignore();
}

void NotationViewInputController::dragMoveEvent(QDragMoveEvent* event)
{
    const QMimeData* mimeData = event->mimeData();
    IF_ASSERT_FAILED(mimeData) {
        return;
    }

    if (mimeData->hasFormat(MIME_SYMBOL_FORMAT)
        || mimeData->hasFormat(MIME_SYMBOLLIST_FORMAT)
        || mimeData->hasFormat(MIME_STAFFLLIST_FORMAT)) {
        if (event->possibleActions() & Qt::CopyAction) {
            event->setDropAction(Qt::CopyAction);
        }
    }

    QPointF pos = m_view->toLogical(event->pos());
    Qt::KeyboardModifiers modifiers = event->keyboardModifiers();

    bool isAccepted = m_view->notationInteraction()->isDropAccepted(pos, modifiers);
    if (isAccepted) {
        event->setAccepted(isAccepted);
    } else {
        event->ignore();
    }
}

void NotationViewInputController::dragLeaveEvent(QDragLeaveEvent*)
{
    m_view->notationInteraction()->endDrop();
}

void NotationViewInputController::dropEvent(QDropEvent* event)
{
    QPointF pos = m_view->toLogical(event->pos());
    Qt::KeyboardModifiers modifiers = event->keyboardModifiers();

    bool isAccepted = m_view->notationInteraction()->drop(pos, modifiers);
    if (isAccepted) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

float NotationViewInputController::hitWidth() const
{
    return configuration()->selectionProximity() * 0.5 / m_view->currentScaling();
}

ElementType NotationViewInputController::selectionType() const
{
    ElementType type = ElementType::INVALID;
    if (m_interactData.hitElement) {
        type = m_interactData.hitElement->type();
    } else {
        type = ElementType::PAGE;
    }

    return type;
}
