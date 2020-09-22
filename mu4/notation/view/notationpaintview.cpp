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
#include <cmath>

#include "log.h"
#include "notationviewinputcontroller.h"
#include "actions/actiontypes.h"

using namespace mu::notation;
using namespace mu::notation;

static constexpr int PREF_UI_CANVAS_MISC_SELECTIONPROXIMITY = 6;

NotationPaintView::NotationPaintView()
    : QQuickPaintedItem()
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
    configuration()->backgroundColorChanged().onReceive(this, [this](const QColor& c) {
        m_backgroundColor = c;
        update();
    });

    // notation
    m_notation = globalContext()->currentNotation();
    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        onCurrentNotationChanged();
    });

    // test
    dispatcher()->reg(this, "copy", [this](const actions::ActionName&) {
        LOGI() << "NotationPaintView copy";
    });
}

NotationPaintView::~NotationPaintView()
{
    delete m_inputController;
    delete m_playbackCursor;
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
        INotationInteraction* ninteraction = m_notation->interaction();
        ninteraction->inputStateChanged().resetOnNotify(this);
        ninteraction->selectionChanged().resetOnNotify(this);
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
    INotationInteraction* ninteraction = notationInteraction();
    ninteraction->inputStateChanged().onNotify(this, [this]() {
        onInputStateChanged();
    });
    ninteraction->selectionChanged().onNotify(this, [this]() {
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
    INotationInputState* is = notationInteraction()->inputState();
    if (is->isNoteEnterMode()) {
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

void NotationPaintView::paint(QPainter* p)
{
    QRect rect(0, 0, width(), height());
    p->fillRect(rect, m_backgroundColor);

    p->setTransform(m_matrix);

    if (m_notation) {
        m_notation->paint(p);

        m_playbackCursor->paint(p);
    } else {
        p->drawText(10, 10, "no notation");
    }
}

qreal NotationPaintView::xoffset() const
{
    return m_matrix.dx();
}

qreal NotationPaintView::yoffset() const
{
    return m_matrix.dy();
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
    m_matrix.translate(0, dy);
    update();
}

void NotationPaintView::scrollHorizontal(int dx)
{
    m_matrix.translate(dx, 0);
    update();
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

    m_matrix.translate(dx, dy);

    update();
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

QPoint NotationPaintView::toLogical(const QPoint& p) const
{
    return m_matrix.inverted().map(p);
}

QRect NotationPaintView::toLogical(const QRect& r) const
{
    return m_matrix.inverted().mapRect(r);
}

QPoint NotationPaintView::toPhysical(const QPoint& p) const
{
    return m_matrix.map(p);
}

bool NotationPaintView::isInited() const
{
    if (m_notation) {
        return true;
    }
    return false;
}

INotationInteraction* NotationPaintView::notationInteraction() const
{
    if (m_notation) {
        return m_notation->interaction();
    }
    return nullptr;
}

INotationPlayback* NotationPaintView::notationPlayback() const
{
    if (m_notation) {
        return m_notation->playback();
    }
    return nullptr;
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
