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
#ifndef MU_DOMAIN_NOTATIONINPUTCONTROLLER_H
#define MU_DOMAIN_NOTATIONINPUTCONTROLLER_H

#include <memory>
#include <vector>
#include <QPointF>

#include "../inotationinputcontroller.h"
#include "igetscore.h"
#include "scorecallbacks.h"

#include "libmscore/element.h"
#include "libmscore/elementgroup.h"

namespace mu {
namespace domain {
namespace notation {
class NotationInputController : public INotationInputController
{
public:
    NotationInputController(IGetScore* getScore);

    Element* hitElement(const QPointF& pos, float width) const override;

    bool isDragStarted() const override;
    void startDrag(const std::vector<Element*>& elems, const QPointF& eoffset, const IsDraggable& isDraggable) override;
    void drag(const QPointF& fromPos, const QPointF& toPos, DragMode mode) override;
    void endDrag() override;
    async::Notification dragChanged() override;

    Ms::Page* point2page(const QPointF& p) const;

private:

    Ms::Score* score() const;
    QList<Element*> hitElements(const QPointF& p_in, float w) const;

    static bool elementIsLess(const Ms::Element* e1, const Ms::Element* e2);

    struct DragData
    {
        QPointF beginMove;
        QPointF elementOffset;
        Ms::EditData editData;
        std::vector<std::unique_ptr<Ms::ElementGroup> > dragGroups;
        void reset();
    };

    IGetScore* m_getScore = nullptr;
    DragData m_dragData;
    ScoreCallbacks* m_scoreCallbacks = nullptr;
    async::Notification m_dragChanged;
};
}
}
}

#endif // MU_DOMAIN_NOTATIONINPUTCONTROLLER_H
