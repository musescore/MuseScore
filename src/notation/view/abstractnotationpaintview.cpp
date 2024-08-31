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
#include "abstractnotationpaintview.h"

#include <QPainter>

#include "actions/actiontypes.h"

#include "log.h"

using namespace mu;
using namespace mu::notation;
using namespace muse;
using namespace muse::ui;
using namespace muse::draw;
using namespace muse::actions;

static constexpr qreal SCROLL_LIMIT_OFF_OVERSCROLL_FACTOR = 0.75;

static void compensateFloatPart(RectF& rect)
{
    rect.adjust(-1, -1, 1, 1);
}

AbstractNotationPaintView::AbstractNotationPaintView(QQuickItem* parent)
    : muse::uicomponents::QuickPaintedView(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
    setFlag(ItemHasContents, true);
    setFlag(ItemAcceptsDrops, true);
    setAcceptedMouseButtons(Qt::AllButtons);

    connect(this, &QQuickPaintedItem::widthChanged, this, &AbstractNotationPaintView::onViewSizeChanged);
    connect(this, &QQuickPaintedItem::heightChanged, this, &AbstractNotationPaintView::onViewSizeChanged);

    connect(this, &AbstractNotationPaintView::horizontalScrollChanged, [this]() {
        m_previousHorizontalScrollPosition = startHorizontalScrollPosition();
    });

    connect(this, &AbstractNotationPaintView::verticalScrollChanged, [this]() {
        m_previousVerticalScrollPosition = startVerticalScrollPosition();
    });

    m_enableAutoScrollTimer.setSingleShot(true);
    connect(&m_enableAutoScrollTimer, &QTimer::timeout, this, [this]() {
        m_autoScrollEnabled = true;
    });
}

AbstractNotationPaintView::~AbstractNotationPaintView()
{
    if (m_notation && isMainView()) {
        m_notation->accessibility()->setMapToScreenFunc(nullptr);
        m_notation->interaction()->setGetViewRectFunc(nullptr);
    }

    clear();
}

void AbstractNotationPaintView::load()
{
    TRACEFUNC;

    m_inputController = std::make_unique<NotationViewInputController>(this, iocContext());
    m_playbackCursor = std::make_unique<PlaybackCursor>(iocContext());
    m_playbackCursor->setVisible(false);
    m_noteInputCursor = std::make_unique<NoteInputCursor>();

    m_loopInMarker = std::make_unique<LoopMarker>(LoopBoundaryType::LoopIn, iocContext());
    m_loopOutMarker = std::make_unique<LoopMarker>(LoopBoundaryType::LoopOut, iocContext());

    m_continuousPanel = std::make_unique<ContinuousPanel>(iocContext());

    //! NOTE For diagnostic tools
    if (!dispatcher()->isReg(this)) {
        dispatcher()->reg(this, "diagnostic-notationview-redraw", [this]() {
            scheduleRedraw();
        });
    }

    m_inputController->setReadonly(m_readonly);
    m_inputController->init();

    onNotationSetup();

    initBackground();
    initNavigatorOrientation();

    configuration()->isLimitCanvasScrollAreaChanged().onNotify(this, [this]() {
        ensureViewportInsideScrollableArea();

        emit horizontalScrollChanged();
        emit verticalScrollChanged();
        emit viewportChanged();
    });

    scheduleRedraw();
}

void AbstractNotationPaintView::initBackground()
{
    emit backgroundColorChanged(configuration()->backgroundColor());

    configuration()->backgroundChanged().onNotify(this, [this]() {
        emit backgroundColorChanged(configuration()->backgroundColor());
        scheduleRedraw();
    });
}

void AbstractNotationPaintView::initNavigatorOrientation()
{
    configuration()->canvasOrientation().ch.onReceive(this, [this](muse::Orientation) {
        moveCanvasToPosition(PointF(0, 0));
    });
}

void AbstractNotationPaintView::moveCanvasToCenter()
{
    TRACEFUNC;

    if (!isInited()) {
        return;
    }

    PointF canvasCenter = this->canvasCenter();
    Transform oldMatrix = m_matrix;

    if (doMoveCanvas(canvasCenter.x(), canvasCenter.y())) {
        onMatrixChanged(oldMatrix, m_matrix, false);
    }
}

void AbstractNotationPaintView::scrollHorizontal(qreal position)
{
    TRACEFUNC;

    qreal scrollStep = position - m_previousHorizontalScrollPosition;
    if (qFuzzyIsNull(scrollStep)) {
        return;
    }

    qreal dx = horizontalScrollableSize() * scrollStep;
    moveCanvasHorizontal(-dx);
}

void AbstractNotationPaintView::scrollVertical(qreal position)
{
    TRACEFUNC;

    qreal scrollStep = position - m_previousVerticalScrollPosition;
    if (qFuzzyIsNull(scrollStep)) {
        return;
    }

    qreal dy = verticalScrollableSize() * scrollStep;
    moveCanvasVertical(-dy);
}

void AbstractNotationPaintView::zoomIn()
{
    m_inputController->zoomIn();
}

void AbstractNotationPaintView::zoomOut()
{
    m_inputController->zoomOut();
}

