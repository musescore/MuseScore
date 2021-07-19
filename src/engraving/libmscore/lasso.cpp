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

#include "lasso.h"
#include "mscore.h"
#include "mscoreview.h"
#include "score.h"
#include "draw/brush.h"

using namespace mu;

namespace Ms {
//---------------------------------------------------------
//   Lasso
//---------------------------------------------------------

Lasso::Lasso(Score* s)
    : Element(s)
{
    setVisible(false);
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Lasso::draw(mu::draw::Painter* painter) const
{
    TRACE_OBJ_DRAW;
    using namespace mu::draw;
    painter->setBrush(Brush(QColor(0, 0, 50, 50)));
    // always 2 pixel width
    qreal w = 2.0 / painter->worldTransform().m11();
    painter->setPen(Pen(MScore::selectColor[0], w));
    painter->drawRect(bbox());
}

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Lasso::editDrag(EditData& ed)
{
    Qt::CursorShape cursorShape = ed.view()->cursor().shape();
    switch (int(ed.curGrip)) {
    case 0:
        cursorShape = Qt::SizeFDiagCursor;
        bbox().setTopLeft(bbox().topLeft() + ed.delta);
        break;
    case 1:
        cursorShape = Qt::SizeBDiagCursor;
        bbox().setTopRight(bbox().topRight() + ed.delta);
        break;
    case 2:
        cursorShape = Qt::SizeFDiagCursor;
        bbox().setBottomRight(bbox().bottomRight() + ed.delta);
        break;
    case 3:
        cursorShape = Qt::SizeBDiagCursor;
        bbox().setBottomLeft(bbox().bottomLeft() + ed.delta);
        break;
    case 4:
        cursorShape = Qt::SizeVerCursor;
        bbox().setTop(bbox().top() + ed.delta.y());
        break;
    case 5:
        cursorShape = Qt::SizeHorCursor;
        bbox().setRight(bbox().right() + ed.delta.x());
        break;
    case 6:
        cursorShape = Qt::SizeVerCursor;
        bbox().setBottom(bbox().bottom() + ed.delta.y());
        break;
    case 7:
        cursorShape = Qt::SizeHorCursor;
        bbox().setLeft(bbox().left() + ed.delta.x());
        break;
    }
    ed.view()->setCursor(QCursor(cursorShape));
}

//---------------------------------------------------------
//   gripsPositions
//---------------------------------------------------------

std::vector<mu::PointF> Lasso::gripsPositions(const EditData&) const
{
    const auto box(bbox());
    return {
        box.topLeft(),
        box.topRight(),
        box.bottomRight(),
        box.bottomLeft(),
        PointF(box.x() + box.width() * .5, box.top()),
        PointF(box.right(), box.y() + box.height() * .5),
        PointF(box.x() + box.width() * .5, box.bottom()),
        PointF(box.left(), box.y() + box.height() * .5)
    };
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Lasso::setProperty(Pid propertyId, const QVariant& v)
{
    switch (propertyId) {
    case Pid::LASSO_POS:
        bbox().moveTo(PointF::fromVariant(v));
        break;
    case Pid::LASSO_SIZE:
        bbox().setSize(SizeF::fromVariant(v));
        break;
    default:
        if (!Element::setProperty(propertyId, v)) {
            return false;
        }
        break;
    }
    score()->setUpdateAll();
    return true;
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Lasso::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::LASSO_POS:
        return QVariant::fromValue(bbox().topLeft());
    case Pid::LASSO_SIZE:
        return QVariant::fromValue(bbox().size());
    default:
        break;
    }
    return Element::getProperty(propertyId);
}
}
