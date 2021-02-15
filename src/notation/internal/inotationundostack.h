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
//  MERCHANTABILITY or FIT-0NESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef MU_SCENE_NOTATION_INOTATIONUNDOSTACK_H
#define MU_SCENE_NOTATION_INOTATIONUNDOSTACK_H

#include "async/notification.h"

namespace mu::notation {
class INotationUndoStack
{
public:
    virtual ~INotationUndoStack() = default;

    virtual bool canUndo() const = 0;
    virtual void undo() = 0;

    virtual bool canRedo() const = 0;
    virtual void redo() = 0;

    virtual void prepareChanges() = 0;
    virtual void rollbackChanges() = 0;
    virtual void commitChanges() = 0;

    virtual async::Notification stackChanged() const = 0;
};

using INotationUndoStackPtr = std::shared_ptr<INotationUndoStack>;
}

#endif // MU_SCENE_NOTATION_INOTATIONUNDOSTACK_H
