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
#include "notationpaintview.h"

#include <QPainter>

#include "actions/actiontypes.h"
#include "stringutils.h"
#include "log.h"

using namespace mu;
using namespace mu::ui;
using namespace mu::draw;
using namespace mu::notation;

static constexpr qreal SCROLL_LIMIT_OFF_OFFSET = 0.75;
static constexpr qreal SCROLL_LIMIT_ON_OFFSET = 0.02;

NotationPaintView::NotationPaintView(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
    setFlag(ItemHasContents, true);
    setFlag(ItemAcceptsDrops, true);
    setFlag(ItemAcceptsInputMethod, true);
    setAcceptedMouseButtons(Qt::AllButtons);
    setAntialiasing(true);

    connect(this, &QQuickPaintedItem::widthChanged, this, &NotationPaintView::onViewSizeChanged);
    connect(this, &QQuickPaintedItem::heightChanged, this, &NotationPaintView::onViewSizeChanged);

    connect(this, &NotationPaintView::horizontalScrollChanged, [this]() {
        m_previousHorizontalScrollPosition = startHorizontalScrollPosition();
    });

    connect(this, &NotationPaintView::verticalScrollChanged, [this]() {
        m_previousVerticalScrollPosition = startVerticalScrollPosition();
    });

    m_inputController = std::make_unique<NotationViewInputController>(this);
    m_playbackCursor = std::make_unique<PlaybackCursor>();
    m_playbackCursor->setVisible(false);
    m_noteInputCursor = std::make_unique<NoteInputCursor>();

    m_loopInMarker = std::make_unique<LoopMarker>(LoopBoundaryType::LoopIn);
    m_loopOutMarker = std::make_unique<LoopMarker>(LoopBoundaryType::LoopOut);

    //! NOTE For diagnostic tools
    dispatcher()->reg(this, "diagnostic-notationview-redraw", [this]() {
        update();
    });
}

void NotationPaintView::load()
{
    TRACEFUNC;

    onNotationSetup();

    initBackground();
    initNavigatorOrientation();

    configuration()->isLimitCanvasScrollAreaChanged().onNotify(this, [this]() {
        ensureViewportInsideScrollableArea();

        emit horizontalScrollChanged();
        emit verticalScrollChanged();
        emit viewportChanged();
    });

    m_inputController->init();
}

void NotationPaintView::initBackground()
{
    emit backgroundColorChanged(configuration()->backgroundColor());

    configuration()->backgroundChanged().onNotify(this, [this]() {
        emit backgroundColorChanged(configuration()->backgroundColor());
        update();
    });
}

void NotationPaintView::initNavigatorOrientation()
{
    configuration()->canvasOrientation().ch.onReceive(this, [this](framework::Orientation) {
        moveCanvasToPosition(PointF(0, 0));
    });
}

void NotationPaintView::moveCanvasToCenter()
{
    TRACEFUNC;
    if (!isInited()) {
        return;
    }

    PointF canvasCenter = this->canvasCenter();
    moveCanvas(canvasCenter.x(), canvasCenter.y());
}

void NotationPaintView::scrollHorizontal(qreal position)
{
    TRACEFUNC;
    qreal scrollStep = position - m_previousHorizontalScrollPosition;
    if (qFuzzyIsNull(scrollStep)) {
        return;
    }

    qreal dx = horizontalScrollableSize() * scrollStep;
    moveCanvasHorizontal(-dx);
}

void NotationPaintView::scrollVertical(qreal position)
{
    TRACEFUNC;
    qreal scrollStep = position - m_previousVerticalScrollPosition;
    if (qFuzzyIsNull(scrollStep)) {
        return;
    }

    qreal dy = verticalScrollableSize() * scrollStep;
    moveCanvasVertical(-dy);
}

void NotationPaintView::zoomIn()
{
    m_inputController->zoomIn();
}

void NotationPaintView::zoomOut()
{
    m_inputController->zoomOut();
}

void NotationPaintView::selectOnNavigationActive()
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

bool NotationPaintView::canReceiveAction(const actions::ActionCode& actionCode) const
{
    if (actionCode == "diagnostic-notationview-redraw") {
        return true;
    }

    return hasFocus();
}

