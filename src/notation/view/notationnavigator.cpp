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
#include "notationnavigator.h"

#include "libmscore/system.h"

using namespace mu::notation;

NotationNavigator::NotationNavigator(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
    setAcceptedMouseButtons(Qt::AllButtons);

    configuration()->navigatorOrientationChanged().onReceive(this, [this](NavigatorOrientation) {
        moveCanvasToPosition(QPoint(0, 0));
        emit orientationChanged();
    });

    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        onCurrentNotationChanged();
    });

    theme()->themeChanged().onNotify(this, [this]() {
        update();
    });
}

bool NotationNavigator::isVerticalOrientation() const
{
    return configuration()->navigatorOrientation() == NavigatorOrientation::Vertical;
}

QRectF NotationNavigator::notationContentRect() const
{
    QRectF result;
    for (const Page* page: pages()) {
        result = result.united(page->bbox().translated(page->pos()));
    }

    return result;
}

PageList NotationNavigator::pages() const
{
    auto notation = globalContext()->currentNotation();
    if (!notation) {
        return {};
    }

    auto elements = notation->elements();
    if (!elements) {
        return {};
    }

    return elements->pages();
}

qreal NotationNavigator::scale() const
{
    return m_matrix.m11();
}

void NotationNavigator::rescale()
{
    PageList pages = this->pages();
    if (pages.empty()) {
        return;
    }

    const Page* lastPage = pages.back();

    if (isVerticalOrientation()) {
        qreal scoreWidth  = lastPage->width();
        qreal m = width() / scoreWidth;
        m_matrix = QTransform(m, 0, 0, m, m_matrix.dx(), m_matrix.dy());
    } else {
        qreal scoreHeight = lastPage->height();
        qreal m  = height() / scoreHeight;
        m_matrix = QTransform(m, 0, 0, m, m_matrix.dx(), m_matrix.dy());
    }
}

QRect NotationNavigator::viewport() const
{
    return toLogical(QRect(0, 0, width(), height()));
}

QPoint NotationNavigator::toLogical(const QPoint& point) const
{
    return m_matrix.inverted().map(point);
}

QRect NotationNavigator::toLogical(const QRect& rect) const
{
    return m_matrix.inverted().mapRect(rect);
}

void NotationNavigator::mousePressEvent(QMouseEvent* event)
{
    QPoint logicPos = toLogical(event->pos());
    m_startMove = logicPos;
    if (m_viewRect.contains(logicPos)) {
        return;
    }

    QRectF viewRect = m_viewRect;
    double dx = logicPos.x() - (viewRect.x() + (viewRect.width() * .5));
    double dy = logicPos.y() - (viewRect.y() + (viewRect.height() * .5));

    moveNotationRequested(-dx, -dy);
}

void NotationNavigator::mouseMoveEvent(QMouseEvent* event)
{
    QPoint logicPos = toLogical(event->pos());
    QPoint delta = logicPos - m_startMove;
    int dx = delta.x();
    int dy = delta.y();
    moveNotationRequested(-dx, -dy);

    m_startMove = logicPos;
}

void NotationNavigator::moveCanvas(int dx, int dy)
{
    m_matrix.translate(dx, dy);
    update();
}

void NotationNavigator::moveCanvasToRect(const QRect& viewRect)
{
    QRectF newViewRect = notationContentRect().intersected(viewRect);

    int dx = 0;
    int dy = 0;
    if (isVerticalOrientation()) {
        newViewRect.setHeight(std::min(viewport().height(), newViewRect.toRect().height()));

        QPoint top = toLogical(newViewRect.topLeft().toPoint()) * scale();
        QPoint bottom = toLogical(newViewRect.bottomRight().toPoint()) * scale();

        if (!notationContentRect().contains(top) && !notationContentRect().contains(bottom)) {
            return;
        }

        if (viewport().top() > top.y()) {
            dy = newViewRect.top() - viewport().top();
        } else if (viewport().bottom() < bottom.y()) {
            dy = newViewRect.bottom() - viewport().bottom();
        }
    } else {
        newViewRect.setWidth(std::min(viewport().width(), newViewRect.toRect().width()));

        QPoint left = toLogical(newViewRect.topLeft().toPoint()) * scale();
        QPoint right = toLogical(newViewRect.bottomRight().toPoint()) * scale();

        if (!notationContentRect().contains(left) && !notationContentRect().contains(right)) {
            return;
        }

        if (viewport().left() > left.x()) {
            dx = newViewRect.left() - viewport().left();
        } else if (viewport().right() < right.x()) {
            dx = newViewRect.right() - viewport().right();
        }
    }

    moveCanvas(-dx, -dy);
}