void AbstractNotationPaintView::selectOnNavigationActive()
{
    TRACEFUNC;

    if (!notation()) {
        return;
    }

    auto interaction = notation()->interaction();
    if (!interaction->selection()->isNone()) {
        return;
    }

    interaction->selectFirstElement(false);
}

bool AbstractNotationPaintView::canReceiveAction(const ActionCode& actionCode) const
{
    if (actionCode == "diagnostic-notationview-redraw") {
        return true;
    }

    return hasFocus();
}

void AbstractNotationPaintView::onCurrentNotationChanged()
{
    TRACEFUNC;

    if (m_notation) {
        onUnloadNotation(m_notation);
    }

    setNotation(globalContext()->currentNotation());

    if (!m_notation) {
        return;
    }

    onLoadNotation(m_notation);
}

void AbstractNotationPaintView::onLoadNotation(INotationPtr)
{
    if (viewport().isValid() && !m_notation->viewState()->isMatrixInited()) {
        m_inputController->initZoom();
    }

    if (publishMode()) {
        m_notation->painting()->setViewMode(ViewMode::PAGE);
    } else {
        m_notation->painting()->setViewMode(m_notation->viewState()->viewMode());
    }

    INotationInteractionPtr interaction = notationInteraction();

    m_notation->notationChanged().onNotify(this, [this, interaction]() {
        interaction->hideShadowNote();
        m_shadowNoteRect = RectF();
        scheduleRedraw();
    });

    onNoteInputStateChanged();
    interaction->noteInput()->stateChanged().onNotify(this, [this]() {
        onNoteInputStateChanged();
    });

    interaction->selectionChanged().onNotify(this, [this]() {
        scheduleRedraw();
    });

    interaction->showItemRequested().onReceive(this, [this](const INotationInteraction::ShowItemRequest& request) {
        onShowItemRequested(request);
    });

    interaction->textEditingStarted().onNotify(this, [this]() {
        setFlag(ItemAcceptsInputMethod, true);
        setFocus(false); // Remove focus once so that the IME reloads the state
        forceFocusIn();
    });

    interaction->textEditingEnded().onReceive(this, [this](const engraving::TextBase*) {
        setFlag(ItemAcceptsInputMethod, false);
        setFocus(false); // Remove focus once so that the IME reloads the state
        forceFocusIn();
    });

    interaction->dropChanged().onNotify(this, [this]() {
        if (!hasActiveFocus()) {
            forceFocusIn(); // grab keyboard focus after element added from palette
        }
    });

    updateLoopMarkers();
    notationPlayback()->loopBoundariesChanged().onNotify(this, [this]() {
        updateLoopMarkers();
    });

    m_notation->viewModeChanged().onNotify(this, [this]() {
        updateLoopMarkers();
        ensureViewportInsideScrollableArea();
    });

    if (isMainView()) {
        connect(this, &QQuickPaintedItem::focusChanged, this, [this](bool focused) {
            if (notation()) {
                notation()->accessibility()->setEnabled(focused);
            }
        });

        notation()->accessibility()->setMapToScreenFunc([this](const RectF& elementRect) {
            if (elementRect.isEmpty()) {
                return RectF(PointF::fromQPointF(mapToGlobal({ 0, 0 })), SizeF(width(), height()));
            }

            auto res = fromLogical(elementRect);
            res = RectF(PointF::fromQPointF(mapToGlobal(res.topLeft().toQPointF())), SizeF(res.width(), res.height()));

            return res;
        });

        notation()->interaction()->setGetViewRectFunc([this]() {
            return viewport();
        });
    }

    forceFocusIn();
    scheduleRedraw();

    emit horizontalScrollChanged();
    emit verticalScrollChanged();
    emit viewportChanged();
}

void AbstractNotationPaintView::onUnloadNotation(INotationPtr)
{
    m_notation->notationChanged().resetOnNotify(this);
    INotationInteractionPtr interaction = m_notation->interaction();
    interaction->noteInput()->stateChanged().resetOnNotify(this);
    interaction->selectionChanged().resetOnNotify(this);

    if (isMainView()) {
        m_notation->accessibility()->setMapToScreenFunc(nullptr);
        m_notation->interaction()->setGetViewRectFunc(nullptr);
    }
}

void AbstractNotationPaintView::setMatrix(const Transform& matrix)
{
    if (m_matrix == matrix) {
        return;
    }

    Transform oldMatrix = m_matrix;
    m_matrix = matrix;

    // If `ensureViewportInsideScrollableArea` returns true, it has already
    // notified about matrix changed, so no need to do it again.
    if (ensureViewportInsideScrollableArea()) {
        return;
    }

    onMatrixChanged(oldMatrix, m_matrix, false);
}

void AbstractNotationPaintView::onMatrixChanged(const Transform& oldMatrix, const Transform& newMatrix, bool overrideZoomType)
{
    UNUSED(overrideZoomType);

    Transform oldMatrixInverted = oldMatrix.inverted();

    if (m_shadowNoteRect.isValid()) {
        RectF logicRect = oldMatrixInverted.map(m_shadowNoteRect);
        m_shadowNoteRect = newMatrix.map(logicRect);
    }

    scheduleRedraw();

    emit horizontalScrollChanged();
    emit verticalScrollChanged();
    emit viewportChanged();
}