void NotationPaintView::onCurrentNotationChanged()
{
    TRACEFUNC;
    if (m_notation) {
        m_notation->notationChanged().resetOnNotify(this);
        INotationInteractionPtr interaction = m_notation->interaction();
        interaction->noteInput()->stateChanged().resetOnNotify(this);
        interaction->selectionChanged().resetOnNotify(this);
        m_notation->accessibility()->setMapToScreenFunc(nullptr);
    }

    m_notation = globalContext()->currentNotation();
    if (!m_notation) {
        return;
    }

    if (publishMode()) {
        m_notation->setViewMode(ViewMode::PAGE);
    } else {
        m_notation->setViewMode(globalContext()->currentProject()->viewSettings()->notationViewMode());
    }

    INotationInteractionPtr interaction = notationInteraction();

    m_notation->notationChanged().onNotify(this, [this, interaction]() {
        interaction->hideShadowNote();
        update();
    });

    onNoteInputModeChanged();
    onSelectionChanged();

    interaction->noteInput()->stateChanged().onNotify(this, [this]() {
        onNoteInputModeChanged();
    });

    interaction->selectionChanged().onNotify(this, [this]() {
        onSelectionChanged();
    });

    interaction->showItemRequested().onReceive(this, [this](const INotationInteraction::ShowItemRequest& request) {
        onShowItemRequested(request);
    });

    interaction->textEditingStarted().onNotify(this, [this]() {
        if (!hasActiveFocus()) {
            forceFocusIn();
        }
    });

    interaction->dropChanged().onNotify(this, [this]() {
        if (!hasActiveFocus()) {
            forceFocusIn(); // grab keyboard focus after element added from palette
        }
    });

    notationPlayback()->loopBoundaries().ch.onReceive(this, [this](const LoopBoundaries& boundaries) {
        updateLoopMarkers(boundaries);
    });

    m_loopInMarker->setStyle(m_notation->style());
    m_loopOutMarker->setStyle(m_notation->style());

    notation()->accessibility()->setMapToScreenFunc([this](const RectF& elementRect) {
        auto res = fromLogical(elementRect);
        res = RectF(PointF::fromQPointF(mapToGlobal(res.topLeft().toQPointF())), SizeF(res.width(), res.height()));

        return res;
    });

    forceFocusIn();
    update();

    emit horizontalScrollChanged();
    emit verticalScrollChanged();
    emit viewportChanged();
}

void NotationPaintView::onViewSizeChanged()
{
    TRACEFUNC;

    if (!notation()) {
        return;
    }

    if (viewport().isValid() && !m_inputController->isZoomInited()) {
        m_inputController->initZoom();
    }

    ensureViewportInsideScrollableArea();

    emit horizontalScrollChanged();
    emit verticalScrollChanged();
    emit viewportChanged();
}

void NotationPaintView::updateLoopMarkers(const LoopBoundaries& boundaries)
{
    TRACEFUNC;
    m_loopInMarker->setRect(boundaries.loopInRect);
    m_loopOutMarker->setRect(boundaries.loopOutRect);

    m_loopInMarker->setVisible(boundaries.visible);
    m_loopOutMarker->setVisible(boundaries.visible);

    update();
}

INotationPtr NotationPaintView::notation() const
{
    return m_notation;
}

INotationInteractionPtr NotationPaintView::notationInteraction() const
{
    return notation() ? notation()->interaction() : nullptr;
}

INotationPlaybackPtr NotationPaintView::notationPlayback() const
{
    return globalContext()->currentMasterNotation() ? globalContext()->currentMasterNotation()->playback() : nullptr;
}

INotationNoteInputPtr NotationPaintView::notationNoteInput() const
{
    return notationInteraction() ? notationInteraction()->noteInput() : nullptr;
}

INotationElementsPtr NotationPaintView::notationElements() const
{
    return notation() ? notation()->elements() : nullptr;
}

INotationStylePtr NotationPaintView::notationStyle() const
{
    return notation() ? notation()->style() : nullptr;
}

