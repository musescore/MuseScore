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

NotationPaintView::NotationPaintView(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
    setFlag(ItemHasContents, true);
    setFlag(ItemAcceptsDrops, true);
    setAcceptedMouseButtons(Qt::AllButtons);
    setAntialiasing(true);

    // view
    //! TODO
    double mag  = 0.267;//preferences.getDouble(PREF_SCORE_MAGNIFICATION) * (mscore->physicalDotsPerInch() / DPI);
    m_matrix = QTransform::fromScale(mag, mag);
    connect(this, &QQuickPaintedItem::widthChanged, this, &NotationPaintView::onViewSizeChanged);
    connect(this, &QQuickPaintedItem::heightChanged, this, &NotationPaintView::onViewSizeChanged);

    // input
    m_inputController = new NotationViewInputController(this);

    // playback
    m_playbackCursor = new PlaybackCursor();
    m_playbackCursor->setColor(configuration()->playbackCursorColor());
    m_playbackCursor->setVisible(false);

    playbackController()->isPlayingChanged().onNotify(this, [this]() {
        onPlayingChanged();
    });

    playbackController()->midiTickPlayed().onReceive(this, [this](uint32_t tick) {
        movePlaybackCursor(tick);
    });

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

    // test
    dispatcher()->reg(this, "copy", [](const actions::ActionName&) {
        LOGI() << "NotationPaintView copy";
    });
}

NotationPaintView::~NotationPaintView()
{
    delete m_inputController;
    delete m_playbackCursor;
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
        interaction->inputStateChanged().resetOnNotify(this);
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

    onInputStateChanged();

    INotationInteractionPtr interaction = notationInteraction();
    interaction->inputStateChanged().onNotify(this, [this]() {
        onInputStateChanged();
    });

    interaction->selectionChanged().onNotify(this, [this]() {
        onSelectionChanged();
    });

    update();
}

void NotationPaintView::onViewSizeChanged()
{
    if (!m_notation) {
        return;
    }

    QPoint p1 = toLogical(QPoint(0, 0));
    QPoint p2 = toLogical(QPoint(width(), height()));
    m_notation->setViewSize(QSizeF(p2.x() - p1.x(), p2.y() - p1.y()));
}

void NotationPaintView::onInputStateChanged()
{
    if (notationInteraction()->inputState()->isNoteEnterMode()) {
        setAcceptHoverEvents(true);
    } else {
        setAcceptHoverEvents(false);
    }

    update();
}

void NotationPaintView::onSelectionChanged()
{
    QRectF selRect = notationInteraction()->selection()->canvasBoundingRect();

    adjustCanvasPosition(selRect);
    update();
}

bool NotationPaintView::isNoteEnterMode() const
{
    if (!m_notation) {
        return false;
    }

    return notationInteraction()->inputState()->isNoteEnterMode();
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

    if (m_notation) {
        m_notation->paint(painter, toLogical(rect));

        m_playbackCursor->paint(painter);
    } else {
        painter->drawText(10, 10, "no notation");
    }
}

QRect NotationPaintView::viewport() const
{
    return toLogical(QRect(0, 0, width(), height()));
}

void NotationPaintView::adjustCanvasPosition(const QRectF& logicRect)
{
    //! TODO This is very simple adjustment of position.
    //! Need to port the logic from ScoreView::adjustCanvasPosition
    QPoint posTL = logicRect.topLeft().toPoint();

    QRect viewRect = viewport();
    if (viewRect.contains(posTL)) {
        return;
    }

    posTL.setX(posTL.x() - 300);
    posTL.setY(posTL.y() - 300);
    moveCanvasToPosition(posTL);
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
}

void NotationPaintView::scrollVertical(int dy)
{
    moveCanvas(0, dy);
}

void NotationPaintView::scrollHorizontal(int dx)
{
    moveCanvas(dx, 0);
}

void NotationPaintView::setZoom(int zoomPercentage, const QPoint& pos)
{
    //! TODO Zoom to point not completed
    qreal mag = static_cast<qreal>(zoomPercentage) / 100.0;
    qreal cmag = m_matrix.m11();

    if (qFuzzyCompare(mag, cmag)) {
        return;
    }

    qreal deltamag = mag / cmag;

    QPointF p1 = m_matrix.inverted().map(pos);

    m_matrix.setMatrix(mag, m_matrix.m12(), m_matrix.m13(), m_matrix.m21(),
                       mag, m_matrix.m23(), m_matrix.dx() * deltamag, m_matrix.dy() * deltamag, m_matrix.m33());

    QPointF p2 = m_matrix.inverted().map(pos);
    QPointF p3 = p2 - p1;
    int dx = std::lrint(p3.x() * cmag);
    int dy = std::lrint(p3.y() * cmag);

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
    m_inputController->keyPressEvent(event);
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
    double scale = configuration()->guiScaling();
    QPoint scaledPoint(point.x() * scale, point.y() * scale);

    return m_matrix.inverted().map(scaledPoint);
}

QRect NotationPaintView::toLogical(const QRect& rect) const
{
    return m_matrix.inverted().mapRect(rect);
}

bool NotationPaintView::isInited() const
{
    if (m_notation) {
        return true;
    }
    return false;
}

INotationInteractionPtr NotationPaintView::notationInteraction() const
{
    return m_notation ? m_notation->interaction() : nullptr;
}

INotationPlaybackPtr NotationPaintView::notationPlayback() const
{
    return m_notation ? m_notation->playback() : nullptr;
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
    return QQuickPaintedItem::scale();
}

void NotationPaintView::onPlayingChanged()
{
    if (!m_notation) {
        return;
    }
    bool isPlaying = playbackController()->isPlaying();
    m_playbackCursor->setVisible(isPlaying);

    if (isPlaying) {
        float playPosSec = playbackController()->playbackPosition();
        int tick = m_notation->playback()->secToTick(playPosSec);
        movePlaybackCursor(tick);
    } else {
        update();
    }
}

void NotationPaintView::movePlaybackCursor(uint32_t tick)
{
    if (!m_notation) {
        return;
    }
    //LOGI() << "tick: " << tick;
    QRect rec = m_notation->playback()->playbackCursorRectByTick(tick);
    m_playbackCursor->move(rec);
    update(); //! TODO set rect to optimization
}