void AbstractNotationPaintView::onViewSizeChanged()
{
    TRACEFUNC;

    if (!notation()) {
        return;
    }

    if (viewport().isValid()) {
        if (!notation()->viewState()->isMatrixInited()) {
            m_inputController->initZoom();
        } else {
            m_inputController->updateZoomAfterSizeChange();
        }
    }

    ensureViewportInsideScrollableArea();

    scheduleRedraw();

    emit horizontalScrollChanged();
    emit verticalScrollChanged();
    emit viewportChanged();
}

void AbstractNotationPaintView::updateLoopMarkers()
{
    TRACEFUNC;

    const LoopBoundaries& loop = notationPlayback()->loopBoundaries();

    m_loopInMarker->move(loop.loopInTick);
    m_loopOutMarker->move(loop.loopOutTick);

    m_loopInMarker->setVisible(loop.enabled);
    m_loopOutMarker->setVisible(loop.enabled);

    scheduleRedraw();
}

INotationPtr AbstractNotationPaintView::notation() const
{
    return m_notation;
}

INotationInteractionPtr AbstractNotationPaintView::notationInteraction() const
{
    return notation() ? notation()->interaction() : nullptr;
}

INotationPlaybackPtr AbstractNotationPaintView::notationPlayback() const
{
    return globalContext()->currentMasterNotation() ? globalContext()->currentMasterNotation()->playback() : nullptr;
}

QQuickItem* AbstractNotationPaintView::asItem()
{
    return this;
}

INotationNoteInputPtr AbstractNotationPaintView::notationNoteInput() const
{
    return notationInteraction() ? notationInteraction()->noteInput() : nullptr;
}

INotationElementsPtr AbstractNotationPaintView::notationElements() const
{
    return notation() ? notation()->elements() : nullptr;
}

INotationStylePtr AbstractNotationPaintView::notationStyle() const
{
    return notation() ? notation()->style() : nullptr;
}

INotationSelectionPtr AbstractNotationPaintView::notationSelection() const
{
    return notationInteraction() ? notationInteraction()->selection() : nullptr;
}

void AbstractNotationPaintView::onNoteInputStateChanged()
{
    TRACEFUNC;

    bool noteEnterMode = isNoteEnterMode();
    setAcceptHoverEvents(noteEnterMode);

    if (noteEnterMode) {
        emit activeFocusRequested();
    }

    if (INotationInteractionPtr interaction = notationInteraction()) {
        interaction->hideShadowNote();
        m_shadowNoteRect = RectF();
        scheduleRedraw();
    }
}

void AbstractNotationPaintView::onShowItemRequested(const INotationInteraction::ShowItemRequest& request)
{
    IF_ASSERT_FAILED(request.item) {
        return;
    }

    RectF viewRect = viewport();

    RectF showRect = request.showRect;
    RectF itemBoundingRect = request.item->canvasBoundingRect();

    if (viewRect.width() < showRect.width()) {
        showRect.setLeft(itemBoundingRect.x());
        showRect.setWidth(itemBoundingRect.width());
    }

    if (viewRect.height() < showRect.height()) {
        showRect.setTop(itemBoundingRect.y());
        showRect.setHeight(itemBoundingRect.height());
    }

    adjustCanvasPosition(showRect);
}

bool AbstractNotationPaintView::isNoteEnterMode() const
{
    return notationNoteInput() ? notationNoteInput()->isNoteInputMode() : false;
}

void AbstractNotationPaintView::showShadowNote(const PointF& pos)
{
    TRACEFUNC;

    bool visible = notationInteraction()->showShadowNote(pos);

    if (m_shadowNoteRect.isValid()) {
        scheduleRedraw(m_shadowNoteRect);

        if (!visible) {
            m_shadowNoteRect = RectF();
            return;
        }
    }

    RectF shadowNoteRect = fromLogical(notationInteraction()->shadowNoteRect());

    if (shadowNoteRect.isValid()) {
        compensateFloatPart(shadowNoteRect);
        scheduleRedraw(shadowNoteRect);
    }

    m_shadowNoteRect = shadowNoteRect;
}

void AbstractNotationPaintView::showContextMenu(const ElementType& elementType, const QPointF& pos)
{
    TRACEFUNC;

    QPointF _pos = pos;
    if (_pos.isNull()) {
        _pos = QPointF(width() / 2, height() / 2);
    }

    emit showContextMenuRequested(static_cast<int>(elementType), pos);
}

void AbstractNotationPaintView::hideContextMenu()
{
    TRACEFUNC;

    if (m_isContextMenuOpen) {
        emit hideContextMenuRequested();
    }
}

void AbstractNotationPaintView::showElementPopup(const ElementType& elementType, const RectF& elementRect)
{
    TRACEFUNC;

    PopupModelType modelType = AbstractElementPopupModel::modelTypeFromElement(elementType);

    emit showElementPopupRequested(modelType, fromLogical(elementRect).toQRectF());
}

void AbstractNotationPaintView::hideElementPopup()
{
    TRACEFUNC;

    if (m_isPopupOpen) {
        emit hideElementPopupRequested();
    }
}

