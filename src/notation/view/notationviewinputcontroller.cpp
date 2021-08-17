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
}

void NotationViewInputController::init()
{
    m_possibleZoomsPercentage = configuration()->possibleZoomPercentageList();

    if (dispatcher() && !m_readonly) {
        dispatcher()->reg(this, "zoomin", this, &NotationViewInputController::zoomIn);
        dispatcher()->reg(this, "zoomout", this, &NotationViewInputController::zoomOut);
        dispatcher()->reg(this, "zoom-page-width", this, &NotationViewInputController::zoomToPageWidth);
        dispatcher()->reg(this, "zoom-whole-page", this, &NotationViewInputController::zoomToWholePage);
        dispatcher()->reg(this, "zoom-two-pages", this, &NotationViewInputController::zoomToTwoPages);
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

INotationInteractionPtr NotationViewInputController::viewInteraction() const
{
    return m_view->notationInteraction();
}

Element* NotationViewInputController::hitElement() const
{
    return viewInteraction()->hitElementContext().element;
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
        PointF d = m_view->toLogical(QPoint(0, abs)) - m_view->toLogical(QPoint(0, 0));
        m_view->moveCanvasHorizontal(d.y());
    } else {
        PointF d = m_view->toLogical(QPoint(dx, dy)) - m_view->toLogical(QPoint(0, 0));
        m_view->moveCanvas(d.x(), d.y());
    }
}

void NotationViewInputController::mousePressEvent(QMouseEvent* event)
{
    PointF logicPos = PointF(m_view->toLogical(event->pos()));
    Qt::KeyboardModifiers keyState = event->modifiers();

    // When using MiddleButton, just start moving the canvas
    if (event->button() == Qt::MiddleButton) {
        m_beginPoint = logicPos;
        return;
    }

    // note enter mode
    if (m_view->isNoteEnterMode()) {
        bool replace = keyState & Qt::ShiftModifier;
        bool insert = keyState & Qt::ControlModifier;
        dispatcher()->dispatch("put-note", ActionData::make_arg3<QPoint, bool, bool>(logicPos.toQPoint(), replace, insert));
        return;
    }

    m_beginPoint = logicPos;

    Element* hitElement = nullptr;
    int hitStaffIndex = -1;

    if (!m_readonly) {
        INotationInteraction::HitElementContext context;
        context.element = viewInteraction()->hitElement(logicPos, hitWidth());
        context.staff = viewInteraction()->hitStaff(logicPos);
        viewInteraction()->setHitElementContext(context);

        hitElement = context.element;
        hitStaffIndex = context.staff ? context.staff->idx() : -1;
    }

    if (hitElement) {
        RetVal<midi::tick_t> tick = m_view->notationPlayback()->playPositionTickByElement(hitElement);

        if (tick.ret) {
            playbackController()->seek(tick.val);
        }
    }

    if (playbackController()->isPlaying()) {
        return;
    }

    if (needSelect(event, logicPos)) {
        SelectType selectType = SelectType::SINGLE;
        if (keyState == Qt::NoModifier) {
            selectType = SelectType::SINGLE;
        } else if (keyState & Qt::ShiftModifier) {
            selectType = SelectType::RANGE;
        } else if (keyState & Qt::ControlModifier) {
            selectType = SelectType::ADD;
        }
        viewInteraction()->select({ hitElement }, selectType, hitStaffIndex);
    }

    if (event->button() == Qt::MouseButton::RightButton) {
        ElementType type = selectionType();
        m_view->showContextMenu(type, event->pos());
    } else if (event->button() == Qt::MouseButton::LeftButton) {
        m_view->hideContextMenu();
    }

    if (viewInteraction()->isHitGrip(logicPos)) {
        viewInteraction()->startEditGrip(logicPos);
        return;
    }

    if (hitElement) {
        playbackController()->playElement(hitElement);
    } else {
        viewInteraction()->endEditGrip();
    }

    if (viewInteraction()->isTextEditingStarted()) {
        if (!hitElement || !hitElement->isTextBase()) {
            viewInteraction()->endEditText();
        } else {
            const mu::RectF& bbox = hitElement->canvasBoundingRect();
            mu::PointF constrainedPt = mu::PointF(
                m_beginPoint.x() < bbox.left() ? bbox.left()
                : m_beginPoint.x() >= bbox.right() ? bbox.right() - 1 : m_beginPoint.x(),
                m_beginPoint.y() < bbox.top() ? bbox.top()
                : m_beginPoint.y() >= bbox.bottom() ? bbox.bottom() - 1 : m_beginPoint.y());
            viewInteraction()->changeTextCursorPosition(constrainedPt);
        }
    }
}

bool NotationViewInputController::needSelect(const QMouseEvent* event, const PointF& clickLogicPos) const
{
    if (!event) {
        return false;
    }

    const Element* hitElement = this->hitElement();
    if (!hitElement) {
        return false;
    }

    Qt::MouseButton button = event->button();

    if (button == Qt::MouseButton::LeftButton && event->modifiers() == Qt::NoModifier) {
        return true;
    }

    bool result = hitElement && !hitElement->selected();

    if (button == Qt::MouseButton::RightButton && result) {
        result &= !viewInteraction()->selection()->range()->containsPoint(clickLogicPos);
    }

    return result;
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

    PointF logicPos = m_view->toLogical(event->pos());
    Qt::KeyboardModifiers keyState = event->modifiers();

    // start some drag operations after a minimum of movement:
    bool isDrag = (logicPos - m_beginPoint).manhattanLength() > 4;
    if (!isDrag) {
        return;
    }

    const Element* hitElement = this->hitElement();

    // drag element
    if (!middleButton && ((hitElement && hitElement->isMovable())
                          || viewInteraction()->isGripEditStarted())) {
        if (hitElement && !viewInteraction()->isDragStarted()) {
            startDragElements(hitElement->type(), hitElement->offset());
        }

        if (viewInteraction()->isGripEditStarted() && !viewInteraction()->isDragStarted()) {
            Element* selectedElement = viewInteraction()->selection()->element();
            startDragElements(selectedElement->type(), selectedElement->offset());
        }

        DragMode mode = DragMode::BothXY;
        if (keyState & Qt::ShiftModifier) {
            mode = DragMode::OnlyY;
        } else if (keyState & Qt::ControlModifier) {
            mode = DragMode::OnlyX;
        }

        viewInteraction()->drag(m_beginPoint, logicPos, mode);
        return;
    } else if (hitElement == nullptr && (keyState & (Qt::ShiftModifier | Qt::ControlModifier))) {
        if (!viewInteraction()->isDragStarted()) {
            viewInteraction()->startDrag(std::vector<Element*>(), PointF(), [](const Element*) { return false; });
        }
        viewInteraction()->drag(m_beginPoint, logicPos,
                                keyState & Qt::ControlModifier ? DragMode::LassoList : DragMode::BothXY);
        return;
    }

    // move canvas
    PointF d = logicPos - m_beginPoint;
    int dx = d.x();
    int dy = d.y();

    if (dx == 0 && dy == 0) {
        return;
    }

    m_view->moveCanvas(dx, dy);
    m_isCanvasDragged = true;
}

void NotationViewInputController::startDragElements(ElementType elementsType, const PointF& elementsOffset)
{
    std::vector<Element*> elements = viewInteraction()->selection()->elements();
    IF_ASSERT_FAILED(!elements.empty()) {
        return;
    }

    bool isFilterType = viewInteraction()->selection()->isRange();
    auto isDraggable = [isFilterType, elementsType](const Element* element) {
        return element && element->selected() && (!isFilterType || elementsType == element->type());
    };

    viewInteraction()->startDrag(elements, elementsOffset, isDraggable);
}

void NotationViewInputController::mouseReleaseEvent(QMouseEvent*)
{
    if (!hitElement() && !m_isCanvasDragged && !viewInteraction()->isGripEditStarted()
        && !viewInteraction()->isDragStarted()) {
        viewInteraction()->clearSelection();
    }

    m_isCanvasDragged = false;

    if (viewInteraction()->isDragStarted()) {
        viewInteraction()->endDrag();
    }
}

void NotationViewInputController::mouseDoubleClickEvent(QMouseEvent* event)
{
    Element* element = viewInteraction()->selection()->element();

    if (!element) {
        return;
    }

    if (element->isTextBase()) {
        viewInteraction()->startEditText(element, m_view->toLogical(event->pos()));
    }
}

void NotationViewInputController::hoverMoveEvent(QHoverEvent* event)
{
    if (m_view->isNoteEnterMode()) {
        PointF pos = m_view->toLogical(event->pos());
        m_view->showShadowNote(pos);
    }
}

void NotationViewInputController::keyPressEvent(QKeyEvent* event)
{
    viewInteraction()->editText(event);
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
        viewInteraction()->startDrop(edata);

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

    PointF pos = m_view->toLogical(event->pos());
    Qt::KeyboardModifiers modifiers = event->keyboardModifiers();

    bool isAccepted = viewInteraction()->isDropAccepted(pos, modifiers);
    if (isAccepted) {
        event->setAccepted(isAccepted);
    } else {
        event->ignore();
    }
}

void NotationViewInputController::dragLeaveEvent(QDragLeaveEvent*)
{
    viewInteraction()->endDrop();
}

void NotationViewInputController::dropEvent(QDropEvent* event)
{
    PointF pos = m_view->toLogical(event->pos());
    Qt::KeyboardModifiers modifiers = event->keyboardModifiers();

    bool isAccepted = viewInteraction()->drop(pos, modifiers);
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
    const Element* hitElement = this->hitElement();
    ElementType type = ElementType::INVALID;

    if (hitElement) {
        type = hitElement->type();
    } else {
        type = ElementType::PAGE;
    }

    return type;
}

mu::PointF NotationViewInputController::hitElementPos() const
{
    if (viewInteraction()->hitElementContext().element) {
        return viewInteraction()->hitElementContext().element->canvasBoundingRect().center();
    }
    return mu::PointF();
}

double NotationViewInputController::guiScalling() const
{
    return configuration()->guiScaling();
}
