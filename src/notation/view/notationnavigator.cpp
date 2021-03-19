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
    : NotationPaintView(parent)
{
    setReadonly(true);
}

void NotationNavigator::load()
{
    initOrientation();
    initVisible();

    uiConfiguration()->currentThemeChanged().onNotify(this, [this]() {
        update();
    });

    NotationPaintView::load();
}

bool NotationNavigator::isVerticalOrientation() const
{
    return configuration()->canvasOrientation().val == framework::Orientation::Vertical;
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

void NotationNavigator::rescale()
{
    PageList pages = this->pages();
    if (pages.empty()) {
        return;
    }

    const Page* lastPage = pages.back();

    qreal _scale = 0;

    if (isVerticalOrientation()) {
        qreal scoreWidth = lastPage->width();
        _scale = width() * guiScaling() / scoreWidth;
    } else {
        qreal scoreHeight = lastPage->height();
        _scale = height() * guiScaling() / scoreHeight;
    }

    scale(_scale, QPoint());
}

void NotationNavigator::wheelEvent(QWheelEvent*)
{
}

void NotationNavigator::mousePressEvent(QMouseEvent* event)
{
    QPoint logicPos = toLogical(event->pos());
    m_startMove = logicPos;
    if (m_cursorRect.contains(logicPos)) {
        return;
    }

    QRectF viewRect = m_cursorRect;
    double dx = logicPos.x() - (viewRect.x() + (viewRect.width() / 2));
    double dy = logicPos.y() - (viewRect.y() + (viewRect.height() / 2));

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

void NotationNavigator::moveCanvasToRect(const QRect& viewRect)
{
    QRectF newViewRect = viewRect;
    QRect viewport = this->viewport();
    QRectF notationContentRect = this->notationContentRect();

    int dx = 0;
    int dy = 0;

    if (isVerticalOrientation()) {
        newViewRect.setHeight(std::min(viewport.height(), newViewRect.toRect().height()));

        QPoint top = newViewRect.topLeft().toPoint();
        QPoint bottom = newViewRect.bottomRight().toPoint();

        if (!notationContentRect.contains(top) && !notationContentRect.contains(bottom)) {
            return;
        }

        if (viewport.top() > top.y()) {
            dy = top.y() - viewport.top();
        } else if (viewport.bottom() < bottom.y()) {
            dy = bottom.y() - viewport.bottom();
        }
    } else {
        newViewRect.setWidth(std::min(viewport.width(), newViewRect.toRect().width()));

        QPoint left = newViewRect.topLeft().toPoint();
        QPoint right = newViewRect.bottomRight().toPoint();

        if (!notationContentRect.contains(left) && !notationContentRect.contains(right)) {
            return;
        }

        if (viewport.left() > left.x()) {
            dx = left.x() - viewport.left();
        } else if (viewport.right() < right.x()) {
            dx = right.x() - viewport.right();
        }
    }

    moveCanvas(-dx, -dy);
}

void NotationNavigator::setCursorRect(const QRect& rect)
{
    QRect newCursorRect = rect;
    if (!newCursorRect.isValid()) {
        return;
    }

    newCursorRect = notationContentRect().intersected(QRectF(newCursorRect)).toRect();

    moveCanvasToRect(newCursorRect);

    m_cursorRect = newCursorRect;
    rescale();
    update();
}

int NotationNavigator::orientation() const
{
    return static_cast<int>(configuration()->canvasOrientation().val);
}

INotationPtr NotationNavigator::currentNotation() const
{
    return globalContext()->currentNotation();
}

void NotationNavigator::initOrientation()
{
    ValCh<framework::Orientation> orientation = configuration()->canvasOrientation();
    orientation.ch.onReceive(this, [this](framework::Orientation) {
        moveCanvasToPosition(QPoint(0, 0));
        emit orientationChanged();
    });

    emit orientationChanged();
}

void NotationNavigator::initVisible()
{
    connect(this, &NotationNavigator::visibleChanged, [this]() {
        update();
    });
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
    if (!isVisible()) {
        return;
    }

    NotationPaintView::paint(painter);
    paintCursor(painter);

    paintPageNumbers(painter);
}

void NotationNavigator::paintCursor(QPainter* painter)
{
    QColor color(configuration()->selectionColor());
    QPen pen(color, configuration()->borderWidth());
    painter->setPen(pen);
    painter->setBrush(QColor(color.red(), color.green(), color.blue(), configuration()->cursorOpacity()));

    painter->drawRect(m_cursorRect);
}

void NotationNavigator::paintPageNumbers(QPainter* painter)
{
    if (notationViewMode() != ViewMode::PAGE) {
        return;
    }

    constexpr int PAGE_NUMBER_FONT_SIZE = 2000;
    QFont font(QString::fromStdString(configuration()->fontFamily()), PAGE_NUMBER_FONT_SIZE);

    for (const Page* page : pages()) {
        painter->translate(page->pos());

        painter->setFont(font);
        painter->setPen(configuration()->layoutBreakColor());
        painter->drawText(page->bbox(), Qt::AlignCenter, QString("%1").arg(page->no() + 1));

        painter->translate(-page->pos());
    }
}