void AbstractNotationPaintView::toggleElementPopup(const ElementType& elementType, const RectF& elementRect)
{
    if (m_isPopupOpen) {
        hideElementPopup();
        return;
    }

    showElementPopup(elementType, elementRect);
}

void AbstractNotationPaintView::paint(QPainter* qp)
{
    TRACEFUNC;

    RectF rect = RectF::fromQRectF(qp->clipBoundingRect());
    rect = correctDrawRect(rect);

    muse::draw::Painter mup(qp, objectName().toStdString());
    muse::draw::Painter* painter = &mup;

    paintBackground(rect, painter);

    if (!isInited()) {
        return;
    }

    qreal guiScaling = configuration()->guiScaling();
    Transform guiScalingCompensation;
    guiScalingCompensation.scale(guiScaling, guiScaling);

    painter->setWorldTransform(m_matrix * guiScalingCompensation);

    bool isPrinting = publishMode() || m_inputController->readonly();
    notation()->painting()->paintView(painter, toLogical(rect), isPrinting);

    m_playbackCursor->paint(painter);
    m_noteInputCursor->paint(painter);
    m_loopInMarker->paint(painter);
    m_loopOutMarker->paint(painter);

    if (notation()->viewMode() == engraving::LayoutMode::LINE) {
        ContinuousPanel::NotationViewContext ctx;
        ctx.xOffset = m_matrix.dx();
        ctx.yOffset = m_matrix.dy();
        ctx.scaling = currentScaling();
        ctx.fromLogical = [this](const PointF& pos) -> PointF { return fromLogical(pos); };
        m_continuousPanel->paint(*painter, ctx);
    }
}

void AbstractNotationPaintView::onNotationSetup()
{
    TRACEFUNC;
    onCurrentNotationChanged();
    onPlayingChanged();

    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        onCurrentNotationChanged();
    });

    playbackController()->isPlayingChanged().onNotify(this, [this]() {
        onPlayingChanged();
    });

    playbackController()->currentPlaybackPositionChanged().onReceive(this, [this](audio::secs_t, midi::tick_t tick) {
        movePlaybackCursor(tick);
    });

    configuration()->foregroundChanged().onNotify(this, [this]() {
        scheduleRedraw();
    });

    uiConfiguration()->currentThemeChanged().onNotify(this, [this]() {
        scheduleRedraw();
    });

    engravingConfiguration()->debuggingOptionsChanged().onNotify(this, [this]() {
        scheduleRedraw();
    });
}

void AbstractNotationPaintView::paintBackground(const RectF& rect, muse::draw::Painter* painter)
{
    TRACEFUNC;

    const QPixmap& wallpaper = configuration()->backgroundWallpaper();

    if (configuration()->backgroundUseColor() || wallpaper.isNull()) {
        painter->fillRect(rect, configuration()->backgroundColor());
    } else {
        painter->drawTiledPixmap(rect, wallpaper, rect.topLeft() - PointF(m_matrix.m31(), m_matrix.m32()));
    }
}

PointF AbstractNotationPaintView::canvasCenter() const
{
    TRACEFUNC;
    RectF canvasRect = m_matrix.map(notationContentRect());

    qreal canvasWidth = canvasRect.width();
    qreal canvasHeight = canvasRect.height();

    qreal x = (width() - canvasWidth) / 2;
    qreal y = (height() - canvasHeight) / 2;

    return toLogical(PointF(x, y));
}

std::pair<qreal, qreal> AbstractNotationPaintView::constraintCanvas(qreal dx, qreal dy) const
{
    TRACEFUNC;
    RectF scrollableArea = scrollableAreaRect();
    RectF viewport = this->viewport();

    // horizontal
    {
        qreal newLeft = viewport.left() - dx;
        if (viewport.width() > scrollableArea.width()) {
            newLeft = scrollableArea.center().x() - viewport.width() / 2;
        } else {
            newLeft = qBound(scrollableArea.left(), newLeft, scrollableArea.right() - viewport.width());
        }
        dx = viewport.left() - newLeft;
    }

    // vertical
    {
        qreal newTop = viewport.top() - dy;
        if (viewport.height() > scrollableArea.height()) {
            newTop = scrollableArea.center().y() - viewport.height() / 2;
        } else {
            newTop = qBound(scrollableArea.top(), newTop, scrollableArea.bottom() - viewport.height());
        }
        dy = viewport.top() - newTop;
    }

    return { dx, dy };
}

PointF AbstractNotationPaintView::viewportTopLeft() const
{
    return toLogical(PointF(0.0, 0.0));
}

RectF AbstractNotationPaintView::viewport() const
{
    return toLogical(RectF(0.0, 0.0, width(), height()));
}

QRectF AbstractNotationPaintView::viewport_property() const
{
    return viewport().toQRectF();
}

RectF AbstractNotationPaintView::notationContentRect() const
{
    TRACEFUNC;
    if (!notationElements()) {
        return RectF();
    }

    RectF result;
    for (const Page* page: notationElements()->pages()) {
        result.unite(page->ldata()->bbox().translated(page->pos()));
    }

    return result;
}

