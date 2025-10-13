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
#include "notationviewinputcontroller.h"

#include <set>

#include <QApplication>
#include <QMimeData>
#include <QQuickItem>
#include <QTimer>
#include <QtMath>

#include "log.h"
#include "commonscene/commonscenetypes.h"
#include "abstractelementpopupmodel.h"

#include "engraving/dom/drumset.h"
#include "engraving/dom/shadownote.h"

#include "defer.h"

using namespace mu;
using namespace mu::notation;
using namespace mu::engraving;
using namespace muse::actions;
using namespace mu::commonscene;

static constexpr int PIXELSSTEPSFACTOR = 5;

static bool seekAllowed(const mu::engraving::EngravingItem* element)
{
    if (!element) {
        return false;
    }

    static const ElementTypeSet ALLOWED_TYPES {
        ElementType::NOTE,
        ElementType::REST,
        ElementType::MMREST,
        ElementType::MEASURE,
        ElementType::BAR_LINE
    };

    return muse::contains(ALLOWED_TYPES, element->type());
}

NotationViewInputController::NotationViewInputController(IControlledView* view, const muse::modularity::ContextPtr& iocCtx)
    : muse::Injectable(iocCtx), m_view(view)
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
        dispatcher()->reg(this, "zoom100", [this]() { setZoom(100, findZoomFocusPoint()); });
        dispatcher()->reg(this, "zoom-x-percent", [this](const ActionData& args) { setZoom(args.arg<int>(0), findZoomFocusPoint()); });

        dispatcher()->reg(this, "view-mode-page", [this]() {
            setViewMode(ViewMode::PAGE);
        });

        if (globalConfiguration()->devModeEnabled()) {
            dispatcher()->reg(this, "view-mode-float", [this]() {
                setViewMode(ViewMode::FLOAT);
            });
        }

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
            m_view->showContextMenu(selectionType(), m_view->fromLogical(selectionElementPos()).toQPointF());
        });

        dispatcher()->reg(this, "notation-popup-menu", [this](const ActionData& args) {
            if (EngravingItem* el = args.arg<EngravingItem*>()) {
                togglePopupForItemIfSupports(el);
            }
        });

        onNotationChanged();
        globalContext()->currentNotationChanged().onNotify(this, [this]() {
            onNotationChanged();
        });
    }
}

void NotationViewInputController::onNotationChanged()
{
    INotationPtr currNotation = currentNotation();
    if (!currNotation) {
        return;
    }

    currNotation->interaction()->selectionChanged().onNotify(this, [this]() {
        const INotationPtr notation = currentNotation();
        if (!notation) {
            return;
        }

        const EngravingItem* selectedItem = notation->interaction()->selection()->element();
        ElementType type = selectedItem ? selectedItem->type() : ElementType::INVALID;

        bool noChanges = selectedItem && m_prevSelectedElement == selectedItem;
        m_prevSelectedElement = selectedItem;

        if (noChanges) {
            return;
        }

        m_view->hideContextMenu();
        m_view->hideElementPopup();

        if (AbstractElementPopupModel::hasElementEditPopup(selectedItem)) {
            m_view->showElementPopup(type);
        }
    });

    currNotation->interaction()->textEditingStarted().onNotify(this, [this] {
        const INotationPtr notation = currentNotation();
        if (!notation) {
            return;
        }

        m_view->hideContextMenu();

        const TextBase* item = notation->interaction()->editedText();
        if (AbstractElementPopupModel::hasTextStylePopup(item)) {
            static const std::set<ElementType> TYPES_NEEDING_STAFF {
                ElementType::LYRICS,
                ElementType::FINGERING,
                ElementType::STICKING,
                ElementType::HARMONY,
                ElementType::FIGURED_BASS
            };
            const bool needToSeeStaffWhileEnteringText = muse::contains(TYPES_NEEDING_STAFF, item->type());
            if (!needToSeeStaffWhileEnteringText || !item->empty()) {
                m_view->showElementPopup(item->type());
                return;
            }
        }

        if (item->isDynamic()) {
            m_view->showElementPopup(item->type());
            return;
        }

        m_view->hideElementPopup();
    });

    currNotation->interaction()->textEditingChanged().onNotify(this, [this] {
        const INotationPtr notation = currentNotation();
        if (!notation) {
            return;
        }

        if (!notation->interaction()->isTextEditingStarted()) {
            return;
        }

        const TextBase* item = notation->interaction()->editedText();
        if (AbstractElementPopupModel::hasTextStylePopup(item) && item->cursor()->hasSelection()) {
            m_view->showElementPopup(item->type());
        }
    });

    currNotation->interaction()->textEditingEnded().onReceive(this, [this](const TextBase*) {
        m_view->hideElementPopup(PopupModelType::TYPE_TEXT);
    });
}

void NotationViewInputController::initZoom()
{
    IF_ASSERT_FAILED(currentNotation()) {
        return;
    }

    ZoomType defaultZoomType = configuration()->defaultZoomType();

    currentNotation()->viewState()->setZoomType(defaultZoomType);

    switch (defaultZoomType) {
    case ZoomType::Percentage:
        setZoom(configuration()->defaultZoom());
        break;
    case ZoomType::PageWidth:
        doZoomToPageWidth();
        break;
    case ZoomType::WholePage:
        doZoomToWholePage();
        break;
    case ZoomType::TwoPages:
        doZoomToTwoPages();
        break;
    }

    currentNotation()->viewState()->setMatrixInited(true);
}

