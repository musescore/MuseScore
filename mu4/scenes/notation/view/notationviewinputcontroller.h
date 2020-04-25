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
#ifndef MU_NOTATIONSCENE_NOTATIONVIEWINPUTCONTROLLER_H
#define MU_NOTATIONSCENE_NOTATIONVIEWINPUTCONTROLLER_H

#include <QWheelEvent>

namespace mu {
namespace scene {
namespace notation {
class NotationPaintView;
class NotationViewInputController
{
public:
    NotationViewInputController(NotationPaintView* view);

    void wheelEvent(QWheelEvent* ev);
    void mousePressEvent(QMouseEvent* ev);
    void mouseMoveEvent(QMouseEvent* ev);
    void mouseReleaseEvent(QMouseEvent* ev);

private:

    struct InteractData {
        QPoint beginPoint;
    };

    NotationPaintView* m_view = nullptr;
    InteractData m_interactData;
};
}
}
}
#endif // MU_NOTATIONSCENE_NOTATIONVIEWINPUTCONTROLLER_H