RectF AbstractNotationPaintView::scrollableAreaRect() const
{
    TRACEFUNC;
    RectF viewport = this->viewport();
    qreal overscrollFactor = configuration()->isLimitCanvasScrollArea() ? 0.0 : SCROLL_LIMIT_OFF_OVERSCROLL_FACTOR;

    qreal overscrollX = viewport.width() * overscrollFactor;
    qreal overscrollY = viewport.height() * overscrollFactor;

    return notationContentRect().adjusted(-overscrollX, -overscrollY, overscrollX, overscrollY);
}

qreal AbstractNotationPaintView::horizontalScrollableSize() const
{
    TRACEFUNC;
    RectF viewport = this->viewport();
    RectF contentRect = notationContentRect();

    qreal left = std::min(contentRect.left(), viewport.left());
    qreal right = std::max(contentRect.right(), viewport.right());

    qreal size = 0;
    if ((left < 0) && (right > 0)) {
        size = std::abs(left) + right;
    } else {
        size = std::abs(right) - std::abs(left);
    }

    return size;
}

qreal AbstractNotationPaintView::verticalScrollableSize() const
{
    TRACEFUNC;
    RectF viewport = this->viewport();
    RectF contentRect = notationContentRect();

    qreal top = std::min(contentRect.top(), viewport.top());
    qreal bottom = std::max(contentRect.bottom(), viewport.bottom());

    qreal size = 0;
    if ((top < 0) && (bottom > 0)) {
        size = std::abs(top) + bottom;
    } else {
        size = std::abs(bottom) - std::abs(top);
    }

    return size;
}

qreal AbstractNotationPaintView::horizontalScrollbarSize() const
{
    TRACEFUNC;
    RectF viewport = this->viewport();
    RectF contentRect = notationContentRect();

    if (viewport.left() < contentRect.left()
        && viewport.right() > contentRect.right()) {
        return 0;
    }

    qreal scrollableWidth = horizontalScrollableSize();
    if (qFuzzyIsNull(scrollableWidth)) {
        return 0;
    }

    return viewport.width() / scrollableWidth;
}

qreal AbstractNotationPaintView::verticalScrollbarSize() const
{
    TRACEFUNC;
    RectF viewport = this->viewport();
    RectF contentRect = notationContentRect();

    if (viewport.top() < contentRect.top()
        && viewport.bottom() > contentRect.bottom()) {
        return 0;
    }

    qreal scrollableHeight = verticalScrollableSize();
    if (qFuzzyIsNull(scrollableHeight)) {
        return 0;
    }

    return viewport.height() / scrollableHeight;
}

qreal AbstractNotationPaintView::startHorizontalScrollPosition() const
{
    TRACEFUNC;
    RectF viewport = this->viewport();
    RectF contentRect = notationContentRect();

    if (!viewport.isValid() || !contentRect.isValid()) {
        return 0.0;
    }

    return (viewport.left() - contentRect.left()) / contentRect.width();
}

qreal AbstractNotationPaintView::startVerticalScrollPosition() const
{
    TRACEFUNC;
    RectF viewport = this->viewport();
    RectF contentRect = notationContentRect();

    if (!viewport.isValid() || !contentRect.isValid()) {
        return 0.0;
    }

    return (viewport.top() - contentRect.top()) / contentRect.height();
}

bool AbstractNotationPaintView::adjustCanvasPosition(const RectF& logicRect, bool adjustVertically)
{
    TRACEFUNC;

    RectF viewRect = viewport();

    double viewArea = viewRect.width() * viewRect.height();
    double logicRectArea = logicRect.width() * logicRect.height();

    if (viewArea < logicRectArea) {
        return false;
    }

    if (viewRect.contains(logicRect)) {
        return false;
    }

    constexpr int BORDER_SPACING_RATIO = 3;

    double _spatium = notationStyle()->styleValue(StyleId::spatium).toDouble();
    qreal border = _spatium * BORDER_SPACING_RATIO;
    qreal _scale = currentScaling();
    if (qFuzzyIsNull(_scale)) {
        _scale = 1;
    }

    PointF pos = viewRect.topLeft();
    PointF oldPos = pos;

    RectF showRect = logicRect;

    if (showRect.left() < viewRect.left()) {
        pos.setX(showRect.left() - border);
    } else if (showRect.left() > viewRect.right()) {
        pos.setX(showRect.right() - width() / _scale + border);
    } else if (viewRect.width() >= showRect.width() && showRect.right() > viewRect.right()) {
        pos.setX(showRect.left() - border);
    }

    if (adjustVertically) {
        if (showRect.top() < viewRect.top() && showRect.bottom() < viewRect.bottom()) {
            pos.setY(showRect.top() - border);
        } else if (showRect.top() > viewRect.bottom()) {
            pos.setY(showRect.bottom() - height() / _scale + border);
        } else if (viewRect.height() >= showRect.height() && showRect.bottom() > viewRect.bottom()) {
            pos.setY(showRect.top() - border);
        }
    }

    pos = alignToCurrentPageBorder(showRect, pos);

    if (pos == oldPos) {
        return false;
    }

    return moveCanvasToPosition(pos);
}

