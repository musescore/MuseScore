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
#ifndef MU_ACTIONS_IACTIONSDISPATCHER_H
#define MU_ACTIONS_IACTIONSDISPATCHER_H

#include <functional>
#include "modularity/imoduleexport.h"
#include "action.h"

namespace mu {
namespace actions {
class IActionsDispatcher : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IActionsDispatcher)
public:
    ~IActionsDispatcher() = default;

    using ActionCall = std::function<void (const ActionName&)>;

    virtual void dispatch(const ActionName& a) = 0;
    virtual void reg(const ActionName& action, const ActionCall& call) = 0;
};
}
}

#endif // MU_ACTIONS_IACTIONSDISPATCHER_H
