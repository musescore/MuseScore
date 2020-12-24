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

    setZoom(configuration()->currentZoom().val, QPoint());

    connect(this, &QQuickPaintedItem::widthChanged, this, &NotationPaintView::onViewSizeChanged);
    connect(this, &QQuickPaintedItem::heightChanged, this, &NotationPaintView::onViewSizeChanged);

    connect(this, &NotationPaintView::horizontalScrollChanged, [this]() {
        m_previousHorizontalScrollPosition = startHorizontalScrollPosition();
    });
    connect(this, &NotationPaintView::verticalScrollChanged, [this]() {
        m_previousVerticalScrollPosition = startVerticalScrollPosition();
    });

    // input
    m_inputController = new NotationViewInputController(this);

    // playback
    m_playbackCursor = new PlaybackCursor();
    m_playbackCursor->setVisible(false);

    playbackController()->isPlayingChanged().onNotify(this, [this]() {
        onPlayingChanged();
    });

    playbackController()->midiTickPlayed().onReceive(this, [this](uint32_t tick) {
        movePlaybackCursor(tick);
    });

    // note input
    m_noteInputCursor = new NoteInputCursor();

    // configuration
    m_backgroundColor = configuration()->backgroundColor();
    configuration()->backgroundColorChanged().onReceive(this, [this](const QColor& color) {
        m_backgroundColor = color;
        update();
    });

    // notation
    m_notation = globalContext()->currentNotation();
    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        onCurrentNotationChanged();
    });
}