bool AbstractNotationPaintView::adjustCanvasPositionSmoothPan(const RectF& cursorRect)
{
    RectF viewRect = viewport();
    PointF pos(cursorRect.x() - (viewRect.width() / 2), viewRect.y());

    if (!viewport().intersects(cursorRect)) {
        pos.setY(cursorRect.y() - (viewRect.height() / 2));
    }

    return moveCanvasToPosition(pos);
}

bool AbstractNotationPaintView::ensureViewportInsideScrollableArea()
{
    TRACEFUNC;

    if (!m_notation) {
        return false;
    }

    auto [dx, dy] = constraintCanvas(0, 0);
    if (qFuzzyIsNull(dx) && qFuzzyIsNull(dy)) {
        return false;
    }

    Transform oldMatrix = m_matrix;
    m_matrix.translate(dx, dy);
    onMatrixChanged(oldMatrix, m_matrix, false);
    return true;
}

bool AbstractNotationPaintView::moveCanvasToPosition(const PointF& logicPos)
{
    TRACEFUNC;

    PointF viewTopLeft = viewportTopLeft();
    Transform oldMatrix = m_matrix;
    if (doMoveCanvas(viewTopLeft.x() - logicPos.x(), viewTopLeft.y() - logicPos.y())) {
        onMatrixChanged(oldMatrix, m_matrix, false);
        return true;
    }

    return false;
}

bool AbstractNotationPaintView::moveCanvas(qreal dx, qreal dy)
{
    TRACEFUNC;

    Transform oldMatrix = m_matrix;
    bool moved = doMoveCanvas(dx, dy);

    if (moved) {
        onMatrixChanged(oldMatrix, m_matrix, false);

        m_autoScrollEnabled = false;
        m_enableAutoScrollTimer.start(2000);

        hideElementPopup();
    }

    return moved;
}

bool AbstractNotationPaintView::doMoveCanvas(qreal dx, qreal dy)
{
    if (!m_notation) {
        return false;
    }

    if (qFuzzyIsNull(dx) && qFuzzyIsNull(dy)) {
        return false;
    }

    auto [correctedDX, correctedDY] = constraintCanvas(dx, dy);
    if (qFuzzyIsNull(correctedDX) && qFuzzyIsNull(correctedDY)) {
        return false;
    }

    m_matrix.translate(correctedDX, correctedDY);

    return true;
}

void AbstractNotationPaintView::scheduleRedraw(const muse::RectF& rect)
{
    QRect qrect = correctDrawRect(rect).toQRect();
    update(qrect);
}

RectF AbstractNotationPaintView::correctDrawRect(const RectF& rect) const
{
    if (!rect.isValid() || rect.isNull()) {
        return RectF(0, 0, width(), height());
    }

    return rect;
}

void AbstractNotationPaintView::moveCanvasVertical(qreal dy)
{
    moveCanvas(0, dy);
}

void AbstractNotationPaintView::moveCanvasHorizontal(qreal dx)
{
    moveCanvas(dx, 0);
}

qreal AbstractNotationPaintView::currentScaling() const
{
    return m_matrix.m11();
}

void AbstractNotationPaintView::setScaling(qreal scaling, const PointF& pos, bool overrideZoomType)
{
    TRACEFUNC;

    if (!m_notation) {
        return;
    }

    hideElementPopup();

    qreal currentScaling = this->currentScaling();

    IF_ASSERT_FAILED(!qFuzzyIsNull(scaling)) {
        return;
    }

    if (qFuzzyCompare(currentScaling, scaling)) {
        return;
    }

    if (qFuzzyIsNull(currentScaling)) {
        currentScaling = 1;
    }

    qreal deltaScaling = scaling / currentScaling;
    scale(deltaScaling, pos, overrideZoomType);
}

void AbstractNotationPaintView::scale(qreal factor, const PointF& pos, bool overrideZoomType)
{
    TRACEFUNC;

    if (!m_notation) {
        return;
    }

    if (qFuzzyCompare(factor, 1.0)) {
        return;
    }

    PointF pointBeforeScaling = toLogical(pos);

    Transform oldMatrix = m_matrix;
    m_matrix.scale(factor, factor);

    PointF pointAfterScaling = toLogical(pos);

    qreal dx = pointAfterScaling.x() - pointBeforeScaling.x();
    qreal dy = pointAfterScaling.y() - pointBeforeScaling.y();

    doMoveCanvas(dx, dy);

    onMatrixChanged(oldMatrix, m_matrix, overrideZoomType);
}

void AbstractNotationPaintView::pinchToZoom(qreal scaleFactor, const QPointF& pos)
{
    if (isInited()) {
        m_inputController->pinchToZoom(scaleFactor, pos);
    }
}

void AbstractNotationPaintView::wheelEvent(QWheelEvent* event)
{
    TRACEFUNC;
    if (isInited()) {
        m_inputController->wheelEvent(event);
    }
}

void AbstractNotationPaintView::forceFocusIn()
{
    TRACEFUNC;
    setFocus(true);
    emit activeFocusRequested();
    forceActiveFocus();
}

void AbstractNotationPaintView::onContextMenuIsOpenChanged(bool open)
{
    m_isContextMenuOpen = open;
}

void AbstractNotationPaintView::onElementPopupIsOpenChanged(bool open)
{
    m_isPopupOpen = open;
}

