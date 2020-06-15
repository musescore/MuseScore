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
#ifndef MU_DOMAIN_INOTATIONINPUTCONTROLLER_H
#define MU_DOMAIN_INOTATIONINPUTCONTROLLER_H

#include <QPointF>
#include <functional>

#include "async/notification.h"

#include "notationtypes.h"

namespace mu {
namespace domain {
namespace notation {
class INotationInputController
{
public:
    virtual ~INotationInputController() = default;

    virtual Element* hitElement(const QPointF& pos, float width) const = 0;

    // drag
    using IsDraggable = std::function<bool (const Element*)>;

    virtual bool isDragStarted() const = 0;
    virtual void startDrag(const std::vector<Element*>& elems, const QPointF& eoffset, const IsDraggable& isDrag) = 0;
    virtual void drag(const QPointF& fromPos, const QPointF& toPos, DragMode mode) = 0;
    virtual void endDrag() = 0;
    virtual async::Notification dragChanged() = 0;
};
}
}
}

#endif // MU_DOMAIN_INOTATIONINPUTCONTROLLER_H