void NotationNavigator::moveCanvasToPosition(const QPoint& position)
{
    QPoint viewTL = toLogical(QPoint(0, 0));
    moveCanvas(viewTL.x() - position.x(), viewTL.y() - position.y());
}

void NotationNavigator::setViewRect(const QRect& rect)
{
    if (!rect.isValid()) {
        return;
    }

    moveCanvasToRect(rect);

    m_viewRect = rect;
    rescale();
    update();
}

int NotationNavigator::orientation() const
{
    return static_cast<int>(configuration()->navigatorOrientation());
}

void NotationNavigator::onCurrentNotationChanged()
{
    auto notation = currentNotation();
    setVisible(notation != nullptr);

    if (!notation) {
        return;
    }

    notation->notationChanged().onNotify(this, [this]() {
        rescale();
        update();
    });
}

INotationPtr NotationNavigator::currentNotation() const
{
    return globalContext()->currentNotation();
}

ViewMode NotationNavigator::notationViewMode() const
{
    auto notation = currentNotation();
    if (!notation) {
        return ViewMode::PAGE;
    }

    return notation->viewMode();
}

void NotationNavigator::paint(QPainter* painter)
{
    painter->fillRect(viewport(), configuration()->backgroundColor());

    paintPages(painter);
    paintViewRect(painter);
}

void NotationNavigator::paintPages(QPainter* painter)
{
    PageList pages = this->pages();
    if (pages.empty()) {
        return;
    }

    ViewMode viewMode = notationViewMode();
    QFont font(QString::fromStdString(configuration()->fontFamily()), 2000);

    painter->setTransform(m_matrix);
    QRectF viewportRect = m_matrix.inverted().mapRect(QRectF(viewport()));

    for (const Page* page : pages) {
        QPointF pos(page->pos());
        QRectF pageRect(page->abbox().translated(pos));
        if (pageRect.right() < viewportRect.left()) {
            continue;
        }
        if (pageRect.left() > viewportRect.right()) {
            break;
        }

        painter->translate(pos);

        painter->fillRect(page->bbox(), configuration()->pageColor());

        //! todo
        Page* _page = const_cast<Page*>(page);
        paintElements(painter, _page->elements());

        if (viewMode == ViewMode::PAGE) {
            painter->setFont(font);
            painter->setPen(configuration()->layoutBreakColor());
            painter->drawText(page->bbox(), Qt::AlignCenter, QString("%1").arg(page->no() + 1));
        }
        painter->translate(-pos);
    }
}

void NotationNavigator::paintElements(QPainter* painter, const QList<Element*>& elements)
{
    for (const Ms::Element* element : elements) {
        if (!element->visible()) {
            continue;
        }

        element->itemDiscovered = false;
        QPointF elementPosition(element->pagePos());

        painter->translate(elementPosition);
        element->draw(painter);
        painter->translate(-elementPosition);
    }
}

void NotationNavigator::paintViewRect(QPainter* painter)
{
    QColor color(configuration()->selectionColor());
    QPen pen(color, 2.0);
    painter->setPen(pen);
    painter->setBrush(QColor(color.red(), color.green(), color.blue(), 40));

    QRectF viewRect = notationContentRect().intersected(m_viewRect);
    painter->drawRect(viewRect);
}
