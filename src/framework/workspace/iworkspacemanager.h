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
#ifndef MU_WORKSPACE_IWORKSPACEMANAGER_H
#define MU_WORKSPACE_IWORKSPACEMANAGER_H

#include "modularity/imoduleexport.h"

#include "retval.h"
#include "iworkspace.h"

namespace mu {
namespace workspace {
class IWorkspaceManager : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IWorkspaceManager)
public:
    virtual ~IWorkspaceManager() = default;

    virtual RetValCh<std::shared_ptr<IWorkspace> > currentWorkspace() const = 0;
};
}
}

#endif // MU_WORKSPACE_IWORKSPACEMANAGER_H
