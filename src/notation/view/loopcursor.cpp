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

#include "loopcursor.h"

using namespace mu::notation;

LoopCursor::LoopCursor(LoopCursorType type)
    : m_type(type)
{
}

void LoopCursor::setRect(const QRect& rect)
{
    m_rect = rect;
}

void LoopCursor::setVisible(bool visible)
{
    m_visible = visible;
}

void LoopCursor::setStyle(INotationStylePtr style)
{
    m_style = style;
}

void LoopCursor::paint(QPainter* painter)
{
    if (!m_visible || !m_style) {
        return;
    }

    QPolygonF triangle(3);

    qreal x = m_rect.left();
    qreal y = m_rect.top();
    qreal h = m_style->styleValue(StyleId::spatium).toDouble() * 2;

    QColor color = configuration()->loopCursorColor();

    switch (m_type) {
    case LoopCursorType::LoopIn: { // draw a right-pointing triangle
        qreal tx = x - 1.0;
        triangle[0] = QPointF(tx, y);
        triangle[1] = QPointF(tx, y + h);
        triangle[2] = QPointF(tx + h, y + h / 2);
    }
        break;
    case LoopCursorType::LoopOut: // draw a left-pointing triangle
        triangle[0] = QPointF(x, y);
        triangle[1] = QPointF(x, y + h);
        triangle[2] = QPointF(x - h, y + h / 2);
        break;
    case LoopCursorType::Unknown: return;
    }

    painter->setPen(QPen(color, 2.0, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
    painter->drawLine(x, y, x, m_rect.bottom());
    painter->setBrush(color);
    painter->drawConvexPolygon(triangle);
}