void NotationViewInputController::initCanvasPos()
{
    RectF notationContentRect = m_view->notationContentRect();
    double totalScoreWidth = notationContentRect.width();
    double totalScoreHeight = notationContentRect.height();

    double curScaling = m_view->currentScaling();
    double viewWidth = m_view->width() / curScaling;
    double viewHeight = m_view->height() / curScaling;

    const double canvasMargin = MScore::horizontalPageGapOdd;

    bool centerHorizontally = totalScoreWidth < viewWidth - 2 * canvasMargin;
    bool centerVertically = totalScoreHeight < viewHeight - 2 * canvasMargin;

    double xMove = centerHorizontally ? 0.5 * (viewWidth - totalScoreWidth) : canvasMargin;
    double yMove = centerVertically ? 0.5 * (viewHeight - totalScoreHeight) : canvasMargin;

    m_view->moveCanvas(xMove, yMove);
}

void NotationViewInputController::updateZoomAfterSizeChange()
{
    IF_ASSERT_FAILED(currentNotation()) {
        return;
    }

    switch (currentNotation()->viewState()->zoomType().val) {
    case ZoomType::Percentage:
        break;
    case ZoomType::PageWidth:
        doZoomToPageWidth();
        break;
    case ZoomType::WholePage:
        doZoomToWholePage();
        break;
    case ZoomType::TwoPages:
        doZoomToTwoPages();
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

const INotationInteraction::HitElementContext& NotationViewInputController::hitElementContext() const
{
    return viewInteraction()->hitElementContext();
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
    IF_ASSERT_FAILED(currentNotation()) {
        return;
    }

    currentNotation()->viewState()->setZoomType(ZoomType::PageWidth);
    doZoomToPageWidth();
}

void NotationViewInputController::doZoomToPageWidth()
{
    if (!notationStyle()) {
        return;
    }

    qreal pageWidth = notationStyle()->styleValue(mu::engraving::Sid::pageWidth).toDouble() * mu::engraving::DPI
                      + 2 * MScore::horizontalPageGapOdd;
    double viewWidth = m_view->width();

    qreal scale = viewWidth / pageWidth;
    setScaling(scale, PointF(), false);
}

void NotationViewInputController::zoomToWholePage()
{
    IF_ASSERT_FAILED(currentNotation()) {
        return;
    }

    currentNotation()->viewState()->setZoomType(ZoomType::WholePage);
    doZoomToWholePage();
}

void NotationViewInputController::doZoomToWholePage()
{
    if (!notationStyle()) {
        return;
    }

    qreal pageWidth = notationStyle()->styleValue(mu::engraving::Sid::pageWidth).toDouble() * mu::engraving::DPI
                      + 2 * MScore::horizontalPageGapOdd;
    qreal pageHeight = notationStyle()->styleValue(mu::engraving::Sid::pageHeight).toDouble() * mu::engraving::DPI
                       + 2 * MScore::horizontalPageGapOdd;

    double viewWidth = m_view->width();
    double viewHeight = m_view->height();

    qreal pageWidthScale = viewWidth / pageWidth;
    qreal pageHeightScale = viewHeight / pageHeight;

    qreal scale = std::min(pageWidthScale, pageHeightScale);
    setScaling(scale, PointF(), false);
}

void NotationViewInputController::zoomToTwoPages()
{
    IF_ASSERT_FAILED(currentNotation()) {
        return;
    }

    currentNotation()->viewState()->setZoomType(ZoomType::TwoPages);
    doZoomToTwoPages();
}

void NotationViewInputController::doZoomToTwoPages()
{
    if (!notationStyle()) {
        return;
    }

    qreal viewWidth = m_view->width();
    qreal viewHeight = m_view->height();
    qreal pageWidth = notationStyle()->styleValue(mu::engraving::Sid::pageWidth).toDouble() * mu::engraving::DPI
                      + 2 * MScore::horizontalPageGapOdd;
    qreal pageHeight = notationStyle()->styleValue(mu::engraving::Sid::pageHeight).toDouble() * mu::engraving::DPI
                       + 2 * MScore::horizontalPageGapOdd;

    qreal pageHeightScale = 0.0;
    qreal pageWidthScale = 0.0;

    if (configuration()->canvasOrientation().val == muse::Orientation::Vertical) {
        constexpr qreal VERTICAL_PAGE_GAP = 5.0;
        pageHeightScale = viewHeight / (pageHeight * 2.0 + VERTICAL_PAGE_GAP);
        pageWidthScale = viewWidth / pageWidth;
    } else {
        constexpr qreal HORIZONTAL_PAGE_GAP = 50.0;
        pageHeightScale = viewHeight / pageHeight;
        pageWidthScale = viewWidth / (pageWidth * 2.0 + HORIZONTAL_PAGE_GAP);
    }

    qreal scale = std::min(pageHeightScale, pageWidthScale);
    setScaling(scale, PointF(), false);
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

void NotationViewInputController::setScaling(qreal scaling, const PointF& pos, bool overrideZoomType)
{
    qreal minScaling = scalingFromZoomPercentage(m_possibleZoomPercentages.first());
    qreal maxScaling = scalingFromZoomPercentage(m_possibleZoomPercentages.last());
    qreal correctedScaling = std::clamp(scaling, minScaling, maxScaling);

    m_view->setScaling(correctedScaling, pos, overrideZoomType);
}

void NotationViewInputController::setZoom(int zoomPercentage, const PointF& pos)
{
    int minZoom = m_possibleZoomPercentages.first();
    int maxZoom = m_possibleZoomPercentages.last();
    int correctedZoom = std::clamp(zoomPercentage, minZoom, maxZoom);

    qreal scaling = scalingFromZoomPercentage(correctedZoom);
    m_view->setScaling(scaling, pos);
}

qreal NotationViewInputController::scalingFromZoomPercentage(int zoomPercentage) const
{
    return configuration()->scalingFromZoomPercentage(zoomPercentage);
}

int NotationViewInputController::zoomPercentageFromScaling(qreal scaling) const
{
    return configuration()->zoomPercentageFromScaling(scaling);
}

void NotationViewInputController::setViewMode(const ViewMode& viewMode)
{
    auto notation = globalContext()->currentNotation();
    if (notation) {
        notation->viewState()->setViewMode(viewMode);
        notation->painting()->setViewMode(viewMode);
    }
}

constexpr qreal scrollStep = .8;
constexpr qreal notationScreenPadding = 25.0;

void NotationViewInputController::moveScreen(int direction)
{
    auto notation = currentNotation();
    if (!notation || muse::RealIsNull(m_view->width())) {
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
    if (configuration()->canvasOrientation().val == muse::Orientation::Vertical) {
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
    mu::engraving::MeasureBase* lastMeasure = notation->elements()->msScore()->lastMeasureMM();
    auto lmRect = lastMeasure->canvasBoundingRect();
    auto scale = m_view->currentScaling();
    qreal desiredX = std::max(-notationScreenPadding, lmRect.right() + notationScreenPadding - m_view->width() / scale);
    qreal desiredY = std::max(-notationScreenPadding, lmRect.bottom()
                              + lastMeasure->score()->style().styleD(mu::engraving::Sid::spatium) * 5 - m_view->height() / scale);
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

// pixelDelta is unreliable on X11
#ifdef Q_OS_LINUX
    if (std::getenv("WAYLAND_DISPLAY") == NULL) {
        // Ignore pixelsScrolled unless Wayland is used
        pixelsScrolled.setX(0);
        pixelsScrolled.setY(0);
    }
#endif

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
    PointF logicPos = m_view->toLogical(event->pos());
    Qt::MouseButton button = event->button();

    m_mouseDownInfo = {
        /*.dragAction =*/ MouseDownInfo::Other,
        /*.physicalBeginPoint =*/ event->pos(),
        /*.logicalBeginPoint =*/ logicPos
    };

    m_shouldStartEditOnLeftClickRelease = false;
    m_ignoreNextMouseContextMenuEvent = false;

    // When using MiddleButton, just start moving the canvas
    if (button == Qt::MiddleButton) {
        return;
    }

    EngravingItem* hitElement = nullptr;
    staff_idx_t hitStaffIndex = muse::nidx;

    DEFER {
        EngravingItem* playbackStartElement = resolveStartPlayableElement();
        if (playbackStartElement) {
            playbackController()->seekElement(playbackStartElement);
        }
    };

    if (!m_readonly) {
        INotationInteraction::HitElementContext context;
        context.element = viewInteraction()->hitElement(logicPos, hitWidth());
        context.staff = viewInteraction()->hitStaff(logicPos);
        viewInteraction()->setHitElementContext(context);

        hitElement = context.element;
        hitStaffIndex = context.staff ? context.staff->idx() : muse::nidx;
    }

    // note enter mode
    if (m_view->isNoteEnterMode()) {
        handleClickInNoteInputMode(event);
        return;
    }

    // triple click
    if (m_tripleClickPending) {
        if (viewInteraction()->isTextEditingStarted()) {
            viewInteraction()->selectText(mu::engraving::SelectTextType::All);
            return;
        }
    }

    if (playbackController()->isPlaying()) {
        return;
    }

    ClickContext ctx;
    ctx.logicClickPos = logicPos;
    ctx.hitElement = hitElement;
    ctx.hitStaff = hitStaffIndex;
    ctx.isHitGrip = viewInteraction()->isHitGrip(logicPos);
    ctx.event = event;

    // Grip
    bool consumed = mousePress_considerGrip(ctx);
    if (consumed) {
        return;
    }

    // End editing
    if (!ctx.hitElement) {
        viewInteraction()->endEditElement();
    }

    // Drag outgoing: element
    consumed = mousePress_considerDragOutgoingElement(ctx);
    if (consumed) {
        return;
    }

    // Drag outgoing: range
    consumed = mousePress_considerDragOutgoingRange(ctx);
    if (consumed) {
        return;
    }

    // Alt+click to paste range
    consumed = mousePress_considerStartPasteRangeOnRelease(ctx);
    if (consumed) {
        return;
    }

    // Select
    mousePress_considerSelect(ctx);

    // Misc
    if (button == Qt::LeftButton) {
        handleLeftClick(ctx);
    } else if (button == Qt::RightButton) {
        handleRightClick(ctx);
    }
}

void NotationViewInputController::handleClickInNoteInputMode(QMouseEvent* event)
{
    const PointF logicPos = m_view->toLogical(event->pos());

    if (event->button() == Qt::RightButton) {
        m_ignoreNextMouseContextMenuEvent = true;
        dispatcher()->dispatch("remove-note", ActionData::make_arg1<PointF>(logicPos));
        return;
    }

    const Qt::KeyboardModifiers keyState = event->modifiers();
    const bool replace = keyState & Qt::ShiftModifier;
    const bool insert = keyState & Qt::ControlModifier;
    dispatcher()->dispatch("put-note", ActionData::make_arg3<PointF, bool, bool>(logicPos, replace, insert));
}

bool NotationViewInputController::mousePress_considerGrip(const ClickContext& ctx)
{
    if (ctx.isHitGrip && ctx.event->button() == Qt::LeftButton) {
        viewInteraction()->startEditGrip(ctx.logicClickPos);
        return true;
    } else if (viewInteraction()->isGripEditStarted()) {
        viewInteraction()->endEditGrip();
    }

    return false;
}

bool NotationViewInputController::mousePress_considerDragOutgoingElement(const ClickContext& ctx)
{
    if (ctx.event->button() != Qt::LeftButton
        || ctx.event->modifiers() != (Qt::ShiftModifier | Qt::ControlModifier)) {
        return false;
    }

    if (!ctx.hitElement || !viewInteraction()->isOutgoingDragElementAllowed(ctx.hitElement)) {
        return false;
    }

    viewInteraction()->select({ ctx.hitElement }, SelectType::SINGLE, ctx.hitStaff);
    m_mouseDownInfo.dragAction = MouseDownInfo::DragOutgoingElement;
    return true;
}

bool NotationViewInputController::mousePress_considerStartPasteRangeOnRelease(const ClickContext& ctx)
{
    // Check if this is a left click with Alt modifier
    if (ctx.event->button() != Qt::LeftButton || !(ctx.event->modifiers() & Qt::AltModifier)) {
        return false;
    }

    // Check if we have a range selection that can be copied
    const INotationSelectionPtr selection = viewInteraction()->selection();
    if (!selection->isRange() || !selection->canCopy()) {
        return false;
    }

    const INotationSelectionRangePtr range = selection->range();
    const Fraction sourceTick = range->startTick();
    const Fraction tickLength = range->endTick() - range->startTick();
    const engraving::staff_idx_t sourceStaffIdx = range->startStaffIndex();
    const size_t numStaves = range->endStaffIndex() - range->startStaffIndex();

    const bool started = viewInteraction()->startDropRange(sourceTick, tickLength, sourceStaffIdx, numStaves,
                                                           /*preserveMeasureAlignment=*/ true);
    if (!started) {
        return false;
    }

    m_mouseDownInfo.dragAction = MouseDownInfo::PasteRangeOnRelease;

    const bool canDrop = viewInteraction()->updateDropRange(ctx.logicClickPos);
    m_view->asItem()->setCursor(canDrop ? Qt::DragCopyCursor : QCursor());

    return true;
}

void NotationViewInputController::mousePress_considerSelect(const ClickContext& ctx)
{
    if (!ctx.hitElement) {
        return;
    }

    m_hitElementWasAlreadySingleSelected = ctx.hitElement == viewInteraction()->selection()->element();

    if (ctx.event->button() == Qt::LeftButton) {
        if (ctx.event->modifiers() & Qt::ControlModifier) {
            const std::vector<EngravingItem*> overlappingHitElements = viewInteraction()->hitElements(ctx.logicClickPos, hitWidth());
            if (overlappingHitElements.size() > 1) {
                cycleOverlappingHitElements(overlappingHitElements, ctx.hitStaff);
                return;
            }
        }
    } else if (ctx.event->button() == Qt::RightButton) {
        if (ctx.hitElement->selected()) {
            return;
        }
    }

    SelectType selectType = SelectType::SINGLE;
    if (ctx.event->modifiers() & Qt::ShiftModifier) {
        selectType = SelectType::RANGE;
    } else if (ctx.event->modifiers() & Qt::ControlModifier) {
        selectType = SelectType::ADD;
    } else {
        const INotationSelectionPtr selection = viewInteraction()->selection();

        if (selection->isRange()
            && (selection->range()->containsItem(ctx.hitElement, ctx.hitStaff)
                || selection->range()->containsPoint(ctx.logicClickPos))) {
            m_shouldSelectOnLeftClickRelease = true;
            return;
        } else if (ctx.hitElement->selected()) {
            if (selection->elements().size() > 1) {
                m_shouldSelectOnLeftClickRelease = true;
            }
            return;
        }
    }

    viewInteraction()->select({ ctx.hitElement }, selectType, ctx.hitStaff);
}

void NotationViewInputController::cycleOverlappingHitElements(const std::vector<EngravingItem*>& hitElements,
                                                              staff_idx_t hitStaffIndex)
{
    const size_t numHitElements = hitElements.size();
    size_t currTop = numHitElements - 1;
    EngravingItem* e = hitElements[currTop];
    std::set<EngravingItem*> selectedAtPosition;
    bool found = false;

    // e is the topmost element in stacking order,
    // but we want to replace it with "first non-measure element after a selected element"
    // (if such an element exists)
    for (size_t i = 0; i <= numHitElements; ++i) {
        if (found) {
            e = hitElements[currTop];
            if (!e->isMeasure()) {
                break;
            }
        } else if (hitElements[currTop]->selected()) {
            found = true;
            selectedAtPosition.emplace(hitElements[currTop]);
            e = nullptr;
        }
        currTop = (currTop + 1) % numHitElements;
    }

    if (e && !e->selected()) {
        for (EngravingItem* selectedElem : selectedAtPosition) {
            selectedElem->score()->deselect(selectedElem);
        }
        viewInteraction()->select({ e }, SelectType::ADD, hitStaffIndex);
    }
}

bool NotationViewInputController::mousePress_considerDragOutgoingRange(const ClickContext& ctx)
{
    if (ctx.event->button() != Qt::LeftButton) {
        return false;
    }

    if (!ctx.hitElement || ctx.hitElement->isMeasure()) {
        const INotationSelectionPtr selection = viewInteraction()->selection();

        if (selection->isRange() && selection->range()->containsPoint(ctx.logicClickPos)) {
            m_mouseDownInfo.dragAction = MouseDownInfo::DragOutgoingRange;
            return true;
        }
    }

    return false;
}

void NotationViewInputController::handleLeftClick(const ClickContext& ctx)
{
    if (!ctx.hitElement || !ctx.hitElement->selected()) {
        return;
    }

    // If it is the only selected element, start editing if needed
    if (ctx.hitElement == viewInteraction()->selection()->element()
        && ctx.hitElement->needStartEditingAfterSelecting()) {
        viewInteraction()->startEditElement(ctx.hitElement);
    }

    if (ctx.hitElement->isPlayable()) {
        playbackController()->playElements({ ctx.hitElement });
    }

    if (viewInteraction()->isTextSelected()) {
        updateTextCursorPosition();
    }
}

void NotationViewInputController::handleRightClick(const ClickContext& ctx)
{
    m_mouseDownInfo.dragAction = MouseDownInfo::Nothing;

#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
    UNUSED(ctx);
    // See also AbstractNotationPaintView::event for context menu event handling
#else
    m_view->showContextMenu(selectionType(), ctx.event->pos());
#endif
}

bool NotationViewInputController::startTextEditingAllowed() const
{
    INotationInteractionPtr interaction = viewInteraction();
    return interaction->isTextSelected() && !interaction->isTextEditingStarted();
}

void NotationViewInputController::updateTextCursorPosition()
{
    if (viewInteraction()->isTextEditingStarted()) {
        viewInteraction()->changeTextCursorPosition(m_mouseDownInfo.logicalBeginPoint);

        // Show text style popup
        const TextBase* item = viewInteraction()->editedText();
        if (AbstractElementPopupModel::hasTextStylePopup(item)
            && (!item->isLyrics() || !item->empty())) {
            m_view->showElementPopup(item->type());
        }
    }
}

bool NotationViewInputController::tryPercussionShortcut(QKeyEvent* event)
{
    INotationNoteInputPtr noteInput = viewInteraction()->noteInput();
    const mu::engraving::Drumset* drumset = noteInput ? noteInput->state().drumset() : nullptr;
    if (!drumset) {
        return false;
    }

    const QKeyCombination noModifiers(Qt::Key(event->key()));
    const int pitchToWrite = drumset->pitchForShortcut(QKeySequence(noModifiers).toString());
    if (pitchToWrite < 0) {
        return false;
    }

    const Qt::KeyboardModifiers mods = event->modifiers();
    NoteAddingMode addingMode = NoteAddingMode::NextChord;

    if (mods & Qt::ShiftModifier && (mods & Qt::ControlModifier || mods & Qt::MetaModifier)) {
        addingMode = NoteAddingMode::InsertChord;
    } else if (mods & Qt::ShiftModifier) {
        addingMode = NoteAddingMode::CurrentChord;
    } else if (mods & ~Qt::NoModifier) {
        // Not a supported modifier combination...
        return false;
    }

    NoteInputParams params;
    params.drumPitch = pitchToWrite;

    const ActionData args = ActionData::make_arg2<NoteInputParams, NoteAddingMode>(params, addingMode);
    dispatcher()->dispatch("note-action", args);

    return true;
}

void NotationViewInputController::mouseMoveEvent(QMouseEvent* event)
{
    if (m_mouseDownInfo.dragAction == MouseDownInfo::Nothing) {
        return;
    }

    const QPointF physicalDragDelta = event->pos() - m_mouseDownInfo.physicalBeginPoint;

    const bool isDragStarted = m_isCanvasDragged
                               || viewInteraction()->isDragStarted()
                               || viewInteraction()->isOutgoingDragStarted();
    if (!isDragStarted) {
        // only start drag operations after a minimum of movement:
        const bool canStartDrag = physicalDragDelta.manhattanLength() > 4;
        if (!canStartDrag) {
            return;
        }
    }

    switch (m_mouseDownInfo.dragAction) {
    case MouseDownInfo::DragOutgoingElement: {
        if (!isDragStarted) {
            const EngravingItem* element = viewInteraction()->hitElementContext().element;
            viewInteraction()->startOutgoingDragElement(element, m_view->asItem());
        }
        return;
    }
    case MouseDownInfo::DragOutgoingRange: {
        if (!isDragStarted) {
            viewInteraction()->startOutgoingDragRange(m_view->asItem());
        }
        return;
    }
    case MouseDownInfo::PasteRangeOnRelease: {
        const PointF logicPos = m_view->toLogical(event->pos());

        std::optional<bool> preserveMeasureAlignment;
        if (physicalDragDelta.manhattanLength() > 4) {
            preserveMeasureAlignment = false;
        }

        const bool canDrop = viewInteraction()->updateDropRange(logicPos, preserveMeasureAlignment);
        m_view->asItem()->setCursor(canDrop ? Qt::DragCopyCursor : QCursor());
        return;
    }
    case MouseDownInfo::Nothing:
        return;
    case MouseDownInfo::Other:
        break;
    }

    const Qt::KeyboardModifiers keyState = event->modifiers();

    m_view->hideContextMenu();
    if (!viewInteraction()->isTextEditingStarted()) {
        m_view->hideElementPopup();
    }

    const PointF logicPos = m_view->toLogical(event->pos());

    const bool isNoteEnterMode = m_view->isNoteEnterMode();
    const bool isMiddleButton  = (event->buttons() & Qt::MiddleButton);
    const bool isDragObjectsAllowed = !(isNoteEnterMode || playbackController()->isPlaying() || isMiddleButton);
    if (isDragObjectsAllowed) {
        const EngravingItem* hitElement = hitElementContext().element;

        // drag element
        if ((hitElement && (hitElement->isMovable() || viewInteraction()->isElementEditStarted()))
            || viewInteraction()->isGripEditStarted()) {
            if (hitElement && !viewInteraction()->isDragStarted()) {
                startDragElements(hitElement->type(), hitElement->offset());
            }

            if (viewInteraction()->isGripEditStarted() && !viewInteraction()->isDragStarted()) {
                const EngravingItem* selectedElement = viewInteraction()->selection()->element();
                ElementType type = selectedElement ? selectedElement->type() : ElementType::INVALID;
                startDragElements(type, selectedElement->offset());
            }

            DragMode mode = DragMode::BothXY;
            if (keyState & Qt::ShiftModifier) {
                mode = DragMode::OnlyY;
            } else if (keyState & Qt::ControlModifier) {
                mode = DragMode::OnlyX;
            }

            const std::vector<int> oldPitches = pitchesBeingDragged();
            viewInteraction()->drag(m_mouseDownInfo.logicalBeginPoint, logicPos, mode);

            if (!oldPitches.empty() && oldPitches != pitchesBeingDragged()) {
                playbackController()->playElements(m_notesBeingDragged);
            }

            return;
        } else if (hitElement == nullptr && (keyState & Qt::ShiftModifier)) {
            if (!viewInteraction()->isDragStarted()) {
                viewInteraction()->startDrag(std::vector<EngravingItem*>(), PointF(), [](const EngravingItem*) { return false; });
            }
            viewInteraction()->drag(m_mouseDownInfo.logicalBeginPoint, logicPos, DragMode::BothXY);

            return;
        }
    }

    // move canvas
    if (!isNoteEnterMode || isMiddleButton) {
        const PointF logicalDragDelta = logicPos - m_mouseDownInfo.logicalBeginPoint;
        m_view->moveCanvas(logicalDragDelta.x(), logicalDragDelta.y());

        m_isCanvasDragged = true;
    }
}

void NotationViewInputController::startDragElements(ElementType elementsType, const PointF& elementsOffset)
{
    if (elementsType == ElementType::INVALID) {
        return;
    }

    const std::vector<EngravingItem*>& elements = viewInteraction()->selection()->elements();
    if (elements.empty()) {
        return;
    }

    bool isFilterType = viewInteraction()->selection()->isRange();
    auto isDraggable = [isFilterType, elementsType](const EngravingItem* element) {
        return element && element->selected() && (!isFilterType || elementsType == element->type());
    };

    viewInteraction()->startDrag(elements, elementsOffset, isDraggable);

    if (!viewInteraction()->isDragStarted()) {
        return;
    }

    for (const EngravingItem* item : elements) {
        if (item->isNote()) {
            m_notesBeingDragged.push_back(item);
        }
    }
}

void NotationViewInputController::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_mouseDownInfo.dragAction == MouseDownInfo::Nothing) {
        return;
    }

    INotationInteractionPtr interaction = viewInteraction();
    INotationNoteInputPtr noteInput = interaction->noteInput();
    const EngravingItem* hitElement = hitElementContext().element;

    DEFER {
        m_mouseDownInfo.dragAction = MouseDownInfo::Nothing;
        m_view->asItem()->setCursor(QCursor());
    };

    if (m_mouseDownInfo.dragAction == MouseDownInfo::PasteRangeOnRelease) {
        INotationSelectionPtr selection = interaction->selection();
        if (!selection->isRange() || !selection->canCopy()) {
            viewInteraction()->endDrop();
            return;
        }
        muse::ByteArray rangeData = selection->mimeData();
        if (rangeData.empty()) {
            viewInteraction()->endDrop();
            return;
        }
        PointF logicPos = m_view->toLogical(event->pos());
        viewInteraction()->dropRange(rangeData.toQByteArrayNoCopy(), logicPos, false);
        return;
    }

    if (event->modifiers() != Qt::ControlModifier && event->modifiers() != Qt::ShiftModifier) {
        if (!hitElement && !m_isCanvasDragged && !interaction->isGripEditStarted()
            && !interaction->isDragStarted() && !noteInput->isNoteInputMode()) {
            interaction->clearSelection();
        }
    }

    if (event->button() == Qt::LeftButton && event->modifiers() == Qt::NoModifier) {
        handleLeftClickRelease(event->pos());
    }

    m_isCanvasDragged = false;

    if (interaction->isDragStarted()) {
        bool isDraggingHairpinSegmentGrip
            = interaction->isGripEditStarted()
              && interaction->selection()->element()
              && interaction->selection()->element()->isHairpinSegment();

        interaction->endDrag();

        // When dragging of hairpin ends on a note or rest, open dynamic popup
        // Check for note or rest happens in Score::addText which is called through addTextToItem in toggleDynamicPopup
        if (isDraggingHairpinSegmentGrip) {
            interaction->toggleDynamicPopup();
        }
    }

    if (interaction->isOutgoingDragStarted()) {
        interaction->endOutgoingDrag();
    }

    m_notesBeingDragged.clear();
}

void NotationViewInputController::handleLeftClickRelease(const QPointF& releasePoint)
{
    if (m_view->isNoteEnterMode() || playbackController()->isPlaying()) {
        return;
    }

    if (m_shouldStartEditOnLeftClickRelease) {
        dispatcher()->dispatch("edit-element", ActionData::make_arg1<PointF>(m_mouseDownInfo.logicalBeginPoint));
        m_shouldStartEditOnLeftClickRelease = false;
        return;
    }

    const INotationInteraction::HitElementContext& ctx = hitElementContext();
    if (!ctx.element) {
        return;
    }

    if (releasePoint != m_mouseDownInfo.physicalBeginPoint) {
        return;
    }

    if (m_shouldSelectOnLeftClickRelease) {
        m_shouldSelectOnLeftClickRelease = false;
        engraving::staff_idx_t staffIndex = ctx.staff ? ctx.staff->idx() : muse::nidx;

        INotationInteractionPtr interaction = viewInteraction();
        interaction->select({ ctx.element }, SelectType::SINGLE, staffIndex);

        if (ctx.element->needStartEditingAfterSelecting()) {
            viewInteraction()->startEditElement(ctx.element);
            return;
        }
    }

    if (!m_hitElementWasAlreadySingleSelected) {
        return;
    }

    // Same element clicked again while it was already selected.
    // Either toggle popup or start text editing.

    if (viewInteraction()->textEditingAllowed(ctx.element)) {
        if (!viewInteraction()->isTextEditingStarted()) {
            viewInteraction()->startEditText(ctx.element, m_mouseDownInfo.logicalBeginPoint);
        }
    } else {
        togglePopupForItemIfSupports(ctx.element);
    }
}

void NotationViewInputController::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (m_view->isNoteEnterMode()) {
        return;
    }

    // Can't pass `this` as the second argument, because not QObject. But `m_view` works as well.
    QTimer::singleShot(QApplication::doubleClickInterval(), m_view->asItem(), [this]() {
        m_tripleClickPending = false;
    });

    m_tripleClickPending = true;

    if (viewInteraction()->isTextEditingStarted()) {
        viewInteraction()->selectText(mu::engraving::SelectTextType::Word);
        return;
    }

    const INotationInteraction::HitElementContext& ctx = hitElementContext();
    if (!ctx.element) {
        return;
    }

    if (viewInteraction()->textEditingAllowed(ctx.element)) {
        viewInteraction()->startEditText(ctx.element, m_mouseDownInfo.logicalBeginPoint);
        return;
    }

    if (ctx.element->isMeasure() && event->modifiers() == Qt::NoModifier) {
        dispatcher()->dispatch("note-input", ActionData::make_arg1<PointF>(m_mouseDownInfo.logicalBeginPoint));
        return;
    }

    if (ctx.element->isInstrumentName()) {
        m_shouldStartEditOnLeftClickRelease = true;
    }
}

void NotationViewInputController::hoverMoveEvent(QHoverEvent* event)
{
    if (!m_view->isNoteEnterMode()) {
        return;
    }

    const PointF oldPos = m_view->toLogical(event->oldPosF());
    PointF pos = m_view->toLogical(event->position());

    const ShadowNote* shadowNote = viewInteraction()->shadowNote();
    if (shadowNote && m_view->elementPopupIsOpen(shadowNote->type())) {
        // Lock the X position to the shadow note X (prevents the popup from jumping horizontally to other input positions)
        pos.setX(shadowNote->canvasX());
    }

    if (oldPos == pos) {
        return;
    }

    m_view->showShadowNote(pos);

    if (event->modifiers() == Qt::ShiftModifier) {
        updateShadowNotePopupVisibility();
    }
}

bool NotationViewInputController::isAnchorEditingEvent(QKeyEvent* event) const
{
    bool anchorEditingKeyCombo = (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right) && event->modifiers() & Qt::ShiftModifier;
    EngravingItem* selectedItem = viewInteraction()->selection()->element();
    return selectedItem && selectedItem->allowTimeAnchor() && anchorEditingKeyCombo;
}

bool NotationViewInputController::shortcutOverrideEvent(QKeyEvent* event)
{
    auto key = event->key();

    const bool editTextKeysFound = key == Qt::Key_Return || key == Qt::Key_Enter;
    if (editTextKeysFound && startTextEditingAllowed()) {
        return true;
    }

    if (key == Qt::Key_Escape && m_mouseDownInfo.dragAction == MouseDownInfo::PasteRangeOnRelease) {
        return true;
    }

    if (viewInteraction()->isElementEditStarted()) {
        return viewInteraction()->isEditAllowed(event);
    }

    if (isAnchorEditingEvent(event)) {
        return true;
    }

    return tryPercussionShortcut(event);
}

void NotationViewInputController::keyPressEvent(QKeyEvent* event)
{
    auto key = event->key();

    if (startTextEditingAllowed() && (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)) {
        dispatcher()->dispatch("edit-text");
        event->accept();
    } else if (event->key() == Qt::Key_Escape && m_mouseDownInfo.dragAction == MouseDownInfo::PasteRangeOnRelease) {
        // Cancel "Alt+click to paste range"
        viewInteraction()->endDrop();
        m_mouseDownInfo.dragAction = MouseDownInfo::Nothing;
        m_view->asItem()->setCursor({});
        event->accept();
    } else if (viewInteraction()->isElementEditStarted()) {
        viewInteraction()->editElement(event);
        if (key == Qt::Key_Shift) {
            viewInteraction()->updateTimeTickAnchors(event);
        }
    } else if (key == Qt::Key_Shift) {
        viewInteraction()->updateTimeTickAnchors(event);
        updateShadowNotePopupVisibility();
        return;
    } else if (isAnchorEditingEvent(event)) {
        viewInteraction()->moveElementAnchors(event);
    }

    updateShadowNotePopupVisibility(/*forceHide*/ true);
}

void NotationViewInputController::keyReleaseEvent(QKeyEvent* event)
{
    if (event->key() != Qt::Key_Shift) {
        return;
    }

    viewInteraction()->editElement(event);
    updateShadowNotePopupVisibility(/*forceHide*/ true);
    viewInteraction()->updateTimeTickAnchors(event);
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
    const QMimeData* mimeData = dragController()->mimeData(event);
    IF_ASSERT_FAILED(mimeData) {
        return;
    }

    if (mimeData->hasFormat(mu::commonscene::MIME_SYMBOL_FORMAT)) {
        event->setDropAction(Qt::CopyAction);
        if (event->dropAction() != Qt::CopyAction) {
            event->ignore();
            return;
        }

        QByteArray edata = mimeData->data(MIME_SYMBOL_FORMAT);
        if (viewInteraction()->startDropSingle(edata)) {
            event->accept();
            return;
        }

        event->ignore();
        return;
    }

    if (mimeData->hasFormat(mu::commonscene::MIME_STAFFLLIST_FORMAT)) {
        bool isInternal = event->source() == m_view->asItem();
        if (!isInternal || event->modifiers() & Qt::AltModifier) {
            event->setDropAction(Qt::CopyAction);
            if (event->dropAction() != Qt::CopyAction) {
                event->ignore();
                return;
            }
        } else {
            event->setDropAction(Qt::MoveAction);
            if (event->dropAction() != Qt::MoveAction) {
                event->ignore();
                return;
            }
        }

        QByteArray edata = mimeData->data(MIME_STAFFLLIST_FORMAT);
        if (viewInteraction()->startDropRange(edata)) {
            event->accept();
            return;
        }

        event->ignore();
        return;
    }

    for (const QUrl& url : mimeData->urls()) {
        if (viewInteraction()->startDropImage(url)) {
            event->accept();
            return;
        }
    }

    event->ignore();
}

void NotationViewInputController::dragMoveEvent(QDragMoveEvent* event)
{
    const QMimeData* mimeData = dragController()->mimeData(event);
    IF_ASSERT_FAILED(mimeData) {
        return;
    }

    PointF pos = m_view->toLogical(event->position());
    Qt::KeyboardModifiers modifiers = event->modifiers();

    bool isAccepted = false;
    if (mimeData->hasFormat(MIME_STAFFLLIST_FORMAT)) {
        bool isInternal = event->source() == m_view->asItem();
        if (!isInternal || modifiers & Qt::AltModifier) {
            event->setDropAction(Qt::CopyAction);
        } else {
            event->setDropAction(Qt::MoveAction);
        }

        isAccepted = viewInteraction()->updateDropRange(pos);
    } else {
        event->setDropAction(Qt::CopyAction);

        isAccepted = viewInteraction()->updateDropSingle(pos, modifiers);
    }

    event->setAccepted(isAccepted);

    if (!uiConfiguration()->isSystemDragSupported()) {
        m_lastDragMoveEvent = {
            event->position(),
            event->modifiers(),
            event->dropAction(),
            event->source()
        };
    }
}

void NotationViewInputController::dragLeaveEvent(QDragLeaveEvent*)
{
    if (!uiConfiguration()->isSystemDragSupported()) {
        dropEvent(m_lastDragMoveEvent);
    }

    viewInteraction()->endDrop();
    m_mouseDownInfo.dragAction = MouseDownInfo::Nothing;
}

void NotationViewInputController::dropEvent(QDropEvent* event)
{
    if (!uiConfiguration()->isSystemDragSupported()) {
        event->setAccepted(false);
        return;
    }

    DragMoveEvent de = {
        event->position(),
        event->modifiers(),
        event->dropAction(),
        event->source()
    };

    bool isAccepted = dropEvent(de, event->mimeData());
    event->setAccepted(isAccepted);
}

bool NotationViewInputController::dropEvent(const DragMoveEvent& event, const QMimeData* mimeData)
{
    if (!mimeData) {
        mimeData = dragController()->mimeData();
    }

    IF_ASSERT_FAILED(mimeData) {
        return false;
    }

    PointF pos = m_view->toLogical(event.position);
    Qt::KeyboardModifiers modifiers = event.modifiers;

    bool isAccepted = false;
    if (mimeData->hasFormat(MIME_STAFFLLIST_FORMAT)) {
        bool isInternal = event.source == m_view->asItem();
        bool isMove = isInternal && event.dropAction == Qt::MoveAction;
        isAccepted = viewInteraction()->dropRange(mimeData->data(MIME_STAFFLLIST_FORMAT), pos, isMove);
    } else {
        isAccepted = viewInteraction()->dropSingle(pos, modifiers);
    }

    return isAccepted;
}

std::vector<int> NotationViewInputController::pitchesBeingDragged() const
{
    std::vector<int> pitches;
    pitches.reserve(m_notesBeingDragged.size());

    for (const EngravingItem* item : m_notesBeingDragged) {
        pitches.push_back(toNote(item)->pitch());
    }

    return pitches;
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

    if (auto hitElement = hitElementContext().element) {
        return hitElement->type();
    } else if (selection->isRange()) {
        return ElementType::MEASURE;
    } else if (auto selectedElement = selection->element()) {
        return selectedElement->type();
    }

    return ElementType::PAGE;
}

muse::PointF NotationViewInputController::selectionElementPos() const
{
    auto selection = viewInteraction()->selection();
    if (!selection) {
        return muse::PointF();
    }

    if (auto hitElement = hitElementContext().element) {
        return hitElement->canvasBoundingRect().center();
    } else if (selection->isRange()) {
        auto range = selection->range();
        return range->boundingArea().front().center();
    } else if (auto selectedElement = selection->element()) {
        return selectedElement->canvasBoundingRect().center();
    }

    return muse::PointF();
}

void NotationViewInputController::togglePopupForItemIfSupports(const EngravingItem* item)
{
    if (!item) {
        return;
    }

    ElementType type = item->type();

    if (AbstractElementPopupModel::hasElementEditPopup(item)) {
        m_view->toggleElementPopup(type);
    }
}

void NotationViewInputController::updateShadowNotePopupVisibility(bool forceHide)
{
    const mu::engraving::ShadowNote* shadowNote = viewInteraction()->shadowNote();
    if (forceHide || !shadowNote || !AbstractElementPopupModel::hasElementEditPopup(shadowNote)) {
        m_view->hideElementPopup(ElementType::SHADOW_NOTE);
        return;
    }

    m_view->showElementPopup(ElementType::SHADOW_NOTE);
}

EngravingItem* NotationViewInputController::resolveStartPlayableElement() const
{
    if (playbackController()->isPlaying()) {
        EngravingItem* hitElement = hitElementContext().element;
        return seekAllowed(hitElement) ? hitElement : nullptr;
    }

    const INotationSelectionPtr selection = viewInteraction()->selection();
    const std::vector<EngravingItem*>& elements = selection->elements();

    if (!selection->isRange() && !elements.empty()) {
        return elements.back();
    }

    EngravingItem* playbackStartElement = hitElementContext().element;

    for (EngravingItem* element: elements) {
        if (!element || element == playbackStartElement) {
            continue;
        }

        if (!seekAllowed(element)) {
            continue;
        }

        if (playbackStartElement && playbackStartElement->tick() <= element->tick()) {
            continue;
        }

        playbackStartElement = element;
    }

    return playbackStartElement;
}
