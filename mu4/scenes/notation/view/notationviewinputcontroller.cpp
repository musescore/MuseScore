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

#include "log.h"
#include "notationpaintview.h"
#include "domain/notation/notationactions.h"

using namespace mu::scene::notation;
using namespace mu::domain::notation;
using namespace mu::actions;

static constexpr int PIXELSSTEPSFACTOR = 5;

NotationViewInputController::NotationViewInputController(NotationPaintView* view)
    : m_view(view)
{
}

void NotationViewInputController::wheelEvent(QWheelEvent* ev)
{
    QPoint pixelsScrolled = ev->pixelDelta();
    QPoint stepsScrolled  = ev->angleDelta();

    int dx = 0;
    int dy = 0;
    qreal steps = 0.0;

    if (!pixelsScrolled.isNull()) {
        dx = pixelsScrolled.x();
        dy = pixelsScrolled.y();
        steps = static_cast<qreal>(dy) / static_cast<qreal>(PIXELSSTEPSFACTOR);
    } else if (!stepsScrolled.isNull()) {
        dx = (stepsScrolled.x() * qMax(2.0, m_view->width() / 10.0)) / 120;
        dy = (stepsScrolled.y() * qMax(2.0, m_view->height() / 10.0)) / 120;
        steps = static_cast<qreal>(stepsScrolled.y()) / 120.0;
    }

    Qt::KeyboardModifiers keyState = ev->modifiers();

    // Windows touch pad pinches also execute this
    if (keyState & Qt::ControlModifier) {
        m_view->zoomStep(steps, m_view->toLogical(ev->pos()));
    } else if (keyState & Qt::ShiftModifier && dx == 0) {
        dx = dy;
        m_view->scrollHorizontal(dx);
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
        dispatcher()->dispatch("domain/notation/put-note",
                               ActionData::make_arg3<QPoint, bool, bool>(logicPos, replace, insert));

        return;
    }

    m_interactData.beginPoint = logicPos;
    m_interactData.hitElement = notationInteraction()->hitElement(logicPos, hitWidth());

    if (m_interactData.hitElement && !m_interactData.hitElement->selected()) {
        SelectType st = SelectType::SINGLE;
        if (keyState == Qt::NoModifier) {
            st = SelectType::SINGLE;
        } else if (keyState & Qt::ShiftModifier) {
            st = SelectType::RANGE;
        } else if (keyState & Qt::ControlModifier) {
            st = SelectType::ADD;
        }

        notationInteraction()->select(m_interactData.hitElement, st);
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
        if (!notationInteraction()->isDragStarted()) {
            startDragElements(m_interactData.hitElement->type(), m_interactData.hitElement->offset());
        }

        DragMode mode = DragMode::BothXY;
        if (keyState & Qt::ShiftModifier) {
            mode = DragMode::OnlyY;
        } else if (keyState & Qt::ControlModifier) {
            mode = DragMode::OnlyX;
        }

        notationInteraction()->drag(m_interactData.beginPoint, logicPos, mode);
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
    std::vector<Element*> els = notationInteraction()->selection()->elements();
    IF_ASSERT_FAILED(els.size() > 0) {
        return;
    }

    const bool isFilterType = notationInteraction()->selection()->isRange();
    const auto isDraggable = [isFilterType, etype](const Element* e) {
                                 return e && e->selected() && (!isFilterType || etype == e->type());
                             };

    notationInteraction()->startDrag(els, eoffset, isDraggable);
}

void NotationViewInputController::mouseReleaseEvent(QMouseEvent* /*ev*/)
{
    if (notationInteraction()->isDragStarted()) {
        notationInteraction()->endDrag();
    }
}

void NotationViewInputController::hoverMoveEvent(QHoverEvent* ev)
{
    if (m_view->isNoteEnterMode()) {
        QPoint pos = m_view->toLogical(ev->pos());
        m_view->showShadowNote(pos);
    }
}

INotationInteraction* NotationViewInputController::notationInteraction() const
{
    auto notation = m_view->notation();
    if (!notation) {
        return nullptr;
    }
    return notation->interaction();
}

float NotationViewInputController::hitWidth() const
{
    return configuration()->selectionProximity() * 0.5 / m_view->scale();
}
