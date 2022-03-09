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

using namespace mu;
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
    m_possibleZoomPercentages = configuration()->possibleZoomPercentageList();

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

        dispatcher()->reg(this, "scr-next", this, &NotationViewInputController::nextScreen);
        dispatcher()->reg(this, "scr-prev", this, &NotationViewInputController::previousScreen);
        dispatcher()->reg(this, "page-next", this, &NotationViewInputController::nextPage);
        dispatcher()->reg(this, "page-prev", this, &NotationViewInputController::previousPage);
        dispatcher()->reg(this, "page-top", this, &NotationViewInputController::startOfScore);
        dispatcher()->reg(this, "page-end", this, &NotationViewInputController::endOfScore);

        dispatcher()->reg(this, "notation-context-menu", [this]() {
            m_view->showContextMenu(selectionType(),
                                    m_view->fromLogical(selectionElementPos()).toQPointF(), true);
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

bool NotationViewInputController::readonly() const
{
    return m_readonly;
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

EngravingItem* NotationViewInputController::hitElement() const
{
    return viewInteraction()->hitElementContext().element;
}

void NotationViewInputController::zoomIn()
{
    int maxIndex = m_possibleZoomPercentages.size() > 0 ? m_possibleZoomPercentages.size() - 1 : 0;
    int currentIndex = std::min(currentZoomIndex() + 1, maxIndex);

    int zoom = m_possibleZoomPercentages[currentIndex];

    setZoom(zoom, findZoomFocusPoint());
}

void NotationViewInputController::zoomOut()
{
    int currentIndex = std::max(currentZoomIndex() - 1, 0);

    int zoom = m_possibleZoomPercentages[currentIndex];

    setZoom(zoom, findZoomFocusPoint());
}

PointF NotationViewInputController::findZoomFocusPoint() const
{
    double resultX = 0.0;
    double resultY = 0.0;

    INotationSelectionPtr selection = m_view->notationInteraction()->selection();

    if (selection->isNone()) {
        // No selection: zoom at the center of the view
        resultX = m_view->width() / 2;
        resultY = m_view->height() / 2;
    } else {
        // Selection: zoom at the center of the selection
        PointF result = m_view->fromLogical(selection->canvasBoundingRect().center());
        resultX = result.x();
        resultY = result.y();
    }

    return PointF(std::clamp(resultX, 0.0, m_view->width()),
                  std::clamp(resultY, 0.0, m_view->height()));
}

void NotationViewInputController::zoomToPageWidth()
{
    if (!notationStyle()) {
        return;
    }

    qreal pageWidth = notationStyle()->styleValue(Ms::Sid::pageWidth).toDouble() * Ms::DPI;

    qreal scale = m_view->width() / pageWidth;
    setScaling(scale);
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

    qreal scale = std::min(pageWidthScale, pageHeightScale);
    setScaling(scale, PointF());
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
        constexpr qreal VERTICAL_PAGE_GAP = 5.0;
        pageHeightScale = viewHeight / (pageHeight * 2.0 + VERTICAL_PAGE_GAP);
        pageWidthScale = viewWidth / pageWidth;
    } else {
        constexpr qreal HORIZONTAL_PAGE_GAP = 50.0;
        pageHeightScale = viewHeight / pageHeight;
        pageWidthScale = viewWidth / (pageWidth * 2.0 + HORIZONTAL_PAGE_GAP);
    }

    qreal scale = std::min(pageHeightScale, pageWidthScale);
    setScaling(scale, PointF());
    m_isZoomInited = true;
}

int NotationViewInputController::currentZoomIndex() const
{
    for (int index = 0; index < m_possibleZoomPercentages.size(); ++index) {
        if (m_possibleZoomPercentages[index] >= currentZoomPercentage()) {
            return index;
        }
    }

    return m_possibleZoomPercentages.isEmpty() ? 0 : m_possibleZoomPercentages.size() - 1;
}

int NotationViewInputController::currentZoomPercentage() const
{
    return zoomPercentageFromScaling(m_view->currentScaling());
}

void NotationViewInputController::setScaling(qreal scaling, const PointF& pos)
{
    qreal minScaling = scalingFromZoomPercentage(m_possibleZoomPercentages.first());
    qreal maxScaling = scalingFromZoomPercentage(m_possibleZoomPercentages.last());
    qreal correctedScaling = std::clamp(scaling, minScaling, maxScaling);

    if (!m_readonly) {
        int zoomPercentage = zoomPercentageFromScaling(correctedScaling);
        configuration()->setCurrentZoom(zoomPercentage);
    }

    m_view->setScaling(correctedScaling, pos);
}

void NotationViewInputController::setZoom(int zoomPercentage, const PointF& pos)
{
    int minZoom = m_possibleZoomPercentages.first();
    int maxZoom = m_possibleZoomPercentages.last();
    int correctedZoom = std::clamp(zoomPercentage, minZoom, maxZoom);

    if (!m_readonly) {
        configuration()->setCurrentZoom(correctedZoom);
    }

    qreal scaling = static_cast<qreal>(correctedZoom) / 100.0 * configuration()->notationScaling();
    m_view->setScaling(scaling, pos);
}

qreal NotationViewInputController::scalingFromZoomPercentage(int zoomPercentage) const
{
    return zoomPercentage / 100.0 * configuration()->notationScaling();
}

int NotationViewInputController::zoomPercentageFromScaling(qreal scaling) const
{
    return qRound(scaling * 100.0 / configuration()->notationScaling());
}

void NotationViewInputController::setViewMode(const ViewMode& viewMode)
{
    auto project = globalContext()->currentProject();
    if (project) {
        project->viewSettings()->setNotationViewMode(viewMode);
    }

    auto notation = globalContext()->currentNotation();
    if (notation) {
        notation->setViewMode(viewMode);
    }
}

constexpr qreal scrollStep = .8;
constexpr qreal notationScreenPadding = 25.0;

void NotationViewInputController::moveScreen(int direction)
{
    auto notation = currentNotation();
    if (!notation || m_view->width() == 0.0) {
        return;
    }
    auto scale = m_view->currentScaling();
    if (notation->viewMode() == ViewMode::LINE) {
        m_view->moveCanvasHorizontal(m_view->width() * direction * scrollStep / scale);
    } else {
        auto offset = m_view->toLogical(QPoint());
        auto rect = m_view->notationContentRect();
        if (direction > 0 && offset.y() <= 0.0) {
            if (offset.x() >= -notationScreenPadding) {
                m_view->moveCanvas(m_view->width() * direction * scrollStep / scale,
                                   offset.y() - notationScreenPadding - (rect.height() - m_view->height() / scale));
            }
        } else if (direction < 0 && offset.y() >= (rect.height() - m_view->height() / scale)) {
            auto dx = m_view->width() * direction * scrollStep / scale;
            if (offset.x() < rect.width() + notationScreenPadding + dx) {
                m_view->moveCanvas(dx, offset.y() + notationScreenPadding);
            }
        } else {
            m_view->moveCanvasVertical(m_view->height() * direction * scrollStep / scale);
        }
    }
}

void NotationViewInputController::movePage(int direction)
{
    auto notation = currentNotation();
    if (!notation) {
        return;
    }
    if (notation->viewMode() != ViewMode::PAGE) {
        moveScreen(direction);
        return;
    }
    Page* page = notation->elements()->msScore()->pages().back();
    if (configuration()->canvasOrientation().val == mu::framework::Orientation::Vertical) {
        qreal offset = std::min((page->height() + notationScreenPadding) * direction, m_view->toLogical(
                                    QPoint()).y() + notationScreenPadding);
        m_view->moveCanvasVertical(offset);
    } else {
        qreal offset
            = std::min((page->width() + notationScreenPadding) * direction, m_view->toLogical(QPoint()).x() + notationScreenPadding);
        m_view->moveCanvasHorizontal(offset);
    }
}

void NotationViewInputController::nextScreen()
{
    moveScreen(-1);
}

void NotationViewInputController::previousScreen()
{
    moveScreen(1);
}

void NotationViewInputController::nextPage()
{
    movePage(-1);
}

void NotationViewInputController::previousPage()
{
    movePage(1);
}

void NotationViewInputController::startOfScore()
{
    auto offset = m_view->toLogical(QPoint());
    m_view->moveCanvas(offset.x() + notationScreenPadding, offset.y() + notationScreenPadding);
}

void NotationViewInputController::endOfScore()
{
    auto notation = currentNotation();
    if (!notation) {
        return;
    }
    Ms::MeasureBase* lastMeasure = notation->elements()->msScore()->lastMeasureMM();
    auto lmRect = lastMeasure->canvasBoundingRect();
    auto scale = m_view->currentScaling();
    qreal desiredX = std::max(-notationScreenPadding, lmRect.right() + notationScreenPadding - m_view->width() / scale);
    qreal desiredY
        = std::max(-notationScreenPadding, lmRect.bottom() + lastMeasure->score()->styleD(Ms::Sid::spatium) * 5 - m_view->height() / scale);
    auto offset = m_view->toLogical(QPoint());
    m_view->moveCanvas(offset.x() - desiredX, offset.y() - desiredY);
}

void NotationViewInputController::pinchToZoom(qreal scaleFactor, const QPointF& pos)
{
    double scale = m_view->currentScaling() * scaleFactor;
    setScaling(scale, PointF::fromQPointF(pos));
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
        stepsX = dx / static_cast<qreal>(PIXELSSTEPSFACTOR);
        stepsY = dy / static_cast<qreal>(PIXELSSTEPSFACTOR);
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
        qreal absSteps = sqrt(stepsX * stepsX + stepsY * stepsY) * (stepsY > -stepsX ? 1 : -1);
        double scale = m_view->currentScaling() * qPow(zoomSpeed, absSteps);
        setScaling(scale, PointF::fromQPointF(event->position()));
    } else {
        qreal correction = 1.0 / m_view->currentScaling();
        if (keyState & Qt::ShiftModifier) {
            int abs = sqrt(dx * dx + dy * dy) * (dy > -dx ? 1 : -1);
            m_view->moveCanvasHorizontal(abs * correction);
        } else {
            m_view->moveCanvas(dx * correction, dy * correction);
        }
    }
}

void NotationViewInputController::mousePressEvent(QMouseEvent* event)
{
    PointF logicPos = PointF(m_view->toLogical(event->pos()));
    Qt::KeyboardModifiers keyState = event->modifiers();
    Qt::MouseButton button = event->button();

    // When using MiddleButton, just start moving the canvas
    if (button == Qt::MiddleButton) {
        m_beginPoint = logicPos;
        return;
    }

    // note enter mode
    if (m_view->isNoteEnterMode()) {
        if (button == Qt::RightButton) {
            dispatcher()->dispatch("remove-note", ActionData::make_arg1<PointF>(logicPos));
            return;
        }

        bool replace = keyState & Qt::ShiftModifier;
        bool insert = keyState & Qt::ControlModifier;
        dispatcher()->dispatch("put-note", ActionData::make_arg3<PointF, bool, bool>(logicPos, replace, insert));
        return;
    }

    m_beginPoint = logicPos;

    EngravingItem* hitElement = nullptr;
    const EngravingItem* prevHitElement = nullptr;
    int hitStaffIndex = -1;

    if (!m_readonly) {
        prevHitElement = viewInteraction()->hitElementContext().element;

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

    ClickContext ctx;
    ctx.logicClickPos = logicPos;
    ctx.hitElement = hitElement;
    ctx.prevHitElement = prevHitElement;
    ctx.event = event;

    if (needSelect(ctx)) {
        SelectType selectType = SelectType::SINGLE;
        if (keyState == Qt::NoModifier) {
            selectType = SelectType::SINGLE;
        } else if (keyState & Qt::ShiftModifier) {
            selectType = SelectType::RANGE;
        } else if (keyState & Qt::ControlModifier) {
            selectType = SelectType::ADD;
        }
        viewInteraction()->select({ hitElement }, selectType, hitStaffIndex);
    } else if (hitElement && event->button() == Qt::LeftButton
               && event->modifiers() == Qt::KeyboardModifier::ControlModifier) {
        viewInteraction()->select({ hitElement }, SelectType::ADD, hitStaffIndex);
    }

    if (button == Qt::LeftButton) {
        handleLeftClick(ctx);
    } else if (button == Qt::RightButton) {
        handleRightClick(ctx);
    }
}

bool NotationViewInputController::needSelect(const ClickContext& ctx) const
{
    if (!ctx.hitElement) {
        return false;
    }

    bool result = true;

    if (ctx.event->button() == Qt::MouseButton::RightButton) {
        result = !viewInteraction()->selection()->range()->containsPoint(ctx.logicClickPos);
    }

    return result;
}

void NotationViewInputController::handleLeftClick(const ClickContext& ctx)
{
    m_view->hideContextMenu();

    if (viewInteraction()->isHitGrip(ctx.logicClickPos)) {
        viewInteraction()->startEditGrip(ctx.logicClickPos);
        return;
    } else if (ctx.hitElement && ctx.hitElement->hasGrips() && ctx.hitElement->needStartEditingAfterSelecting()) {
        viewInteraction()->startEditGrip(ctx.hitElement, ctx.hitElement->defaultGrip());
        return;
    }

    if (!ctx.hitElement) {
        viewInteraction()->endEditElement();
        return;
    }

    if (ctx.hitElement->isPlayable()) {
        playbackController()->playElement(ctx.hitElement);
    }

    if (!viewInteraction()->isTextSelected()) {
        return;
    }

    if (ctx.hitElement == ctx.prevHitElement) {
        if (!viewInteraction()->isTextEditingStarted()) {
            dispatcher()->dispatch("edit-text");
        }
    }

    updateTextCursorPosition();
}

void NotationViewInputController::handleRightClick(const ClickContext& ctx)
{
    m_view->showContextMenu(selectionType(), ctx.event->pos());

    if (!ctx.hitElement) {
        viewInteraction()->endEditElement();
        return;
    }

    updateTextCursorPosition();
}

bool NotationViewInputController::startTextEditingAllowed() const
{
    INotationInteractionPtr interaction = viewInteraction();
    return interaction->isTextSelected() && !interaction->isTextEditingStarted();
}

void NotationViewInputController::updateTextCursorPosition()
{
    if (viewInteraction()->isTextEditingStarted()) {
        mu::RectF bbox = viewInteraction()->editedText()->canvasBoundingRect();
        mu::PointF constrainedPt = mu::PointF(
            m_beginPoint.x() < bbox.left() ? bbox.left()
            : m_beginPoint.x() >= bbox.right() ? bbox.right() - 1 : m_beginPoint.x(),
            m_beginPoint.y() < bbox.top() ? bbox.top()
            : m_beginPoint.y() >= bbox.bottom() ? bbox.bottom() - 1 : m_beginPoint.y());
        viewInteraction()->changeTextCursorPosition(constrainedPt);
    }
}

void NotationViewInputController::mouseMoveEvent(QMouseEvent* event)
{
    PointF logicPos = m_view->toLogical(event->pos());
    Qt::KeyboardModifiers keyState = event->modifiers();

    PointF dragDelta = logicPos - m_beginPoint;
    // start some drag operations after a minimum of movement:
    bool isDrag = dragDelta.manhattanLength() > 4;
    if (!isDrag) {
        return;
    }

    bool isNoteEnterMode = m_view->isNoteEnterMode();
    bool isDragObjectsAllowed = !(isNoteEnterMode || playbackController()->isPlaying());
    if (isDragObjectsAllowed) {
        const EngravingItem* hitElement = this->hitElement();

        // drag element
        if ((hitElement && (hitElement->isMovable() || viewInteraction()->isElementEditStarted()))
            || viewInteraction()->isGripEditStarted()) {
            if (hitElement && !viewInteraction()->isDragStarted()) {
                startDragElements(hitElement->type(), hitElement->offset());
            }

            if (viewInteraction()->isGripEditStarted() && !viewInteraction()->isDragStarted()) {
                EngravingItem* selectedElement = viewInteraction()->selection()->element();
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
                viewInteraction()->startDrag(std::vector<EngravingItem*>(), PointF(), [](const EngravingItem*) { return false; });
            }
            viewInteraction()->drag(m_beginPoint, logicPos, keyState & Qt::ControlModifier ? DragMode::LassoList : DragMode::BothXY);

            return;
        }
    }

    // move canvas
    if (!isNoteEnterMode) {
        m_view->moveCanvas(dragDelta.x(), dragDelta.y());
        m_isCanvasDragged = true;
    }
}

void NotationViewInputController::startDragElements(ElementType elementsType, const PointF& elementsOffset)
{
    std::vector<EngravingItem*> elements = viewInteraction()->selection()->elements();
    IF_ASSERT_FAILED(!elements.empty()) {
        return;
    }

    bool isFilterType = viewInteraction()->selection()->isRange();
    auto isDraggable = [isFilterType, elementsType](const EngravingItem* element) {
        return element && element->selected() && (!isFilterType || elementsType == element->type());
    };

    viewInteraction()->startDrag(elements, elementsOffset, isDraggable);
}

void NotationViewInputController::mouseReleaseEvent(QMouseEvent*)
{
    INotationInteractionPtr interaction = viewInteraction();
    INotationNoteInputPtr noteInput = interaction->noteInput();

    if (!hitElement() && !m_isCanvasDragged && !interaction->isGripEditStarted()
        && !interaction->isDragStarted() && !noteInput->isNoteInputMode()) {
        interaction->clearSelection();
    }

    m_isCanvasDragged = false;

    if (interaction->isDragStarted()) {
        interaction->endDrag();
    }
}

void NotationViewInputController::mouseDoubleClickEvent(QMouseEvent*)
{
    if (m_view->isNoteEnterMode()) {
        return;
    }

    if (viewInteraction()->isElementEditStarted()) {
        return;
    }

    dispatcher()->dispatch("edit-element");
}

void NotationViewInputController::hoverMoveEvent(QHoverEvent* event)
{
    if (!m_view->isNoteEnterMode()) {
        return;
    }

    PointF oldPos = m_view->toLogical(event->oldPosF());
    PointF pos = m_view->toLogical(event->posF());

    if (oldPos == pos) {
        return;
    }

    m_view->showShadowNote(pos);
}

bool NotationViewInputController::shortcutOverrideEvent(QKeyEvent* event)
{
    if (viewInteraction()->isElementEditStarted()) {
        return viewInteraction()->isEditAllowed(event);
    } else if (startTextEditingAllowed()) {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
            return true;
        }
    }

    return false;
}

void NotationViewInputController::keyPressEvent(QKeyEvent* event)
{
    if (viewInteraction()->isElementEditStarted()) {
        viewInteraction()->editElement(event);
    } else if (startTextEditingAllowed()) {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
            dispatcher()->dispatch("edit-text");
            event->accept();
        }
    }
}

