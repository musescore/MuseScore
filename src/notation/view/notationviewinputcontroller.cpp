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

    static const ElementTypeSet playableTypes = {
        ElementType::NOTE,
        ElementType::REST,
        ElementType::MMREST,
        ElementType::MEASURE,
        ElementType::BAR_LINE
    };

    return muse::contains(playableTypes, element->type());
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

        if (AbstractElementPopupModel::supportsPopup(selectedItem)) {
            m_view->showElementPopup(type, selectedItem->canvasBoundingRect());
        }
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
    PointF logicPos = PointF(m_view->toLogical(event->pos()));
    Qt::KeyboardModifiers keyState = event->modifiers();
    Qt::MouseButton button = event->button();

    m_mouseDownInfo = {
        /*.dragAction =*/ MouseDownInfo::Other,
        /*.physicalBeginPoint =*/ event->pos(),
        /*.logicalBeginPoint =*/ logicPos
    };

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
        m_prevHitElement = hitElementContext().element;

        INotationInteraction::HitElementContext context;
        context.element = viewInteraction()->hitElement(logicPos, hitWidth());
        context.staff = viewInteraction()->hitStaff(logicPos);
        viewInteraction()->setHitElementContext(context);

        hitElement = context.element;
        hitStaffIndex = context.staff ? context.staff->idx() : muse::nidx;
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

    if (m_tripleClickPending) {
        if (viewInteraction()->isTextEditingStarted()) {
            viewInteraction()->selectText(mu::engraving::SelectTextType::All);
            return;
        }
    }

    if (playbackController()->isPlaying()) {
        return;
    }

    if (keyState == (Qt::ShiftModifier | Qt::ControlModifier)) {
        m_mouseDownInfo.dragAction = viewInteraction()->isOutgoingDragElementAllowed(hitElement)
                                     ? MouseDownInfo::DragOutgoingElement
                                     : MouseDownInfo::Nothing;
        return;
    }

    if (button == Qt::LeftButton && (!hitElement || hitElement->isMeasure())) {
        INotationSelectionPtr selection = viewInteraction()->selection();

        if (selection->isRange() && selection->range()->containsPoint(logicPos)) {
            m_mouseDownInfo.dragAction = MouseDownInfo::DragOutgoingRange;
            return;
        }
    }

    ClickContext ctx;
    ctx.logicClickPos = logicPos;
    ctx.hitElement = hitElement;
    ctx.hitStaff = hitStaffIndex;
    ctx.isHitGrip = viewInteraction()->isHitGrip(logicPos);
    ctx.event = event;

    m_shouldTogglePopupOnLeftClickRelease = hitElement && hitElement->selected();

    if (needSelect(ctx)) {
        SelectType selectType = SelectType::SINGLE;
        if (keyState == Qt::NoModifier) {
            selectType = SelectType::SINGLE;
        } else if (keyState & Qt::ShiftModifier) {
            selectType = SelectType::RANGE;
        } else if (keyState & Qt::ControlModifier) {
            selectType = SelectType::ADD;
        }

        std::vector<EngravingItem*> hitElements = viewInteraction()->hitElements(ctx.logicClickPos, hitWidth());
        size_t numHitElements = hitElements.size();

        // overlapping elements with ctrl modifier
        if (numHitElements > 1 && (keyState & Qt::ControlModifier)) {
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
        } else {
            viewInteraction()->select({ hitElement }, selectType, hitStaffIndex);
        }
    }

    if (button == Qt::LeftButton) {
        handleLeftClick(ctx);
    } else if (button == Qt::RightButton) {
        handleRightClick(ctx);
    }
}

bool NotationViewInputController::needSelect(const ClickContext& ctx) const
{
    if (ctx.isHitGrip && ctx.event->button() == Qt::LeftButton) {
        return false;
    }

    if (!ctx.hitElement) {
        return false;
    }

    INotationSelectionPtr selection = viewInteraction()->selection();

    if (ctx.event->button() == Qt::LeftButton && ctx.event->modifiers() & Qt::ControlModifier) {
        return true;
    } else if (ctx.event->button() == Qt::LeftButton && ctx.event->modifiers() & Qt::ShiftModifier) {
        return !selection->isRange() || !selection->range()->containsItem(ctx.hitElement, ctx.hitStaff);
    } else if (ctx.event->button() == Qt::RightButton && selection->isRange()) {
        return !selection->range()->containsItem(ctx.hitElement, ctx.hitStaff);
    } else if (!ctx.hitElement->selected()) {
        return true;
    }

    return false;
}

