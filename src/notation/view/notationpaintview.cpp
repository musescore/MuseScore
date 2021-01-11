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
#include "notationpaintview.h"

#include <QPainter>

#include "log.h"
#include "actions/actiontypes.h"

using namespace mu::notation;
using namespace mu::framework;

static constexpr qreal MIN_SCROLL_SIZE = 0.2;
static constexpr qreal MAX_SCROLL_SIZE = 1.0;

static constexpr qreal CANVAS_SIDE_MARGIN = 8000;

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
}

void NotationPaintView::load()
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

    initBackground();
}

void NotationPaintView::initBackground()
{
    m_backgroundColor = configuration()->backgroundColor();

    configuration()->backgroundColorChanged().onReceive(this, [this](const QColor& color) {
        m_backgroundColor = color;
        update();
    });
}

void NotationPaintView::moveCanvasToCenter()
{
    if (!isInited()) {
        return;
    }

    QRectF canvasRect = m_matrix.mapRect(notationContentRect());

    int canvasWidth = canvasRect.width() / guiScaling();
    int canvasHeight = canvasRect.height() / guiScaling();

    int dx = (width() - canvasWidth) / 2;
    int dy = (height() - canvasHeight) / 2;

    QPoint newTopLeft = toLogical(QPoint(dx, dy));

    moveCanvas(newTopLeft.x(), newTopLeft.y());
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

void NotationPaintView::handleAction(const QString& actionCode)
{
    dispatcher()->dispatch(actionCode.toStdString());
}

bool NotationPaintView::canReceiveAction(const actions::ActionCode& actionCode) const
{
    if (actionCode == "file-open") {
        return true;
    }

    return hasFocus();
}

void NotationPaintView::onCurrentNotationChanged()
{
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

    onViewSizeChanged(); //! NOTE Set view size to notation

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

    update();
}

void NotationPaintView::onViewSizeChanged()
{
    if (!notation()) {
        return;
    }

    QPoint topLeft = toLogical(QPoint(0, 0));
    QPoint bottomRight = toLogical(QPoint(width(), height()));

    notation()->setViewSize(QSizeF(bottomRight.x() - topLeft.x(), bottomRight.y() - topLeft.y()));
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

void NotationPaintView::onNoteInputChanged()
{
    if (isNoteEnterMode()) {
        setAcceptHoverEvents(true);
        QRectF cursorRect = notationNoteInput()->cursorRect();
        adjustCanvasPosition(cursorRect);
    } else {
        setAcceptHoverEvents(false);
    }

    update();
}

void NotationPaintView::onSelectionChanged()
{
    if (notationInteraction()->selection()->isNone()) {
        return;
    }

    QRectF selRect = notationInteraction()->selection()->canvasBoundingRect();

    adjustCanvasPosition(selRect);
    update();
}

bool NotationPaintView::isNoteEnterMode() const
{
    return notationNoteInput() ? notationNoteInput()->isNoteInputMode() : false;
}

void NotationPaintView::showShadowNote(const QPointF& pos)
{
    notationInteraction()->showShadowNote(pos);
    update();
}

void NotationPaintView::showContextMenu(const ElementType& elementType, const QPoint& pos)
{
    QVariantList menuItems;

    for (const MenuItem& menuItem: notationContextMenu()->items(elementType)) {
        menuItems << menuItem.toVariantMap();
    }

    emit openContextMenuRequested(menuItems, pos);
}

void NotationPaintView::paint(QPainter* painter)
{
    if (!notation()) {
        return;
    }

    QRect rect(0, 0, width(), height());
    painter->fillRect(rect, m_backgroundColor);

    painter->setTransform(m_matrix);

    notation()->paint(painter, toLogical(rect));

    m_playbackCursor->paint(painter);
    m_noteInputCursor->paint(painter);
}

QRect NotationPaintView::viewport() const
{
    return toLogical(QRect(0, 0, width(), height()));
}

QRectF NotationPaintView::notationContentRect() const
{
    if (!notationElements()) {
        return QRectF();
    }

    QRectF result;
    for (const Page* page: notationElements()->pages()) {
        result = result.united(page->bbox().translated(page->pos()));
    }

    return result;
}

QRectF NotationPaintView::canvasRect() const
{
    QRectF result = notationContentRect();
    result.adjust(-CANVAS_SIDE_MARGIN, -CANVAS_SIDE_MARGIN, CANVAS_SIDE_MARGIN, CANVAS_SIDE_MARGIN);
    return result;
}

qreal NotationPaintView::horizontalScrollableAreaSize() const
{
    if (viewport().left() < notationContentRect().left()
        && viewport().right() > notationContentRect().right()) {
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
    QRect contentRect = notationContentRect().toRect();

    qreal left = std::min(viewport().left(), contentRect.left());
    qreal right = std::max(viewport().right(), contentRect.right());

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
    if (viewport().top() < notationContentRect().top()
        && viewport().bottom() > notationContentRect().bottom()) {
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
    QRect contentRect = notationContentRect().toRect();

    qreal top = std::min(viewport().top(), contentRect.top());
    qreal bottom = std::max(viewport().bottom(), contentRect.bottom());

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

void NotationPaintView::moveCanvasToPosition(const QPoint& logicPos)
{
    QPoint viewTopLeft = toLogical(QPoint(0, 0));
    moveCanvas(viewTopLeft.x() - logicPos.x(), viewTopLeft.y() - logicPos.y());
}

void NotationPaintView::moveCanvas(int dx, int dy)
{
    m_matrix.translate(dx, dy);
    update();

    emit horizontalScrollChanged();
    emit verticalScrollChanged();
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

void NotationPaintView::scale(qreal scaling, const QPoint& pos)
{
    qreal currentScaling = this->currentScaling();

    if (qFuzzyCompare(currentScaling, scaling)) {
        return;
    }

    if (qFuzzyIsNull(currentScaling)) {
        currentScaling = 1;
    }

    QPoint pointBeforeScaling = toLogical(pos);

    qreal deltaScaling = scaling / currentScaling;
    m_matrix.scale(deltaScaling, deltaScaling);

    QPoint pointAfterScaling = toLogical(pos);

    int dx = pointAfterScaling.x() - pointBeforeScaling.x();
    int dy = pointAfterScaling.y() - pointBeforeScaling.y();

    moveCanvas(dx, dy);
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

void NotationPaintView::keyReleaseEvent(QKeyEvent* event)
{
    if (isInited()) {
        m_inputController->keyReleaseEvent(event);
    }
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
    initBackground();
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

QPoint NotationPaintView::toLogical(const QPoint& point) const
{
    double scale = guiScaling();
    QPoint scaledPoint(point.x() * scale, point.y() * scale);

    return m_matrix.inverted().map(scaledPoint);
}

double NotationPaintView::guiScaling() const
{
    return configuration()->guiScaling();
}

QRect NotationPaintView::toLogical(const QRect& rect) const
{
    double scale = guiScaling();

    QRect scaledRect = rect;
    scaledRect.setBottomRight(rect.bottomRight() * scale);

    return m_matrix.inverted().mapRect(scaledRect);
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
    QRectF contentRect = notationContentRect();
    if (!contentRect.isValid()) {
        return 0;
    }

    if (viewport().left() < contentRect.left()) {
        return 0;
    }

    if (viewport().right() > contentRect.right()) {
        return MAX_SCROLL_SIZE - horizontalScrollableAreaSize();
    }

    qreal position = viewport().left() / contentRect.width();
    return std::abs(position);
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
    QRectF contentRect = notationContentRect();
    if (!contentRect.isValid()) {
        return 0;
    }

    if (viewport().top() < contentRect.top()) {
        return 0;
    }

    if (viewport().bottom() > contentRect.bottom()) {
        return MAX_SCROLL_SIZE - verticalScrollableAreaSize();
    }

    qreal position = viewport().top() / contentRect.height();
    return std::abs(position);
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

    bool isPlaying = playbackController()->isPlaying();
    m_playbackCursor->setVisible(isPlaying);

    if (isPlaying) {
        float playPosSec = playbackController()->playbackPosition();
        int tick = notationPlayback()->secToTick(playPosSec);
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

    //LOGI() << "tick: " << tick;
    QRect rec = notationPlayback()->playbackCursorRectByTick(tick);
    m_playbackCursor->move(rec);

    adjustCanvasPosition(rec);
    update(); //! TODO set rect to optimization
}

const Page* NotationPaintView::pointToPage(const QPointF& point) const
{
    if (!notationElements()) {
        return nullptr;
    }

    PageList pages = notationElements()->pages();
    if (notation()->viewMode() == Ms::LayoutMode::LINE) {
        return pages.empty() ? 0 : pages.front();
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
    const Page* page = pointToPage(showRect.topLeft().toPoint());
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
