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

#include "exampleview.h"

#include <cmath>
#include <QMimeData>

#include "engraving/dom/engravingitem.h"
#include "engraving/dom/page.h"
#include "engraving/dom/system.h"

#include "log.h"

using namespace mu;
using namespace muse::draw;
using namespace mu::notation;
using namespace mu::engraving;

ExampleView::ExampleView(QWidget* parent)
    : QFrame(parent), muse::Injectable(muse::iocCtxForQWidget(this))
{
    m_score = nullptr;
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
            LOGD("no valid pixmap %s", qPrintable(wallpaperPath));
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

    m_defaultScaling = 0.9 * notationConfiguration()->notationScaling();
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
    m_matrix = Transform(m_defaultScaling, 0.0, 0.0, m_defaultScaling, _spatium, -_spatium * 7.0);
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

void ExampleView::adjustCanvasPosition(const EngravingItem* /*el*/, int)
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

void ExampleView::drawBackground(Painter* p, const RectF& r) const
{
    if (m_backgroundPixmap == 0 || m_backgroundPixmap->isNull()) {
        p->fillRect(r, m_backgroundColor);
    } else {
        p->drawTiledPixmap(r, *m_backgroundPixmap, r.topLeft() - PointF(m_matrix.dx(), m_matrix.dy()));
    }
}

void ExampleView::drawElements(Painter& painter, const std::vector<EngravingItem*>& el)
{
    for (EngravingItem* e : el) {
        e->itemDiscovered = 0;
        PointF pos(e->pagePos());
        painter.translate(pos);
        e->renderer()->drawItem(e, &painter);
        painter.translate(-pos);
    }
}

void ExampleView::paintEvent(QPaintEvent* event)
{
    QFrame::paintEvent(event);

    if (!m_score) {
        return;
    }

    Painter painter(this, "exampleview");
    painter.setAntialiasing(true);
    const RectF rect = RectF::fromQRectF(event->rect());

    drawBackground(&painter, rect);

    painter.setWorldTransform(m_matrix);

    Page* page = m_score->pages().front();
    std::vector<EngravingItem*> ell = page->items(m_matrix.inverted().map(rect));
    std::sort(ell.begin(), ell.end(), elementLessThan);
    drawElements(painter, ell);
}

void ExampleView::mousePressEvent(QMouseEvent* event)
{
    m_moveStartPoint = toLogical(event->position());
}

PointF ExampleView::toLogical(const QPointF& point)
{
    return m_matrix.inverted().map(PointF::fromQPointF(point));
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
    PointF delta = PointF::fromQPointF(event->position()) - m_matrix.map(m_moveStartPoint);
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
    qreal xstart = m_score->pages().front()->system(0)->ldata()->bbox().left() - SPATIUM20;
    qreal xend = m_score->pages().front()->system(0)->ldata()->bbox().right() + 2.0 * SPATIUM20;
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