void NotationViewInputController::handleLeftClick(const ClickContext& ctx)
{
    if (ctx.isHitGrip) {
        viewInteraction()->startEditGrip(ctx.logicClickPos);
        return;
    } else {
        viewInteraction()->endEditGrip();
    }

    INotationSelectionPtr selection = viewInteraction()->selection();

    if (!selection->isRange() && ctx.hitElement && ctx.hitElement->needStartEditingAfterSelecting()) {
        if (ctx.hitElement->hasGrips() && !ctx.hitElement->isImage() && selection->elements().size() == 1) {
            viewInteraction()->startEditGrip(ctx.hitElement, ctx.hitElement->defaultGrip());
        } else {
            viewInteraction()->startEditElement(ctx.hitElement, false);
        }
    }

    if (!ctx.hitElement) {
        viewInteraction()->endEditElement();
        return;
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
    m_view->showContextMenu(selectionType(), ctx.event->pos());

    if (!ctx.hitElement) {
        viewInteraction()->endEditElement();
        return;
    }
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

    QPointF physicalDragDelta = event->pos() - m_mouseDownInfo.physicalBeginPoint;

    bool isDragStarted = m_isCanvasDragged
                         || viewInteraction()->isDragStarted()
                         || viewInteraction()->isOutgoingDragStarted();
    if (!isDragStarted) {
        // only start drag operations after a minimum of movement:
        bool canStartDrag = physicalDragDelta.manhattanLength() > 4;
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
    case MouseDownInfo::Nothing:
        return;
    case MouseDownInfo::Other:
        break;
    }

    Qt::KeyboardModifiers keyState = event->modifiers();

    m_view->hideContextMenu();
    m_view->hideElementPopup();

    PointF logicPos = m_view->toLogical(event->pos());

    bool isNoteEnterMode = m_view->isNoteEnterMode();
    bool isMiddleButton  = (event->buttons() & Qt::MiddleButton);
    bool isDragObjectsAllowed = !(isNoteEnterMode || playbackController()->isPlaying() || isMiddleButton);
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

            viewInteraction()->drag(m_mouseDownInfo.logicalBeginPoint, logicPos, mode);

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
        PointF logicalDragDelta = logicPos - m_mouseDownInfo.logicalBeginPoint;
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
}

void NotationViewInputController::mouseReleaseEvent(QMouseEvent* event)
{
    INotationInteractionPtr interaction = viewInteraction();
    INotationNoteInputPtr noteInput = interaction->noteInput();
    const EngravingItem* hitElement = hitElementContext().element;

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

    engraving::staff_idx_t staffIndex = ctx.staff ? ctx.staff->idx() : muse::nidx;

    INotationInteractionPtr interaction = viewInteraction();
    interaction->select({ ctx.element }, SelectType::SINGLE, staffIndex);

    if (ctx.element && ctx.element->needStartEditingAfterSelecting()) {
        viewInteraction()->startEditElement(ctx.element, /*editTextualProperties*/ false);
        return;
    }

    if (m_shouldTogglePopupOnLeftClickRelease) {
        togglePopupForItemIfSupports(ctx.element);
    }

    if (ctx.element != m_prevHitElement) {
        return;
    }

    if (interaction->isTextEditingStarted()) {
        return;
    }

    if (interaction->textEditingAllowed(ctx.element)) {
        interaction->startEditText(ctx.element, m_mouseDownInfo.logicalBeginPoint);
    }
}

void NotationViewInputController::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (m_view->isNoteEnterMode()) {
        return;
    }

    QTimer::singleShot(QApplication::doubleClickInterval(), [this]() {
        m_tripleClickPending = false;
    });

    m_tripleClickPending = true;

    const INotationInteraction::HitElementContext& ctx = hitElementContext();
    if (viewInteraction()->isTextEditingStarted()) {
        viewInteraction()->selectText(mu::engraving::SelectTextType::Word);
        return;
    } else if (viewInteraction()->textEditingAllowed(ctx.element)) {
        viewInteraction()->startEditText(ctx.element, m_mouseDownInfo.logicalBeginPoint);
    }

    PointF logicPos = m_view->toLogical(event->pos());
    const EngravingItem* hitElement = viewInteraction()->hitElement(logicPos, hitWidth());

    if (!hitElement) {
        return;
    }

    if (hitElement->isMeasure() && event->modifiers() == Qt::NoModifier) {
        dispatcher()->dispatch("note-input", ActionData::make_arg1<PointF>(m_mouseDownInfo.logicalBeginPoint));
    } else if (hitElement->isInstrumentName()) {
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

bool NotationViewInputController::shortcutOverrideEvent(QKeyEvent* event)
{
    const bool editTextKeysFound = event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter;
    if (editTextKeysFound && startTextEditingAllowed()) {
        return true;
    }

    if (viewInteraction()->isElementEditStarted()) {
        return viewInteraction()->isEditAllowed(event);
    }

    return tryPercussionShortcut(event);
}

void NotationViewInputController::keyPressEvent(QKeyEvent* event)
{
    if (startTextEditingAllowed() && (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)) {
        dispatcher()->dispatch("edit-text");
        event->accept();
    } else if (viewInteraction()->isElementEditStarted()) {
        viewInteraction()->editElement(event);
    } else if (event->key() == Qt::Key_Shift) {
        updateShadowNotePopupVisibility();
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
    const QMimeData* mimeData = event->mimeData();
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
}

void NotationViewInputController::dragLeaveEvent(QDragLeaveEvent*)
{
    viewInteraction()->endDrop();
}

void NotationViewInputController::dropEvent(QDropEvent* event)
{
    const QMimeData* mimeData = event->mimeData();
    IF_ASSERT_FAILED(mimeData) {
        return;
    }

    PointF pos = m_view->toLogical(event->position());
    Qt::KeyboardModifiers modifiers = event->modifiers();

    bool isAccepted = false;
    if (mimeData->hasFormat(MIME_STAFFLLIST_FORMAT)) {
        bool isInternal = event->source() == m_view->asItem();
        bool isMove = isInternal && event->dropAction() == Qt::MoveAction;
        isAccepted = viewInteraction()->dropRange(mimeData->data(MIME_STAFFLLIST_FORMAT), pos, isMove);
    } else {
        isAccepted = viewInteraction()->dropSingle(pos, modifiers);
    }

    event->setAccepted(isAccepted);
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

    if (AbstractElementPopupModel::supportsPopup(item)) {
        m_view->toggleElementPopup(type, item->canvasBoundingRect());
    }
}

void NotationViewInputController::updateShadowNotePopupVisibility(bool forceHide)
{
    const mu::engraving::ShadowNote* shadowNote = viewInteraction()->shadowNote();
    if (forceHide || !shadowNote || !AbstractElementPopupModel::supportsPopup(shadowNote)) {
        m_view->hideElementPopup(ElementType::SHADOW_NOTE);
        return;
    }

    RectF noteHeadRect = shadowNote->symBbox(shadowNote->noteheadSymbol());
    noteHeadRect.translate(shadowNote->canvasPos().x(), shadowNote->canvasPos().y());
    m_view->showElementPopup(ElementType::SHADOW_NOTE, noteHeadRect);
}

EngravingItem* NotationViewInputController::resolveStartPlayableElement() const
{
    EngravingItem* hitElement = hitElementContext().element;

    if (playbackController()->isPlaying()) {
        return seekAllowed(hitElement) ? hitElement : nullptr;
    }

    INotationSelectionPtr selection = viewInteraction()->selection();
    if (!selection->isRange()) {
        return hitElement;
    }

    EngravingItem* playbackStartElement = hitElement;

    for (EngravingItem* element: selection->elements()) {
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
