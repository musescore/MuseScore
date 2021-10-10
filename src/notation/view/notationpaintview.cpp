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

using namespace mu::notation;
using namespace mu::ui;
using namespace mu;

static constexpr qreal MIN_SCROLL_SIZE = 0.2;
static constexpr qreal MAX_SCROLL_SIZE = 1.0;

static constexpr qreal SCROLL_LIMIT_OFF_OFFSET = 0.75;
static constexpr qreal SCROLL_LIMIT_ON_OFFSET = 0.02;

NotationPaintView::NotationPaintView(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
    setFlag(ItemHasContents, true);
    setFlag(ItemAcceptsDrops, true);
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

    qApp->installEventFilter(this);
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
        emit viewportChanged(viewport());
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
        moveCanvasToPosition(QPoint(0, 0));
    });
}

void NotationPaintView::moveCanvasToCenter()
{
    if (!isInited()) {
        return;
    }

    PointF canvasCenter = this->canvasCenter();
    moveCanvas(canvasCenter.x(), canvasCenter.y());
}

void NotationPaintView::scrollHorizontal(qreal position)
{
    if (position == m_previousHorizontalScrollPosition) {
        return;
    }

    qreal scrollStep = position - m_previousHorizontalScrollPosition;
    qreal dx = horizontalScrollableSize() * scrollStep;

    moveCanvasHorizontal(-dx);
}

void NotationPaintView::scrollVertical(qreal position)
{
    if (position == m_previousVerticalScrollPosition) {
        return;
    }

    qreal scrollStep = position - m_previousVerticalScrollPosition;
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
    }

    m_notation = globalContext()->currentNotation();
    if (!m_notation) {
        return;
    }

    m_notation->notationChanged().onNotify(this, [this]() {
        update();
    });

    onNoteInputChanged();

    INotationInteractionPtr interaction = notationInteraction();
    interaction->noteInput()->stateChanged().onNotify(this, [this]() {
        onNoteInputChanged();
    });
    interaction->noteInput()->noteAdded().onNotify(this, [this]() {
        onNoteInputChanged();
    });

    interaction->selectionChanged().onNotify(this, [this]() {
        onSelectionChanged();
    });

    interaction->textEditingStarted().onNotify(this, [this]() {
        if (!hasActiveFocus()) {
            emit textEdittingStarted();
        }
    });

    notationPlayback()->loopBoundaries().ch.onReceive(this, [this](const LoopBoundaries& boundaries) {
        updateLoopMarkers(boundaries);
    });

    m_loopInMarker->setStyle(m_notation->style());
    m_loopOutMarker->setStyle(m_notation->style());

    update();

    emit horizontalScrollChanged();
    emit verticalScrollChanged();
    emit viewportChanged(viewport());
}

void NotationPaintView::onViewSizeChanged()
{
    if (!notation()) {
        return;
    }

    if (viewport().isValid() && !m_inputController->isZoomInited()) {
        m_inputController->initZoom();
    }

    ensureViewportInsideScrollableArea();

    emit horizontalScrollChanged();
    emit verticalScrollChanged();
    emit viewportChanged(viewport());
}

