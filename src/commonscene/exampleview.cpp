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

#include "exampleview.h"

#include <cmath>
#include <QMimeData>

#include "engraving/rw/xml.h"

#include "libmscore/masterscore.h"
#include "libmscore/engravingitem.h"
#include "libmscore/page.h"
#include "libmscore/system.h"
#include "libmscore/actionicon.h"
#include "libmscore/chord.h"
#include "libmscore/factory.h"

#include "commonscenetypes.h"

using namespace mu;
using namespace mu::engraving;

namespace Ms {
ExampleView::ExampleView(QWidget* parent)
    : QFrame(parent)
{
    m_score = nullptr;
    setAcceptDrops(true);
    setFocusPolicy(Qt::StrongFocus);
    resetMatrix();
    m_backgroundPixmap = nullptr;
    m_backgroundColor  = Qt::white;

    if (notationConfiguration()->foregroundUseColor()) {
        m_backgroundColor = notationConfiguration()->foregroundColor();
    } else {
        QString wallpaperPath = notationConfiguration()->foregroundWallpaperPath().toQString();

        m_backgroundPixmap = new QPixmap(wallpaperPath);
        if (m_backgroundPixmap == 0 || m_backgroundPixmap->isNull()) {
            qDebug("no valid pixmap %s", qPrintable(wallpaperPath));
        }
    }

    // setup drag canvas state
    m_stateMachine = new QStateMachine(this);
    QState* stateActive = new QState;

    QState* s1 = new QState(stateActive);
    s1->setObjectName("example-normal");
    s1->assignProperty(this, "cursor", QCursor(Qt::ArrowCursor));

    QState* s = new QState(stateActive);
    s->setObjectName("example-drag");
    s->assignProperty(this, "cursor", QCursor(Qt::SizeAllCursor));
    QEventTransition* cl = new QEventTransition(this, QEvent::MouseButtonRelease);
    cl->setTargetState(s1);
    s->addTransition(cl);
    s1->addTransition(new DragTransitionExampleView(this));

    m_stateMachine->addState(stateActive);
    stateActive->setInitialState(s1);
    m_stateMachine->setInitialState(stateActive);

    m_stateMachine->start();

    m_defaultScaling = 0.8 * notationConfiguration()->guiScaling() * notationConfiguration()->notationScaling();
}

ExampleView::~ExampleView()
{
    if (m_backgroundPixmap) {
        delete m_backgroundPixmap;
    }
}

void ExampleView::resetMatrix()
{
    qreal _spatium = SPATIUM20 * m_defaultScaling;
    // example would normally be 10sp from top of page; this leaves 3sp margin above
    m_matrix = mu::Transform(m_defaultScaling, 0.0, 0.0, m_defaultScaling, _spatium, -_spatium * 7.0);
}

void ExampleView::layoutChanged()
{
}

void ExampleView::dataChanged(const RectF&)
{
}

void ExampleView::updateAll()
{
    update();
}

void ExampleView::adjustCanvasPosition(const EngravingItem* /*el*/, bool /*playBack*/, int)
{
}

void ExampleView::setScore(Score* s)
{
    delete m_score;
    m_score = s;
    m_score->addViewer(this);
    m_score->setLayoutMode(LayoutMode::LINE);

    ScoreLoad sl;
    m_score->doLayout();
    resetMatrix();
    update();
}

void ExampleView::removeScore()
{
}

void ExampleView::changeEditElement(EngravingItem*)
{
}

void ExampleView::setDropRectangle(const RectF&)
{
}

void ExampleView::cmdAddSlur(Note* /*firstNote*/, Note* /*lastNote*/)
{
}

void ExampleView::drawBackground(mu::draw::Painter* p, const RectF& r) const
{
    if (m_backgroundPixmap == 0 || m_backgroundPixmap->isNull()) {
        p->fillRect(r, m_backgroundColor);
    } else {
        p->drawTiledPixmap(r, *m_backgroundPixmap, r.topLeft() - PointF(m_matrix.dx(), m_matrix.dy()));
    }
}

void ExampleView::drawElements(mu::draw::Painter& painter, const QList<EngravingItem*>& el)
{
    for (EngravingItem* e : el) {
        e->itemDiscovered = 0;
        PointF pos(e->pagePos());
        painter.translate(pos);
        e->draw(&painter);
        painter.translate(-pos);
    }
}

void ExampleView::paintEvent(QPaintEvent* event)
{
    QFrame::paintEvent(event);

    if (!m_score) {
        return;
    }

    mu::draw::Painter painter(this, "exampleview");
    painter.setAntialiasing(true);
    const RectF rect = RectF::fromQRectF(event->rect());

    drawBackground(&painter, rect);

    painter.setWorldTransform(m_matrix);

    Page* page = m_score->pages().front();
    QList<EngravingItem*> ell = page->items(m_matrix.inverted().map(rect));
    std::stable_sort(ell.begin(), ell.end(), elementLessThan);
    drawElements(painter, ell);
}

void ExampleView::dragEnterEvent(QDragEnterEvent* event)
{
    const QMimeData* d = event->mimeData();
    if (d->hasFormat(mu::commonscene::MIME_SYMBOL_FORMAT)) {
        event->acceptProposedAction();

        QByteArray a = d->data(mu::commonscene::MIME_SYMBOL_FORMAT);

// qDebug("ExampleView::dragEnterEvent Symbol: <%s>", a.data());

        XmlReader e(a);
        PointF dragOffset;
        Fraction duration;      // dummy
        ElementType type = EngravingItem::readType(e, &dragOffset, &duration);

        m_dragElement = Factory::createItem(type, m_score->dummy());
        if (m_dragElement) {
            m_dragElement->resetExplicitParent();
            m_dragElement->read(e);
            m_dragElement->layout();
        }
        return;
    }
}

void ExampleView::dragLeaveEvent(QDragLeaveEvent*)
{
    if (m_dragElement) {
        delete m_dragElement;
        m_dragElement = 0;
    }
    setDropTarget(0);
}

struct MoveContext
{
    PointF pos;
    Ms::Score* score = nullptr;
};

static void moveElement(void* data, EngravingItem* e)
{
    MoveContext* ctx = (MoveContext*)data;
    ctx->score->addRefresh(e->canvasBoundingRect());
    e->setPos(ctx->pos);
    ctx->score->addRefresh(e->canvasBoundingRect());
}

void ExampleView::dragMoveEvent(QDragMoveEvent* event)
{
    event->acceptProposedAction();

    if (!m_dragElement || m_dragElement->isActionIcon()) {
        return;
    }

    PointF position = m_matrix.inverted().map(PointF::fromQPointF(event->posF()));
    QList<EngravingItem*> el = elementsAt(position);
    bool found = false;
    foreach (const EngravingItem* e, el) {
        if (e->type() == ElementType::NOTE) {
            setDropTarget(const_cast<EngravingItem*>(e));
            found = true;
            break;
        }
    }
    if (!found) {
        setDropTarget(0);
    }

    MoveContext ctx{ position, m_score };
    m_dragElement->scanElements(&ctx, moveElement, false);
    m_score->update();
    return;
}

void ExampleView::setDropTarget(const EngravingItem* el)
{
    if (m_dropTarget != el) {
        if (m_dropTarget) {
            m_dropTarget->setDropTarget(false);
            m_dropTarget = 0;
        }
        m_dropTarget = el;
        if (m_dropTarget) {
            m_dropTarget->setDropTarget(true);
        }
    }
    if (!m_dropAnchor.isNull()) {
        QRectF r;
        r.setTopLeft(m_dropAnchor.p1());
        r.setBottomRight(m_dropAnchor.p2());
        m_dropAnchor = QLineF();
    }
    if (m_dropRectangle.isValid()) {
        m_dropRectangle = QRectF();
    }
    update();
}

void ExampleView::dropEvent(QDropEvent* event)
{
    PointF position = m_matrix.inverted().map(PointF::fromQPointF(event->posF()));

    if (!m_dragElement) {
        return;
    }

    if (m_dragElement->isActionIcon()) {
        delete m_dragElement;
        m_dragElement = 0;
        return;
    }

    foreach (EngravingItem* e, elementsAt(position)) {
        if (e->type() == ElementType::NOTE) {
            ActionIcon* icon = static_cast<ActionIcon*>(m_dragElement);
            Chord* chord = static_cast<Note*>(e)->chord();
            emit beamPropertyDropped(chord, icon);
            switch (icon->actionType()) {
            case ActionIconType::BEAM_START:
                chord->setBeamMode(BeamMode::BEGIN);
                break;
            case ActionIconType::BEAM_MID:
                chord->setBeamMode(BeamMode::AUTO);
                break;
            case ActionIconType::BEAM_BEGIN_32:
                chord->setBeamMode(BeamMode::BEGIN32);
                break;
            case ActionIconType::BEAM_BEGIN_64:
                chord->setBeamMode(BeamMode::BEGIN64);
                break;
            default:
                break;
            }
            score()->doLayout();
            break;
        }
    }

    event->acceptProposedAction();
    delete m_dragElement;
    m_dragElement = nullptr;
    setDropTarget(nullptr);
}

void ExampleView::mousePressEvent(QMouseEvent* event)
{
    PointF position = m_matrix.inverted().map(PointF::fromQPointF(event->pos()));
    m_moveStartPoint = position;

    foreach (EngravingItem* e, elementsAt(position)) {
        if (e->type() == ElementType::NOTE) {
            emit noteClicked(static_cast<Note*>(e));
            break;
        }
    }
}

QSize ExampleView::sizeHint() const
{
    qreal mag = m_defaultScaling;
    qreal _spatium = SPATIUM20 * mag;
    // staff is 4sp tall with 3sp margin above; this leaves 3sp margin below
    qreal height = 10.0 * _spatium;
    if (score() && score()->pages().size() > 0) {
        height = score()->pages()[0]->tbbox().height() * mag + (6 * _spatium);
    }
    return QSize(1000 * mag, height);
}

//---------------------------------------------------------
//   dragExampleView
//     constrained scrolling ensuring that this ExampleView won't be moved past the borders of its QFrame
//---------------------------------------------------------

void ExampleView::dragExampleView(QMouseEvent* event)
{
    PointF delta = PointF::fromQPointF(event->pos()) - m_matrix.map(m_moveStartPoint);
    int dx = delta.x();
    if (dx == 0) {
        return;
    }

    constraintCanvas(&dx);

    // Perform the actual scrolling
    m_matrix.translate(dx, 0);
    scroll(dx, 0);
}

void DragTransitionExampleView::onTransition(QEvent* event)
{
    QStateMachine::WrappedEvent* wrappedEvent = static_cast<QStateMachine::WrappedEvent*>(event);
    QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(wrappedEvent->event());
    canvas->dragExampleView(mouseEvent);
}

void ExampleView::wheelEvent(QWheelEvent* event)
{
    QPoint pixelsScrolled = event->pixelDelta();
    QPoint stepsScrolled = event->angleDelta();
    int dx = 0, dy = 0;
    if (!pixelsScrolled.isNull()) {
        dx = pixelsScrolled.x();
        dy = pixelsScrolled.y();
    } else if (!stepsScrolled.isNull()) {
        dx = static_cast<qreal>(stepsScrolled.x()) * qMax(2, width() / 10) / 120;
        dy = static_cast<qreal>(stepsScrolled.y()) * qMax(2, height() / 10) / 120;
    }

    if (dx == 0) {
        if (dy == 0) {
            return;
        } else {
            dx = dy;
        }
    }

    constraintCanvas(&dx);

    m_matrix.translate(dx, 0);
    scroll(dx, 0);
}

//-----------------------------------------------------------------------------
//   constraintCanvas
//-----------------------------------------------------------------------------

void ExampleView::constraintCanvas(int* dxx)
{
    int dx = *dxx;

    Q_ASSERT(m_score->pages().front()->system(0));   // should exist if doLayout ran

    // form rectangle bounding the system with a spatium margin and translate relative to view space
    qreal xstart = m_score->pages().front()->system(0)->bbox().left() - SPATIUM20;
    qreal xend = m_score->pages().front()->system(0)->bbox().right() + 2.0 * SPATIUM20;
    QRectF systemScaledViewRect(xstart * m_matrix.m11(), 0, xend * m_matrix.m11(), 0);
    systemScaledViewRect.translate(m_matrix.dx(), 0);

    qreal frameWidth = static_cast<QFrame*>(this)->frameRect().width();

    // constrain the dx of scrolling so that this ExampleView won't be moved past the borders of its QFrame
    if (dx > 0) {
        // when moving right, ensure the left edge of systemScaledViewRect won't be right of frame's left edge
        if (systemScaledViewRect.left() + dx > 0) {
            dx = -systemScaledViewRect.left();
        }
    } else {
        // never move left if entire system already fits entirely within the frame
        if (systemScaledViewRect.width() < frameWidth) {
            dx = 0;
        }
        // when moving left, ensure the right edge of systemScaledViewRect won't be left of frame's right edge
        else if (systemScaledViewRect.right() + dx < frameWidth) {
            dx = frameWidth - systemScaledViewRect.right();
        }
    }

    *dxx = dx;
}
}
