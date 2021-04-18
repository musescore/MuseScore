//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2013 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "exampleview.h"

#include <cmath>
#include <QMimeData>

#include "libmscore/score.h"
#include "libmscore/element.h"
#include "libmscore/page.h"
#include "libmscore/system.h"
#include "libmscore/icon.h"
#include "libmscore/chord.h"
#include "libmscore/xml.h"

#include "libmscore/draw/qpainterprovider.h"

#include "commonscenetypes.h"

namespace Ms {
//---------------------------------------------------------
//   ExampleView
//---------------------------------------------------------

ExampleView::ExampleView(QWidget* parent)
    : QFrame(parent)
{
    _score = 0;
    setAcceptDrops(true);
    setFocusPolicy(Qt::StrongFocus);
    resetMatrix();
    _fgPixmap = nullptr;
    _fgColor  = Qt::white;

    if (notationConfiguration()->foregroundUseColor()) {
        _fgColor = notationConfiguration()->foregroundColor();
    } else {
        QString wallpaperPath = notationConfiguration()->foregroundWallpaperPath().toQString();

        _fgPixmap = new QPixmap(wallpaperPath);
        if (_fgPixmap == 0 || _fgPixmap->isNull()) {
            qDebug("no valid pixmap %s", qPrintable(wallpaperPath));
        }
    }
    // setup drag canvas state
    sm          = new QStateMachine(this);
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

    sm->addState(stateActive);
    stateActive->setInitialState(s1);
    sm->setInitialState(stateActive);

    sm->start();

    m_defaultScaling = 0.9 * uiConfiguration()->physicalDotsPerInch() / DPI; // 90% of nominal
}

//---------------------------------------------------------
//   ~ExampleView
//---------------------------------------------------------

ExampleView::~ExampleView()
{
    if (_fgPixmap) {
        delete _fgPixmap;
    }
}

//---------------------------------------------------------
//   resetMatrix
//    used to reset scrolling in case time signature num or denom changed
//---------------------------------------------------------

void ExampleView::resetMatrix()
{
    double mag = m_defaultScaling;
    qreal _spatium = SPATIUM20 * mag;
    // example would normally be 10sp from top of page; this leaves 3sp margin above
    _matrix  = QTransform(mag, 0.0, 0.0, mag, _spatium, -_spatium * 7.0);
    imatrix  = _matrix.inverted();
}

void ExampleView::layoutChanged()
{
}

void ExampleView::dataChanged(const QRectF&)
{
}

void ExampleView::updateAll()
{
    update();
}

void ExampleView::adjustCanvasPosition(const Element* /*el*/, bool /*playBack*/, int)
{
}

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void ExampleView::setScore(Score* s)
{
    delete _score;
    _score = s;
    _score->addViewer(this);
    _score->setLayoutMode(LayoutMode::LINE);

    ScoreLoad sl;
    _score->doLayout();
    update();
}

void ExampleView::removeScore()
{
}

void ExampleView::changeEditElement(Element*)
{
}

QCursor ExampleView::cursor() const
{
    return QCursor();
}

void ExampleView::setCursor(const QCursor&)
{
}

void ExampleView::setDropRectangle(const QRectF&)
{
}

void ExampleView::cmdAddSlur(Note* /*firstNote*/, Note* /*lastNote*/)
{
}

Element* ExampleView::elementNear(QPointF)
{
    return 0;
}

void ExampleView::drawBackground(mu::draw::Painter* p, const QRectF& r) const
{
    if (_fgPixmap == 0 || _fgPixmap->isNull()) {
        p->fillRect(r, _fgColor);
    } else {
        p->drawTiledPixmap(r, *_fgPixmap, r.topLeft()
                           - QPoint(lrint(_matrix.dx()), lrint(_matrix.dy())));
    }
}

//---------------------------------------------------------
//   drawElements
//---------------------------------------------------------

void ExampleView::drawElements(mu::draw::Painter& painter, const QList<Element*>& el)
{
    for (Element* e : el) {
        e->itemDiscovered = 0;
        QPointF pos(e->pagePos());
        painter.translate(pos);
        e->draw(&painter);
        painter.translate(-pos);
    }
}

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void ExampleView::paintEvent(QPaintEvent* ev)
{
    if (_score) {
        mu::draw::Painter p(this, "exampleview");
        p.setAntialiasing(true);
        const QRect r(ev->rect());

        drawBackground(&p, r);

        p.setWorldTransform(_matrix);
        QRectF fr = imatrix.mapRect(QRectF(r));

        QRegion r1(r);
        Page* page = _score->pages().front();
        QList<Element*> ell = page->items(fr);
        std::stable_sort(ell.begin(), ell.end(), elementLessThan);
        drawElements(p, ell);
    }
    QFrame::paintEvent(ev);
}

//---------------------------------------------------------
//   dragEnterEvent
//---------------------------------------------------------

void ExampleView::dragEnterEvent(QDragEnterEvent* event)
{
    const QMimeData* d = event->mimeData();
    if (d->hasFormat(mu::commonscene::MIME_SYMBOL_FORMAT)) {
        event->acceptProposedAction();

        QByteArray a = d->data(mu::commonscene::MIME_SYMBOL_FORMAT);

// qDebug("ExampleView::dragEnterEvent Symbol: <%s>", a.data());

        XmlReader e(a);
        QPointF dragOffset;
        Fraction duration;      // dummy
        ElementType type = Element::readType(e, &dragOffset, &duration);

        dragElement = Element::create(type, _score);
        if (dragElement) {
            dragElement->setParent(0);
            dragElement->read(e);
            dragElement->layout();
        }
        return;
    }
}

//---------------------------------------------------------
//   dragLeaveEvent
//---------------------------------------------------------

void ExampleView::dragLeaveEvent(QDragLeaveEvent*)
{
    if (dragElement) {
        delete dragElement;
        dragElement = 0;
    }
    setDropTarget(0);
}

//---------------------------------------------------------
//   moveElement
//---------------------------------------------------------

static void moveElement(void* data, Element* e)
{
    QPointF* pos = (QPointF*)data;
    e->score()->addRefresh(e->canvasBoundingRect());
    e->setPos(*pos);
    e->score()->addRefresh(e->canvasBoundingRect());
}

//---------------------------------------------------------
//   dragMoveEvent
//---------------------------------------------------------

void ExampleView::dragMoveEvent(QDragMoveEvent* event)
{
    event->acceptProposedAction();

    if (!dragElement || dragElement->type() != ElementType::ICON) {
        return;
    }

    QPointF pos(imatrix.map(QPointF(event->pos())));
    QList<Element*> el = elementsAt(pos);
    bool found = false;
    foreach (const Element* e, el) {
        if (e->type() == ElementType::NOTE) {
            setDropTarget(const_cast<Element*>(e));
            found = true;
            break;
        }
    }
    if (!found) {
        setDropTarget(0);
    }
    dragElement->scanElements(&pos, moveElement, false);
    _score->update();
    return;
}

//---------------------------------------------------------
//   setDropTarget
//---------------------------------------------------------

void ExampleView::setDropTarget(const Element* el)
{
    if (dropTarget != el) {
        if (dropTarget) {
            dropTarget->setDropTarget(false);
            dropTarget = 0;
        }
        dropTarget = el;
        if (dropTarget) {
            dropTarget->setDropTarget(true);
        }
    }
    if (!dropAnchor.isNull()) {
        QRectF r;
        r.setTopLeft(dropAnchor.p1());
        r.setBottomRight(dropAnchor.p2());
        dropAnchor = QLineF();
    }
    if (dropRectangle.isValid()) {
        dropRectangle = QRectF();
    }
    update();
}

//---------------------------------------------------------
//   dropEvent
//---------------------------------------------------------

void ExampleView::dropEvent(QDropEvent* event)
{
    QPointF pos(imatrix.map(QPointF(event->pos())));

    if (!dragElement) {
        return;
    }
    if (dragElement->type() != ElementType::ICON) {
        delete dragElement;
        dragElement = 0;
        return;
    }
    foreach (Element* e, elementsAt(pos)) {
        if (e->type() == ElementType::NOTE) {
            Icon* icon = static_cast<Icon*>(dragElement);
            Chord* chord = static_cast<Note*>(e)->chord();
            emit beamPropertyDropped(chord, icon);
            switch (icon->iconType()) {
            case IconType::SBEAM:
                chord->setBeamMode(Beam::Mode::BEGIN);
                break;
            case IconType::MBEAM:
                chord->setBeamMode(Beam::Mode::AUTO);
                break;
            case IconType::BEAM32:
                chord->setBeamMode(Beam::Mode::BEGIN32);
                break;
            case IconType::BEAM64:
                chord->setBeamMode(Beam::Mode::BEGIN64);
                break;
            default:
                break;
            }
            score()->doLayout();
            break;
        }
    }
    event->acceptProposedAction();
    delete dragElement;
    dragElement = 0;
    setDropTarget(0);
}

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void ExampleView::mousePressEvent(QMouseEvent* event)
{
    startMove  = imatrix.map(QPointF(event->pos()));
    QPointF pos(imatrix.map(QPointF(event->pos())));

    foreach (Element* e, elementsAt(pos)) {
        if (e->type() == ElementType::NOTE) {
            emit noteClicked(static_cast<Note*>(e));
            break;
        }
    }
}

//---------------------------------------------------------
//   sizeHint
//---------------------------------------------------------

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

void ExampleView::dragExampleView(QMouseEvent* ev)
{
    QPoint d = ev->pos() - _matrix.map(startMove).toPoint();
    int dx   = d.x();
    if (dx == 0) {
        return;
    }

    constraintCanvas(&dx);

    // Perform the actual scrolling
    _matrix.setMatrix(_matrix.m11(), _matrix.m12(), _matrix.m13(), _matrix.m21(),
                      _matrix.m22(), _matrix.m23(), _matrix.dx() + dx, _matrix.dy(), _matrix.m33());
    imatrix = _matrix.inverted();
    scroll(dx, 0);
}

void DragTransitionExampleView::onTransition(QEvent* e)
{
    QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(e);
    QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
    canvas->dragExampleView(me);
}

//---------------------------------------------------------
//   wheelEvent
//---------------------------------------------------------

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

    _matrix.setMatrix(_matrix.m11(), _matrix.m12(), _matrix.m13(), _matrix.m21(),
                      _matrix.m22(), _matrix.m23(), _matrix.dx() + dx, _matrix.dy(), _matrix.m33());
    imatrix = _matrix.inverted();
    scroll(dx, 0);
}

//-----------------------------------------------------------------------------
//   constraintCanvas
//-----------------------------------------------------------------------------

void ExampleView::constraintCanvas(int* dxx)
{
    int dx = *dxx;

    Q_ASSERT(_score->pages().front()->system(0));   // should exist if doLayout ran

    // form rectangle bounding the system with a spatium margin and translate relative to view space
    qreal xstart = _score->pages().front()->system(0)->bbox().left() - SPATIUM20;
    qreal xend = _score->pages().front()->system(0)->bbox().right() + 2.0 * SPATIUM20;
    QRectF systemScaledViewRect(xstart * _matrix.m11(), 0, xend * _matrix.m11(), 0);
    systemScaledViewRect.translate(_matrix.dx(), 0);

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