void NotationViewInputController::inputMethodEvent(QInputMethodEvent* event)
{
    if (viewInteraction()->isTextEditingStarted()) {
        viewInteraction()->editText(event);
    }
}

bool NotationViewInputController::canHandleInputMethodQuery(Qt::InputMethodQuery query) const
{
    if (!viewInteraction()->isTextEditingStarted()) {
        return false;
    }

    static const QList<Qt::InputMethodQuery> allowedQueries {
        Qt::ImCursorRectangle,
        Qt::ImEnabled,
        Qt::ImHints
    };

    return allowedQueries.contains(query);
}

//! NOTE: Copied from ScoreView::inputMethodQuery
QVariant NotationViewInputController::inputMethodQuery(Qt::InputMethodQuery query) const
{
    if (!viewInteraction()->isTextEditingStarted()) {
        return QVariant();
    }

    switch (query) {
    case Qt::ImCursorRectangle: {
        const TextBase* editedText = viewInteraction()->editedText();
        RectF cursorRect = editedText->cursor()->cursorRect().translated(editedText->canvasPos());

        QRectF rect = m_view->fromLogical(cursorRect).toQRectF();
        rect.setWidth(1); // InputMethod doesn't display properly if width left at 0
        rect.setHeight(rect.height() + 10); // add a little margin under the cursor

        return rect;
    }
    case Qt::ImEnabled:
        return true; // TextBase will always accept input method input
    case Qt::ImHints:
        return Qt::ImhNone; // No hints for now, but maybe in future will give hints
    default:
        break;
    }

    return QVariant();
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

    QList<QUrl> urls = mimeData->urls();
    if (urls.count() > 0) {
        QUrl url = urls.first();

        if (viewInteraction()->startDrop(url)) {
            event->accept();
            return;
        }
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
    ElementType type = ElementType::INVALID;

    auto selection = viewInteraction()->selection();
    if (!selection) {
        return type;
    }

    if (auto hitElement = this->hitElement()) {
        return hitElement->type();
    } else if (selection->isRange()) {
        return ElementType::STAFF;
    } else if (auto selectedElement = selection->element()) {
        return selectedElement->type();
    } else {
        return ElementType::PAGE;
    }

    return type;
}

mu::PointF NotationViewInputController::selectionElementPos() const
{
    auto selection = viewInteraction()->selection();
    if (!selection) {
        return mu::PointF();
    }

    if (auto hitElement = this->hitElement()) {
        return hitElement->canvasBoundingRect().center();
    } else if (selection->isRange()) {
        auto range = selection->range();
        return range->boundingArea().front().center();
    } else if (auto selectedElement = selection->element()) {
        return selectedElement->canvasBoundingRect().center();
    }

    return mu::PointF();
}
