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

#include "draw/types/brush.h"

#include "score.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   Lasso
//---------------------------------------------------------

Lasso::Lasso(Score* s)
    : EngravingItem(ElementType::LASSO, s)
{
    setVisible(false);
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Lasso::draw(mu::draw::Painter* painter) const
{
    TRACE_ITEM_DRAW;
    using namespace mu::draw;
    painter->setBrush(Brush(engravingConfiguration()->lassoColor()));
    // always 2 pixel width
    double w = 2.0 / painter->worldTransform().m11() * engravingConfiguration()->guiScaling();
    painter->setPen(Pen(engravingConfiguration()->selectionColor(), w));
    painter->drawRect(bbox());
}

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Lasso::editDrag(EditData& ed)
{
    //Qt::CursorShape cursorShape = Qt::ArrowCursor;
    switch (int(ed.curGrip)) {
    case 0:
        //cursorShape = Qt::SizeFDiagCursor;
        bbox().setTopLeft(bbox().topLeft() + ed.delta);
        break;
    case 1:
        //cursorShape = Qt::SizeBDiagCursor;
        bbox().setTopRight(bbox().topRight() + ed.delta);
        break;
    case 2:
        //cursorShape = Qt::SizeFDiagCursor;
        bbox().setBottomRight(bbox().bottomRight() + ed.delta);
        break;
    case 3:
        //cursorShape = Qt::SizeBDiagCursor;
        bbox().setBottomLeft(bbox().bottomLeft() + ed.delta);
        break;
    case 4:
        //cursorShape = Qt::SizeVerCursor;
        bbox().setTop(bbox().top() + ed.delta.y());
        break;
    case 5:
        //cursorShape = Qt::SizeHorCursor;
        bbox().setRight(bbox().right() + ed.delta.x());
        break;
    case 6:
        //cursorShape = Qt::SizeVerCursor;
        bbox().setBottom(bbox().bottom() + ed.delta.y());
        break;
    case 7:
        //cursorShape = Qt::SizeHorCursor;
        bbox().setLeft(bbox().left() + ed.delta.x());
        break;
    }

    // ed.view()->setCursor(QCursor(cursorShape));
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
}