void AbstractNotationPaintView::mousePressEvent(QMouseEvent* event)
{
    TRACEFUNC;
    forceFocusIn();

    if (isInited()) {
        m_inputController->mousePressEvent(event);
    }
}

void AbstractNotationPaintView::mouseMoveEvent(QMouseEvent* event)
{
    TRACEFUNC;
    if (isInited()) {
        m_inputController->mouseMoveEvent(event);
    }
}

void AbstractNotationPaintView::mouseDoubleClickEvent(QMouseEvent* event)
{
    TRACEFUNC;
    forceFocusIn();

    if (isInited()) {
        m_inputController->mouseDoubleClickEvent(event);
    }
}

void AbstractNotationPaintView::mouseReleaseEvent(QMouseEvent* event)
{
    if (isInited()) {
        m_inputController->mouseReleaseEvent(event);
    }
}

void AbstractNotationPaintView::hoverMoveEvent(QHoverEvent* event)
{
    if (isInited()) {
        m_inputController->hoverMoveEvent(event);
    }
}

bool AbstractNotationPaintView::shortcutOverride(QKeyEvent* event)
{
    if (isInited()) {
        return m_inputController->shortcutOverrideEvent(event);
    }

    return false;
}

void AbstractNotationPaintView::keyPressEvent(QKeyEvent* event)
{
    if (isInited()) {
        m_inputController->keyPressEvent(event);
    }

    if (event->key() == m_lastAcceptedKey) {
        // required to prevent Qt-Quick from changing focus on tab
        m_lastAcceptedKey = -1;
    }

    return;
}

void AbstractNotationPaintView::keyReleaseEvent(QKeyEvent* event)
{
    if (isInited()) {
        m_inputController->keyReleaseEvent(event);
    }
}

bool AbstractNotationPaintView::event(QEvent* event)
{
    QEvent::Type eventType = event->type();
    auto keyEvent = dynamic_cast<QKeyEvent*>(event);

    bool isContextMenuEvent = ((eventType == QEvent::ShortcutOverride && keyEvent->key() == Qt::Key_Menu)
                               || eventType == QEvent::Type::ContextMenu) && hasFocus();

    if (isContextMenuEvent) {
        showContextMenu(m_inputController->selectionType(),
                        fromLogical(m_inputController->selectionElementPos()).toQPointF());
    } else if (eventType == QEvent::Type::ShortcutOverride) {
        bool shouldOverrideShortcut = shortcutOverride(keyEvent);

        if (shouldOverrideShortcut) {
            m_lastAcceptedKey = keyEvent->key();
            keyEvent->accept();
            return true;
        }
    }

    return QQuickPaintedItem::event(event);
}

void AbstractNotationPaintView::inputMethodEvent(QInputMethodEvent* event)
{
    if (isInited()) {
        m_inputController->inputMethodEvent(event);
    }
}

QVariant AbstractNotationPaintView::inputMethodQuery(Qt::InputMethodQuery query) const
{
    if (isInited() && m_inputController->canHandleInputMethodQuery(query)) {
        return m_inputController->inputMethodQuery(query);
    }

    return QQuickPaintedItem::inputMethodQuery(query);
}

void AbstractNotationPaintView::dragEnterEvent(QDragEnterEvent* event)
{
    if (isInited()) {
        m_inputController->dragEnterEvent(event);
    }
}

void AbstractNotationPaintView::dragLeaveEvent(QDragLeaveEvent* event)
{
    if (isInited()) {
        m_inputController->dragLeaveEvent(event);
    }
}

void AbstractNotationPaintView::dragMoveEvent(QDragMoveEvent* event)
{
    if (isInited()) {
        m_inputController->dragMoveEvent(event);
    }
}

void AbstractNotationPaintView::dropEvent(QDropEvent* event)
{
    if (isInited()) {
        m_inputController->dropEvent(event);
    }
}

void AbstractNotationPaintView::setNotation(INotationPtr notation)
{
    m_notation = notation;
    m_continuousPanel->setNotation(m_notation);
    m_playbackCursor->setNotation(m_notation);
    m_loopInMarker->setNotation(m_notation);
    m_loopOutMarker->setNotation(m_notation);
}

void AbstractNotationPaintView::setReadonly(bool readonly)
{
    m_readonly = readonly;
    if (m_inputController) {
        m_inputController->setReadonly(m_readonly);
    }
}

void AbstractNotationPaintView::clear()
{
    Transform oldMatrix = m_matrix;
    m_matrix = Transform();
    m_previousHorizontalScrollPosition = 0;
    m_previousVerticalScrollPosition = 0;
    m_shadowNoteRect = RectF();
    onMatrixChanged(oldMatrix, m_matrix, false);
}

qreal AbstractNotationPaintView::width() const
{
    return QQuickPaintedItem::width();
}

qreal AbstractNotationPaintView::height() const
{
    return QQuickPaintedItem::height();
}

PointF AbstractNotationPaintView::toLogical(const PointF& point) const
{
    return m_matrix.inverted().map(point);
}

PointF AbstractNotationPaintView::toLogical(const QPointF& point) const
{
    return toLogical(PointF::fromQPointF(point));
}