INotationSelectionPtr NotationPaintView::notationSelection() const
{
    return notationInteraction() ? notationInteraction()->selection() : nullptr;
}

void NotationPaintView::onNoteInputModeChanged()
{
    TRACEFUNC;

    bool noteEnterMode = isNoteEnterMode();
    setAcceptHoverEvents(noteEnterMode);

    if (noteEnterMode) {
        emit activeFocusRequested();
    }

    if (INotationInteractionPtr interaction = notationInteraction()) {
        interaction->hideShadowNote();
        update();
    }
}

void NotationPaintView::onSelectionChanged()
{
    TRACEFUNC;

    RectF selectionRect = notationSelection()->canvasBoundingRect();
    if (selectionRect.isNull()) {
        return;
    }

    if (adjustCanvasPosition(selectionRect)) {
        return;
    }

    update();
}

void NotationPaintView::onShowItemRequested(const INotationInteraction::ShowItemRequest& request)
{
    IF_ASSERT_FAILED(request.item) {
        return;
    }

    RectF viewRect = viewport();

    RectF showRect = request.showRect;
    RectF itemBoundingRect = request.item->canvasBoundingRect();

    if (viewRect.width() < showRect.width()) {
        showRect.setX(itemBoundingRect.x());
        showRect.setWidth(itemBoundingRect.width());
    }

    if (viewRect.height() < showRect.height()) {
        showRect.setY(itemBoundingRect.y());
        showRect.setHeight(itemBoundingRect.height());
    }

    adjustCanvasPosition(showRect);
}

bool NotationPaintView::isNoteEnterMode() const
{
    return notationNoteInput() ? notationNoteInput()->isNoteInputMode() : false;
}

void NotationPaintView::showShadowNote(const PointF& pos)
{
    TRACEFUNC;
    notationInteraction()->showShadowNote(pos);
    update();
}

void NotationPaintView::showContextMenu(const ElementType& elementType, const QPointF& pos, bool activateFocus)
{
    TRACEFUNC;

    QPointF _pos = pos;
    if (_pos.isNull()) {
        _pos = QPointF(width() / 2, height() / 2);
    }

    emit showContextMenuRequested(static_cast<int>(elementType), pos);

    if (activateFocus) {
        dispatcher()->dispatch("nav-first-control");
    }
}

void NotationPaintView::hideContextMenu()
{
    TRACEFUNC;
    emit hideContextMenuRequested();
}

void NotationPaintView::paint(QPainter* qp)
{
    TRACEFUNC;

    mu::draw::Painter mup(qp, objectName().toStdString());
    mu::draw::Painter* painter = &mup;

    RectF rect(0.0, 0.0, width(), height());
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
}

void NotationPaintView::onNotationSetup()
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

    playbackController()->midiTickPlayed().onReceive(this, [this](uint32_t tick) {
        movePlaybackCursor(tick);
    });

    configuration()->foregroundChanged().onNotify(this, [this]() {
        update();
    });

    uiConfiguration()->currentThemeChanged().onNotify(this, [this]() {
        update();
    });
}

void NotationPaintView::paintBackground(const RectF& rect, draw::Painter* painter)
{
    TRACEFUNC;
    QString wallpaperPath = configuration()->backgroundWallpaperPath().toQString();

    if (configuration()->backgroundUseColor() || wallpaperPath.isEmpty()) {
        painter->fillRect(rect, configuration()->backgroundColor());
    } else {
        static QPixmap pixmap(wallpaperPath);
        static QString lastPath = wallpaperPath;

        if (lastPath != wallpaperPath) {
            pixmap = QPixmap(wallpaperPath);
            lastPath = wallpaperPath;
        }

        painter->drawTiledPixmap(rect, pixmap, rect.topLeft() - PointF(m_matrix.m31(), m_matrix.m32()));
    }
}

PointF NotationPaintView::canvasCenter() const
{
    TRACEFUNC;
    RectF canvasRect = m_matrix.map(notationContentRect());

    qreal canvasWidth = canvasRect.width();
    qreal canvasHeight = canvasRect.height();

    qreal x = (width() - canvasWidth) / 2;
    qreal y = (height() - canvasHeight) / 2;

    return toLogical(PointF(x, y));
}

