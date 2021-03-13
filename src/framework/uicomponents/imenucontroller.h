//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_UICOMPONENTS_IMENUCONTROLLER_H
#define MU_UICOMPONENTS_IMENUCONTROLLER_H

#include "modularity/imoduleexport.h"
#include "actions/actiontypes.h"
#include "async/channel.h"
#include "uicomponentstypes.h"

namespace mu::uicomponents {
class IMenuController
{
public:
    virtual ~IMenuController() = default;

    virtual bool contains(const actions::ActionCode& actionCode) const = 0;
    virtual ActionState actionState(const actions::ActionCode& actionCode) const = 0;

    virtual async::Channel<actions::ActionCodeList> actionsAvailableChanged() const = 0;
};

using IMenuControllerPtr = std::shared_ptr<IMenuController>;
using IMenuControllerPtrList = std::vector<IMenuControllerPtr>;
}

#endif // MU_UICOMPONENTS_IMENUCONTROLLER_H