NotationPaintView::~NotationPaintView()
{
    delete m_inputController;
    delete m_playbackCursor;
    delete m_noteInputCursor;
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

void NotationPaintView::handleAction(const QString& actionName)
{
    dispatcher()->dispatch(actionName.toStdString());
}

bool NotationPaintView::canReceiveAction(const actions::ActionName& action) const
{
    if (action == "file-open") {
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
    if (!currentNotation()) {
        return;
    }

    QPoint p1 = toLogical(QPoint(0, 0));
    QPoint p2 = toLogical(QPoint(width(), height()));
    currentNotation()->setViewSize(QSizeF(p2.x() - p1.x(), p2.y() - p1.y()));
}

INotationPtr NotationPaintView::currentNotation() const
{
    return m_notation;
}

INotationNoteInputPtr NotationPaintView::currentNotationNoteInput() const
{
    auto notation = currentNotation();
    if (!notation) {
        return nullptr;
    }

    auto interaction = notation->interaction();
    if (!interaction) {
        return nullptr;
    }

    return interaction->noteInput();
}

INotationElementsPtr NotationPaintView::currentNotationElements() const
{
    auto notation = currentNotation();
    if (!notation) {
        return nullptr;
    }

    return notation->elements();
}

INotationStylePtr NotationPaintView::currentNotationStyle() const
{
    auto notation = currentNotation();
    if (!notation) {
        return nullptr;
    }

    return notation->style();
}

void NotationPaintView::onNoteInputChanged()
{
    if (currentNotationNoteInput()->isNoteInputMode()) {
        setAcceptHoverEvents(true);
        QRectF cursorRect = currentNotationNoteInput()->cursorRect();
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
    if (!currentNotation()) {
        return false;
    }

    return notationInteraction()->noteInput()->isNoteInputMode();
}

void NotationPaintView::showShadowNote(const QPointF& pos)
{
    notationInteraction()->showShadowNote(pos);
    update();
}

void NotationPaintView::showContextMenu(const ElementType& elementType, const QPoint& pos)
{
    INotationActionsRepositoryPtr actionsRepository = actionsFactory()->actionsRepository(elementType);

    QVariantList menuItems;

    for (const actions::Action& action: actionsRepository->actions()) {
        QVariantMap actionObj;
        actionObj["name"] = QString::fromStdString(action.name);
        actionObj["title"] = QString::fromStdString(action.title);
        actionObj["icon"] = action.iconCode != IconCode::Code::NONE ? static_cast<int>(action.iconCode) : 0;

        shortcuts::Shortcut shortcut = shortcutsRegister()->shortcut(action.name);
        if (shortcut.isValid()) {
            actionObj["shortcut"] = QString::fromStdString(shortcut.sequence);
        }

        menuItems << actionObj;
    }

    emit openContextMenuRequested(menuItems, pos);
}

void NotationPaintView::paint(QPainter* painter)
{
    QRect rect(0, 0, width(), height());
    painter->fillRect(rect, m_backgroundColor);

    painter->setTransform(m_matrix);

    if (currentNotation()) {
        currentNotation()->paint(painter, toLogical(rect));

        m_playbackCursor->paint(painter);
        m_noteInputCursor->paint(painter);
    } else {
        painter->drawText(10, 10, "no notation");
    }
}

QRect NotationPaintView::viewport() const
{
    return toLogical(QRect(0, 0, width(), height()));
}

QRectF NotationPaintView::notationContentRect() const
{
    if (!m_notation) {
        return QRectF();
    }

    auto notationElements = m_notation->elements();
    if (!notationElements) {
        return QRectF();
    }

    QRectF result;
    for (const Page* page: notationElements->pages()) {
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

    constexpr int borderSpacingRatio = 3;

    double _spatium = currentNotationStyle()->styleValue(StyleId::spatium).toDouble();
    qreal border = _spatium * borderSpacingRatio;
    qreal _scale = scale();
    if (qFuzzyIsNull(_scale)) {
        _scale = 1;
    }

    QPointF pos = QPointF(viewRect.topLeft().x(), viewRect.topLeft().y());
    QPointF oldPos = pos;

    if (showRect.left() < viewRect.left()) {
        pos.setX(showRect.left() - border);
    } else if (showRect.left() > viewRect.right()) {
        pos.setX(showRect.right() - width() / scale() + border);
    } else if (viewRect.width() >= showRect.width() && showRect.right() > viewRect.right()) {
        pos.setX(showRect.left() - border);
    }

    if (showRect.top() < viewRect.top() && showRect.bottom() < viewRect.bottom()) {
        pos.setY(showRect.top() - border);
    } else if (showRect.top() > viewRect.bottom()) {
        pos.setY(showRect.bottom() - height() / scale() + border);
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
    QPoint viewTL = toLogical(QPoint(0, 0));
    moveCanvas(viewTL.x() - logicPos.x(), viewTL.y() - logicPos.y());
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

void NotationPaintView::setZoom(int zoomPercentage, const QPoint& pos)
{
    qreal newScale = static_cast<qreal>(zoomPercentage) / 100.0 * configuration()->notationScaling();
    qreal currentScale = scale();

    if (qFuzzyCompare(newScale, currentScale)) {
        return;
    }

    if (qFuzzyIsNull(currentScale)) {
        currentScale = 1;
    }

    QPoint pointBeforeScaling = toLogical(pos);

    qreal deltaScale = newScale / currentScale;
    m_matrix.scale(deltaScale, deltaScale);

    QPoint pointAfterScaling = toLogical(pos);

    int dx = pointAfterScaling.x() - pointBeforeScaling.x();
    int dy = pointAfterScaling.y() - pointBeforeScaling.y();

    moveCanvas(dx, dy);
}

void NotationPaintView::wheelEvent(QWheelEvent* ev)
{
    if (!isInited()) {
        return;
    }
    m_inputController->wheelEvent(ev);
}

void NotationPaintView::mousePressEvent(QMouseEvent* ev)
{
    setFocus(true);

    if (!isInited()) {
        return;
    }
    m_inputController->mousePressEvent(ev);
}

void NotationPaintView::mouseMoveEvent(QMouseEvent* ev)
{
    if (!isInited()) {
        return;
    }
    m_inputController->mouseMoveEvent(ev);
}

void NotationPaintView::mouseDoubleClickEvent(QMouseEvent* event)
{
    forceActiveFocus();
    if (!isInited()) {
        return;
    }
    m_inputController->mouseDoubleClickEvent(event);
}

void NotationPaintView::mouseReleaseEvent(QMouseEvent* ev)
{
    if (!isInited()) {
        return;
    }
    m_inputController->mouseReleaseEvent(ev);
}

void NotationPaintView::hoverMoveEvent(QHoverEvent* ev)
{
    if (!isInited()) {
        return;
    }
    m_inputController->hoverMoveEvent(ev);
}

void NotationPaintView::keyReleaseEvent(QKeyEvent* event)
{
    if (!isInited()) {
        return;
    }
    m_inputController->keyReleaseEvent(event);
}

void NotationPaintView::dragEnterEvent(QDragEnterEvent* ev)
{
    if (!isInited()) {
        return;
    }
    m_inputController->dragEnterEvent(ev);
}

void NotationPaintView::dragLeaveEvent(QDragLeaveEvent* ev)
{
    if (!isInited()) {
        return;
    }
    m_inputController->dragLeaveEvent(ev);
}

void NotationPaintView::dragMoveEvent(QDragMoveEvent* ev)
{
    if (!isInited()) {
        return;
    }
    m_inputController->dragMoveEvent(ev);
}

void NotationPaintView::dropEvent(QDropEvent* ev)
{
    if (!isInited()) {
        return;
    }
    m_inputController->dropEvent(ev);
}

QPoint NotationPaintView::toLogical(const QPoint& point) const
{
    double scale = guiScale();
    QPoint scaledPoint(point.x() * scale, point.y() * scale);

    return m_matrix.inverted().map(scaledPoint);
}

double NotationPaintView::guiScale() const
{
    return configuration()->guiScaling();
}

QRect NotationPaintView::toLogical(const QRect& rect) const
{
    double scale = guiScale();

    QRect scaledRect = rect;
    scaledRect.setBottomRight(rect.bottomRight() * scale);

    return m_matrix.inverted().mapRect(scaledRect);
}

bool NotationPaintView::isInited() const
{
    return currentNotation() != nullptr;
}

INotationInteractionPtr NotationPaintView::notationInteraction() const
{
    auto notation = currentNotation();
    if (!notation) {
        return nullptr;
    }

    return notation->interaction();
}

INotationPlaybackPtr NotationPaintView::notationPlayback() const
{
    auto notation = currentNotation();
    if (!notation) {
        return nullptr;
    }

    return notation->playback();
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

qreal NotationPaintView::width() const
{
    return QQuickPaintedItem::width();
}

qreal NotationPaintView::height() const
{
    return QQuickPaintedItem::height();
}

qreal NotationPaintView::scale() const
{
    return m_matrix.m11();
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

const Page* NotationPaintView::point2page(const QPointF& p) const
{
    if (!currentNotation() || !currentNotationElements()) {
        return nullptr;
    }

    PageList pages = currentNotationElements()->pages();
    if (currentNotation()->viewMode() == Ms::LayoutMode::LINE) {
        return pages.empty() ? 0 : pages.front();
    }

    for (const Page* page: pages) {
        if (page->bbox().translated(page->pos()).contains(p)) {
            return page;
        }
    }

    return nullptr;
}

QPointF NotationPaintView::alignToCurrentPageBorder(const QRectF& showRect, const QPointF& pos) const
{
    QPointF result = pos;
    const Page* page = point2page(showRect.topLeft().toPoint());
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