std::pair<qreal, qreal> NotationPaintView::constraintCanvas(qreal dx, qreal dy) const
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

PointF NotationPaintView::viewportTopLeft() const
{
    return toLogical(PointF(0.0, 0.0));
}

RectF NotationPaintView::viewport() const
{
    return toLogical(RectF(0.0, 0.0, width(), height()));
}

QRectF NotationPaintView::viewport_property() const
{
    return viewport().toQRectF();
}

RectF NotationPaintView::notationContentRect() const
{
    TRACEFUNC;
    if (!notationElements()) {
        return RectF();
    }

    RectF result;
    for (const Page* page: notationElements()->pages()) {
        result.unite(page->bbox().translated(page->pos()));
    }

    return result;
}

RectF NotationPaintView::scrollableAreaRect() const
{
    TRACEFUNC;
    RectF viewport = this->viewport();
    qreal overscrollFactor = configuration()->isLimitCanvasScrollArea() ? SCROLL_LIMIT_ON_OFFSET : SCROLL_LIMIT_OFF_OFFSET;

    qreal overscrollX = viewport.width() * overscrollFactor;
    qreal overscrollY = viewport.height() * overscrollFactor;

    return notationContentRect().adjusted(-overscrollX, -overscrollY, overscrollX, overscrollY);
}

qreal NotationPaintView::horizontalScrollableSize() const
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

qreal NotationPaintView::verticalScrollableSize() const
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

qreal NotationPaintView::horizontalScrollbarSize() const
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

qreal NotationPaintView::verticalScrollbarSize() const
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

qreal NotationPaintView::startHorizontalScrollPosition() const
{
    TRACEFUNC;
    RectF viewport = this->viewport();
    RectF contentRect = notationContentRect();

    if (!viewport.isValid() || !contentRect.isValid()) {
        return 0.0;
    }

    return (viewport.left() - contentRect.left()) / contentRect.width();
}

qreal NotationPaintView::startVerticalScrollPosition() const
{
    TRACEFUNC;
    RectF viewport = this->viewport();
    RectF contentRect = notationContentRect();

    if (!viewport.isValid() || !contentRect.isValid()) {
        return 0.0;
    }

    return (viewport.top() - contentRect.top()) / contentRect.height();
}

bool NotationPaintView::adjustCanvasPosition(const RectF& logicRect)
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

    if (showRect.top() < viewRect.top() && showRect.bottom() < viewRect.bottom()) {
        pos.setY(showRect.top() - border);
    } else if (showRect.top() > viewRect.bottom()) {
        pos.setY(showRect.bottom() - height() / _scale + border);
    } else if (viewRect.height() >= showRect.height() && showRect.bottom() > viewRect.bottom()) {
        pos.setY(showRect.top() - border);
    }

    pos = alignToCurrentPageBorder(showRect, pos);

    if (pos == oldPos) {
        return false;
    }

    return moveCanvasToPosition(pos);
}

void NotationPaintView::ensureViewportInsideScrollableArea()
{
    TRACEFUNC;

    auto [dx, dy] = constraintCanvas(0, 0);
    if (qFuzzyIsNull(dx) && qFuzzyIsNull(dy)) {
        return;
    }

    m_matrix.translate(dx, dy);
    update();
}

bool NotationPaintView::moveCanvasToPosition(const PointF& logicPos)
{
    TRACEFUNC;

    PointF viewTopLeft = viewportTopLeft();
    return moveCanvas(viewTopLeft.x() - logicPos.x(), viewTopLeft.y() - logicPos.y());
}

bool NotationPaintView::moveCanvas(qreal dx, qreal dy)
{
    TRACEFUNC;
    if (qFuzzyIsNull(dx) && qFuzzyIsNull(dy)) {
        return false;
    }

    auto [correctedDX, correctedDY] = constraintCanvas(dx, dy);
    if (qFuzzyIsNull(correctedDX) && qFuzzyIsNull(correctedDY)) {
        return false;
    }

    m_matrix.translate(correctedDX, correctedDY);
    update();

    emit horizontalScrollChanged();
    emit verticalScrollChanged();
    emit viewportChanged();

    return true;
}