RectF AbstractNotationPaintView::toLogical(const RectF& rect) const
{
    return m_matrix.inverted().map(rect);
}

PointF AbstractNotationPaintView::fromLogical(const PointF& point) const
{
    return m_matrix.map(point);
}

RectF AbstractNotationPaintView::fromLogical(const RectF& rect) const
{
    return m_matrix.map(rect);
}

bool AbstractNotationPaintView::isInited() const
{
    if (qFuzzyIsNull(width()) || qFuzzyIsNull(height())) {
        return false;
    }

    return notation() != nullptr;
}

void AbstractNotationPaintView::onPlayingChanged()
{
    TRACEFUNC;

    if (!notationPlayback()) {
        return;
    }

    bool isPlaying = playbackController()->isPlaying();
    m_playbackCursor->setVisible(isPlaying);

    m_autoScrollEnabled = true;
    m_enableAutoScrollTimer.stop();

    if (isPlaying) {
        audio::secs_t pos = globalContext()->playbackState()->playbackPosition();
        muse::midi::tick_t tick = notationPlayback()->secToTick(pos);
        movePlaybackCursor(tick);
    } else {
        scheduleRedraw();
    }
}

void AbstractNotationPaintView::movePlaybackCursor(muse::midi::tick_t tick)
{
    TRACEFUNC;

    if (!notationPlayback()) {
        return;
    }

    RectF oldCursorRect = m_playbackCursor->rect();
    m_playbackCursor->move(tick);
    const RectF& newCursorRect = m_playbackCursor->rect();

    if (!m_playbackCursor->visible() || newCursorRect.isNull()) {
        return;
    }

    if (configuration()->isAutomaticallyPanEnabled()) {
        if ((notation()->viewMode() == engraving::LayoutMode::LINE) && configuration()->isSmoothPanning()
            && adjustCanvasPositionSmoothPan(newCursorRect)) {
            return;
        }

        if (m_autoScrollEnabled) {
            bool adjustVertically = needAdjustCanvasVerticallyWhilePlayback(newCursorRect);
            if (adjustCanvasPosition(newCursorRect, adjustVertically)) {
                return;
            }
        }
    }

    //! NOTE: redraw in a slightly larger area than the cursor rect to avoid graphical artifacts
    RectF dirtyRect1 = fromLogical(oldCursorRect).adjusted(-1, -1, 2, 1);
    RectF dirtyRect2 = fromLogical(newCursorRect).adjusted(-1, -1, 2, 1);

    double dx = std::abs(dirtyRect2.x() - dirtyRect1.x());
    double dy = std::abs(dirtyRect2.y() - dirtyRect1.y());

    //! NOTE: the difference between the old cursor rect and the new one is not big, so we redraw their united rect
    if (dx < 1.0 && dy < 1.0) {
        scheduleRedraw(dirtyRect1.united(dirtyRect2));
    } else {
        scheduleRedraw(dirtyRect1);
        scheduleRedraw(dirtyRect2);
    }
}

bool AbstractNotationPaintView::needAdjustCanvasVerticallyWhilePlayback(const RectF& cursorRect)
{
    if (!viewport().intersects(cursorRect)) {
        return true;
    }

    const Page* page = pageByPoint(cursorRect.topRight());
    if (!page) {
        return false;
    }

    int nonEmptySystemCount = 0;

    for (const System* system : page->systems()) {
        if (!system->staves().empty()) {
            nonEmptySystemCount++;
        }
    }

    return nonEmptySystemCount > 1;
}

const Page* AbstractNotationPaintView::pageByPoint(const PointF& point) const
{
    INotationElementsPtr elements = notationElements();
    return elements ? elements->pageByPoint(point) : nullptr;
}

PointF AbstractNotationPaintView::alignToCurrentPageBorder(const RectF& showRect, const PointF& pos) const
{
    TRACEFUNC;

    PointF result = pos;
    const Page* page = pageByPoint(showRect.topLeft());
    if (!page) {
        return result;
    }

    RectF viewRect = viewport();

    if (result.x() < page->x() || viewRect.width() >= page->width()) {
        result.setX(page->x());
    } else if (viewRect.width() < page->width() && viewRect.width() + pos.x() > page->width() + page->x()) {
        result.setX((page->width() + page->x()) - viewRect.width());
    }
    if (result.y() < page->y() || viewRect.height() >= page->height()) {
        result.setY(page->y());
    } else if (viewRect.height() < page->height() && viewRect.height() + pos.y() > page->height() + page->y()) {
        result.setY((page->height() + page->y()) - viewRect.height());
    }

    return result;
}

bool AbstractNotationPaintView::publishMode() const
{
    return m_publishMode;
}

void AbstractNotationPaintView::setPublishMode(bool arg)
{
    if (m_publishMode == arg) {
        return;
    }

    m_publishMode = arg;
    emit publishModeChanged();
}

bool AbstractNotationPaintView::isMainView() const
{
    return m_isMainView;
}

void AbstractNotationPaintView::setIsMainView(bool isMainView)
{
    if (m_isMainView == isMainView) {
        return;
    }

    m_isMainView = isMainView;
    emit isMainViewChanged(m_isMainView);
}
