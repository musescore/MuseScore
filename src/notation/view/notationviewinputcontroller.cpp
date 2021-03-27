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
}

void NotationViewInputController::init()
{
    if (dispatcher() && !m_readonly) {
        dispatcher()->reg(this, "zoomin", this, &NotationViewInputController::zoomIn);
        dispatcher()->reg(this, "zoomout", this, &NotationViewInputController::zoomOut);
        dispatcher()->reg(this, "zoom-page-width", this, &NotationViewInputController::zoomToPageWidth);
        dispatcher()->reg(this, "zoom100", [this]() { setZoom(100); });
        dispatcher()->reg(this, "zoom-x-percent", [this](const ActionData& args) { setZoom(args.arg<int>(0)); });

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

    globalContext()->currentMasterNotationChanged().onNotify(this, [this]() {
        m_isZoomInited = false;
        initZoom();
    });
}

bool NotationViewInputController::isZoomInited()
{
    return m_isZoomInited;
}

void NotationViewInputController::initZoom()
{
    ZoomType defaultZoomType = configuration()->defaultZoomType();
    switch (defaultZoomType) {
    case ZoomType::Percentage:
        setZoom(configuration()->defaultZoom());
        m_isZoomInited = true;
        break;
    case ZoomType::PageWidth:
        zoomToPageWidth();
        break;
    case ZoomType::WholePage:
        zoomToWholePage();
        break;
    case ZoomType::TwoPages:
        zoomToTwoPages();
        break;
    }
}

void NotationViewInputController::setReadonly(bool readonly)
{
    m_readonly = readonly;
}

INotationPtr NotationViewInputController::currentNotation() const
{
    return globalContext()->currentNotation();
}

INotationStylePtr NotationViewInputController::notationStyle() const
{
    return currentNotation() ? currentNotation()->style() : nullptr;
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
    if (!notationStyle()) {
        return;
    }

    qreal pageWidth = notationStyle()->styleValue(Ms::Sid::pageWidth).toDouble() * Ms::DPI;
    qreal scale = m_view->width() / pageWidth / configuration()->notationScaling();

    setZoom(scale * 100, QPoint());
    m_isZoomInited = true;
}

void NotationViewInputController::zoomToWholePage()
{
    if (!notationStyle()) {
        return;
    }

    qreal pageWidth = notationStyle()->styleValue(Ms::Sid::pageWidth).toDouble() * Ms::DPI;
    qreal pageHeight = notationStyle()->styleValue(Ms::Sid::pageHeight).toDouble() * Ms::DPI;

    qreal pageWidthScale = m_view->width() / pageWidth;
    qreal pageHeightScale = m_view->height() / pageHeight;

    qreal scale = std::min(pageWidthScale, pageHeightScale) / configuration()->notationScaling();

    setZoom(scale * 100, QPoint());
    m_isZoomInited = true;
}

void NotationViewInputController::zoomToTwoPages()
{
    if (!notationStyle()) {
        return;
    }

    qreal viewWidth = m_view->width();
    qreal viewHeight = m_view->height();
    qreal pageWidth = notationStyle()->styleValue(Ms::Sid::pageWidth).toDouble() * Ms::DPI;
    qreal pageHeight = notationStyle()->styleValue(Ms::Sid::pageHeight).toDouble() * Ms::DPI;

    qreal pageHeightScale = 0.0;
    qreal pageWidthScale = 0.0;

    if (configuration()->canvasOrientation().val == framework::Orientation::Vertical) {
        static const qreal VERTICAL_PAGE_GAP = 5.0;
        pageHeightScale = viewHeight / (pageHeight * 2.0 + VERTICAL_PAGE_GAP);
        pageWidthScale = viewWidth / pageWidth;
    } else {
        static const qreal HORIZONTAL_PAGE_GAP = 50.0;
        pageHeightScale = viewHeight / pageHeight;
        pageWidthScale = viewWidth / (pageWidth * 2.0 + HORIZONTAL_PAGE_GAP);
    }

    qreal scale = std::min(pageHeightScale, pageWidthScale) / configuration()->notationScaling();

    setZoom(scale * 100, QPoint());
    m_isZoomInited = true;
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
    return qRound(m_view->currentScaling() * 100.0 / notationScaling());
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

    qreal scaling = static_cast<qreal>(correctedZoom) / 100.0 * notationScaling();
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

    int dx = 0;
    int dy = 0;
    qreal stepsX = 0.0;
    qreal stepsY = 0.0;

    if (!pixelsScrolled.isNull()) {
        dx = pixelsScrolled.x();
        dy = pixelsScrolled.y();
        stepsX = static_cast<qreal>(dx) / static_cast<qreal>(PIXELSSTEPSFACTOR);
        stepsY = static_cast<qreal>(dy) / static_cast<qreal>(PIXELSSTEPSFACTOR);
    } else if (!stepsScrolled.isNull()) {
        dx = (stepsScrolled.x() * qMax(2.0, m_view->width() / 10.0)) / QWheelEvent::DefaultDeltasPerStep;
        dy = (stepsScrolled.y() * qMax(2.0, m_view->height() / 10.0)) / QWheelEvent::DefaultDeltasPerStep;
        stepsX = static_cast<qreal>(stepsScrolled.x()) / static_cast<qreal>(QWheelEvent::DefaultDeltasPerStep);
        stepsY = static_cast<qreal>(stepsScrolled.y()) / static_cast<qreal>(QWheelEvent::DefaultDeltasPerStep);
    }

    Qt::KeyboardModifiers keyState = event->modifiers();

    // Windows touch pad pinches also execute this
    if (keyState & Qt::ControlModifier) {
        double zoomSpeed = qPow(2.0, 1.0 / configuration()->mouseZoomPrecision());
        int absSteps = sqrt(stepsX * stepsX + stepsY * stepsY) * (stepsY > -stepsX ? 1 : -1);
        double zoomAmount = currentZoomPercentage() * qPow(zoomSpeed, absSteps);
        int zoom = absSteps > 0 ? qCeil(zoomAmount) : qFloor(zoomAmount);
        setZoom(zoom, event->position().toPoint());
    } else if (keyState & Qt::ShiftModifier) {
        int abs = sqrt(dx * dx + dy * dy) * (dy > -dx ? 1 : -1);
        QPoint d = m_view->toLogical(QPoint(0, abs)) - m_view->toLogical(QPoint(0, 0));
        m_view->moveCanvasHorizontal(d.y());
    } else {
        QPoint d = m_view->toLogical(QPoint(dx, dy)) - m_view->toLogical(QPoint(0, 0));
        m_view->moveCanvas(d.x(), d.y());
    }
}

void NotationViewInputController::mousePressEvent(QMouseEvent* event)
{
    QPoint logicPos = m_view->toLogical(event->pos());
    Qt::KeyboardModifiers keyState = event->modifiers();

    // When using MiddleButton, just start moving the canvas
    if (event->button() == Qt::MiddleButton) {
        m_interactData.beginPoint = logicPos;
        return;
    }

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
            SelectType selectType = SelectType::SINGLE;
            if (keyState == Qt::NoModifier) {
                selectType = SelectType::SINGLE;
            } else if (keyState & Qt::ShiftModifier) {
                selectType = SelectType::RANGE;
            } else if (keyState & Qt::ControlModifier) {
                selectType = SelectType::ADD;
            }

            m_view->notationInteraction()->select({ m_interactData.hitElement }, selectType, m_interactData.hitStaffIndex);
        }
    }

    if (m_view->notationInteraction()->isHitGrip(logicPos)) {
        m_view->notationInteraction()->startEditGrip(logicPos);
        return;
    }

    if (event->button() == Qt::MouseButton::RightButton) {
        ElementType type = selectionType();
        m_view->showContextMenu(type, event->pos());
    }

    if (m_interactData.hitElement) {
        playbackController()->playElementOnClick(m_interactData.hitElement);
    } else {
        m_view->notationInteraction()->endEditGrip();
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
    bool middleButton = event->buttons() == Qt::MiddleButton;
    if (!isDragAllowed() && !middleButton) {
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
    if (!middleButton && ((m_interactData.hitElement && m_interactData.hitElement->isMovable())
                          || m_view->notationInteraction()->isGripEditStarted())) {
        if (m_interactData.hitElement && !m_view->notationInteraction()->isDragStarted()) {
            startDragElements(m_interactData.hitElement->type(), m_interactData.hitElement->offset());
        }

        if (m_view->notationInteraction()->isGripEditStarted() && !m_view->notationInteraction()->isDragStarted()) {
            Element* selectedElement = m_view->notationInteraction()->selection()->element();
            startDragElements(selectedElement->type(), selectedElement->offset());
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
    m_isCanvasDragged = true;
}

void NotationViewInputController::startDragElements(ElementType elementsType, const QPointF& elementsOffset)
{
    std::vector<Element*> elements = m_view->notationInteraction()->selection()->elements();
    IF_ASSERT_FAILED(!elements.empty()) {
        return;
    }

    bool isFilterType = m_view->notationInteraction()->selection()->isRange();
    auto isDraggable = [isFilterType, elementsType](const Element* element) {
        return element && element->selected() && (!isFilterType || elementsType == element->type());
    };

    m_view->notationInteraction()->startDrag(elements, elementsOffset, isDraggable);
}

void NotationViewInputController::mouseReleaseEvent(QMouseEvent*)
{
    if (!m_interactData.hitElement && !m_isCanvasDragged && !m_view->notationInteraction()->isGripEditStarted()) {
        m_view->notationInteraction()->clearSelection();
    }

    m_isCanvasDragged = false;

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

double NotationViewInputController::guiScalling() const
{
    return configuration()->guiScaling();
}