void NotationPaintView::moveCanvasVertical(qreal dy)
{
    moveCanvas(0, dy);
}

void NotationPaintView::moveCanvasHorizontal(qreal dx)
{
    moveCanvas(dx, 0);
}

qreal NotationPaintView::currentScaling() const
{
    return m_matrix.m11();
}

void NotationPaintView::setScaling(qreal scaling, const PointF& pos)
{
    TRACEFUNC;
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
    scale(deltaScaling, pos);
}

void NotationPaintView::scale(qreal factor, const PointF& pos)
{
    TRACEFUNC;
    if (qFuzzyCompare(factor, 1.0)) {
        return;
    }

    PointF pointBeforeScaling = toLogical(pos);

    m_matrix.scale(factor, factor);

    PointF pointAfterScaling = toLogical(pos);

    qreal dx = pointAfterScaling.x() - pointBeforeScaling.x();
    qreal dy = pointAfterScaling.y() - pointBeforeScaling.y();

    // If canvas has moved, moveCanvas will call update();
    // Otherwise, it needs to be called here
    if (!moveCanvas(dx, dy)) {
        update();
    }
}

void NotationPaintView::pinchToZoom(qreal scaleFactor, const QPointF& pos)
{
    if (isInited()) {
        m_inputController->pinchToZoom(scaleFactor, pos);
    }
}

void NotationPaintView::wheelEvent(QWheelEvent* event)
{
    TRACEFUNC;
    if (isInited()) {
        m_inputController->wheelEvent(event);
    }
}

void NotationPaintView::forceFocusIn()
{
    TRACEFUNC;
    setFocus(true);
    emit activeFocusRequested();
    forceActiveFocus();
}

void NotationPaintView::mousePressEvent(QMouseEvent* event)
{
    TRACEFUNC;
    forceFocusIn();

    if (isInited()) {
        m_inputController->mousePressEvent(event);
    }
}

void NotationPaintView::mouseMoveEvent(QMouseEvent* event)
{
    TRACEFUNC;
    if (isInited()) {
        m_inputController->mouseMoveEvent(event);
    }
}

void NotationPaintView::mouseDoubleClickEvent(QMouseEvent* event)
{
    TRACEFUNC;
    forceFocusIn();

    if (isInited()) {
        m_inputController->mouseDoubleClickEvent(event);
    }
}

void NotationPaintView::mouseReleaseEvent(QMouseEvent* event)
{
    if (isInited()) {
        m_inputController->mouseReleaseEvent(event);
    }
}

void NotationPaintView::hoverMoveEvent(QHoverEvent* event)
{
    if (isInited()) {
        m_inputController->hoverMoveEvent(event);
    }
}

bool NotationPaintView::shortcutOverride(QKeyEvent* event)
{
    if (isInited()) {
        return m_inputController->shortcutOverrideEvent(event);
    }

    return false;
}

void NotationPaintView::keyPressEvent(QKeyEvent* event)
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