void NotationPaintView::updateLoopMarkers(const LoopBoundaries& boundaries)
{
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
    return notation() ? notation()->playback() : nullptr;
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

void NotationPaintView::onNoteInputChanged()
{
    TRACEFUNC;

    if (isNoteEnterMode()) {
        setAcceptHoverEvents(true);
        QRectF cursorRect = notationNoteInput()->cursorRect();
        adjustCanvasPosition(cursorRect);
        emit activeFocusRequested();
    } else {
        setAcceptHoverEvents(false);
    }

    update();
}

void NotationPaintView::onSelectionChanged()
{
    if (notationSelection()->isNone()) {
        return;
    }

    TRACEFUNC;

    QRectF selectionRect = notationSelection()->canvasBoundingRect();

    adjustCanvasPosition(selectionRect);
    update();
}

bool NotationPaintView::isNoteEnterMode() const
{
    return notationNoteInput() ? notationNoteInput()->isNoteInputMode() : false;
}

void NotationPaintView::showShadowNote(const PointF& pos)
{
    notationInteraction()->showShadowNote(pos);
    update();
}

void NotationPaintView::showContextMenu(const ElementType& elementType, const QPoint& pos)
{
    emit showContextMenuRequested(static_cast<int>(elementType), pos);
}

void NotationPaintView::hideContextMenu()
{
    emit hideContextMenuRequested();
}

void NotationPaintView::paint(QPainter* qp)
{
    if (!isInited()) {
        return;
    }

    TRACEFUNC;

    mu::draw::Painter mup(qp, "notationview");
    mu::draw::Painter* painter = &mup;

    RectF rect(0.0, 0.0, width(), height());
    paintBackground(rect, painter);

    painter->setWorldTransform(mu::Transform::fromQTransform(m_matrix));

    notation()->paint(painter, toLogical(rect.toQRect()));

    m_playbackCursor->paint(painter);
    m_noteInputCursor->paint(painter);
    m_loopInMarker->paint(painter);
    m_loopOutMarker->paint(painter);
}

void NotationPaintView::onNotationSetup()
{
    m_notation = globalContext()->currentNotation();

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

    engravingConfiguration()->scoreInversionChanged().onNotify(this, [this]() {
        update();
    });

    uiConfiguration()->currentThemeChanged().onNotify(this, [this]() {
        update();
    });
}

void NotationPaintView::paintBackground(const RectF& rect, draw::Painter* painter)
{
    QString wallpaperPath = configuration()->backgroundWallpaperPath().toQString();

    if (configuration()->backgroundUseColor() || wallpaperPath.isEmpty()) {
        painter->fillRect(rect, configuration()->backgroundColor());
    } else {
        QPixmap pixmap(wallpaperPath);
        painter->drawTiledPixmap(rect, pixmap, rect.topLeft() - PointF(m_matrix.m31(), m_matrix.m32()));
    }
}

PointF NotationPaintView::canvasCenter() const
{
    QRectF canvasRect = m_matrix.mapRect(notationContentRect());

    int canvasWidth = canvasRect.width() / guiScaling();
    int canvasHeight = canvasRect.height() / guiScaling();

    int x = (width() - canvasWidth) / 2;
    int y = (height() - canvasHeight) / 2;

    return toLogical(QPoint(x, y));
}

std::pair<int, int> NotationPaintView::constraintCanvas(int dx, int dy) const
{
    QRectF scrollableArea = scrollableAreaRect();
    QRectF viewport = this->viewport();

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

QColor NotationPaintView::backgroundColor() const
{
    return configuration()->backgroundColor();
}

QRect NotationPaintView::viewport() const
{
    return toLogical(QRect(0, 0, width(), height())).toQRect();
}

QRectF NotationPaintView::notationContentRect() const
{
    if (!notationElements()) {
        return QRectF();
    }

    RectF result;
    for (const Page* page: notationElements()->pages()) {
        result = result.united(page->bbox().translated(page->pos()));
    }

    return result.toQRectF();
}

QRectF NotationPaintView::scrollableAreaRect() const
{
    QRectF viewport = this->viewport();
    qreal overscrollFactor = configuration()->isLimitCanvasScrollArea() ? SCROLL_LIMIT_ON_OFFSET : SCROLL_LIMIT_OFF_OFFSET;

    qreal overscrollX = viewport.width() * overscrollFactor;
    qreal overscrollY = viewport.height() * overscrollFactor;

    return notationContentRect().adjusted(-overscrollX, -overscrollY, overscrollX, overscrollY);
}

qreal NotationPaintView::horizontalScrollableAreaSize() const
{
    if (viewport().left() < scrollableAreaRect().left()
        && viewport().right() > scrollableAreaRect().right()) {
        return 0;
    }

    qreal scrollableWidth = horizontalScrollableSize();
    if (qFuzzyIsNull(scrollableWidth)) {
        return 0;
    }

    return viewport().width() / scrollableWidth;
}

qreal NotationPaintView::horizontalScrollableSize() const
{
    QRectF scrollableArea = scrollableAreaRect();

    qreal left = scrollableArea.left();
    qreal right = scrollableArea.right();

    qreal size = 0;
    if ((left < 0) && (right > 0)) {
        size = std::abs(left) + right;
    } else {
        size = std::abs(right) - std::abs(left);
    }

    return size;
}

qreal NotationPaintView::verticalScrollableAreaSize() const
{
    if (viewport().top() < scrollableAreaRect().top()
        && viewport().bottom() > scrollableAreaRect().bottom()) {
        return 0;
    }

    qreal scrollableHeight = verticalScrollableSize();
    if (qFuzzyIsNull(scrollableHeight)) {
        return 0;
    }

    return viewport().height() / scrollableHeight;
}

qreal NotationPaintView::verticalScrollableSize() const
{
    QRectF scrollableArea = scrollableAreaRect();

    qreal top = scrollableArea.top();
    qreal bottom = scrollableArea.bottom();

    qreal size = 0;
    if ((top < 0) && (bottom > 0)) {
        size = std::abs(top) + bottom;
    } else {
        size = std::abs(bottom) - std::abs(top);
    }

    return size;
}

void NotationPaintView::adjustCanvasPosition(const QRectF& logicRect)
{
    QRectF viewRect = viewport();
    QRectF showRect = logicRect;

    if (viewRect.contains(showRect)) {
        return;
    }

    constexpr int BORDER_SPACING_RATIO = 3;

    double _spatium = notationStyle()->styleValue(StyleId::spatium).toDouble();
    qreal border = _spatium * BORDER_SPACING_RATIO;
    qreal _scale = currentScaling();
    if (qFuzzyIsNull(_scale)) {
        _scale = 1;
    }

    QPointF pos = QPointF(viewRect.topLeft().x(), viewRect.topLeft().y());
    QPointF oldPos = pos;

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
        return;
    }

    moveCanvasToPosition(pos.toPoint());

    update();
}

void NotationPaintView::ensureViewportInsideScrollableArea()
{
    auto [dx, dy] = constraintCanvas(0, 0);
    if (dx == 0 && dy == 0) {
        return;
    }

    m_matrix.translate(dx, dy);
    update();
}

void NotationPaintView::moveCanvasToPosition(const QPoint& logicPos)
{
    PointF viewTopLeft = toLogical(QPoint(0, 0));
    moveCanvas(viewTopLeft.x() - logicPos.x(), viewTopLeft.y() - logicPos.y());
}

bool NotationPaintView::moveCanvas(int dx, int dy)
{
    if (dx == 0 && dy == 0) {
        return false;
    }

    auto [correctedDX, correctedDY] = constraintCanvas(dx, dy);
    if (correctedDX == 0 && correctedDY == 0) {
        return false;
    }

    m_matrix.translate(correctedDX, correctedDY);
    update();

    emit horizontalScrollChanged();
    emit verticalScrollChanged();
    emit viewportChanged(viewport());

    return true;
}

void NotationPaintView::moveCanvasVertical(int dy)
{
    moveCanvas(0, dy);
}

void NotationPaintView::moveCanvasHorizontal(int dx)
{
    moveCanvas(dx, 0);
}

qreal NotationPaintView::currentScaling() const
{
    return m_matrix.m11();
}

void NotationPaintView::setScaling(qreal scaling, const QPoint& pos)
{
    qreal currentScaling = this->currentScaling();

    if (qFuzzyCompare(currentScaling, scaling)) {
        return;
    }

    if (qFuzzyIsNull(currentScaling)) {
        currentScaling = 1;
    }

    qreal deltaScaling = scaling / currentScaling;
    scale(deltaScaling, pos);
}

void NotationPaintView::scale(qreal factor, const QPoint& pos)
{
    if (qFuzzyCompare(factor, 1.0)) {
        return;
    }

    PointF pointBeforeScaling = toLogical(pos);

    m_matrix.scale(factor, factor);

    PointF pointAfterScaling = toLogical(pos);

    int dx = pointAfterScaling.x() - pointBeforeScaling.x();
    int dy = pointAfterScaling.y() - pointBeforeScaling.y();

    // If canvas has moved, moveCanvas will call update();
    // Otherwise, it needs to be called here
    if (!moveCanvas(dx, dy)) {
        update();
    }
}

void NotationPaintView::wheelEvent(QWheelEvent* event)
{
    if (isInited()) {
        m_inputController->wheelEvent(event);
    }
}

void NotationPaintView::mousePressEvent(QMouseEvent* event)
{
    setFocus(true);
    emit activeFocusRequested();
    forceActiveFocus();

    if (isInited()) {
        m_inputController->mousePressEvent(event);
    }
}

void NotationPaintView::mouseMoveEvent(QMouseEvent* event)
{
    if (isInited()) {
        m_inputController->mouseMoveEvent(event);
    }
}

void NotationPaintView::mouseDoubleClickEvent(QMouseEvent* event)
{
    forceActiveFocus();

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

void NotationPaintView::shortcutOverride(QKeyEvent* event)
{
    if (isInited()) {
        m_inputController->keyPressEvent(event);
    }
}

bool NotationPaintView::event(QEvent* ev)
{
    if (ev->type() == QEvent::Type::ShortcutOverride) {
        shortcutOverride(dynamic_cast<QKeyEvent*>(ev));
    }
    return QQuickPaintedItem::event(ev);
}

bool NotationPaintView::eventFilter(QObject* obj, QEvent* ev)
{
    if (hasFocus() && ev->type() == QEvent::Type::ContextMenu) {
        if (m_inputController->selectionType() == ElementType::PAGE) {
            showContextMenu(m_inputController->selectionType(), QPoint(width() / 2, height() / 2));
        } else {
            showContextMenu(m_inputController->selectionType(), m_matrix.map(m_inputController->hitElementPos().toQPoint()));
        }
    }
    return QObject::eventFilter(obj, ev);
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
    m_matrix = QTransform();
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

PointF NotationPaintView::canvasPos() const
{
    return PointF(m_matrix.dx(), m_matrix.dy());
}

PointF NotationPaintView::toLogical(const QPoint& point) const
{
    double scale = guiScaling();
    PointF scaledPoint(point.x() * scale, point.y() * scale);

    return PointF::fromQPointF(m_matrix.inverted().map(scaledPoint.toQPoint()));
}

double NotationPaintView::guiScaling() const
{
    return configuration()->guiScaling();
}

RectF NotationPaintView::toLogical(const QRect& rect) const
{
    double scale = guiScaling();

    QRect scaledRect = rect;
    scaledRect.setBottomRight(rect.bottomRight() * scale);

    return RectF::fromQRectF(m_matrix.inverted().mapRect(scaledRect));
}

bool NotationPaintView::isInited() const
{
    if (qFuzzyIsNull(width()) || qFuzzyIsNull(height())) {
        return false;
    }

    return notation() != nullptr;
}

qreal NotationPaintView::startHorizontalScrollPosition() const
{
    QRectF viewport = this->viewport();
    QRectF scrollableArea = scrollableAreaRect();
    if (!viewport.isValid() || !scrollableArea.isValid()) {
        return 0.0;
    }

    qreal position = (viewport.left() - scrollableArea.left()) / scrollableArea.width();
    return qBound(0.0, position, MAX_SCROLL_SIZE);
}

qreal NotationPaintView::horizontalScrollSize() const
{
    qreal area = horizontalScrollableAreaSize();
    if (qFuzzyIsNull(area)) {
        return 0;
    }

    qreal size = std::max(area, MIN_SCROLL_SIZE);
    return size;
}

qreal NotationPaintView::startVerticalScrollPosition() const
{
    QRectF viewport = this->viewport();
    QRectF scrollableArea = scrollableAreaRect();
    if (!viewport.isValid() || !scrollableArea.isValid()) {
        return 0.0;
    }

    qreal position = (viewport.top() - scrollableArea.top()) / scrollableArea.height();
    return qBound(0.0, position, MAX_SCROLL_SIZE);
}

qreal NotationPaintView::verticalScrollSize() const
{
    qreal area = verticalScrollableAreaSize();
    if (qFuzzyIsNull(area)) {
        return 0;
    }

    qreal size = std::max(area, MIN_SCROLL_SIZE);
    return size;
}

void NotationPaintView::onPlayingChanged()
{
    if (!notationPlayback()) {
        return;
    }

    TRACEFUNC;

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
    if (!notationPlayback()) {
        return;
    }

    TRACEFUNC;

    QRect cursorRect = notationPlayback()->playbackCursorRectByTick(tick);
    m_playbackCursor->setRect(RectF::fromQRectF(cursorRect));

    if (configuration()->isAutomaticallyPanEnabled()) {
        adjustCanvasPosition(cursorRect);
    }

    update(); //! TODO set rect to optimization
}

const Page* NotationPaintView::pointToPage(const PointF& point) const
{
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

QPointF NotationPaintView::alignToCurrentPageBorder(const QRectF& showRect, const QPointF& pos) const
{
    QPointF result = pos;
    const Page* page = pointToPage(PointF::fromQPointF(showRect.topLeft()));
    if (!page) {
        return result;
    }

    QRectF viewRect = viewport();

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
