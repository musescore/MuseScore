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
#ifndef MU_NOTATION_INOTATIONINPUTSTATE_H
#define MU_NOTATION_INOTATIONINPUTSTATE_H

#include "async/notification.h"
#include "notationtypes.h"

namespace mu {
namespace notation {
class INotationInputState
{
public:
    virtual ~INotationInputState() = default;

    virtual bool isNoteEnterMode() const = 0;
    virtual DurationType duration() const = 0;
};

using INotationInputStatePtr = std::shared_ptr<INotationInputState>;
}
}

#endif // MU_NOTATION_INOTATIONINPUTSTATE_H