bool NotationPaintView::event(QEvent* event)
{
    QEvent::Type eventType = event->type();
    auto keyEvent = dynamic_cast<QKeyEvent*>(event);

    bool isContextMenuEvent = ((eventType == QEvent::ShortcutOverride && keyEvent->key() == Qt::Key_Menu)
                               || eventType == QEvent::Type::ContextMenu) && hasFocus();

    if (isContextMenuEvent) {
        showContextMenu(m_inputController->selectionType(),
                        fromLogical(m_inputController->selectionElementPos()).toQPointF(), true);
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

void NotationPaintView::inputMethodEvent(QInputMethodEvent* event)
{
    if (isInited()) {
        m_inputController->inputMethodEvent(event);
    }
}

QVariant NotationPaintView::inputMethodQuery(Qt::InputMethodQuery query) const
{
    if (isInited() && m_inputController->canHandleInputMethodQuery(query)) {
        return m_inputController->inputMethodQuery(query);
    }

    return QQuickPaintedItem::inputMethodQuery(query);
}

void NotationPaintView::dragEnterEvent(QDragEnterEvent* event)
{
    if (isInited()) {
        m_inputController->dragEnterEvent(event);
    }
}

void NotationPaintView::dragLeaveEvent(QDragLeaveEvent* event)
{
    if (isInited()) {
        m_inputController->dragLeaveEvent(event);
    }
}

void NotationPaintView::dragMoveEvent(QDragMoveEvent* event)
{
    if (isInited()) {
        m_inputController->dragMoveEvent(event);
    }
}

void NotationPaintView::dropEvent(QDropEvent* event)
{
    if (isInited()) {
        m_inputController->dropEvent(event);
    }
}

void NotationPaintView::setNotation(INotationPtr notation)
{
    clear();
    m_notation = notation;
    update();
}

void NotationPaintView::setReadonly(bool readonly)
{
    m_inputController->setReadonly(readonly);
}

void NotationPaintView::clear()
{
    m_matrix = Transform();
    m_previousHorizontalScrollPosition = 0;
    m_previousVerticalScrollPosition = 0;
}

qreal NotationPaintView::width() const
{
    return QQuickPaintedItem::width();
}

qreal NotationPaintView::height() const
{
    return QQuickPaintedItem::height();
}

PointF NotationPaintView::toLogical(const PointF& point) const
{
    return m_matrix.inverted().map(point);
}

PointF NotationPaintView::toLogical(const QPointF& point) const
{
    return toLogical(PointF::fromQPointF(point));
}

RectF NotationPaintView::toLogical(const RectF& rect) const
{
    return m_matrix.inverted().map(rect);
}

PointF NotationPaintView::fromLogical(const PointF& point) const
{
    return m_matrix.map(point);
}

RectF NotationPaintView::fromLogical(const RectF& rect) const
{
    return m_matrix.map(rect);
}

bool NotationPaintView::isInited() const
{
    if (qFuzzyIsNull(width()) || qFuzzyIsNull(height())) {
        return false;
    }

    return notation() != nullptr;
}

void NotationPaintView::onPlayingChanged()
{
    TRACEFUNC;

    if (!notationPlayback()) {
        return;
    }

    bool isPlaying = playbackController()->isPlaying();
    m_playbackCursor->setVisible(isPlaying);

    if (isPlaying) {
        float playPosSec = playbackController()->playbackPositionInSeconds();
        uint32_t tick = notationPlayback()->secToTick(playPosSec);
        movePlaybackCursor(tick);
    } else {
        update();
    }
}

void NotationPaintView::movePlaybackCursor(uint32_t tick)
{
    TRACEFUNC;

    if (!notationPlayback()) {
        return;
    }

    RectF cursorRect = notationPlayback()->playbackCursorRectByTick(tick);
    m_playbackCursor->setRect(cursorRect);

    if (!m_playbackCursor->visible()) {
        return;
    }

    if (configuration()->isAutomaticallyPanEnabled()) {
        if (adjustCanvasPosition(cursorRect)) {
            return;
        }
    }

    update(); //! TODO set rect to optimization
}

const Page* NotationPaintView::pointToPage(const PointF& point) const
{
    TRACEFUNC;
    if (!notationElements()) {
        return nullptr;
    }

    PageList pages = notationElements()->pages();
    if (notation()->viewMode() == engraving::LayoutMode::LINE) {
        return pages.empty() ? nullptr : pages.front();
    }

    for (const Page* page: pages) {
        if (page->bbox().translated(page->pos()).contains(point)) {
            return page;
        }
    }

    return nullptr;
}

PointF NotationPaintView::alignToCurrentPageBorder(const RectF& showRect, const PointF& pos) const
{
    TRACEFUNC;

    PointF result = pos;
    const Page* page = pointToPage(showRect.topLeft());
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

bool NotationPaintView::publishMode() const
{
    return m_publishMode;
}

void NotationPaintView::setPublishMode(bool arg)
{
    if (m_publishMode == arg) {
        return;
    }
    m_publishMode = arg;
    emit publishModeChanged();
}
